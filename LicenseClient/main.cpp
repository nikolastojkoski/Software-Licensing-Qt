#include "mainwindow.h"

#include <QApplication>
#include <string>
#include "licensemanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LicenseManager licenseManager(argv[0]);

    MainWindow w(licenseManager);

    if(licenseManager.isAlreadyVerified())
    {
        std::string decryptionCode = licenseManager.getSavedDecryptionCode();
        licenseManager.decryptAndRun(decryptionCode);
    }
    else
    {
        w.show();
    }

    return a.exec();
}
