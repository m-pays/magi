#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
class QModelIndex;
class UpdateCheck;
QT_END_NAMESPACE

namespace Ui {
    class OverviewPage;
}
class WalletModel;
class TxViewDelegate;
class TransactionFilterProxy;

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget *parent = 0);
    ~OverviewPage();

    void setModel(WalletModel *model);
    void showOutOfSyncWarning(bool fShow);
    void showUpdateWarning(bool fShow);
    void showUpdateLayout(bool fShow);

public slots:
    void setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance);
    void setNumTransactions(int count);
    void updateValues();

signals:
    void transactionClicked(const QModelIndex &index);
    void valueChanged();

private:
    Ui::OverviewPage *ui;
    WalletModel *model;
    qint64 currentBalance;
    qint64 currentStake;
    qint64 currentUnconfirmedBalance;
    qint64 currentImmatureBalance;
    qint64 currentTotalBalance;

    TxViewDelegate *txdelegate;
    TransactionFilterProxy *filter;

    UpdateCheck *m_pUpdCtrl;
    QTimer *updateTimer;

    double rPriceInBTC;
    double rPriceInUSD;
    QNetworkAccessManager mUSDPriceCheck;
    QNetworkAccessManager mBTCPriceCheck;

private slots:
    void updateDisplayUnit();
    void handleTransactionClicked(const QModelIndex &index);
    void timerUpdate();
    void checkForUpdates();
    void updateValueInUSD(QNetworkReply* resp);
    void updateValueInBTC(QNetworkReply* resp);
};

#endif // OVERVIEWPAGE_H
