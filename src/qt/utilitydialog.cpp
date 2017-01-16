#include "utilitydialog.h"

#include "ui_helpmessagedialog.h"

#include "magigui.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "guiutil.h"

#include "clientversion.h"
#include "init.h"
#include "util.h"

#include <stdio.h>

#include <QCloseEvent>
#include <QLabel>
#include <QRegExp>
#include <QTextTable>
#include <QTextCursor>
#include <QVBoxLayout>

HelpMessageDialog::HelpMessageDialog(QWidget *parent, int n) :
    QDialog(parent),
    ui(new Ui::HelpMessageDialog)
{
    ui->setupUi(this);
	// this->setStyleSheet("background-color: #effbef;");
    GUIUtil::restoreWindowGeometry("nHelpMessageDialogWindow", this->size(), this);

    startExecutor(n);
}

HelpMessageDialog::~HelpMessageDialog()
{
    GUIUtil::saveWindowGeometry("nHelpMessageDialogWindow", this);
    delete ui;
}

void HelpMessageDialog::on_okButton_accepted()
{
    close();
}

void HelpMessageDialog::startExecutor(int n)
{
    switch (n)
    {
        case 1: // about coin magi
            showAboutCoinMagi();
            break;
        case 2: // about mPoW
            showAboutmPoW();
            break;
        case 3: // about mPoS
            showAboutmPoS();
            break;
    }
}

void HelpMessageDialog::showAboutCoinMagi()
{
    QString version = tr("Magi Core") + " " + tr("version") + " " + QString::fromStdString(FormatFullVersion());
    /* On x86 add a bit specifier to the version so that users can distinguish between
     * 32 and 64 bit builds. On other architectures, 32/64 bit may be more ambigious.
     */
#if defined(__x86_64__)
    version += " " + tr("(%1-bit)").arg(64);
#elif defined(__i386__ )
    version += " " + tr("(%1-bit)").arg(32);
#endif

    /// HTML-format the license message from the core
    QString licenseInfo = QString::fromStdString(LicenseInfo());

    QString licenseInfoHTML = licenseInfo;
    // Make URLs clickable
    QRegExp uri("<(.*)>", Qt::CaseSensitive, QRegExp::RegExp2);
    uri.setMinimal(true); // use non-greedy matching
    licenseInfoHTML.replace(uri, "<a href=\"\\1\">\\1</a>");
    // Replace newlines with HTML breaks
    licenseInfoHTML.replace("\n", "<br>");

    ui->aboutMessage->setTextFormat(Qt::RichText);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    text = version + "\n" + licenseInfo;
    ui->aboutMessage->setText(version + "<br><br>" + licenseInfoHTML);
    ui->aboutMessage->setWordWrap(true);
    ui->helpMessage->setVisible(false);
}

void HelpMessageDialog::showAboutmPoW()
{
    /// HTML-format the license message from the core
    QString info = "mPoW, a magi's proof-of-work (PoW) protocol, in addition to required computational works to be done to deter denial of service attacks, is also a network-dependent rewarding model system. The mPoW rewards participants who solve complicated cryptographical questions not only to validate transactions but also to create new blocks in order to generate coins. The coin minting via mPoW is adjusted and balanced by two primary mechanisms: 1) triggering network activities by issuing rewards, and 2) mitigating redundant mining sources by reducing rewards.";

    QString infoHTML = info;
    // Make URLs clickable
    QRegExp uri("<(.*)>", Qt::CaseSensitive, QRegExp::RegExp2);
    uri.setMinimal(true); // use non-greedy matching
    infoHTML.replace(uri, "<a href=\"\\1\">\\1</a>");
    // Replace newlines with HTML breaks
    infoHTML.replace("\n", "<br>");
    infoHTML.replace("mPoW", "<b>mPoW</b>");

    ui->aboutMessage->setTextFormat(Qt::RichText);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->aboutMessage->setText(infoHTML);
    ui->aboutMessage->setWordWrap(true);
    ui->helpMessage->setVisible(false);
}

void HelpMessageDialog::showAboutmPoS()
{
    /// HTML-format the license message from the core
    QString info = "mPoS, a magi's proof-of-stake (PoS) protocol, aims to achieve distributed consensus in addition to mPoW. mPoS is designed to reject potential attacks; those attacks typically take place through accumulating a large amount of coins or offline stake time. Magi hybridizes PoW with PoS, and integrate both consensus approaches in order to acquire benefits from the two mechanisms and create a more robust payment system.";

    QString infoHTML = info;
    // Make URLs clickable
    QRegExp uri("<(.*)>", Qt::CaseSensitive, QRegExp::RegExp2);
    uri.setMinimal(true); // use non-greedy matching
    infoHTML.replace(uri, "<a href=\"\\1\">\\1</a>");
    // Replace newlines with HTML breaks
    infoHTML.replace("\n", "<br>");
    infoHTML.replace("mPoW", "<b>mPoW</b>");
    infoHTML.replace("mPoS", "<b>mPoS</b>");

    ui->aboutMessage->setTextFormat(Qt::RichText);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->aboutMessage->setText(infoHTML);
    ui->aboutMessage->setWordWrap(true);
    ui->helpMessage->setVisible(false);
}

/** "Shutdown" window */
ShutdownWindow::ShutdownWindow(QWidget *parent, Qt::WindowFlags f):
    QWidget(parent, f)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QLabel(
        tr("Bitcoin Core is shutting down...") + "<br /><br />" +
        tr("Do not shut down the computer until this window disappears.")));
    setLayout(layout);
}

void ShutdownWindow::showShutdownWindow(BitcoinGUI *window)
{
    if (!window)
        return;

    // Show a simple window indicating shutdown status
    QWidget *shutdownWindow = new ShutdownWindow();
    // We don't hold a direct pointer to the shutdown window after creation, so use
    // Qt::WA_DeleteOnClose to make sure that the window will be deleted eventually.
    shutdownWindow->setAttribute(Qt::WA_DeleteOnClose);
    shutdownWindow->setWindowTitle(window->windowTitle());

    // Center shutdown window at where main window was
    const QPoint global = window->mapToGlobal(window->rect().center());
    shutdownWindow->move(global.x() - shutdownWindow->width() / 2, global.y() - shutdownWindow->height() / 2);
    shutdownWindow->show();
}

void ShutdownWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
}
