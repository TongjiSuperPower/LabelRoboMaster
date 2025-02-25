//
// Created by xinyang on 2021/4/28.
//

#include "model.hpp"
#include <fstream>
#include <QFile>
#include <opencv2/core/utils/logger.hpp>
QString tag_name[12];

const std::vector<std::tuple<int, int>> armor_properties = {
    {0, 0}, {1, 0}, {2, 0},
    {0, 1}, {1, 1}, {2, 1},
    {0, 2}, {1, 2}, {2, 2},
    {0, 3}, {1, 3}, {2, 3},
    {0, 4}, {1, 4}, {2, 4},
    {0, 5}, {1, 5}, {2, 5},
    {0, 6}, {1, 6}, {2, 6},
    {0, 7}, {1, 7}, {2, 7}, {3, 7},       
    {0, 8}, {1, 8}, {2, 8}, {3, 8},    
    {0, 9}, {1, 9}, {2, 9}, 
    {0, 10}, {1, 10}, {2, 10},  
    {0, 11}, {1, 11}, {2, 11}};

template<class F, class T, class ...Ts>
T reduce(F &&func, T x, Ts ...xs) {
    if constexpr (sizeof...(Ts) > 0) {
        return func(x, reduce(std::forward<F>(func), xs...));
    } else {
        return x;
    }
}

template<class T, class ...Ts>
T reduce_min(T x, Ts ...xs) {
    return reduce([](auto a, auto b) { return std::min(a, b); }, x, xs...);
}

template<class T, class ...Ts>
T reduce_max(T x, Ts ...xs) {
    return reduce([](auto a, auto b) { return std::max(a, b); }, x, xs...);
}

// 判断目标外接矩形是否相交，用于nms。
// 等效于thres=0的nms。
static inline bool is_overlap(const QPointF pts1[4], const QPointF pts2[4]) {
    cv::Rect2f box1, box2;
    box1.x = reduce_min(pts1[0].x(), pts1[1].x(), pts1[2].x(), pts1[3].x());
    box1.y = reduce_min(pts1[0].y(), pts1[1].y(), pts1[2].y(), pts1[3].y());
    box1.width = reduce_max(pts1[0].x(), pts1[1].x(), pts1[2].x(), pts1[3].x()) - box1.x;
    box1.height = reduce_max(pts1[0].y(), pts1[1].y(), pts1[2].y(), pts1[3].y()) - box1.y;
    box2.x = reduce_min(pts2[0].x(), pts2[1].x(), pts2[2].x(), pts2[3].x());
    box2.y = reduce_min(pts2[0].y(), pts2[1].y(), pts2[2].y(), pts2[3].y());
    box2.width = reduce_max(pts2[0].x(), pts2[1].x(), pts2[2].x(), pts2[3].x()) - box2.x;
    box2.height = reduce_max(pts2[0].y(), pts2[1].y(), pts2[2].y(), pts2[3].y()) - box2.y;
    return (box1 & box2).area() > 0;
}

static inline int argmax(const float *ptr, int len) {
    int max_arg = 0;
    for (int i = 1; i < len; i++) {
        if (ptr[i] > ptr[max_arg]) max_arg = i;
    }
    return max_arg;
}

float inv_sigmoid(float x) {
    return -std::log(1 / x - 1);
}

float sigmoid(float x) {
    return 1 / (1 + std::exp(-x));
}


SmartModel::SmartModel() {
    qDebug("initializing smart model... please wait.");
    try {
        auto model = core_.read_model("resource/yolo11.xml");
        device_ = "AUTO";

        ov::preprocess::PrePostProcessor ppp(model);
        auto & input = ppp.input();

        input.tensor()
        .set_element_type(ov::element::u8)
        .set_shape({1, 640, 640, 3})
        .set_layout("NHWC")
        .set_color_format(ov::preprocess::ColorFormat::BGR);
    
        input.model().set_layout("NCHW");
        
        input.preprocess()
            .convert_element_type(ov::element::f32)
            .convert_color(ov::preprocess::ColorFormat::RGB)
            .scale(255.0);
        
        // TODO: ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY)
        model = ppp.build();
        compiled_model_ = core_.compile_model(
            model, device_, ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY));
        std::cout << "use yolo11" << std::endl;
        return;
    } catch (cv::Exception &e) {
        std::cout << "fail to use yolo11" << std::endl;
        // openvino int8 unavailable
    }

    // int8模型不可用，加载fp32模型
    QFile onnx_file("resource/best.onnx");
    onnx_file.open(QIODevice::ReadOnly);
    auto onnx_bytes = onnx_file.readAll();

    net = cv::dnn::readNetFromONNX(onnx_bytes.data(), onnx_bytes.size());

    try {
        // 尝试使用openvino模式运行fp32模型
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_INFERENCE_ENGINE);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        cv::Mat input(640, 640, CV_8UC3);
        auto x = cv::dnn::blobFromImage(input) / 255.;
        net.setInput(x);
        net.forward();
        mode = "openvino-fp32-cpu"; // 设置当前模型模式
        std::cout << "use openvino-fp32-cpu" << std::endl;
    } catch (cv::Exception &e) {
        qDebug(e.what());
        // 无法使用openvino运行fp32模型，则使用默认的opencv-dnn模式。
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        mode = "dnn-fp32-cpu";      // 设置当前模型模式
        std::cout << "use dnn-fp32-cpu" << std::endl;
    }
}

bool SmartModel::run(const QString &image_file, QVector<box_t> &boxes) {
    try {
        // 加载图片，并等比例resize为640x640。空余部分用0进行填充。
        auto bgr_img = cv::imread(image_file.toStdString());
        
        auto x_scale = static_cast<double>(640) / bgr_img.rows;
        auto y_scale = static_cast<double>(640) / bgr_img.cols;
        auto scale = std::min(x_scale, y_scale);
        auto h = static_cast<int>(bgr_img.rows * scale);
        auto w = static_cast<int>(bgr_img.cols * scale);

        // preproces
        auto input = cv::Mat(640, 640, CV_8UC3, cv::Scalar(0, 0, 0));
        auto roi = cv::Rect(0, 0, w, h);
        cv::resize(bgr_img, input(roi), {w, h});
        ov::Tensor input_tensor(ov::element::u8, {1, 640, 640, 3}, input.data);

        /// infer
        auto infer_request = compiled_model_.create_infer_request();
        infer_request.set_input_tensor(input_tensor);
        infer_request.infer();

        // postprocess
        auto output_tensor = infer_request.get_output_tensor();
        auto output_shape = output_tensor.get_shape();
        cv::Mat output(output_shape[1], output_shape[2], CV_32F, output_tensor.data());
        cv::transpose(output, output);
        QVector<box_t> before_nms;
        std::vector<float> confidences;
        std::vector<cv::Rect> nms_boxes;
        double score_threshold_ = 0.7;
        double nms_threshold_ = 0.3;
        for (int r = 0; r < output.rows; r++) {
            box_t box;
            auto xywh = output.row(r).colRange(0, 4);
            auto scores = output.row(r).colRange(4, 4 + 38);
            auto one_key_points = output.row(r).colRange(4 + 38, 50);
        
            std::vector<cv::Point2f> armor_key_points;
        
            double score;
            cv::Point max_point;
            cv::minMaxLoc(scores, nullptr, &score, nullptr, &max_point);
        
            if (score < score_threshold_) continue;

            auto x = xywh.at<float>(0);
            auto y = xywh.at<float>(1);
            auto w = xywh.at<float>(2);
            auto h = xywh.at<float>(3);
            auto left = static_cast<int>((x - 0.5 * w) / scale);
            auto top = static_cast<int>((y - 0.5 * h) / scale);
            auto width = static_cast<int>(w / scale);
            auto height = static_cast<int>(h / scale);

            for (int i = 0; i < 4; i++) {
                float x = one_key_points.at<float>(0, i * 2 + 0) / scale;
                float y = one_key_points.at<float>(0, i * 2 + 1) / scale;
                box.pts[i].rx() = x;
                box.pts[i].ry() = y;
            }
            box.color_id = std::get<0>(armor_properties[max_point.x]);
            box.tag_id = std::get<1>(armor_properties[max_point.x]);
            box.conf = score;
            confidences.emplace_back(score);
            nms_boxes.emplace_back(left, top, width, height);
            before_nms.append(box);
        }

        std::vector<int> indices;
        cv::dnn::NMSBoxes(nms_boxes, confidences, score_threshold_, nms_threshold_, indices);
        boxes.clear();
        boxes.reserve(nms_boxes.size());
        for (const auto & i : indices) {
            if(before_nms[i].conf > 0.7) boxes.append(before_nms[i]);
            else continue;
        }

        return true;
    } catch (std::exception &e) {
        std::ofstream ofs("warning.txt", std::ios::app);
        time_t t;
        time(&t);
        ofs << asctime(localtime(&t)) << "\t" << e.what() << std::endl;
        return false;
    }
}