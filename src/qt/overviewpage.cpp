#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "magiunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "util.h"
#include "guiconstants.h"
#include "askpassphrasedialog.h"
#include "updatecheck.h"
#include "version.h"
#include "clientversion.h"
#include <vector>

#include <QAbstractItemDelegate>
#include <QPainter>

#include <QDesktopServices>  //Added for openURL()
#include <QTimer>            //Added for update timer
#include <QUrl>

#define DECORATION_SIZE 44
#ifdef Q_OS_MAC
#define NUM_ITEMS 5
#define BAL_FONT_SIZE 11
#define TRANS_FONT_SIZE 11
#else
#define NUM_ITEMS 5
#define BAL_FONT_SIZE 8
#define TRANS_FONT_SIZE 8
#endif

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
    setBalanceLabel();

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE-1));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);
    QFont fnt;
    fnt.setPointSize(TRANS_FONT_SIZE);
    fnt.setKerning(true);
    ui->listTransactions->setFont(fnt);
    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));
#ifdef Q_OS_MAC
    ui->frameBalance->setMinimumWidth(240);
    ui->listTransactions->setMinimumWidth(260);
#else
#ifdef Q_OS_WIN
    ui->frameBalance->setMinimumWidth(240);
    ui->labelTotalText->setMinimumWidth(90);
    ui->listTransactions->setMinimumWidth(270);
#else
    ui->listTransactions->setMinimumWidth(275);
#endif
#endif
    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
    ui->labelTransactionsStatus->setText("(" + tr("out of sync") + ")");

    // Setup a timer to regularly check for updates to the wallet
    // Have it trigger once, immediately, then set it to check once ever 24 hours
    // The update timer conversion factor (_UPDATE_MS_TO_HOURS) is located in
    // updatecheck.h
    setClientUpdateCheck();
    timerUpdate();
    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    updateTimer->start(_UPDATE_INTERVAL*_UPDATE_MS_TO_HOURS);

    // check price
    connect(this, SIGNAL(valueChanged()), this, SLOT(updateValues()));
    priceInfo = new GUIUtil::QPriceInfo();
    setPriceUpdateCheck();
    checkPrice();

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

void OverviewPage::setBalanceLabel()
{
    QFont font;
    font.setPointSize(BAL_FONT_SIZE);
    font.setBold(true);
    font.setItalic(false);
    font.setUnderline(false);
    font.setWeight(QFont::Bold);
    font.setStrikeOut(false);
    font.setKerning(true);

    ui->labelBalance->setFont(font);
    ui->labelStake->setFont(font);
    ui->labelUnconfirmed->setFont(font);
    ui->labelImmature->setFont(font);
    ui->labelTotal->setFont(font);
    ui->labelTotalInUSD->setFont(font);
}

void OverviewPage::setClientUpdateCheck()
{
    labelUpdateStatic = new QLabel(ui->frameBalance);
    labelUpdateStatic->setObjectName(QStringLiteral("labelUpdateStatic"));
    QFont font5;
    font5.setPointSize(BAL_FONT_SIZE);
    font5.setBold(true);
    font5.setItalic(false);
    font5.setUnderline(false);
    font5.setWeight(QFont::Bold);
    font5.setStrikeOut(false);
    font5.setKerning(true);
    labelUpdateStatic->setFont(font5);
//    labelUpdateStatic->setStyleSheet(QStringLiteral("color: #1D62F0;"));
    labelUpdateStatic->setText(QStringLiteral("New version available:"));
    labelUpdateStatic->setText(QApplication::translate("OverviewPage", "New version available:", 0));

    labelUpdateStatus = new QLabel(ui->frameBalance);
    labelUpdateStatus->setObjectName(QStringLiteral("labelUpdateStatus"));
    labelUpdateStatus->setStyleSheet(QStringLiteral("QLabel { color: red; }"));
    labelUpdateStatus->setText(QStringLiteral("(checking)"));
    labelUpdateStatus->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

    labelUpdateStatus->setText("(" + tr("checking for updates") + ")");
    labelUpdateStatus->setTextFormat(Qt::RichText);
    labelUpdateStatus->setTextInteractionFlags(Qt::TextBrowserInteraction);
    labelUpdateStatus->setOpenExternalLinks(true);

    ui->formLayout_3->setWidget(2, QFormLayout::LabelRole, labelUpdateStatic);
    ui->formLayout_3->setWidget(2, QFormLayout::FieldRole, labelUpdateStatus);
}
void OverviewPage::setPriceUpdateCheck()
{
    /*
    QLabel *labelPriceText = new QLabel(ui->frameBalance);
    labelPriceText->setObjectName(QStringLiteral("labelPriceText"));
    QFont font1;
    font1.setPointSize(9);
    labelPriceText->setFont(font1);
//    labelPriceText->setStyleSheet(QStringLiteral("color: #464747;"));
    labelPriceText->setText("Price:");
    ui->formLayout_3->setWidget(0, QFormLayout::LabelRole, labelPriceText);
    */

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    QFont font2;
    font2.setPointSize(BAL_FONT_SIZE);
    font2.setBold(false);
    font2.setWeight(QFont::Normal);
    labelPriceInBTC = new GUIUtil::QCLabel("", ui->frameBalance);
    sizePolicy.setHeightForWidth(labelPriceInBTC->sizePolicy().hasHeightForWidth());
    labelPriceInBTC->setSizePolicy(sizePolicy);

    labelPriceInBTC->setObjectName(QStringLiteral("labelPriceInBTC"));
    labelPriceInBTC->setFont(font2);
    labelPriceInBTC->setLayoutDirection(Qt::LeftToRight);
//    labelPriceInBTC->setStyleSheet(QStringLiteral("color: #1D62F0;"));
    labelPriceInBTC->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    labelPriceInBTC->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
    labelPriceInBTC->setToolTip(QApplication::translate("OverviewPage", "Price in BTC, click to refresh", 0));
    labelPriceInBTC->setText(QApplication::translate("OverviewPage", "0 BTC/XMG", 0));

    labelPriceInUSD = new GUIUtil::QCLabel("", ui->frameBalance);
    sizePolicy.setHeightForWidth(labelPriceInUSD->sizePolicy().hasHeightForWidth());
    labelPriceInUSD->setSizePolicy(sizePolicy);

    labelPriceInUSD->setObjectName(QStringLiteral("labelPriceInUSD"));
    labelPriceInUSD->setFont(font2);
    labelPriceInUSD->setLayoutDirection(Qt::LeftToRight);
//    labelPriceInUSD->setStyleSheet(QStringLiteral("color: #1D62F0;"));
    labelPriceInUSD->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    labelPriceInUSD->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
    labelPriceInUSD->setToolTip(QApplication::translate("OverviewPage", "Price in USD, click to refresh", 0));
    labelPriceInUSD->setText(QApplication::translate("OverviewPage", "0 USD/XMG", 0));

#ifdef Q_OS_MAC
    ui->labelPriceText->setMinimumWidth(98);
#else
#ifdef Q_OS_WIN
    ui->labelPriceText->setMinimumWidth(93);
#else
    ui->labelPriceText->setMinimumWidth(100);
#endif
#endif
    ui->formLayout_3->setWidget(0, QFormLayout::FieldRole, labelPriceInBTC);
    ui->formLayout_3->setWidget(1, QFormLayout::FieldRole, labelPriceInUSD);

    connect(labelPriceInBTC, SIGNAL(clicked()), this, SLOT(checkPrice()));
    connect(labelPriceInUSD, SIGNAL(clicked()), this, SLOT(checkPrice()));
    connect(priceInfo, SIGNAL(finished()), this, SLOT(updateValues()));
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
    currentTotalBalance = balance + stake + unconfirmedBalance;
    ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balance));
    ui->labelStake->setText(BitcoinUnits::formatWithUnit(unit, stake));
    ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, immatureBalance));
    ui->labelTotal->setText(BitcoinUnits::formatWithUnit(unit, currentTotalBalance));

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    ui->labelImmature->setVisible(showImmature);
    ui->labelImmatureText->setVisible(showImmature);

    // emit a value-change signal per balance updating
    emit valueChanged();
}

void OverviewPage::setNumTransactions(int count)
{
//    ui->labelNumTransactions->setText(QLocale::system().toString(count));
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

//        setNumTransactions(model->getNumTransactions());
//        connect(model, SIGNAL(numTransactionsChanged(int)), this, SLOT(setNumTransactions(int)));

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
    labelUpdateStatus->setVisible(fShow);
}

void OverviewPage::showUpdateLayout(bool fShow)
{
    labelUpdateStatic->setVisible(fShow);
    labelUpdateStatus->setVisible(fShow);
}

void OverviewPage::timerUpdate()
{
    // create a connection to the website to check for updates
    QUrl updateUrl(_UPDATE_VERSION_URL);
    m_pUpdCtrl = new UpdateCheck(updateUrl, this);
    connect(m_pUpdCtrl, SIGNAL (downloaded()), this, SLOT (checkForUpdates()));
}

void OverviewPage::checkPrice()
{
    priceInfo->checkPrice();
}

void OverviewPage::updateValues()
{
//    qint64 valueInBTC = currentTotalBalance * priceInfo->getPriceInBTC();
    qint64 valueInUSD = currentTotalBalance * priceInfo->getPriceInUSD();
//    ui->labelTotalInBTC->setText(BitcoinUnits::format(0, valueInBTC, false) + QString(" BTC"));
    ui->labelTotalInUSD->setText(BitcoinUnits::format(0, valueInUSD, false) + QString(" USD"));
    labelPriceInBTC->setText(BitcoinUnits::format(0, priceInfo->getPriceInBTC()*100000000, false) + QString(" BTC/XMG"));
    labelPriceInUSD->setText(BitcoinUnits::format(0, priceInfo->getPriceInUSD()*100000000, false) + QString(" USD/XMG"));
}

void OverviewPage::checkForUpdates()
{
    // Grab the internal wallet version and the online version string
    QString qsCurrentClientVersion(m_pUpdCtrl->downloadedData());
    QString report = QString("Client is up to date");

    // Split the online version string and compare it against the internal version
    // If at any point we find that the internal version is less than the online
    // version, exit without setting bool isUpToDate. If the internal version is
    // equal to or greater than the online version, set isUpToDate = true.
    bool isUpToDate = true;
    unsigned int nVersion = m_pUpdCtrl->parseClientVersion(qsCurrentClientVersion.toStdString(), '.');
    if (nVersion > GetClientVersion(CLIENT_VERSION, CLIENT_VERSION_RELEASE_CANDIDATE))
        isUpToDate = false;

    // If versions are the same, remove the update section, otherwise make sure
    // it is visible and show a link to the wallet download site
    if (isUpToDate) {
        showUpdateLayout(false);
    } else {
        report = QString(_UPDATE_DOWNLOAD_URL);
        report = QString("<a href=\"" + report + "\">v" + qsCurrentClientVersion + " available</a>");
        showUpdateLayout(true);
    }

    // Set the update label text and exit
    labelUpdateStatus->setText(report);
}
