#ifndef RPCCONSOLE_H
#define RPCCONSOLE_H

#include <QDialog>

/* Object for executing console RPC commands in a separate thread.
*/
class RPCExecutor: public QObject
{
    Q_OBJECT
public slots:
    void start();
    void request(const QString &command);
signals:
    void reply(int category, const QString &command);
};

namespace Ui {
    class RPCConsole;
}
class ClientModel;

/** Local Bitcoin RPC console. */
class RPCConsole: public QDialog
{
    Q_OBJECT

public:
    explicit RPCConsole(QWidget *parent = 0);
    ~RPCConsole();

    void setClientModel(ClientModel *model);

    enum MessageClass {
        MC_ERROR,
        MC_DEBUG,
        CMD_REQUEST,
        CMD_REPLY,
        CMD_ERROR
    };

protected:
//    virtual bool eventFilter(QObject* obj, QEvent *event);

private slots:
    void on_tabWidget_currentChanged(int index);
    /** open the debug.log from the current datadir */
    void on_openDebugLogfileButton_clicked();
    /** display messagebox with program parameters (same as magi-qt --help) */
    void on_showCLOptionsButton_clicked();

public slots:
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
    /** Set number of blocks shown in the UI */
    void setNumBlocks(int count, int countOfPeers);
signals:
    // For RPC command executor
    void stopExecutor();
    void cmdRequest(const QString &command);

private:
    Ui::RPCConsole *ui;
    ClientModel *clientModel;
    QStringList history;
    int historyPtr;

    void startExecutor();
};

#endif // RPCCONSOLE_H
