#ifndef __quotes2time_h__
#define __quotes2time_h__

#include "orderinfo.h"
#include <QMap>
#include <QDateTime>
#include <QSharedPointer>

////////////////////////////////////////////////////////////
QT_BEGIN_NAMESPACE;
QT_END_NAMESPACE;

////////////////////////////////////////////////////////////
struct FncEvent
{
    qint32      eventId_;
    qint32      orderId_;
    OrdStatus   status_;
    qreal       currentRateAck_;
    qreal       currentRateBid_;

    FncEvent();
    static qint32 NOEVENT;
};

//////////////////////////////////////////////////////////////////////////////
// Container store time in milliseconds relative the 1 Jan 1970 UTC
// An initial time by default equals the current local time
// in case when it not specified by object construction
class Quotes2Time : public QObject, public QMap<qint32,FncEvent>
{
    Q_OBJECT

    typedef QMap<qint32,FncEvent> BaseT;

Q_SIGNALS:
    void marketEvent(const FncEvent& e);
    void tradingEvent(const FncEvent& e);

public:
    Quotes2Time(qint64 initMsecsFrom1Jan1970UTC = 0);

    iterator insert(const qint32& key, const FncEvent& value);

    // Start time in milliseconds from 1970-01-01T00:00:00 UTC 
    inline qint64 startTime() const
    { return initTime_; }

    // Milliseconds part of start time
    inline qint64 startMsecs() const
    { return initTime_%1000; }

    // Finish local time in milliseconds from 1970-01-01T00:00:00 UTC
    inline qint64 finishTime() const
    { return empty() ? startTime() : (initTime_+(--end()).key()); }

    // Milliseconds part of finish time
    inline qint16 finishMsecs() const
    { return empty() ? startMsecs() : (initTime_+(--end()).key())%1000; }

    // Length of container in milliseconds
    inline qint64 duration() const
    { return finishTime() - startTime(); }

    // Duration to current time container in milliseconds
    inline qint64 durationAbsolute() const
    { return QDateTime::currentMSecsSinceEpoch() - startTime(); }

private:
    qint64 initTime_;
};

typedef QSharedPointer<Quotes2Time> Q2TimeShared;

#endif // __quotes2time_h__
