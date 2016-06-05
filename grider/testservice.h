#ifndef __testservice_h__
#define __testservice_h__

#include "quotes.h"
#include <QThread>
#include <QMutex>


////////////////////////////////////////////////////////////
class TestService : public QThread
{
    //Q_OBJECT;

public:
    TestService(Q2TimeShared& q2t, QObject* parent);
    ~TestService();

    void start();
    void stop();

protected:
    // resize from outer
    void run();
    void processRandom();

    inline qint64 currentMSecs() const
    { return QDateTime::currentMSecsSinceEpoch(); }

private:
    Q2TimeShared& q2t_;
    QMutex sync_;
    bool  shutdown_;
    bool  stopped_;
    qint64 intencityChangePeriod_;
    qint64 growChangePeriod_;
    qreal  intencity_;
    qreal  growStep_;
};

#endif // __eventline_h__
