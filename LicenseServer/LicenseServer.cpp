#include "LicenseServer.h"
#include <QFile>
#include <QTcpSocket>
#include <sstream>
#include <string>
#include <fstream>

#include <iostream>

LicenseServer::LicenseServer(QObject *parent):
    SslServer(parent)
{
    //Load licenses file, entries in file have format "<ID> <LICENSE> [0|1]"
    std::ifstream inpLicenses(licensesFilename_);
    std::string line;
    while (std::getline(inpLicenses, line))
    {
        std::istringstream iss(line);

        std::string fileId, license, isValid;
        iss >> fileId >> license >> isValid;

        licenses_[fileId][license] = (isValid == std::string("1"));
    }

    //Load decryption codes file, entries in file have format "<ID> <CODE>"
    std::ifstream inpCodes(codesFilename_);
    while(std::getline(inpCodes, line))
    {
        std::istringstream iss(line);

        std::string fileId, decryptionCode;
        iss >> fileId >> decryptionCode;

        decryptionCodes_[fileId] = decryptionCode;
    }

    //Load fileId locks
    std::ifstream inpLocks(locksFilename_);
    while(std::getline(inpLocks, line))
    {
        std::istringstream iss(line);

        std::string fileId;
        bool isLocked;

        iss >> fileId >> isLocked;

        locks_[fileId] = isLocked;
    }

}

void LicenseServer::rx()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());

    while(clientSocket->canReadLine())
    {
        std::string line = clientSocket->readLine().toStdString();
        std::cout << "Received: " << line;

        std::istringstream iss(line);

        std::string command, fileId;
        iss >> command >> fileId;

        //std::cout << "command = " << command << std::endl;
        //std::cout << "fileId = " << fileId << std::endl;

        if(command == "ADD")
        {
            std::string license;
            iss >> license;
            //std::cout << "license = " << license << std::endl;
            addLicense(fileId, license);
        }
        else if(command == "SET")
        {
            std::string decryptionCode;
            iss >> decryptionCode;

            //std::cout << "decryptionCode = " << decryptionCode << std::endl;
            setDecryptionCode(fileId, decryptionCode);
        }
        else if(command == "CHECK")
        {
            std::string license;
            iss >> license;

            //std::cout << "license = " << license << std::endl;

            if(isLicenseValid(fileId, license))
            {
                std::cout << "license is valid" << std::endl;
                clientSocket->write("true\n");
            }
            else
            {
                std::cout << "license is not valid" << std::endl;
                clientSocket->write("false\n");
            }
        }
        else if(command == "GET")
        {
            std::string license;
            iss >> license;

            //std::cout << "license = " << license << std::endl;

            std::string decryptionCode = getDecryptionCode(fileId, license);

            //std::cout << "m_decryptionCode = " << decryptionCode << std::endl;
            clientSocket->write(std::string(decryptionCode + "\n").c_str());
        }
        else if(command == "LOCK")
        {
            lockFile(fileId);
        }
        else
        {
            clientSocket->write("invalid request\n");
        }

    }
}

void LicenseServer::saveStateToDisk()
{
    //write licenses file
    std::ofstream outLicenses(licensesFilename_);
    for(auto it = licenses_.begin(); it != licenses_.end(); it++)
    {
        for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            outLicenses << it->first << " " << it2->first << " " << it2->second << std::endl;
        }
    }
    outLicenses.flush();
    outLicenses.close();

    //write decryption codes file
    std::ofstream outCodes(codesFilename_);
    for(auto it = decryptionCodes_.begin(); it != decryptionCodes_.end(); it++)
    {
        outCodes << it->first << " " << it->second << std::endl;
    }
    outCodes.flush();
    outCodes.close();

    //write file locsk
    std::ofstream outLocks(locksFilename_);
    for(auto it = locks_.begin(); it != locks_.end(); it++)
    {
        outLocks << it->first << " " << it->second << std::endl;
    }
    outLocks.flush();
    outLocks.close();

}

void LicenseServer::addLicense(const std::string &fileId, const std::string &license)
{
    if(!locks_[fileId])
    {
        licenses_[fileId][license] = true;
    }
}

void LicenseServer::setDecryptionCode(const std::string &fileId, const std::string &decryptionCode)
{
    if(!locks_[fileId])
    {
        decryptionCodes_[fileId] = decryptionCode;
    }
}

std::string LicenseServer::getDecryptionCode(const std::string &fileId, const std::string &license)
{
    if(isLicenseValid(fileId, license))
    {
        //invalidate license
        licenses_[fileId][license] = false;

        return decryptionCodes_[fileId];
    }
    return std::string("-1");
}

bool LicenseServer::isLicenseValid(const std::string &fileId, const std::string &license)
{
    return licenses_[fileId][license];
}

void LicenseServer::lockFile(const std::string &fileId)
{
    locks_[fileId] = true;
    saveStateToDisk();
}
