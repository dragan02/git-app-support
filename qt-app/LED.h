/**
 * @file LED.h
 * @brief LED declarations
 *
 * Header file with needed declarations for LED
 *
 * @date 2021
 * @author Dragan Bozinovic (bozinovicdragan96@gmail.com)
 *
 * @version [1.0 @ 04/2021] Initial version
 */

#ifndef _LED_H_
#define _LED_H_

#include <QtDesigner/QtDesigner>
#include <QWidget>

/* Needed for QTimer instance */
class QTimer;

/**
 * LED class
 */
class QDESIGNER_WIDGET_EXPORT LED : public QWidget
{
    /* Inherent properties of QObject */
    Q_OBJECT

    /**
     * @brief Diameter of LED widget
     * @accessors %diameter(), setDiameter()
     */
    Q_PROPERTY(double diameter READ diameter WRITE setDiameter) // mm

    /**
     * @brief Color of LED widget
     * @accessors %color(), setColor()
     */
    Q_PROPERTY(QColor color READ color WRITE setColor)

    /**
     * @brief Aligment of LED widget
     * @accessors %alignment(), setAlignment()
     */
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)

    /**
     * @brief State of LED widget
     * @accessors %state(), setState()
     */
    Q_PROPERTY(bool state READ state WRITE setState)

    /**
     * @brief GPIO pin
     * @accessors %gpioPin(), setGpioPin()
     */
    Q_PROPERTY(int gpioPin READ gpioPin WRITE setGpioPin)

    /**
     * @brief Refresh rate for reading GPIO pin
     * @accessors %refreshRate(), setRefreshRate()
     */
    Q_PROPERTY(int refreshRate READ refreshRate WRITE setRefreshRate)

public:
    /**
     * @brief Constructor
     *
     * Constructor initializes its' members, maps data to the
     * shared memory, starts timer and makes preparations for
     * refreshing read GPIO value
     */
    explicit LED(QWidget* parent=0);

    /**
     * @brief Destructor
     *
     * Destructor unlinks shared memory
     */
    ~LED();

    /** Method returning the diameter value */
    double diameter() const;
    /** Method which sets the LED widget diameter */
    void setDiameter(double diameter);

    /** Method returning the color value */
    QColor color() const;
    /** Method which sets the LED widget color */
    void setColor(const QColor& color);

    /** Method returning the aligment value */
    Qt::Alignment alignment() const;
    /** Method which sets the LED widget aligment */
    void setAlignment(Qt::Alignment alignment);

    /** Method returning the LED state */
    bool state() const;

    /** Method returning the GPIO pin value */
    int gpioPin() const;
    /** Method which sets the LED GPIO pin */
    void setGpioPin(int gpioPin);

    /** Method returning the refresh rate */
    int refreshRate() const;
    /** Method which sets the refresh rate */
    void setRefreshRate(int refreshRate);

public slots:
    /** Slot function user for reading LED state */
    void setState(bool state);

    /** Slot function used for reading the GPIO pin value */
    void refreshGpio();

public:
    /** Method which returns the preferred height for the widget given the width */
    int heightForWidth(int width) const;

    /** Method which gives a clue about how to size the widget */
    QSize sizeHint() const;

    /** Method which returns the recommended minimum size for your widget*/
    QSize minimumSizeHint() const;

protected:
    /** Method which paints LED when it's shown or updated.
     * It's called upon update()
     */
    void paintEvent(QPaintEvent* event);

    /** Mapping shared memory data to private data */
    void mapToSharedMemory();

private:
    double diameter_; /**< LED widget diameter */
    QColor color_; /**< LED widget color */
    Qt::Alignment alignment_; /**< LED widget aligment */
    bool state_; /**< LED state */
    int gpioPin_; /**< Appropriate GPIO pin value */
    int refreshRate_; /**< Refresh rate of reading from GPIO */
    unsigned int *data_; /**< Pointer to shared memory containing data */

    int pixX_, pixY_; /**< Pixels per mm for x and y */
    int diamX_, diamY_;  /**< Scaled values for x and y diameter */

    QRadialGradient gradient_; /**< LED widget gradient */
    QTimer* timer_; /**< Timer */
};

#endif
