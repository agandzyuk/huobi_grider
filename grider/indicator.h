#ifndef __indicator_h__
#define __indicator_h__

#include "qcustomplot.h"
#include "quotes.h"

#include <QAbstractScrollArea>

#define MIN_MSECS_PERPIXEL          (15)
#define MAX_MSECS_PERPIXEL          (2000000L)
#define DEF_MSECS_PERPIXEL          (1000L)
#define DEF_SCALE_VALUE             (1.0)

////////////////////////////////////////////////////////////
class Indicator : public QCustomPlot
{
    Q_OBJECT
    friend class EventLine;

Q_SIGNALS:
    void notifyPlaysize(qint32 newWidth, qint32 newHeight);

protected slots:
    void onRangeChanged(const QCPRange& newRange);
    void onMarketEvent(const FncEvent& e);
    void onTradingEvent(const FncEvent& e);
    void onMouseMove(QMouseEvent* e);

protected:
    Indicator(const Q2TimeShared& q2t, QWidget* parent);

    void start();
    void stop();
    bool rescale(qreal ratio);
    void scrollTo(qint32 offset);
    void scrollBegin();
    void scrollEnd();

    void resizeEvent(QResizeEvent* e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent* e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent* e) Q_DECL_OVERRIDE;

    inline bool isTimebarVisible() const
    { return (currentMSecs() <= xAxis->range().upper) || (prevTimerMsecs_ == 0); }

    inline qint32 timebarPosition() const
    { return xAxis->coordToPixel(currentMSecs()); }

    inline qreal axisWidth() const
    { return xAxis->axisRect()->width(); }

    inline qreal axisDuration() const
    { return pixel2time(xAxis->axisRect()->width()); }

private:
    void recalGraphWidth();
    bool nextTickAlignment(const QCPRange& range, qint64* from) const;

    // get coordinates on graph bounding the given point
    // return null rect if no bounding found
    bool getBounds(qreal xMousePt, QRectF* bounds) const;
    qreal getRateToTrack(const QPoint& mousePos) const;

    // ~0.5 - 0.91 ratio of axis time duration
    inline qreal freeSpaceRatio() const;
    inline qint64 recalMinRange() const;
    inline qint64 recalMaxRange() const;
    inline qreal scale() const;
    inline qint64 currentMSecs() const;
    inline qint32 pixel2time(qreal pixels) const;
    inline qreal time2pixel(qint32 msecs) const;
    inline void parentScrollTo(qint32 val);
    inline void parentScrollEnd();

private:
    QBasicTimer timer_;
    const Q2TimeShared& q2t_;
    QCPDataMap map_;

    QScopedPointer<QCPItemLine> timebar_;
    QScopedPointer<QCPItemText> timelabel_;
    QScopedPointer<QCPItemText> asklabel_;

    qint64  prevTimerMsecs_;
    qreal   ratio_;
    qint64  scrollPos_;
};

inline qreal Indicator::freeSpaceRatio() const 
{ return (ratio_<3.8 ? 0.5+ratio_*(0.12+ratio_/100) : 0.85+0.3/ratio_); }

inline qint64 Indicator::recalMinRange() const { 
    if( scrollPos_ == -1 )
        return currentMSecs() - axisDuration()*freeSpaceRatio(); 
    return q2t_->startTime() + pixel2time(scrollPos_);
}
inline qint64 Indicator::recalMaxRange() const 
{ return currentMSecs() + axisDuration()*(1 - freeSpaceRatio()); }

inline qreal Indicator::scale() const
{ return ratio_; }

inline qint64 Indicator::currentMSecs() const
{ return QDateTime::currentMSecsSinceEpoch(); }

inline qint32 Indicator::pixel2time(qreal pixels) const
{ return (qint32)(ratio_*pixels*DEF_MSECS_PERPIXEL); }

inline qreal Indicator::time2pixel(qint32 msecs) const
{ return (qreal(msecs)/(ratio_*DEF_MSECS_PERPIXEL)); }

inline void Indicator::parentScrollTo(qint32 val) { 
    QAbstractScrollArea* p = qobject_cast<QAbstractScrollArea*>(parentWidget());
    if(p) p->horizontalScrollBar()->setValue(val);
}
inline void Indicator::parentScrollEnd() { 
    QAbstractScrollArea* p = qobject_cast<QAbstractScrollArea*>(parentWidget());
    if(p) p->horizontalScrollBar()->setValue(p->horizontalScrollBar()->maximum());
}

#endif // __indicator_h__
