#include "datamanager.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DataManager dm;
    return a.exec();
}
