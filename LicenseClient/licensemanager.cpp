#include "licensemanager.h"
#include <QTemporaryDir>
#include <QProcess>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDesktopServices>
#include <QCryptographicHash>
#include <fstream>
#include <QSettings>
#include "windows.h"

LicenseManager::LicenseManager(const std::string &selfPath):
    selfPath_(selfPath)
{
    fileId_ = getFileId(selfPath);
    client_.connectToServer();
    process = new QProcess();
}

bool LicenseManager::isAlreadyVerified()
{
    if(getSavedDecryptionCode().size() > 1)
    {
        return true;
    }
    return false;
}

bool LicenseManager::verifyLicense(const std::string &license)
{
    QString query = "CHECK %1 %2\n";
    client_.sendData(query.arg(fileId_.c_str()).arg(license.c_str()));

    QString ans = client_.readData();
    return ans == "true\n";
}

std::string LicenseManager::getDecryptionCode(const std::string &license)
{
    QString query = "GET %1 %2\n";
    client_.sendData(query.arg(fileId_.c_str()).arg(license.c_str()));
    std::string decryptionCode = client_.readData().toStdString();
    decryptionCode = decryptionCode.substr(0, decryptionCode.size() - 1);

    qDebug() << "got decryption code = " << decryptionCode.c_str();

    //Add decryptionCode to registry
    QSettings settings("LicenseManager", fileId_.c_str());
    settings.setValue("DecryptionCode", QString::fromStdString(decryptionCode));

    return decryptionCode;
}

std::string LicenseManager::getSavedDecryptionCode()
{
    std::string fileId = getFileId(selfPath_);
    QSettings settings("LicenseManager", fileId.c_str());
    return settings.value("DecryptionCode").toString().toStdString();
}

void LicenseManager::decryptAndRun(const std::string& decryptionCode)
{

    std::ofstream log("licenseClient_log.txt");


    std::string filename = getFileNameFromPath(selfPath_);
    log << "filename = " << filename << std::endl;

    //Read file into array
    std::vector<char> fileMe = readAllBytes(selfPath_);
    const size_t mySize = fileMe.size();
    log << "mySize = " << mySize << std::endl;

    //Get embedded file start position and size
    size_t startPos;
    memcpy(&startPos, &fileMe[mySize - sizeof(size_t)], sizeof(size_t));
    size_t embeddedFileSize = mySize - sizeof(size_t) - startPos;
    log << "startPos = " << startPos << std::endl;
    log << "embeddedFileSize = " << embeddedFileSize << std::endl;

    //Get embedded file extension
    size_t fileExtSize;
    memcpy(&fileExtSize, &fileMe[mySize - 2*sizeof(size_t)], sizeof(size_t));
    log << fileExtSize << std::endl;
    std::vector<char> fileExtension_c(fileExtSize);
    memcpy(&fileExtension_c[0], &fileMe[mySize - 2*sizeof(size_t) - fileExtSize], fileExtSize);
    std::string fileExtension(fileExtension_c.begin(), fileExtension_c.end());
    log << "fileExtension = " << fileExtension << std::endl;

    //Decrypt file
    decryptRange(fileMe, decryptionCode, startPos, startPos + embeddedFileSize);
    log << "decrypted" << std::endl;

    //Write file to temp location
    QTemporaryDir dir;
    std::string tempDirPath = dir.path().toStdString();
    std::string outFilePath = tempDirPath + "/" + filename;
    log << "tempDirPath = " << tempDirPath << std::endl;
    log << "outFilePath = " << outFilePath << std::endl;
    outFilePath = outFilePath.substr(0, outFilePath.size()-3) + fileExtension;
    log << "changed ext outFilePath = " << outFilePath << std::endl;

    std::ofstream fout(outFilePath, std::ios::binary | std::ios::out);
    fout.write((char*)&fileMe[startPos], embeddedFileSize);
    fout.flush();
    fout.close();
    log << "file written to disk" << std::endl;

    //Run file
    if(fileExtension == "exe")
    {
        int ret = process->execute(QString::fromStdString(outFilePath));
        log << "outFilePath parameter = " << outFilePath << std::endl;
        log << "process->execute ret code = " << ret << std::endl;
        process->waitForFinished(-1);
    }
    else
    {
        QDesktopServices::openUrl(QUrl::fromUserInput(QString::fromStdString(outFilePath)));
        Sleep(10000);
    }

    //Delete decrypted file and its temp directory
    QFile file(outFilePath.c_str());
    file.remove();
    dir.remove();


    log.flush();
    log.close();

    //Quit qt application
    exit(0);
}

std::string LicenseManager::getFileId(const std::string &filepath)
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

std::vector<char> LicenseManager::readAllBytes(const std::string &filename)
{

  std::ifstream ifs(filename, std::ios::binary|std::ios::ate);
  std::ifstream::pos_type pos = ifs.tellg();

  std::vector<char> result(pos);

  ifs.seekg(0, std::ios::beg);
  ifs.read(&result[0], pos);

  return result;

}

void LicenseManager::decryptRange(std::vector<char> &file, const std::string &decryptionKey,
                                  size_t startPos, size_t endPos)
{
  for(size_t i = startPos; i < endPos; i++)
  {
    for(int j = decryptionKey.size() - 1; j >= 0; j--)
    {
      file[i] ^= decryptionKey[j];
    }
  }
}

std::string LicenseManager::getFileNameFromPath(const std::string &filepath)
{
    std::string filename = filepath;
    const size_t last_slash_idx = filename.find_last_of("\\/");
    if (std::string::npos != last_slash_idx)
    {
        filename.erase(0, last_slash_idx + 1);
    }
    return filename;
}

std::string LicenseManager::getFileExtension(const std::string &filepath)
{
    if(filepath.find_last_of(".") != std::string::npos)
        return filepath.substr(filepath.find_last_of(".")+1);
    return "";
}
