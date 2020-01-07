#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include <QPushButton>
#include <QLineEdit>
#include <string>
#include <vector>
#include "SslClient.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    SslClient client_;

    QString filePath_;
    QString licensesPath_;
    std::string fileId_;
    std::string encryptionKey_;

    QPushButton* filePathButton;
    QPushButton* licensesPathButton;
    QPushButton* embedButton;
    QLineEdit* filePathLineEdit;
    QLineEdit* licensesPathLineEdit;
    QLineEdit* serverIpLineEdit;
    QLineEdit* serverPortLineEdit;

    void registerFileLicenses();
    std::vector<char> readAllBytes(const std::string& filename);
    void encrypt(std::vector<char> &vec, const std::string& encryptionKey);
    std::string getFileNameFromPath(const std::string& filepath);
    std::string getFileId(const std::string& filepath);
    std::string generateRandomEncryptionKey();
    std::string getFileExtension(const std::string &filepath);

};
#endif // MAINWINDOW_H
