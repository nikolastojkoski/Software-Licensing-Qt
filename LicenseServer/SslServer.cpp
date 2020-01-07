#include "SslServer.h"
#include <QSslSocket>
#include <QFile>

SslServer::SslServer(QObject *parent) : QTcpServer(parent)
{
    QFile keyFile("C:/ssl_certificates/red_local.key");
    keyFile.open(QIODevice::ReadOnly);
    key = QSslKey(keyFile.readAll(), QSsl::Rsa);
    keyFile.close();

    QFile certFile("C:/ssl_certificates/red_local.pem");
    certFile.open(QIODevice::ReadOnly);
    cert = QSslCertificate(certFile.readAll());
    certFile.close();

    if (!listen(QHostAddress("127.0.0.1"), 12345))
    {
        // FQDN in red_local.pem is set to 127.0.0.1.  If you change this, it will not authenticate.
        qCritical() << "Unable to start the TCP server";
        exit(0);
    }
    connect(this, &SslServer::newConnection, this, &SslServer::link);
}

void SslServer::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket *sslSocket = new QSslSocket(this);

    connect(sslSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
    sslSocket->setSocketDescriptor(socketDescriptor);
    sslSocket->setPrivateKey(key);
    sslSocket->setLocalCertificate(cert);
    sslSocket->addCaCertificates("C:/ssl_certificates/blue_ca.pem");
    sslSocket->setPeerVerifyMode(QSslSocket::VerifyPeer);
    sslSocket->startServerEncryption();

    addPendingConnection(sslSocket);
}

void SslServer::sslErrors(const QList<QSslError> &errors)
{
    foreach (const QSslError &error, errors)
        qDebug() << error.errorString();
}

void SslServer::link()
{
    QTcpSocket *clientSocket;

    clientSocket = nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &SslServer::rx);
    connect(clientSocket, &QTcpSocket::disconnected, this, &SslServer::disconnected);
}

void SslServer::rx()
{

//    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());

//    QStringList list;
//    while(clientSocket->canReadLine())
//    {
//        QString data = clientSocket->readLine();
//        list.append(data);
//    }
//    //qDebug() << clientSocket->readAll();
//    clientSocket->write("Server says Hello");
}

void SslServer::disconnected()
{
    qDebug("Client Disconnected");
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    clientSocket->deleteLater();
}
