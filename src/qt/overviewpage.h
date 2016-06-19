#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include "guiutil.h"

#include <QLabel>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QModelIndex;
class UpdateCheck;
QT_END_NAMESPACE

namespace Ui {
    class OverviewPage;
}
class ClientModel;
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

    void setBalanceLabel();

    void setClientUpdateCheck();
    QLabel *labelUpdateStatic;
    QLabel *labelUpdateStatus;
    UpdateCheck *m_pUpdCtrl;
    QTimer *updateTimer;

    void setPriceUpdateCheck();
    GUIUtil::QCLabel *labelPriceInBTC;
    GUIUtil::QCLabel *labelPriceInUSD;
    GUIUtil::QPriceInfo *priceInfo;

private slots:
    void updateDisplayUnit();
    void handleTransactionClicked(const QModelIndex &index);
    void timerUpdate();
    void checkForUpdates();
    void checkPrice();
};

#endif // OVERVIEWPAGE_H
