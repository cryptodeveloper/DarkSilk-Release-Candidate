#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include <QWidget>
#include <QTimer>

#include "amount.h"

class ClientModel;
class WalletModel;
class TxViewDelegate;
class TransactionFilterProxy;

namespace Ui {
    class OverviewPage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/// Overview ("home") page widget
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget *parent = 0);
    ~OverviewPage();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);
    void showOutOfSyncWarning(bool fShow);

public slots:
    void sandStormStatus();
    void setBalance(const CAmount& balance, const CAmount& stake, const CAmount& unconfirmedBalance,
                    const CAmount& immatureBalance, const CAmount& anonymizedBalance, const CAmount& watchOnlyBalance,
                    const CAmount& watchOnlyStake, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);

signals:
    void transactionClicked(const QModelIndex &index);

private:
    QTimer *timer;
    Ui::OverviewPage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
    CAmount currentBalance;
    CAmount currentStake;
    CAmount currentUnconfirmedBalance;
    CAmount currentImmatureBalance;
    CAmount currentAnonymizedBalance;
    CAmount currentWatchOnlyBalance;
    CAmount currentWatchUnconfBalance;
    CAmount currentWatchImmatureBalance;
    CAmount currentWatchOnlyStake;
    int nDisplayUnit;

    qint64 lastNewBlock;

    int showingSandStormMessage;
    int sandstormActionCheck;
    int cachedNumBlocks;
    int cachedTxLocks;
    TxViewDelegate *txdelegate;
    TransactionFilterProxy *filter;

private slots:
    void toggleSandstorm();
    void sandstormAuto();
    void sandstormReset();
    void updateDisplayUnit();
    void updateSandstormProgress();
    void handleTransactionClicked(const QModelIndex &index);
    void updateAlerts(const QString &warnings);
    void updateWatchOnlyLabels(bool showWatchOnly);
};

#endif // OVERVIEWPAGE_H
