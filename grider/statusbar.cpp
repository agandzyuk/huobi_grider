#include "globals.h"
#include "statusbar.h"
#include "requesthandler.h"
#include "sslclient.h"
#include "maindialog.h"
#include "eventline.h"
#include "quotestablemodel.h"

#include <QtWidgets>

////////////////////////////////////////////////////////////////////////////////////
StatusBar::StatusBar(QWidget* parent)
    : QWidget(parent),
    state_(InitialState),
    hasError_(false)
{
    QHBoxLayout* hlayout = new QHBoxLayout();
    setLayout(hlayout);
    
    hlayout->addWidget(statusText_ = new QTextEdit(this), 0, Qt::AlignLeft);
    //hlayout->setContentsMargins(0,0,0,0);
    statusText_->setFont(*Global::nativeBold);
    statusText_->setFixedWidth(parent->width() - 20);
    statusText_->setText(tr("Ready"));
    statusText_->setReadOnly(true);
    statusText_->setFrameStyle(QFrame::NoFrame);

    hlayout->addWidget(abortButton_ = new QPushButton(tr("Abort")), 0, Qt::AlignRight);
    QObject::connect(abortButton_, SIGNAL(clicked()), parent, SLOT(asynñStop()));
    abortButton_->setText(tr("Abort"));

    btnZoomOut_ = new QPushButton(tr("-"),this);
    QObject::connect(btnZoomOut_, SIGNAL(clicked()), this, SLOT(onZoomOut()));
    btnZoomOut_->setFixedSize(20,20);
    btnZoomOut_->setAutoRepeat(true);
    hlayout->addWidget(btnZoomOut_, 10, Qt::AlignRight);

    sldZoom_ = new QSlider(Qt::Horizontal, this);
    QObject::connect(sldZoom_, SIGNAL(valueChanged(int)), this, SLOT(onZoomSlider(int)));
    sldZoom_->setFixedWidth(50);
    sldZoom_->setRange(-28,30);
    sldZoom_->setSliderPosition(0);
    hlayout->addWidget(sldZoom_,1,Qt::AlignRight);

    btnZoomIn_ = new QPushButton(tr("+"),this);
    QObject::connect(btnZoomIn_, SIGNAL(clicked()), this, SLOT(onZoomIn()));
    btnZoomIn_->setFixedSize(20,20);
    btnZoomIn_->setAutoRepeat(true);
    hlayout->addWidget(btnZoomIn_,0,Qt::AlignRight);

    adjustColors();
    adjustButton();
}

void StatusBar::connecting(const QString& host)
{
    hasError_ = false;
    host_   = host;
    statusText_->setText(tr("Connecting to ") + host_ + ":443");

    state_ = ProgressState;
    adjustColors();
    adjustButton();
}

void StatusBar::reconnecting()
{
    hasError_ = false;
    statusText_->setText(tr("Reconnecting to ") + host_ + ":443");

    state_ = ProgressState;
    adjustColors();
    adjustButton();
}

void StatusBar::established(const QString& warning)
{
    hasError_ = false;
    if( warning.isEmpty() ) {
        statusText_->setText(tr("Established"));
        state_ = EstablishState;
    }
    else {
        statusText_->setText(tr("Warning: ") + warning);
        state_ = EstablishWarnState;
    }

    adjustColors();
    adjustButton();
}

void StatusBar::disconnected(const QString& reason)
{
    if( reason.isEmpty() ) {
        statusText_->setText("Connection closed");
        state_ = InitialState;
        adjustColors();
    }
    else {
        state_ = ClosedFailureState;
        if( !hasError_ ) {
            statusText_->setText(reason);
            adjustColors();
        }
        hasError_ = true;
    }
    adjustButton();
}

void StatusBar::loggedOut(const QString& reason)
{
    if( reason.isEmpty() ) {
        statusText_->setText("Stopped");
        state_ = InitialState;
        adjustColors();
    }
    else {
        state_ = ForcedClosingState;
        if( !hasError_ ) {
            statusText_->setText(reason);
            adjustColors();
        }
        hasError_ = true;
    }
    adjustButton();
}

void StatusBar::adjustColors()
{
    QPalette pal;
    if( state_ == ClosedFailureState )
        pal.setColor(QPalette::Text, TEXT_RED);
    else if( state_ == EstablishState )
        pal.setColor(QPalette::Text, TEXT_GREEN);
    else if( state_ == EstablishWarnState )
        pal.setColor(QPalette::Text, TEXT_YELLOW);
    else if( state_ == ProgressState )
        pal.setColor(QPalette::Text, TEXT_LIME);
    else if( state_ == ForcedClosingState )
        pal.setColor(QPalette::Text, TEXT_BROWN);
    else
        pal.setColor(QPalette::Text, QColor(0,0,0));

    pal.setColor(QPalette::Base, palette().color(QPalette::Background));
    statusText_->setPalette(pal);
}

void StatusBar::adjustButton()
{
    if( state_ == ProgressState ) {
        abortButton_->show();
        btnZoomIn_->hide();
        sldZoom_->hide();
        btnZoomOut_->hide();
    }
    else  {
        abortButton_->hide();
        btnZoomIn_->show();
        sldZoom_->show();
        btnZoomOut_->show();
    }
}

void StatusBar::onZoomIn()
{
    int newVal = sldZoom_->value()+1;
    if( newVal <= sldZoom_->maximum() )
        sldZoom_->setValue(newVal);
}

void StatusBar::onZoomOut()
{
    int newVal = sldZoom_->value()-1;
    if( newVal >= sldZoom_->minimum() )
        sldZoom_->setValue(newVal);
}

void StatusBar::onZoomSlider(int value)
{
    btnZoomIn_->setEnabled(value < sldZoom_->maximum());
    btnZoomOut_->setEnabled(value > sldZoom_->minimum());

    value =- value;

    qreal r = 1.0;
    if( value <= 0) {
        r = 0.018*(31+value);
    }
    else {
        while(--value)
            r *= 1.312;
    }

    qobject_cast<MainDialog*>(parentWidget())->indicator_->rescale(r);
}
