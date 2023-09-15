#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include <opencv2/opencv.hpp>
#include <QMap>
#include <QString>
#include <QPoint>
#include <QPolygon>
#include <QDebug>

extern QString tag_name[12];
extern int last_color_id, last_tag_id;

class box_t {
public:
    QPointF pts[5];
    int color_id = last_color_id, tag_id = last_tag_id;
    float conf = -1;

    QString getName() const {
        return tag_name[tag_id];
    }

    void set_class(const int color, const int tag) {
        color_id = color;
        tag_id = tag;
    }

    QPolygonF getStandardPloygon() const {
        QPolygonF pts;
        pts.append({0., 0.});
        pts.append({0., (2 <= tag_id && tag_id <= 7) ? (725.) : (660.)});
        pts.append({(2 <= tag_id && tag_id <= 7) ? (780.) : (1180.), (2 <= tag_id && tag_id <= 7) ? (725.) : (660.)});
        pts.append({(2 <= tag_id && tag_id <= 7) ? (780.) : (1180.), 0.});
        return pts;
    }
};

class SmartModel {
public:
    explicit SmartModel();

    bool run(const QString &image_file, QVector<box_t> &boxes);

    QString get_mode() const { return mode; }

private:
    cv::dnn::Net net;
    QString mode;
};

#endif /* _MODEL_HPP_ */
