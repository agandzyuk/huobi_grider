#ifndef __maindialog_h__
#define __maindialog_h__

#include "requesthandler.h"
#include <QDialog>

class EventLine;
class Quotes2Time;
class QuotesTableView;
class QuotesTableModel;
class NetworkManager;
class StatusBar;

QT_BEGIN_NAMESPACE
class QPushButton;
class QCheckBox;
class QVBoxLayout;
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT

friend class StatusBar;
public:
    MainDialog();
    ~MainDialog();

protected:
    void setupButtons();
    void setupTable();
    void setupIndicator();
    void setupStatus();

public slots:
    void onReconnectSetCheck(bool on);

    // UI slots
protected slots:
    void onStart();
    void onStop();
    void onSettings();
    void asyncStart();
    void asyncStop();
    void onStateChanged(quint8 state, const QString& reason);
    
private:
    QPushButton* startButton_;
    QPushButton* stopButton_;
    QPushButton* colorButton_;
    QPushButton* settingsButton_;
    QCheckBox*   reconnectBox_;
    StatusBar*   statusBar_;

    QSharedPointer<QuotesTableView> tableview_;
    QSharedPointer<NetworkManager>  netman_;
    QSharedPointer<EventLine> indicator_;
    QSharedPointer<Quotes2Time> q2t_;
};

#endif // __maindialog_h__
