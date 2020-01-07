#ifndef LICENSEMANAGER_H
#define LICENSEMANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <QProcess>
#include "SslClient.h"

class LicenseManager
{

public:

    LicenseManager(const std::string& selfPath);

    bool isAlreadyVerified();
    bool verifyLicense(const std::string& license);
    std::string getDecryptionCode(const std::string& license);
    std::string getSavedDecryptionCode();
    void decryptAndRun(const std::string& decryptionCode);

private:

    SslClient client_;
    QProcess *process;

    std::string selfPath_;
    std::string fileId_;

    std::string getFileId(const std::string& filepath);
    std::string getFileNameFromPath(const std::string& filepath);
    std::string getFileExtension(const std::string& filepath);
    std::vector<char> readAllBytes(const std::string& filename);
    void decryptRange(std::vector<char> &file, const std::string &decryptionKey, size_t startPos, size_t endPos);


};

#endif // LICENSEMANAGER_H
