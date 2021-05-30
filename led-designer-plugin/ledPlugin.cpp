#include "LED.h"
#include "LEDPlugin.h"

#include <QtPlugin>

LEDPlugin::LEDPlugin(QObject* parent) :
	QObject(parent),
	initialized(false)
{
}


bool LEDPlugin::isContainer() const
{
    return false;
}

QIcon LEDPlugin::icon() const
{
    return QIcon();
}

QString LEDPlugin::group() const
{
    return tr("LED Custom Widget");
}

QString LEDPlugin::includeFile() const
{
    return "LED.h";
}

QString LEDPlugin::name() const
{
	return "LED";
}

QString LEDPlugin::toolTip() const
{
	return tr("An LED");
}

QString LEDPlugin::whatsThis() const
{
	return tr("An LED");
}

QWidget * LEDPlugin::createWidget(QWidget *parent)
{
	return new LED(parent);
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_EXPORT_PLUGIN2(ledplugin, LEDPlugin)
#endif
