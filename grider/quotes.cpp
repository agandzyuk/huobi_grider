#include "quotes.h"

////////////////////////////////////////////////////////////
qint32  FncEvent::NOEVENT   = -1;

FncEvent::FncEvent()
    : eventId_(NOEVENT),
    orderId_(0),
    status_(status_NEW),
    currentRateAck_(0.0),
    currentRateBid_(0.0)
{}

Quotes2Time::Quotes2Time(qint64 initMsecsFrom1Jan1970UTC)
    : initTime_(initMsecsFrom1Jan1970UTC)
{
    if( initTime_ == 0 )
        initTime_ = QDateTime::currentMSecsSinceEpoch();

    qRegisterMetaType<FncEvent>("FncEvent"); 
}

Quotes2Time::iterator Quotes2Time::insert(const qint32& key, const FncEvent& value)
{
    iterator it = BaseT::insert(key, value);
    if( it->eventId_ == FncEvent::NOEVENT )
        emit marketEvent(it.value());
    else
        emit tradingEvent(it.value());
    return it;
}
