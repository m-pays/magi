#include "console.h"
#include "rpcconsole.h"
#include "ui_console.h"

#include "clientmodel.h"
#include "magirpc.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QTime>
#include <QTimer>
#include <QThread>
#include <QTextEdit>
#include <QKeyEvent>
#include <QUrl>
#include <QScrollBar>

// TODO: make it possible to filter out categories (esp debug messages when implemented)
// TODO: receive errors and debug messages through ClientModel

//#include "console.moc"

Console::Console(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Console),
    historyPtr(0)
{
    ui->setupUi(this);

    // Install event filter for up and down arrow
    ui->consoleLineEdit->installEventFilter(this);
    ui->consoleMessagesWidget->installEventFilter(this);

    connect(ui->consoleClearButton, SIGNAL(clicked()), this, SLOT(clear()));

    startExecutor();

    clear();
}

Console::~Console()
{
    emit stopExecutor();
    delete ui;
}

bool Console::eventFilter(QObject* obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress) // Special key handling
    {
        QKeyEvent *keyevt = static_cast<QKeyEvent*>(event);
        int key = keyevt->key();
        Qt::KeyboardModifiers mod = keyevt->modifiers();
        switch(key)
        {
        case Qt::Key_Up: if(obj == ui->consoleLineEdit) { browseHistory(-1); return true; } break;
        case Qt::Key_Down: if(obj == ui->consoleLineEdit) { browseHistory(1); return true; } break;
        case Qt::Key_PageUp: /* pass paging keys to messages widget */
        case Qt::Key_PageDown:
            if(obj == ui->consoleLineEdit)
            {
                QApplication::postEvent(ui->consoleMessagesWidget, new QKeyEvent(*keyevt));
                return true;
            }
            break;
        default:
            // Typing in messages widget brings focus to line edit, and redirects key there
            // Exclude most combinations and keys that emit no text, except paste shortcuts
            if(obj == ui->consoleMessagesWidget && (
                  (!mod && !keyevt->text().isEmpty() && key != Qt::Key_Tab) ||
                  ((mod & Qt::ControlModifier) && key == Qt::Key_V) ||
                  ((mod & Qt::ShiftModifier) && key == Qt::Key_Insert)))
            {
                ui->consoleLineEdit->setFocus();
                QApplication::postEvent(ui->consoleLineEdit, new QKeyEvent(*keyevt));
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}


static QString consoleCategoryClass(int category)
{
    switch(category)
    {
    case RPCConsole::CMD_REQUEST:  return "cmd-request"; break;
    case RPCConsole::CMD_REPLY:    return "cmd-reply"; break;
    case RPCConsole::CMD_ERROR:    return "cmd-error"; break;
    default:                       return "misc";
    }
}

void Console::clear()
{
    ui->consoleMessagesWidget->clear();
    ui->consoleLineEdit->clear();
    ui->consoleLineEdit->setFocus();

    // Add smoothly scaled icon images.
    // (when using width/height on an img, Qt uses nearest instead of linear interpolation)
    for(int i=0; CONSOLE_ICON_MAPPING[i].url; ++i)
    {
        ui->consoleMessagesWidget->document()->addResource(
                    QTextDocument::ImageResource,
                    QUrl(CONSOLE_ICON_MAPPING[i].url),
                    QImage(CONSOLE_ICON_MAPPING[i].source).scaled(CONSOLE_ICON_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    // Set default style sheet
    ui->consoleMessagesWidget->document()->setDefaultStyleSheet(
                "table { }"
                "td.time { color: #808080; padding-top: 3px; } "
                "td.message { font-family: Monospace; font-size: 12px; } "
                "td.cmd-request { color: #006060; } "
                "td.cmd-error { color: red; } "
                "b { color: #006060; } "
                );

    message(CMD_REPLY, (tr("Welcome to the Magi RPC console.") + "<br>" +
                        tr("Use up and down arrows to navigate history, and <b>Ctrl-L</b> to clear screen.") + "<br>" +
                        tr("Type <b>help</b> for an overview of available commands.")), true);
}

void Console::message(int category, const QString &message, bool html)
{
    QTime time = QTime::currentTime();
    QString timeString = time.toString();
    QString out;
    out += "<table><tr><td class=\"time\" width=\"65\">" + timeString + "</td>";
    out += "<td class=\"icon\" width=\"32\"><img src=\"" + consoleCategoryClass(category) + "\"></td>";
    out += "<td class=\"message " + consoleCategoryClass(category) + "\" valign=\"middle\">";
    if(html)
        out += message;
    else
        out += GUIUtil::HtmlEscape(message, true);
    out += "</td></tr></table>";
    ui->consoleMessagesWidget->append(out);
}

void Console::on_consoleLineEdit_returnPressed()
{
    QString cmd = ui->consoleLineEdit->text();
    ui->consoleLineEdit->clear();

    if(!cmd.isEmpty())
    {
        message(CMD_REQUEST, cmd);
        emit cmdRequest(cmd);
        // Truncate history from current position
        history.erase(history.begin() + historyPtr, history.end());
        // Append command to history
        history.append(cmd);
        // Enforce maximum history size
        while(history.size() > CONSOLE_HISTORY)
            history.removeFirst();
        // Set pointer to end of history
        historyPtr = history.size();
        // Scroll console view to end
        scrollToEnd();
    }
}

void Console::browseHistory(int offset)
{
    historyPtr += offset;
    if(historyPtr < 0)
        historyPtr = 0;
    if(historyPtr > history.size())
        historyPtr = history.size();
    QString cmd;
    if(historyPtr < history.size())
        cmd = history.at(historyPtr);
    ui->consoleLineEdit->setText(cmd);
}

void Console::startExecutor()
{
    QThread* thread = new QThread;
    RPCExecutor *executor = new RPCExecutor();
    executor->moveToThread(thread);

    // Notify executor when thread started (in executor thread)
    connect(thread, SIGNAL(started()), executor, SLOT(start()));
    // Replies from executor object must go to this object
    connect(executor, SIGNAL(reply(int,QString)), this, SLOT(message(int,QString)));
    // Requests from this object must go to executor
    connect(this, SIGNAL(cmdRequest(QString)), executor, SLOT(request(QString)));
    // On stopExecutor signal
    // - queue executor for deletion (in execution thread)
    // - quit the Qt event loop in the execution thread
    connect(this, SIGNAL(stopExecutor()), executor, SLOT(deleteLater()));
    connect(this, SIGNAL(stopExecutor()), thread, SLOT(quit()));
    // Queue the thread for deletion (in this thread) when it is finished
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    // Default implementation of QThread::run() simply spins up an event loop in the thread,
    // which is what we want.
    thread->start();
}

void Console::scrollToEnd()
{
    QScrollBar *scrollbar = ui->consoleMessagesWidget->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}
