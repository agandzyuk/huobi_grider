#ifndef __logger_h__
#define __logger_h__

#include "external.h"

#include <QTextStream>
#include <QFile>
#include <QDebug>

/////////////////////////////////////////////////////////////////
class CLogFile: public QFile
{
public:
    CLogFile();
    ~CLogFile();

protected:
    bool checkOpened();
    qint64 writeData(const char *data, qint64 len) Q_DECL_OVERRIDE;
};

/////////////////////////////////////////////////////////////////
class CDebug: public QTextStream
{
public:
    CDebug(bool enableTimestamp = true);
};

#endif // __logger_h__
