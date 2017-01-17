#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QRegExp>

#include "guiconstants.h"
class BitcoinGUI;
class ClientModel;

namespace Ui {
    class HelpMessageDialog;
}

/** "Help message" dialog box */
class HelpMessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpMessageDialog(QWidget *parent, int n);
    ~HelpMessageDialog();

    void printToConsole();
    void showOrPrint();
    void showAboutCoinMagi();
    void showAboutmPoW();
    void showAboutmPoS();
    void startExecutor();

private:
    Ui::HelpMessageDialog *ui;
    QString text;
    int nwhich;

private Q_SLOTS:
    void on_okButton_accepted();
};

/** "Shutdown" window */
class ShutdownWindow : public QWidget
{
    Q_OBJECT

public:
    ShutdownWindow(QWidget *parent=0, Qt::WindowFlags f=0);
    static void showShutdownWindow(BitcoinGUI *window);

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // ABOUTDIALOG_H
