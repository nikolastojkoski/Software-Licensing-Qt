#include "SslClient.h"
#include <QSslKey>
#include <QSslCertificate>

SslClient::SslClient()
{
    //connect(&server, &QSslSocket::readyRead, this, &SslClient::rx);
    connect(&server, &QSslSocket::disconnected, this, &SslClient::serverDisconnect);
    connect(&server, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
    server.addCaCertificates("C:/ssl_certificates/red_ca.pem");
    server.setPrivateKey("C:/ssl_certificates/blue_local.key");
    server.setLocalCertificate("C:/ssl_certificates/blue_local.pem");
    server.setPeerVerifyMode(QSslSocket::VerifyPeer);
}

void SslClient::connectToServer()
{
    // FQDN in red_local.pem is set to 127.0.0.1.
    //If you change this, it will not authenticate.

    server.connectToHostEncrypted("127.0.0.1", 12345);

    if (server.waitForEncrypted(5000))
    {
        qDebug() << "Authentication Succeeded";
    }
    else
    {
        qDebug() << "Unable to connect to server";
        exit(1);
    }
}

void SslClient::sendData(const QString &data)
{
    server.write(data.toStdString().c_str());
}

QString SslClient::readData()
{
    if(server.waitForReadyRead())
        return server.readAll();
    return QString();
}

void SslClient::sslErrors(const QList<QSslError> &errors)
{
    foreach (const QSslError &error, errors)
        qDebug() << error.errorString();
}

void SslClient::serverDisconnect(void)
{
    qDebug("Server disconnected");
    exit(0);
}

void SslClient::rx(void)
{
    //rx_ready = true;
    //server.write("ADD 12345-ABCDEF-GHIJK-67890");
    //qDebug() << server.readAll();
}
