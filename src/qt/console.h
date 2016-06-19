#ifndef CONSOLE_H
#define CONSOLE_H

#include <QDialog>

namespace Ui {
class Console;
}

class ClientModel;

class Console : public QDialog
{
    Q_OBJECT

public:
    explicit Console(QWidget *parent = 0);
    ~Console();

    void setClientModel(ClientModel *model);

    enum MessageClass {
        MC_ERROR,
        MC_DEBUG,
        CMD_REQUEST,
        CMD_REPLY,
        CMD_ERROR
    };

protected:
    virtual bool eventFilter(QObject* obj, QEvent *event);

private slots:
    void on_consoleLineEdit_returnPressed();

public slots:
    void clear();
    void message(int category, const QString &message, bool html = false);
    /** Go forward or back in history */
    void browseHistory(int offset);
    /** Scroll console view to end */
    void scrollToEnd();
signals:
    // For RPC command executor
    void stopExecutor();
    void cmdRequest(const QString &command);

private:
    Ui::Console *ui;
    ClientModel *clientModel;
    QStringList history;
    int historyPtr;

    void startExecutor();
};

#endif // CONSOLE_H
