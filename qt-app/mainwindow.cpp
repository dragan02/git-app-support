/**
 * @file mainwindow.cpp
 * @brief Main window functionalities
 *
 * File which defines behavior of main window
 *
 * @date 2021
 * @author Dragan Bozinovic (bozinovicdragan96@gmail.com)
 *
 * @version [1.0 @ 04/2021] Initial version
 */

/* Default includes */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>

/* Includes needed for debugging */
#include <QString>
#include <QDebug>

/* Needed for custom LED widget */
#include <LED.h>

/* Aditional includes from C library, needed for shared memory */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // Needed for ftruncate
#include <fcntl.h>      // Defines O_* constants
#include <sys/stat.h>   // Defines mode constants
#include <sys/mman.h>   // Defines mmap flags

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    /* Build the widget tree on the parent widget */
    ui->setupUi(this);

    /* Setting window title */
    this->setWindowTitle("User interface");

    /* When pushButton is clicked, its' public slot function is called */
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(pinValue()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(pinValue()));
    connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(pinValue()));
    connect(ui->pushButton_4, SIGNAL(clicked()), this, SLOT(pinValue()));

    connect(ui->slider_1, SIGNAL(valueChanged(int)), this, SLOT(i2cValue(int)));
    connect(ui->slider_2, SIGNAL(valueChanged(int)), this, SLOT(mmsValue(int)));

    /* Setting LED gpio pins */
    ui->led->setGpioPin(4);
    ui->led_2->setGpioPin(5);
    ui->led_3->setGpioPin(6);
    ui->led_4->setGpioPin(7);

    /* Map shared memory segment to appropriate variable */
    linkGPIOData();
    linkI2CData();
    linkMMSData();

    /* Initialize named semaphore */
    initializeSemaphore();

}

MainWindow::~MainWindow()
{
    /* Delete parent widget */
    delete ui;

    /* Unlink shared memory */
    if (shm_unlink("gpio") == -1){
        qDebug() << "Unlinking sh. mem. file descriptor failed!\n";
    }

    if (shm_unlink("i2c") == -1){
        qDebug() << "Unlinking sh. mem. file descriptor failed!\n";
    }

    if (shm_unlink("mmsens") == -1){
        qDebug() << "Unlinking sh. mem. file descriptor failed!\n";
    }

    /* Close semaphore */
    if (sem_close(sem) == -1){
        qDebug() << "Closing named semaphore failed!\n";
    }

    /* Unlink semapore */
    if (sem_unlink("/gpio") == -1){
        qDebug() << "Unlinking named semaphore failed!\n";
    }


}

void MainWindow::linkGPIOData()
{
    /* File descriptor and auxiliary variable for storing size of shared memory */
    int fd, segSize;
    segSize = sizeof(quint32);

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
    data_gpio = (unsigned int*)mmap(NULL, segSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data_gpio == MAP_FAILED) {
        qDebug() << "Memory mapping failed!\n";
    }
}

void MainWindow::linkI2CData()
{
    /* File descriptor and auxiliary variable for storing size of shared memory */
    int fd, segSize;
    segSize = sizeof(quint32);

    /* Create new shared memory object */
    fd = shm_open("i2c", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1){
        qDebug() << "Function shm_open failed!\n";
    }

    /* Set sh. mem. segment size */
    if (ftruncate(fd, segSize) == -1){
        qDebug() << "Truncating shared memory failed!\n";
    }

    /* Map shared memory object to process virtual address space */
    data_i2c = (unsigned int*)mmap(NULL, segSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data_i2c == MAP_FAILED) {
        qDebug() << "I2C memory mapping failed!\n";
    }
}

void MainWindow::linkMMSData()
{
    /* File descriptor and auxiliary variable for storing size of shared memory */
    int fd, segSize;
    segSize = sizeof(quint32);

    /* Create new shared memory object */
    fd = shm_open("mmsens", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1){
        qDebug() << "Function shm_open failed!\n";
    }

    /* Set sh. mem. segment size */
    if (ftruncate(fd, segSize) == -1){
        qDebug() << "Truncating shared memory failed!\n";
    }

    /* Map shared memory object to process virtual address space */
    data_mms = (unsigned int*)mmap(NULL, segSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data_mms == MAP_FAILED) {
        qDebug() << "MM sens memory mapping failed!\n";
    }
}

void MainWindow::initializeSemaphore()
{
    sem = sem_open("/gpio", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
    if (sem == SEM_FAILED){
        qDebug() << "Creating named semaphore failed!\n";
    }

}

void MainWindow::pinValue()
{
    /* Determine which button was clicked */
    QObject* button = QObject::sender();
    /* Auxiliary variables needed for changing GPIO pin value */
    unsigned int mask, bit;

    /* Determine mask value */
    if (button == ui->pushButton)
    {
        mask = 1 << 0;
    }
    else if (button == ui->pushButton_2)
    {
        mask = 1 << 1;
    }
    else if (button == ui->pushButton_3)
    {
        mask = 1 << 2;
    }
    else
    {
        mask = 1 << 3;
    }

    /* Get data bit value and toggle it */
    bit = *data_gpio & mask;

    if (bit){
        *data_gpio -= mask;
    }
    else{
        *data_gpio += mask;
    }

    /* Increment semaphore value */
    if (sem_post(sem) == -1){
        qDebug() << "Failed to increment named sem. value!\n";
    }

}

void MainWindow::i2cValue(int val){
    /* String which will be printed if debugging */
    QString outString;

    *data_i2c = val;
}

void MainWindow::mmsValue(int val){
    /* String which will be printed if debugging */
    QString outString;

    *data_mms = val;
}

