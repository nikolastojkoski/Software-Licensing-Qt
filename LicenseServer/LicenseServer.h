#ifndef LICENSESERVER_H
#define LICENSESERVER_H

#include "SslServer.h"
#include <map>
#include <string>

class LicenseServer: public SslServer
{
    Q_OBJECT

public:

    LicenseServer(QObject *parent = nullptr);
    virtual ~LicenseServer() override = default;

protected slots:

    virtual void rx() override;

private:

    const std::string licensesFilename_ = "licenses.dat";
    const std::string codesFilename_ = "codes.dat";
    const std::string locksFilename_ = "locks.dat";

    std::map<std::string, std::map<std::string, bool>> licenses_;
    std::map<std::string, std::string> decryptionCodes_;
    std::map<std::string, bool> locks_;

    void saveStateToDisk();
    void addLicense(const std::string& fileId, const std::string& license);
    void setDecryptionCode(const std::string& fileId, const std::string& decryptionCode);
    std::string getDecryptionCode(const std::string& fileId, const std::string& license);
    bool isLicenseValid(const std::string& fileId, const std::string& license);
    void lockFile(const std::string& fileId);


};

#endif // LICENSESERVER_H
