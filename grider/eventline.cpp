#include "globals.h"
#include "eventline.h"
#include "indicator.h"

#include <QtWidgets>

////////////////////////////////////////////////////////////////////////////
qint32 EventLine::sMaxGraphViewPixels_;
qint32 EventLine::sMaxGraphViewNodes_;

/////////////////////////////////////////////////////////////////////////////
EventLine::EventLine(const Q2TimeShared& q2t, QWidget* parent)
    : QScrollArea(parent),
    followBar_(true)
   
{
    // init static members
    sMaxGraphViewPixels_ = Global::desktop.width()*0.8*20; // 0.8 window width, 20 max pages
    sMaxGraphViewNodes_  = 5000; // maximum nodes to perform

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSizeAdjustPolicy(AdjustToContentsOnFirstShow);

    indicator_ = new Indicator(q2t, this);

    QScrollBar* hbs = horizontalScrollBar();
    QScrollBar* vbs = verticalScrollBar();

    QObject::connect(hbs, SIGNAL(valueChanged(int)), this, SLOT(onHorizontalBarValueChanged(int)));
    QObject::connect(vbs, SIGNAL(valueChanged(int)), this, SLOT(onVerticalBarValueChanged(int)));
    QObject::connect(indicator_, SIGNAL(notifyPlaysize(qint32,qint32)),
                     this, SLOT(onPlaysizeChanged(qint32,qint32)));
    setContentsMargins(0,0,0,0);
    setMinimumWidth(320);
}

EventLine::~EventLine()
{
    indicator_->stop();
}

void EventLine::start()
{
    indicator_->start();
}

void EventLine::stop()
{
    indicator_->stop();
}

qreal EventLine::getScale() const
{
    return indicator_->scale();
}

bool EventLine::rescale(qreal ratio)
{
    followBar_ = indicator_->isTimebarVisible();
    return indicator_->rescale(ratio);
}

void EventLine::resizeEvent(QResizeEvent* e)
{
    QScrollArea::resizeEvent(e);
    int rw = qMax(indicator_->width(), e->size().width());
    indicator_->resize(rw, e->size().height());
}

void EventLine::onPlaysizeChanged(qint32 newWidth, qint32 newHeight)
{
    qint32 areaW = viewport()->width();
    qint32 areaH = viewport()->height();

    horizontalScrollBar()->setPageStep(areaW);
    horizontalScrollBar()->setRange(0, newWidth-areaW);
    
    verticalScrollBar()->setPageStep(areaH);
    verticalScrollBar()->setRange(0, newHeight-areaH);
    
    if( followBar_ ) onFollowTimebar();

    viewport()->update();
}

void EventLine::onFollowTimebar()
{
    horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
}

void EventLine::onVerticalBarValueChanged(int value)
{
    QPoint pt = viewport()->pos();
    indicator_->move(pt.x(), pt.y() - value);
}

static qint64 prevTime = 0;
void EventLine::onHorizontalBarValueChanged(int value)
{
    // set scroll refresh to minimum 25 fps
    qint64 c = indicator_->currentMSecs();
    if( c-prevTime < 1000/25)
        return;
    prevTime = c;

    if( horizontalScrollBar()->maximum() - value < 20 )
        indicator_->scrollEnd();
    else if( value < 20 )
        indicator_->scrollBegin();
    else
        indicator_->scrollTo(value);

    followBar_ = indicator_->isTimebarVisible();
}   
