#include "configuredialog.h"
#include "ui_configuredialog.h"

Configuredialog::Configuredialog(Configure *config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Configuredialog)
{
    ui->setupUi(this);
    connect(ui->cancelPushButton, &QPushButton::clicked, [=](){close();});
    connect(ui->PDSlider, &QSlider::valueChanged, [=](){ui->pointDistanceLabel->setText(QString::number(ui->PDSlider->value() / 10.));});
    connect(ui->VRSlider, &QSlider::valueChanged, [=](){ui->vRateLabel->setText(QString::number(ui->VRSlider->value() / 10.));});
    configure = config;
}

Configuredialog::~Configuredialog()
{
    delete ui;
}

void Configuredialog::on_OKPushButton_clicked()
{
    configure->point_distance = ui->PDSlider->value() / 10.;
    configure->V_rate = ui->VRSlider->value() / 10.;
    close();
}

void Configuredialog::show_configure()
{
    ui->PDSlider->setValue(int(configure->point_distance * 10));
    ui->VRSlider->setValue(int(configure->V_rate * 10));
    show();
}