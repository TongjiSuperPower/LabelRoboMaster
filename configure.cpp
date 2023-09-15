#include "configure.hpp"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#define VERSION QString("CONFIGURE_1.0")

Configure::Configure()
{
    QFileInfo label_file("configure.txt");
    if (label_file.exists()) {
        QFile fp(label_file.absoluteFilePath());
        if (fp.open(QIODevice::ReadOnly)) {
            QTextStream stream(&fp);
            QString version;
            stream >> version;
            if(version == VERSION)
                stream >> point_distance >> V_rate >> last_open >> last_pic >> last_mode >> auto_enhance_V;
        }
    }
}

Configure::~Configure()
{
    QFile fp("configure.txt");
    if (fp.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        QTextStream stream(&fp);
        stream << VERSION << '\n' << point_distance << '\n' << V_rate << '\n' << last_open << '\n' << last_pic << '\n' << last_mode << '\n' << auto_enhance_V;
    }
}