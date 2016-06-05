#include "eventline.h"
#include "indicator.h"

#include <QtWidgets>

////////////////////////////////////////////////////////////////////////////
Indicator::Indicator(const Q2TimeShared& q2t, QWidget* parent)
    : QCustomPlot(parent), 
    q2t_(q2t),
    ratio_(DEF_SCALE_VALUE),
    prevTimerMsecs_(0),
    scrollPos_(-1)
{
    QObject::connect(xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onRangeChanged(QCPRange)));
    QObject::connect(q2t_.data(), SIGNAL(marketEvent(FncEvent)), this, SLOT(onMarketEvent(FncEvent)));
    QObject::connect(q2t_.data(), SIGNAL(tradingEvent(FncEvent)), this, SLOT(onTradingEvent(FncEvent)));
    QObject::connect(this, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMove(QMouseEvent*)));
    

    setBackground(QColor(0,0,0));
    xAxis->setAutoTickStep(false);
    xAxis->setAutoTicks(false);
    //xAxis->setAutoTickLabels(false);

    xAxis->setTickLabelColor(Qt::green);
    xAxis->setBasePen(QPen(Qt::white, 1));
    xAxis->setTickLabelType(QCPAxis::ltDateTimeMsec);

    yAxis2->setLabel("BTC/CNY");
    yAxis2->setBasePen(QPen(Qt::white));
    yAxis2->setTickLabelColor(Qt::green);
    yAxis2->setLabelColor(Qt::lightGray);
    yAxis->setRange(500,1500);
    yAxis->setVisible(false);
    yAxis2->setRange(500,1500);
}

void Indicator::stop()
{
    timer_.stop();
}

void Indicator::start()
{
    // reset timer
    if( timer_.isActive() )
        timer_.stop();

    qint32 msecsPerTick = ratio_*DEF_MSECS_PERPIXEL;
    timer_.start(msecsPerTick, this);

    prevTimerMsecs_ = 0;
    recalGraphWidth();
    timerEvent(0);
}

void Indicator::resizeEvent(QResizeEvent* e)
{
    QCustomPlot::resizeEvent(e);

    qint32 pxRange = time2pixel(xAxis->range().upper-q2t_->startTime());
    if( pxRange < axisWidth())
        pxRange = axisWidth();
    emit notifyPlaysize(pxRange, e->size().height());
}

void Indicator::showEvent(QShowEvent* e)
{
    QCustomPlot::showEvent(e);

    if( 0 == graphCount() )
    {
        QPen pen(Qt::yellow);
        pen.setWidth(2);
        addGraph(xAxis,yAxis2);
        graph(0)->setPen(pen);
        yAxis2->setVisible(true);
    }
    start();
}

void Indicator::onMouseMove(QMouseEvent* e)
{
    qreal rate = getRateToTrack(e->pos());
    if( rate != 0.0 )
    {
        if( asklabel_.isNull() ) {
            asklabel_.reset(new QCPItemText(this));
            asklabel_->setColor( graph(0)->pen().color() );
            asklabel_->setPositionAlignment(Qt::AlignTop);
            asklabel_->position->setAxes(xAxis, yAxis2);
        }
        asklabel_->setText(QVariant(rate).toString());
        qreal xCoord = xAxis->pixelToCoord(e->pos().x());
        qreal yCoord = yAxis2->pixelToCoord(e->pos().y()+20);
        asklabel_->position->setCoords(xCoord,yCoord);
        asklabel_->setVisible(true);
    }
    else if( !asklabel_.isNull() ) 
        asklabel_->setVisible(false);
}

bool Indicator::rescale(qreal ratio)
{
    qint32 msecsPerTick = ratio*DEF_MSECS_PERPIXEL;
    if( msecsPerTick < MIN_MSECS_PERPIXEL || msecsPerTick > MAX_MSECS_PERPIXEL ) 
        return false;

    ratio_ = ratio;

    // reset timer
    if( timer_.isActive() )
        timer_.stop();
    timer_.start(msecsPerTick, this);

    prevTimerMsecs_ = 0;
    recalGraphWidth();
    timerEvent(0);
    if( scrollPos_ != -1 )
        parentScrollTo(scrollPos_);

    return true;
}

void Indicator::scrollTo(qint32 offset)
{
    if( q2t_->startTime() > xAxis->range().lower )
        return;

    scrollPos_ = offset;
    qreal min = q2t_->startTime()+pixel2time(offset);
    qreal max = min + axisDuration();
    xAxis->setRange(min,max);
}

void Indicator::scrollBegin()
{
    prevTimerMsecs_ = 0;
    scrollPos_ = 0;
    timerEvent(0);
}

void Indicator::scrollEnd()
{
    prevTimerMsecs_ = 0;
    scrollPos_ = -1;
    timerEvent(0);
}

void Indicator::onRangeChanged(const QCPRange& newRange)
{
    if( ratio_ < 0.25 )
        xAxis->setDateTimeFormat("m:ss");
    else if( ratio_ < 0.51 )
        xAxis->setDateTimeFormat("h:mm:ss");
    else if( ratio_ < 3.0 )
        xAxis->setDateTimeFormat("h'h':mm");
    else if( ratio_ < 16.0 )
        xAxis->setDateTimeFormat("h'h':mm");
    else if(ratio_ < 34.0 )
        xAxis->setDateTimeFormat("hh:mm\nd.MM.yy");
    else if( ratio_ > 880.0 )
        xAxis->setDateTimeFormat("d.MM.yy");
    else 
        xAxis->setDateTimeFormat("hh 'h'\nd.MM.yy");

    QVector<qreal> tickVector;
    qint64 aligned = qint64(newRange.lower/1000)-1;
    while( nextTickAlignment(newRange, &aligned) ) 
        tickVector.push_back(aligned*1000);

    xAxis->setTickVector(tickVector);
    xAxis->setSubTickCount(5);
}

bool Indicator::nextTickAlignment(const QCPRange& range, qint64* from) const
{
    qint64 v = *from;
    if( v >= quint32(range.upper/1000) ) return false;

    if(ratio_ < 0.02)
        *from += 1;
    else if(ratio_ < 0.053)
        *from += v%2?1:2;
    else if(ratio_ < 0.1)
        *from += v%5?(5-v%5):5;
    else if(ratio_ < 0.25)
        *from += v%10?(10-v%10):10;
    else if(ratio_ < 0.51)
        *from += v%30?(30-v%30):30;
    else if(ratio_ < 1.3)
        *from += v%60?(60-v%60):60;
    else if(ratio_ < 2.9)
        *from += v%120?(120-v%120):120;
    else if(ratio_ < 6.0)
        *from += v%300?(300-v%300):300;
    else if(ratio_ < 15.0)
        *from += v%600?(600-v%600):600;
    else if(ratio_ < 34.0)
        *from += v%1800?(1800-v%1800):1800;
    else if(ratio_ < 77.0)
        *from += v%3600?(3600-v%3600):3600;

    if( ratio_ > 77.0)
    {
        v += QDateTime::fromTime_t(v).offsetFromUtc();
        if(ratio_ < 128.0)
            *from += v%7200?(7200-v%7200):7200;
        else if(ratio_ < 225.0)
            *from += v%10800?(10800-v%10800):10800;
        else if(ratio_ < 390.0)
            *from += v%21600?(21600-v%21600):21600;
        else if(ratio_ < 887)
            *from += v%43200?(43200-v%43200):43200;
        else 
            *from += v%86400?(86400-v%86400):86400;
    }
    return true;
}

void Indicator::timerEvent(QTimerEvent* e)
{
    Q_UNUSED(e);

    // recalculate a range at least per one second 
    qint64 c = currentMSecs();
    if( c - prevTimerMsecs_ > 1000 && isTimebarVisible() )
    {
        if( (prevTimerMsecs_ == 0) || 
            (time2pixel(xAxis->range().upper-c) < 70.0) ) // 70 pixels free space
        {
            qint64 min = recalMinRange();
            qint64 max = min+axisDuration();
            xAxis->setRange(min, max);
            recalGraphWidth();
        }
        prevTimerMsecs_ = c;
    }

    // setup timebar
    if( timebar_.isNull() ) {
        timebar_.reset(new QCPItemLine(this));
        timebar_->setPen(QPen(Qt::blue));
        timebar_->setClipAxisRect(yAxis2->axisRect());
        timebar_->setHead(QCPLineEnding::esBar);
        timebar_->setTail(QCPLineEnding::esBar);
        timebar_->start->setAxes(xAxis, yAxis2);
        timebar_->end->setAxes(xAxis, yAxis2);
    }

    // set time position 
    timebar_->start->setCoords(currentMSecs(),500);
    timebar_->end->setCoords(currentMSecs(),1500);

    // setup timebar label 
    if( timelabel_.isNull() ) {
        timelabel_.reset(new QCPItemText(this));
        timelabel_->setColor(QColor(50,150,255));
        timelabel_->setPositionAlignment(Qt::AlignLeft);
    }

    // set timebar text and move to postion
    QString timeSpec = (ratio_ < 0.51 ? "mm:ss.zzz" : (ratio_ < 16.0 ? "hh:mm:ss" : 0 ));
    if( !timeSpec.isEmpty() )
        timelabel_->setText("  " + QDateTime::fromMSecsSinceEpoch(currentMSecs()).toString(timeSpec));
    else
        timelabel_->setText("  " + QDateTime::fromMSecsSinceEpoch(currentMSecs()).toString());
    timelabel_->position->setCoords(currentMSecs(),545);

    replot();
}

void Indicator::recalGraphWidth()
{
    qint32 w = axisWidth();
    qint64 rangeMax = recalMaxRange();
    qint64 startAbsolute = q2t_->startTime();
    if( rangeMax - axisDuration() >= startAbsolute )
        w = time2pixel(rangeMax - q2t_->startTime());
    
    //QCustomPlot::setMinimumWidth(wdgWidth);
    emit notifyPlaysize(w,height());
}

bool Indicator::getBounds(qreal xMousePt, QRectF* bounds) const
{
    QCPDataMap* data = graphCount() ? graph(0)->data() : NULL;
    if( data ) {
        QCPDataMap::const_iterator It = data->lowerBound(xMousePt);
        if( It != data->begin() && It != data->end() ) 
        {
            --It;
            bounds->setLeft(It->key);
            bounds->setTop(It->value);
            It = data->upperBound(xMousePt);
            if( It != data->end() ) {
                bounds->setRight(It->key);
                bounds->setBottom(It->value);
                return true;
            }
        }
    }
    return false;
}

qreal Indicator::getRateToTrack(const QPoint& mousePos) const
{
    qreal mX = mousePos.x();
    qreal mY = mousePos.y();
    qreal center = xAxis->pixelToCoord(mX);

    QRectF bounds; 
    if( getBounds(center, &bounds) ) 
    {
        qreal begX = xAxis->coordToPixel(bounds.left());
        qreal endX = xAxis->coordToPixel(bounds.right());
        qreal begY = yAxis2->coordToPixel(bounds.top());
        qreal endY = yAxis2->coordToPixel(bounds.bottom());

        qreal wdX = endX-begX;
        qreal htY = abs(endY-begY);

        // algorithm for too small or zero X distance
        if( wdX < 10 ) {
            if( mY > qMin(begY,endY)-5 && mY < qMax(begY,endY)+5 )
                return abs(mY-begY) < abs(mY-endY) ? bounds.top() : bounds.bottom();
            else
                return 0.0;
        }

        // algorithm for good visible X distance
        if( endX-mX > mX-begX ) {
            center = endY -  (endY>begY?1:-1)*(htY*(endX-mX)/wdX);
            return (abs(center-mY) < 6 ? bounds.top() : 0.0);
        }
        center = begY - (endY>begY?-1:1)*(htY*(mX-begX)/wdX);
        return (abs(center-mY) < 6 ? bounds.bottom() : 0.0);
    }
    return 0.0;
}

void Indicator::onMarketEvent(const FncEvent& e)
{
    if( 0 == graphCount() ) return;
    graph(0)->addData(currentMSecs(), e.currentRateAck_);
    replot();
}

void Indicator::onTradingEvent(const FncEvent& e)
{
    if( 0 == graphCount() ) return;
}
