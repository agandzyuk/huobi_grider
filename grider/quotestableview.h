#ifndef __quotestableview_h__
#define __quotestableview_h__

#include <QTableView>

////////////////////////////////////////////////////////////////
class QuotesTableModel;

////////////////////////////////////////////////////////////////
class QuotesTableView : public QTableView
{
    Q_OBJECT

public:
    QuotesTableView(QWidget *parent = 0);
    void updateStyles();
    void setModel(QAbstractItemModel* dataModel);
    QuotesTableModel* model() const;

protected:
    bool event(QEvent* e);
    void contextMenuEvent(QContextMenuEvent* e);
    void onToolTip(QHelpEvent* e);

protected slots:
    void onSendSubscribe() { onSendRequest(true); }
    void onSendUnSubscribe() { onSendRequest(false); }
    void onSetIndicator();

private:
    void onSendRequest(bool subscribe);
};

#endif // __quotestableview_h__
