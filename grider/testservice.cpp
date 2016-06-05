#include "testservice.h"

#include <algorithm>
#include <time.h>

////////////////////////////////////////////////////////////
namespace {
    static time_t zeroRnd = 0;

    // randomize floating or integer number within restricted range
    qreal randomFloatStep(qreal min, qreal max)
    {

        qreal range = max-min;
        qreal floatpart = range-(int)range;

        int fpt = 0;
        if( floatpart)
            fpt = int(qreal(1)/floatpart); // for 12.034017, exp = (.034017), fpt = 6

        if( fpt ) {
            int c = fpt>5 ? 5 : fpt;
            fpt = (rand()%c); // set random fpt from 0-5, where 1-5 is floating, 0 is integer
        }
        range = min + (int(range) ? rand()%int(range) : 0);

        qreal expPart = 0.0;
        while( fpt-- ) {
            int r = rand()%10;
            r = (r==0)?1:r;
            expPart = (expPart==0)?1.0:expPart;
            expPart -= expPart/r;
        }
        if( expPart != 0.0 )
            range = (range+expPart > max)? qMax(expPart-range, range-expPart):range+expPart;
        return max < 0 ? -range : range;
    }

    // 5 - 200
    inline int randomPeakWidth()
    {  return 5+(rand()%195); }

    // negative is loss, positive is trade
    qreal randomTradeOrLoss()
    { return (0 == rand()%2 ? -1 : 1); }

    // 30-100
    inline int randomGranularity()
    { return 30+(rand()%70); }

    qint64 generateNextTime(qint64 start, qint32 range) 
    {
        return start+(rand()%range);
    }
};

/////////////////////////////////////////////////////////////////////////////
TestService::TestService(Q2TimeShared& q2t, QObject* parent)
    : QThread(parent),
    q2t_(q2t),
    shutdown_(false),
    stopped_(true),
    intencityChangePeriod_(0),
    growChangePeriod_(0),
    intencity_(0.5),
    growStep_(0.0)
{}

TestService::~TestService()
{
    stop();
    {
        QMutexLocker g(&sync_);
        shutdown_ = true;
    }
    wait();
}

void TestService::start()
{
    {
        QMutexLocker g(&sync_);
        stopped_ = false;
    }
    if( !QThread::isRunning() )
        QThread::start();

    srand(time(0));
}

void TestService::stop()
{
    QMutexLocker g(&sync_);
    stopped_ = true;
}

void TestService::run()
{
    bool shutdownFl = false, stoppedFl = false;

    while(!shutdownFl) 
    {
        if( stoppedFl ) 
            msleep(150);
        else
            processRandom();

        QMutexLocker g(&sync_);
        stoppedFl = stopped_;
        shutdownFl = shutdown_;
    }
}

void TestService::processRandom()
{
    // create period with constant intencity
    qint64 curr = currentMSecs();

    if( curr > intencityChangePeriod_ ) {
        intencityChangePeriod_ = generateNextTime(curr, 3000); // max 3 sec
        intencity_ = randomFloatStep(0.015, qreal(intencityChangePeriod_-curr)/100);
    }

    // create period with constant trade or loss
    if( curr > growChangePeriod_ ) {
        growChangePeriod_ = generateNextTime(currentMSecs(), 3000); // max 3 sec
        growStep_ = randomTradeOrLoss() * randomFloatStep(20,200.1111);
    }
    
    // random next rate time
    qint32 timeDelta;
    while(0 == (timeDelta = generateNextTime(0, intencity_*1000)));

    // random next rate
    FncEvent evt = (--q2t_->end()).value();
    if( (evt.currentRateAck_ > 1200 && growStep_ > 0) ||
        (evt.currentRateAck_ < 800 && growStep_ < 0) )
    {
        growStep_ *= -1;
    }
         
    qreal deltaRate = randomFloatStep(0, growStep_);
    evt.currentRateAck_ += deltaRate;
    evt.eventId_ = FncEvent::NOEVENT;

    // generate event
    q2t_->insert( currentMSecs()+timeDelta, evt);

    // wait the random delta
    msleep(timeDelta<16?15:timeDelta);
}
