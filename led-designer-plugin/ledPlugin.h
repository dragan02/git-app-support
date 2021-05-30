/**
 * @file LEDPlugin.h
 * @brief LEDPlugin declarations
 *
 * Header file with needed declarations for LEDPlugin with
 * custom widget properties
 *
 * @date 2021
 * @author Dragan Bozinovic (bozinovicdragan96@gmail.com)
 *
 * @version [1.0 @ 04/2021] Initial version
 */

#ifndef _LED_PLUGIN_H_
#define _LED_PLUGIN_H_

/*  The methods in LEDPlugin are implementations of pure
 *  virtual methods in QDesignerCustomWidgetInterface
 */
#include <QtUiPlugin/QDesignerCustomWidgetInterface>

/**
 * LEDPlugin class
 */
class LEDPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    Q_PLUGIN_METADATA(IID "customWidget")
#endif

public:
	LEDPlugin(QObject* parent=0);

    bool isContainer() const;
    QIcon icon() const;
    QString group() const;
    QString includeFile() const;
    QString name() const;
    QString toolTip() const;
    QString whatsThis() const;
    QWidget *createWidget(QWidget *parent);

private:
    bool initialized;
};

#endif
