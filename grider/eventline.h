#ifndef __eventline_h__
#define __eventline_h__

#include "quotes.h"

#include <QScrollArea>

////////////////////////////////////////////////////////////
class Indicator;

////////////////////////////////////////////////////////////
class EventLine : public QScrollArea
{
    Q_OBJECT;
    friend class Indicator;
public:
    EventLine(const Q2TimeShared& q2t, QWidget* parent);
    ~EventLine();

    void start();
    void stop();

    bool  rescale(qreal ratio);
    qreal getScale() const;
    bool  isPointVisible(const QPoint& pt);

protected:
    // resize from outer
    void resizeEvent(QResizeEvent* e);
    void onFollowTimebar();

protected slots:
    void onVerticalBarValueChanged(int value);
    void onHorizontalBarValueChanged(int value);
    void onPlaysizeChanged(qint32 newWidth, qint32 newHeight);

private:
    Indicator* indicator_;
    bool followBar_;

    static qint32 sMaxGraphViewPixels_;
    static qint32 sMaxGraphViewNodes_;
};

#endif // __eventline_h__
