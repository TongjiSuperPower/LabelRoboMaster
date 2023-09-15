#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>
#include "configure.hpp"

namespace Ui {
class Configuredialog;
}

class Configuredialog : public QDialog
{
    Q_OBJECT

public:
    explicit Configuredialog(Configure *config = nullptr, QWidget *parent = nullptr);
    void show_configure();
    ~Configuredialog();

signals:
    void set_configures_signal(float PD, float VR);

private slots:
    void on_OKPushButton_clicked();

private:
    Ui::Configuredialog *ui;
    Configure *configure;
};

#endif // CONFIGUREDIALOG_H
