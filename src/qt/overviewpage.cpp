#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "askpassphrasedialog.h"
#include "updatecheck.h"
#include "../version.h"
#include <vector>

#include <QAbstractItemDelegate>
#include <QPainter>

#include <QDesktopServices>  //Added for openURL()
#include <QTimer>            //Added for update timer
#include <QUrl>

#define DECORATION_SIZE 64
#define NUM_ITEMS 6

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(BitcoinUnits::BTC)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
#if QT_VERSION < 0x050000
        if(qVariantCanConvert<QColor>(value))
        {
            foreground = qvariant_cast<QColor>(value);
        }
#else
        if(value.canConvert<QBrush>())
        {
            QBrush brush = qvariant_cast<QBrush>(value);
            foreground = brush.color();
        }
#endif
        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address);

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;

};
#include "overviewpage.moc"

OverviewPage::OverviewPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    currentBalance(-1),
    currentStake(0),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    txdelegate(new TxViewDelegate()),
    filter(0)
{
    ui->setupUi(this);

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
    ui->labelTransactionsStatus->setText("(" + tr("out of sync") + ")");
    ui->labelUpdateStatus->setText("(" + tr("checking for updates") + ")");
    ui->labelUpdateStatus->setTextFormat(Qt::RichText);
    ui->labelUpdateStatus->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->labelUpdateStatus->setOpenExternalLinks(true);

    // Setup a timer to regularly check for updates to the wallet
    // Have it trigger once, immediately, then set it to check once ever 24 hours
    // The update timer conversion factor (_UPDATE_MS_TO_HOURS) is located in
    // updatecheck.h
    timerUpdate();
    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    updateTimer->start(_UPDATE_INTERVAL*_UPDATE_MS_TO_HOURS);

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);

    // show the "checking for updates" warning
    showUpdateWarning(true);
}

void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    int unit = model->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balance));
    ui->labelStake->setText(BitcoinUnits::formatWithUnit(unit, stake));
    ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, immatureBalance));
    ui->labelTotal->setText(BitcoinUnits::formatWithUnit(unit, (balance + stake + unconfirmedBalance)));

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    ui->labelImmature->setVisible(showImmature);
    ui->labelImmatureText->setVisible(showImmature);
}

void OverviewPage::setNumTransactions(int count)
{
    ui->labelNumTransactions->setText(QLocale::system().toString(count));
}

void OverviewPage::setModel(WalletModel *model)
{
    this->model = model;
    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));

        setNumTransactions(model->getNumTransactions());
        connect(model, SIGNAL(numTransactionsChanged(int)), this, SLOT(setNumTransactions(int)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    // update the display unit, to not use the default ("XMG")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        if(currentBalance != -1)
            setBalance(currentBalance, model->getStake(), currentUnconfirmedBalance, currentImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = model->getOptionsModel()->getDisplayUnit();

        ui->listTransactions->update();
    }
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);
    ui->labelTransactionsStatus->setVisible(fShow);
}

void OverviewPage::showUpdateWarning(bool fShow)
{
    ui->labelUpdateStatus->setVisible(fShow);
}

void OverviewPage::showUpdateLayout(bool fShow)
{
    ui->labelUpdateStatic->setVisible(fShow);
    ui->labelUpdateStatus->setVisible(fShow);
}

void OverviewPage::timerUpdate()
{
    // Create a connection to the website to check for updates
    QUrl updateUrl(_UPDATE_VERSION_URL);
    m_pUpdCtrl = new UpdateCheck(updateUrl, this);
    
    connect(m_pUpdCtrl, SIGNAL (downloaded()), this, SLOT (checkForUpdates()));
}

void OverviewPage::checkForUpdates()
{
    // Grab the internal wallet version and the online version string
    QString siteVersion(m_pUpdCtrl->downloadedData());
    QString internalVersion;
    internalVersion = QString::number(DISPLAY_VERSION_MAJOR);
    internalVersion = internalVersion + "." + QString::number(DISPLAY_VERSION_MINOR);
    internalVersion = internalVersion + "." + QString::number(DISPLAY_VERSION_REVISION);
    internalVersion = internalVersion + "." + QString::number(DISPLAY_VERSION_BUILD);

    QString report = QString("The wallet is up to date");

    // Split the online version string and compare it against the internal version
    // If at any point we find that the internal version is less than the online
    // version, exit without setting bool isUpToDate. If the internal version is
    // equal to or greater than the online version, set isUpToDate = true.
    bool isUpToDate = true;
    int value = 0;
    std::vector<std::string> v_siteString = m_pUpdCtrl->splitString(siteVersion.toStdString(), ".");
    if (v_siteString.size() != (size_t) 4)
    {
        report = QString("Malformed wallet version retrieved");
    }

    // Here, we need to check version very thoroughly, otherwise the version numbers of
    // higher significance (Major, Minor, Revision) can be ignored if the lower
    // significance numbers are higher
    else
    {
        value = atoi(v_siteString.at(0).c_str());
        if (value > DISPLAY_VERSION_MAJOR)
        {
            isUpToDate = false;
        }
        else
        {
            value = atoi(v_siteString.at(1).c_str());
            if (value > DISPLAY_VERSION_MINOR)
            {
                // Check to make sure major version is also greater than or equal to
                // the current version
                value = atoi(v_siteString.at(0).c_str());
                if (value >= DISPLAY_VERSION_MAJOR)
                {
                    isUpToDate = false;
                }
            }
            else
            {
                value = atoi(v_siteString.at(2).c_str());
                if (value > DISPLAY_VERSION_REVISION)
                {
                    // Check to make sure minor version is also greater than or equal
                    // to the current version
                    value = atoi(v_siteString.at(1).c_str());
                    if (value >= DISPLAY_VERSION_MINOR)
                    {
                        // Check to make sure major version is also greater than or
                        // equal to the current version
                        value = atoi(v_siteString.at(0).c_str());
                        if (value >= DISPLAY_VERSION_MAJOR)
                        {
                            isUpToDate = false;
                        }
                    }
                }
                else
                {
                    value = atoi(v_siteString.at(3).c_str());
                    if (value > DISPLAY_VERSION_BUILD)
                    {
                        // Check to make sure revision number is also greater than or
                        // equal to the current revision
                        value = atoi(v_siteString.at(2).c_str());
                        if (value >= DISPLAY_VERSION_REVISION)
                         {
                            // Check to make sure minor version is also greater than or
                            // equal to the current version
                            value = atoi(v_siteString.at(1).c_str());
                            if (value >= DISPLAY_VERSION_MINOR)
                            {
                                // Check to make sure major version is also greater than
                                // or equal to the current version
                                value = atoi(v_siteString.at(0).c_str());
                                if (value >= DISPLAY_VERSION_MAJOR)
                                {
                                    isUpToDate = false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // If versions are the same, remove the update section, otherwise make sure
    // it is visible and show a link to the wallet download site
    if (isUpToDate)
    {
        showUpdateLayout(false);
    }
    else
    {
        report = QString(_UPDATE_DOWNLOAD_URL);
        report = QString("<a href=\"" + report + "\">Wallet version " + siteVersion + " available!</a>");
        showUpdateLayout(true);
    }

    // Set the update label text and exit
    ui->labelUpdateStatus->setText(report);
}