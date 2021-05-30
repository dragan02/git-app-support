/**
 * @file mainwindow.h
 * @brief Main window declarations
 *
 * Header file with needed declarations for main window
 *
 * @date 2021
 * @author Dragan Bozinovic (bozinovicdragan96@gmail.com)
 *
 * @version [1.0 @ 04/2021] Initial version
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <semaphore.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * Main window class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     *
     * Constructor does set up for main window, as well as
     * proper adjustment to button and LED widgets
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor
     *
     * Destructor unlinks shared memory
     */
    ~MainWindow();

private slots:
    /**
     * @brief Changing GPIO pin value
     *
     * Function changes GPIO pin value depending on
     * which button is pressed
     */
    void pinValue();

    void i2cValue(int val);

    void mmsValue(int val);

protected:
    /**
     * @brief GPIO shared memory mapping
     *
     * Mapping GPIO shared memory data to private data
     */
    void linkGPIOData();

    /**
     * @brief I2C shared memory mapping
     *
     * Mapping I2C shared memory data to private data
     */
    void linkI2CData();

    /**
     * @brief MMS shared memory mapping
     *
     * Mapping MMS shared memory data to private data
     */
    void linkMMSData();

    /**
     * @brief GPIO semaphore initialization
     *
     * Initialization of named semapore needed for GPIO synchronization
     */
    void initializeSemaphore();

private:
    Ui::MainWindow *ui;         /**< Pointer to parent widget */
    unsigned int *data_gpio;    /**< Pointer to shared memory containing GPIO data */
    unsigned int *data_i2c;     /**< Pointer to shared memory containing I2C data */
    unsigned int *data_mms;     /**< Pointer to shared memory containing MMS data */
    sem_t *sem;                 /**< Pointer to named semaphore */

};
#endif // MAINWINDOW_H
