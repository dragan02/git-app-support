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

protected:
    /**
     * @brief Shared memory mapping
     *
     * Mapping shared memory data to private data
     */
    void mapToSharedMemory();

private:
    Ui::MainWindow *ui; /**< Pointer to parent widget */
    unsigned int *data; /**< Pointer to shared memory containing data */
};
#endif // MAINWINDOW_H
