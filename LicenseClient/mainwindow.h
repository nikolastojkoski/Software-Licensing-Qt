#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include "licensemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(LicenseManager &licenseManager, QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    LicenseManager &licenseManager_;

    QLineEdit *licenseLineEdit;
    QPushButton *verifyPushButton;

    QPushButton *queryServerPushButton;


};
#endif // MAINWINDOW_H
