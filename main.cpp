#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString config_filename(CONFIG_FILENAME);
    CConfigLoader config(config_filename);

    MainWindow w(config);
    w.show();
    return a.exec();
}
