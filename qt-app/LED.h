#ifndef _LED_H_
#define _LED_H_

#include <QtDesigner/QtDesigner>
#include <QWidget>

class QTimer;

class QDESIGNER_WIDGET_EXPORT LED : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(double diameter READ diameter WRITE setDiameter) // mm
	Q_PROPERTY(QColor color READ color WRITE setColor)
	Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool state READ state WRITE setState)
    Q_PROPERTY(int gpioPin READ gpioPin WRITE setGpioPin)
    Q_PROPERTY(int refreshRate READ refreshRate WRITE setRefreshRate)

public:
	explicit LED(QWidget* parent=0);
	~LED();

	double diameter() const;
	void setDiameter(double diameter);

	QColor color() const;
	void setColor(const QColor& color);

	Qt::Alignment alignment() const;
	void setAlignment(Qt::Alignment alignment);

    bool state() const;

    int gpioPin() const;
    void setGpioPin(int gpioPin);

    int refreshRate() const;
    void setRefreshRate(int refreshRate);

public slots:
    void setState(bool state);
    void refreshGpio();

public:
	int heightForWidth(int width) const;
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

protected:
	void paintEvent(QPaintEvent* event);

    /* Aditional parameters related to GPIO */
    void connectToGpio();

private:
	double diameter_;
	QColor color_;
	Qt::Alignment alignment_;
	bool initialState_;
    bool state_;
    int gpioPin_; // pin number(0,1,2,3)
    int refreshRate_; // custom value
    unsigned int *data_; // shmem data

    /* Pixels per mm for x and y */
	int pixX_, pixY_;

    /* Scaled values for x and y diameter */
	int diamX_, diamY_;

	QRadialGradient gradient_;
	QTimer* timer_;
};

#endif
