#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslKey>
#include <QSslCertificate>

//http://www.infidigm.net/articles/qsslsocket_for_ssl_beginners/
class SslServer : public QTcpServer
{
    Q_OBJECT

public:

    SslServer(QObject *parent = nullptr);
    virtual ~SslServer() = default;

protected slots:
    virtual void rx();

protected:

    void incomingConnection(qintptr socketDescriptor);

private slots:

    void sslErrors(const QList<QSslError> &errors);
    void link();
    void disconnected();

private:

    QSslKey key;
    QSslCertificate cert;

};

#endif // SSLSERVER_H
