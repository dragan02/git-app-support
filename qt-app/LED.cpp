/**
 * @file LED.cpp
 * @brief LED functionalities
 *
 * File which defines behavior of LED
 *
 * @date 2021
 * @author Dragan Bozinovic (bozinovicdragan96@gmail.com)
 *
 * @version [1.0 @ 04/2021] Initial version
 */

#include <QPainter>
#include <QGradient>
#include <QPaintDevice>
#include <QTimer>
#include <QDebug>

#include <math.h>
#include <cstddef> // for NULL
#include "LED.h"

/* Aditional includes from C library, needed for shared memory */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* ftruncate */
#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <sys/mman.h> /* Defines mmap flags */

LED::LED(QWidget* parent) :
    QWidget(parent),
    diameter_(10),
    color_(QColor("green")),
    alignment_(Qt::AlignCenter),
    state_(true),
    gpioPin_(0),
    refreshRate_(100),
    data_(NULL)
{
    setDiameter(diameter_);
    setRefreshRate(refreshRate_);

    /* Map shared memory segment to appropriate variable */
    mapToSharedMemory();

    /* Instantiate timer, connect its' timeout signal to
     * refresh slot function and start it
     */
    timer_ = new QTimer(this);
    connect(timer_, SIGNAL(timeout()), this, SLOT(refreshGpio()));
    timer_->start(refreshRate_);
}

LED::~LED()
{
}


double LED::diameter() const
{
    return diameter_;
}

void LED::setDiameter(double diameter)
{
    diameter_ = diameter;

    pixX_ = round(double(height())/heightMM());
    pixY_ = round(double(width())/widthMM());

    diamX_ = diameter_*pixX_;
    diamY_ = diameter_*pixY_;

    update();
}


QColor LED::color() const
{
    return color_;
}

void LED::setColor(const QColor& color)
{
    color_ = color;
    update();
}

Qt::Alignment LED::alignment() const
{
    return alignment_;
}

void LED::setAlignment(Qt::Alignment alignment)
{
    alignment_ = alignment;

    update();
}


bool LED::state() const
{
    return state_;
}

int LED::gpioPin() const
{
    return gpioPin_;
}

void LED::setGpioPin(int gpioPin)
{
    gpioPin_ = gpioPin;
}

int LED::
refreshRate() const{
    return refreshRate_;
}

void LED::
setRefreshRate(int rate)
{
    refreshRate_ = rate;

    update();
}

void LED::
setState(bool state)
{
    state_ = state;
    update();
}

void LED::refreshGpio()
{
    /* Auxiliary variables */
    unsigned int mask, bit, pin;

    /* Determine mask value */
    pin = gpioPin();
    mask = 1 << pin;

    /* Get data bit value and toggle it */
    bit = *data_ & mask;

    if (bit){
        setState(true);
    }
    else{
        setState(false);
    }
}

int LED::heightForWidth(int width) const
{
    return width;
}

QSize LED::sizeHint() const
{
    return QSize(diamX_, diamY_);
}

QSize LED::minimumSizeHint() const
{
    return QSize(diamX_, diamY_);
}

void LED::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);

    QRect geo = geometry();
    int width = geo.width();
    int height = geo.height();

    int x=0, y=0;
    if ( alignment_ & Qt::AlignLeft )
        x = 0;
    else if ( alignment_ & Qt::AlignRight )
        x = width-diamX_;
    else if ( alignment_ & Qt::AlignHCenter )
        x = (width-diamX_)/2;
    else if ( alignment_ & Qt::AlignJustify )
        x = 0;

    if ( alignment_ & Qt::AlignTop )
        y = 0;
    else if ( alignment_ & Qt::AlignBottom )
        y = height-diamY_;
    else if ( alignment_ & Qt::AlignVCenter )
        y = (height-diamY_)/2;

    QRadialGradient g(x+diamX_/2, y+diamY_/2, diamX_*0.4,
        diamX_*0.4, diamY_*0.4);

    g.setColorAt(0, Qt::white);
    if ( state_ )
        g.setColorAt(1, color_);
    else
        g.setColorAt(1, Qt::black);
    QBrush brush(g);

    p.setPen(color_);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(brush);
    p.drawEllipse(x, y, diamX_-1, diamY_-1);

}

void LED::mapToSharedMemory()
{
    /* File descriptor and auxiliary variable for storing size of shared memory */
    int fd, segSize;
    segSize = sizeof(unsigned int);

    /* Create new shared memory object */
    fd = shm_open("gpio", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1){
        qDebug() << "Function shm_open failed!\n";
    }

    /* Set sh. mem. segment size */
    if (ftruncate(fd, segSize) == -1){
        qDebug() << "Truncating shared memory failed!\n";
    }

    /* Map shared memory object to process virtual address space */
    data_ = (unsigned int*)mmap(NULL, segSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data_ == MAP_FAILED) {
        qDebug() << "Memory mapping failed!\n";
    }

    update();
}
