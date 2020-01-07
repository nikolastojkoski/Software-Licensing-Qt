#include <QCoreApplication>
//#include "licenseserver.h"
#include "LicenseServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    LicenseServer server;

    return a.exec();
}
