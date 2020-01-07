#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QCryptographicHash>
#include <QDebug>
#include <QTime>
#include <fstream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("License Creator");
    setWindowIcon(QIcon("license_icon.ico"));

    filePathButton = findChild<QPushButton*>("filePathButton");
    licensesPathButton = findChild<QPushButton*>("licensesPathButton");
    embedButton = findChild<QPushButton*>("embedButton");
    filePathLineEdit = findChild<QLineEdit*>("filePathLineEdit");
    licensesPathLineEdit = findChild<QLineEdit*>("licensesPathLineEdit");
    serverIpLineEdit = findChild<QLineEdit*>("serverIpLineEdit");
    serverPortLineEdit = findChild<QLineEdit*>("serverPortLineEdit");

    filePathLineEdit->setReadOnly(true);
    licensesPathLineEdit->setReadOnly(true);
    serverIpLineEdit->setReadOnly(true);
    serverPortLineEdit->setReadOnly(true);

    connect(filePathButton, &QPushButton::clicked, [&](){
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::AnyFile);
        QStringList fileNames;
        if(dialog.exec())
        {
            fileNames = dialog.selectedFiles();
            filePath_ = fileNames.first();
            filePathLineEdit->setText(filePath_);
        }
    });

    connect(licensesPathButton, &QPushButton::clicked, [&](){
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::AnyFile);
        QStringList fileNames;
        if(dialog.exec())
        {
            fileNames = dialog.selectedFiles();
            licensesPath_ = fileNames.first();
            licensesPathLineEdit->setText(licensesPath_);
        }
    });

    connect(embedButton, &QPushButton::clicked, [&](){

        std::string licenseClientFilePath = "LicenseClient.exe";
        std::string userFilePath = filePath_.toStdString();
        encryptionKey_ = generateRandomEncryptionKey();

        std::vector<char> licenseClientFile = readAllBytes(licenseClientFilePath);
        std::vector<char> userFile = readAllBytes(userFilePath);
        encrypt(userFile, encryptionKey_);

        std::string ext = getFileExtension(userFilePath);
        std::vector<char> fileExtension(ext.begin(), ext.end());
        size_t fileExtSize = fileExtension.size();
        std::vector<char> fileExtSize_c(sizeof(size_t));
        memcpy(&fileExtSize_c[0], &fileExtSize, sizeof(size_t));

        size_t startPos = licenseClientFile.size();
        std::vector<char> startPos_c(sizeof(size_t));
        memcpy(&startPos_c[0], &startPos, sizeof(size_t));

        std::string outFileName = "_" + getFileNameFromPath(userFilePath);
        std::string outExt = getFileExtension(outFileName);
        if(outExt != "exe")
        {
            outFileName = outFileName.substr(0, outFileName.size() - outExt.size()) + "exe";
        }

        std::ofstream fout(outFileName, std::ios::binary | std::ios::out);
        fout.write((char*) &licenseClientFile[0], licenseClientFile.size());
        fout.write((char*) &userFile[0], userFile.size());
        fout.write((char*) &fileExtension[0], fileExtension.size());
        fout.write((char*) &fileExtSize_c[0], fileExtSize_c.size());
        fout.write((char*) &startPos_c[0], startPos_c.size());
        fout.close();

        fileId_ = getFileId(outFileName);
        licensesPath_ = licensesPathLineEdit->text();
        registerFileLicenses();

        QMessageBox messageBox;
        messageBox.setText("Sucess!");
        messageBox.exec();
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::registerFileLicenses()
{
    client_.connectToServer();

    //Set licenses
    std::ifstream inpLicenses(licensesPath_.toStdString());
    std::string line;
    while (std::getline(inpLicenses, line))
    {
        QString query = "ADD %1 %2\n";
        client_.sendData(query.arg(fileId_.c_str()).arg(line.c_str()));
    }

    //Set decryption code
    QString query = "SET %1 %2\n";
    client_.sendData(query.arg(fileId_.c_str()).arg(encryptionKey_.c_str()));

    //Lock file
    query = "LOCK %1\n";
    client_.sendData(query.arg(fileId_.c_str()));

}

std::vector<char> MainWindow::readAllBytes(const std::string &filename)
{
    std::ifstream ifs(filename, std::ios::binary|std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();

    std::vector<char> result(pos);

    ifs.seekg(0, std::ios::beg);
    ifs.read(&result[0], pos);

    return result;
}

void MainWindow::encrypt(std::vector<char> &vec, const std::string &encryptionKey)
{
    for(size_t i = 0; i < vec.size(); i++)
    {
      for(size_t j = 0; j < encryptionKey.size(); j++)
      {
        vec[i] ^= encryptionKey[j];
      }
    }
}

std::string MainWindow::getFileNameFromPath(const std::string &filepath)
{
    std::string filename = filepath;
    const size_t last_slash_idx = filename.find_last_of("\\/");
    if (std::string::npos != last_slash_idx)
    {
        filename.erase(0, last_slash_idx + 1);
    }
    return filename;
}

std::string MainWindow::getFileId(const std::string &filepath)
{
    QFile f(QString::fromStdString(filepath));
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Algorithm::Sha1);
        if (hash.addData(&f)) {
            return hash.result().toHex().toStdString();
        }
    }
    return QByteArray().toHex().toStdString();
}

std::string MainWindow::generateRandomEncryptionKey()
{
    qsrand(QTime::currentTime().msec());

    std::string seed = "X683nFHF8_9iF83kDUI_HoHl2a";
    int keyLength = 10 + qrand() % 20;
    std::string encryptionKey;

    for(int i = 0; i < keyLength; i++)
    {
        int idx = qrand() % (seed.length() - 1);
        encryptionKey += seed[idx];
    }
    return encryptionKey;
}

std::string MainWindow::getFileExtension(const std::string &filepath)
{
    if(filepath.find_last_of(".") != std::string::npos)
        return filepath.substr(filepath.find_last_of(".")+1);
    return "";
}
