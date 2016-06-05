#include "globals.h"

#include <windows.h>
#include <string>
#include <algorithm>

#include <QtCore>

static bool enableTimestamp = true;
static bool firstStart = true;

//QMutex mdbg;

////////////////////////////////////////////////////////////
CDebug::CDebug(bool timestamp)
    : QTextStream( Global::infoLogFile )
{
    enableTimestamp = timestamp;

    // only once
    if( firstStart ) 
    {
        firstStart = false;
        enableTimestamp = false;
        std::string info = Global::productFullName().toStdString() + " - Session started at " + Global::timestamp();
        (*this) << info.c_str() << "\n";
    }
}

////////////////////////////////////////////////////////////
CLogFile::CLogFile()
    : QFile(FILENAME_DEBUGINFO)
{
    Global::truncateMbFromLog(FILENAME_DEBUGINFO, MAX_DEBUGINFO_FILESIZE);
}

CLogFile::~CLogFile()
{}

bool CLogFile::checkOpened()
{
    if( !isOpen() ) 
    {
        if( !open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text) ) {
            QString info = QString("Cannot open logging file %1\nLogging will be disabled!").arg(FILENAME_DEBUGINFO);
            ::MessageBoxA(NULL, info.toStdString().c_str(), "Error", MB_OK|MB_ICONSTOP);
            return false;
        }
    }
    return true;
}

qint64 CLogFile::writeData(const char *data, qint64 len)
{
    if( !checkOpened() )
        return len;

    std::string str(data,len);
#ifndef NDEBUG
//    QMutexLocker lock(&mdbg);
    qDebug() << str.c_str();
#endif

//    std::string thr = QString("%1").arg(GetCurrentThreadId()).toStdString();

    if( enableTimestamp )
        str = "[" + Global::timestamp() + /*"]:" + thr  + "  -  "*/ "]  -  " + str + "\n";
    else if( 0 == memcmp(data,"HUOBI",4) )
        str += "\n";
    else if( 0 == memcmp(data,"<< ",3) || 0 == memcmp(data,"<< ",3) )
        str = "                         " + str + "\n";
    else
        str = "                            " + str + "\n";
    return QFile::writeData(str.c_str(),str.length());
}
