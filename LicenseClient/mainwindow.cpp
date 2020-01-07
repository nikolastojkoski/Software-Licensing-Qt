#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QDebug>
#include <QFile>
#include <string>
#include <windows.h>
#include "SslClient.h"

MainWindow::MainWindow(LicenseManager &licenseManager, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    licenseManager_(licenseManager)
{
    ui->setupUi(this);

    setWindowTitle("License Verification");
    setWindowIcon(QIcon("license_icon.ico"));

    verifyPushButton = findChild<QPushButton*>("verifyPushButton");
    licenseLineEdit = findChild<QLineEdit*>("licenseLineEdit");

    connect(verifyPushButton, &QPushButton::clicked, [&](){

        std::string license = licenseLineEdit->text().toStdString();
        if(licenseManager.verifyLicense(license))
        {
            std::string decryptionCode = licenseManager_.getDecryptionCode(license);
            hide();
            licenseManager_.decryptAndRun(decryptionCode);
        }
        else
        {
            QMessageBox messageBox;
            messageBox.setText("Invalid license number!");
            messageBox.exec();
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

