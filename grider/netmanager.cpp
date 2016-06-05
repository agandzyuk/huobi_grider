#include "globals.h"
#include "netmanager.h"

#include "quotestablemodel.h"
#include "maindialog.h"
#include "scheduler.h"
#include "sslclient.h"

#include <QMutex>
#include <QtWidgets>
#include <QSslSocket>

#include <vector>
#include <algorithm>

using namespace std;

////////////////////////////////////////////////////////
NetworkManager::NetworkManager(QWidget* parent) 
    : QObject(parent),
    stateLock_(new QMutex()),
    state_(Initial)
{
    if( !QSslSocket::supportsSsl() ) {
        QMessageBox::information(NULL, "SSL", "OpenSSL libraries not found!");
        throw std::exception();
    }

    model_.reset(new QuotesTableModel(parent));
    scheduler_.reset( new Scheduler(this) );

    QObject::connect( model(), SIGNAL(activateRequest(Instrument)), 
                      scheduler(), SLOT(activateRequest(Instrument)) );
    QObject::connect( model(), SIGNAL(unsubscribeImmediate(Instrument)),
                      this, SLOT(onHaveToUnSubscribe(Instrument)) );
    QObject::connect( model(), SIGNAL(activateResponse(Instrument)), 
                      scheduler(), SLOT(activateResponse(Instrument)) );
    QObject::connect( this, SIGNAL(notifyStateChanged(quint8, QString)), 
                      parent, SLOT(onStateChanged(quint8, QString)) );
    QObject::connect( model(), SIGNAL(notifyServerLogout(QString)), 
                      this, SLOT(onServerLogout(QString)) );
}

NetworkManager::~NetworkManager()
{
    if(model_->loggedIn())
        stop();

    { QMutexLocker g(stateLock_); }
    delete stateLock_;
    stateLock_ = NULL;
}

void NetworkManager::start()
{
    {
        QMutexLocker g(stateLock_);
        state_ = Initial;
    }

    QSsl::SslProtocol  ssnproto = QSsl::AnyProtocol;
    QString proto = model_->value(ProtocolParam);

    if( proto == ProtoSSLv2 )
        ssnproto = QSsl::SslV2; 
    else if( proto == ProtoSSLv3 )
        ssnproto = QSsl::SslV3; 
    else if( proto == ProtoTLSv1_0 )
        ssnproto = QSsl::TlsV1_0; 
    else if( proto == ProtoTLSv1_x )
        ssnproto = QSsl::TlsV1_0OrLater; 
    else if( proto == ProtoTLSv1_1 )
        ssnproto = QSsl::TlsV1_1; 
    else if( proto == ProtoTLSv1_1x )
        ssnproto = QSsl::TlsV1_1OrLater; 
    else if( proto == ProtoTLSv1_2 )
        ssnproto = QSsl::TlsV1_2; 
    else if( proto == ProtoTLSv1_2x )
        ssnproto = QSsl::TlsV1_2OrLater; 
    else if( proto == ProtoTLSv1_SSLv3 )
        ssnproto = QSsl::TlsV1SslV3; 

    connection_.reset(new SslClient(ssnproto, this));
    connection_->establish(model_->value(ServerParam), 443);

    QObject::connect( model(), SIGNAL(notifySendingManual(QByteArray)), 
                      this, SLOT(onHaveToSendMessage(QByteArray)) );
}

void NetworkManager::stop()
{
    {
        QMutexLocker g(stateLock_);
        state_ = ForcedClosingState;
    }

    onHaveToLogout();
    connection_.reset();
}

void NetworkManager::reconnect()
{
    scheduler_->activateSSLReconnect();
}

void NetworkManager::onStateChanged(ConnectionState state)
{
    QMutexLocker g(stateLock_);
    if( state == state_ )
        return;

    QString txtState;
    switch( state )
    {
        case Initial:
            txtState = "Initial"; break;
        case Establish:
            txtState = "Establish"; break;
        case EstablishWithWarning:
            txtState = "EstablishWithWarning"; break;
        case Connecting:
            txtState = "Connecting"; break;
        case EngineClosing:
            txtState = "EngineClosing"; break;
        case ForcedClosing:
            txtState = "ForcedClosing"; break;
        case DisconnectByRemote:
            txtState = "DisconnectByRemote"; break;
        case DisconnectByFailure:
            txtState = "DisconnectByFailure"; break;
        case Unconnect:
            txtState = "Unconnect"; break;
    }

    QString reason;
    if( state == DisconnectByFailure || state == DisconnectByRemote || state == ForcedClosing )
    {
        // restore fix flags 
        model_->setLoggedIn(false);
        model_->setTestRequestSent(false);
        reason = connection_->lastError();
    }

    CDebug() << "NetworkManager:onStateChanged " << txtState << 
            (reason.isEmpty() ? "" : (" - \"" + reason + "\""));

    if( state_ == ForcedClosing && state == Unconnect )
    {
        if( scheduler_->reconnectEnabled() ) {
            // update statusbar with normal disconnect state
            state_ = Unconnect;
            emit notifyStateChanged(ForcedClosing, "");
            return;
        }
        state_ = state;
    }
    else if( state == Connecting && state_ != Initial ) {
        state_ = Reconnecting;
    }
    else if( state == Connecting ) {
        state_ = Connecting;
    }
    else if( state == Establish || state == EstablishWithWarning ) {
        onHaveToLogin();
        state_ = state;
    }
    else {
        state_ = state;
    }

    emit notifyStateChanged(state_, reason);
}

void NetworkManager::onServerLogout(const QString& reason)
{
    MainDialog* md = qobject_cast<MainDialog*>(parent());
    if(md)
        emit md->onReconnectSetCheck(false);
    scheduler_->setReconnectEnabled(false);
    state_ = ForcedClosing;
    emit notifyStateChanged(ForcedClosing, reason);
}

RequestHandler::ConnectionState NetworkManager::getState() const
{
    QMutexLocker g(stateLock_);
    return state_;
}

void NetworkManager::onHaveToLogin()
{
    if( model_->loggedIn() )
        return;
    onHaveToSendMessage( model_->makeLogon() );
}

void NetworkManager::onHaveToLogout()
{
    model_->beforeLogout();

    if( !model_->loggedIn() )
        return;

    onHaveToSendMessage( model_->makeLogout() );
    model_->setLoggedIn(false);
}

void NetworkManager::onHaveToTestRequest()
{
    if( !model_->loggedIn() )
        return;

    int hbi = model_->getHeartbeatInterval() + 1000;

    int delta = Global::time() - model_->getLastIncoming();
    if( delta >= hbi )
    {
        if( !model_->testRequestSent() )
        {
            onHaveToSendMessage( model_->makeTestRequest() );
        }
        else
        {
            // next time it will work
            model_->setTestRequestSent(false);
        }
        delta = hbi;
    }
    else 
        delta = hbi - delta;

    scheduler_->activateTestrequest(delta);
}

void NetworkManager::onHaveToHeartbeat()
{
    if( !model_->loggedIn() )
        return;

    int hbi = model_->getHeartbeatInterval();

    int delta = Global::time() - model_->getLastOutgoing();
    if( delta >= hbi )
    {
        onHaveToSendMessage( model_->makeHeartBeat() );
        delta = hbi;
    }
    else
        delta = hbi-delta;

    scheduler_->activateHeartbeat(delta);
}

void NetworkManager::onHaveToSubscribe(const Instrument& inst)
{
    QByteArray message = model_->makeSubscribe(inst);
    if( !message.isNull() )
        onHaveToSendMessage(message);
    model()->activateResponse(Instrument("FullUpdate",-1));
}

void NetworkManager::onHaveToUnSubscribe(const Instrument& inst)
{
    QByteArray message = model_->makeUnSubscribe(inst);
    if( !message.isNull() )
        onHaveToSendMessage(message);
    model()->activateResponse(Instrument("FullUpdate",-1));
}

void NetworkManager::onMessageReceived(const QByteArray& message)
{
    int ret = model_->process(message);
    if( ret > 0 ) 
    {
        QByteArray message;
        do {
            message.swap( model_->takeOutgoing() );
            if( message.isEmpty() )
                break;
            onHaveToSendMessage(message);
            message.clear();
        }
        while( model_->loggedIn() );
    }
    else if( ret == 1 ) // Server Login
    {
        scheduler_->activateHeartbeat(model_->getHeartbeatInterval());
        scheduler_->activateTestrequest(model_->getHeartbeatInterval());
    }
    else if( ret == -1 ) // Server Logout
    {
        scheduler_->activateLogout();
    }
}

bool NetworkManager::onHaveToSendMessage(const QByteArray& message)
{
    if( message.isEmpty() )
        return false;

    string type = FIX::getField(message,"35");
    if(type.empty())
        return false;

    string info;
    string f262sym;
    qint32 f48code;
    switch( type[0] )
    {
    case '0': { 
            string f112 = FIX::getField(message,"112");
            info = f112.empty() ? ("skip") : ("Test Request Response type=\"0\" on TestReqID=\"" + f112 + "\" is sent");
        }
        break;
    case 'V': { 
            f262sym = FIX::getField(message,"262");
            f48code = model_->getCode( f262sym.c_str() );
            char buf[10] = {0};
            _ltoa_s(f48code, buf, 10);
            string insname = f262sym + ":" + string(buf);
            info = "Market Request type=\"V\" for \"" + insname + "\" is sent";
            model_->storeRequestSeqnum(message, f262sym.c_str() );
        }
        break;
    case 'A':
        info = "Logon type=\"A\" is sent";
        break;
    case '5':
        info = "Logout type=\"5\" is sent";
        break;
    case '1':
        info = "TestRequest type=\"1\" is sent";
        break;
    default:
        return false;
    }

    emit connection_->asyncSending(message);
    if( info != "skip" ) {
        CDebug() << QString::fromStdString(info);
        CDebug(false) << ">> " << message;
        if( type[0] == 'V')
            model()->activateResponse(Instrument("FullUpdate",-1));
    }

    return true;
}
