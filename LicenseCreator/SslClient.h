#ifndef SSLCLIENT_H
#define SSLCLIENT_H

#include <QSslSocket>

//http://www.infidigm.net/articles/qsslsocket_for_ssl_beginners/
class SslClient: public QObject
{
    Q_OBJECT

public:
    SslClient();
    void connectToServer();
    void sendData(const QString &data);
    QString readData();

private:
    QSslSocket server;

Q_SIGNALS:
    void disconnected(void);

private slots:
    void sslErrors(const QList<QSslError> &errors);
    void rx(void);
    void serverDisconnect(void);

};

#endif // SSLCLIENT_H
