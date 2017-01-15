#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "clientmodel.h"

#include "version.h"
std::string LicenseInfo();

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
	// this->setStyleSheet("background-color: #effbef;");

//    ui->aboutMessage->setText("dsfsdf");

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
    ui->aboutMessage->setText(licenseInfoHTML);
    ui->aboutMessage->setWordWrap(true);



}

void AboutDialog::setModel(ClientModel *model)
{
    if(model)
    {
        ui->versionLabel->setText(model->formatFullVersion());
    }
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_buttonBox_accepted()
{
    close();
}
