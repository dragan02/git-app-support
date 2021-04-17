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
    this->setWindowTitle("GPIO");

    /* When pushButton is clicked, its' public slot function is called */
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(pinValue()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(pinValue()));
    connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(pinValue()));
    connect(ui->pushButton_4, SIGNAL(clicked()), this, SLOT(pinValue()));

    /* Setting LED gpio pins */
    /*ui->led->setGpioPin(0);
    ui->led_2->setGpioPin(1);
    ui->led_3->setGpioPin(2);
    ui->led_4->setGpioPin(3);*/
    ui->led->setGpioPin(4);
    ui->led_2->setGpioPin(5);
    ui->led_3->setGpioPin(6);
    ui->led_4->setGpioPin(7);

    /* Map shared memory segment to appropriate variable */
    mapToSharedMemory();

}

MainWindow::~MainWindow()
{
    /* Delete parent widget */
    delete ui;

    /* Unlink shared memory */
    if (shm_unlink("test1") == -1){
        qDebug() << "Unlinking sh. mem. file descriptor failed!\n";
    }
}

void MainWindow::mapToSharedMemory()
{
    /* File descriptor and auxiliary variable for storing size of shared memory */
    int fd, segSize;
    segSize = sizeof(quint32);

    /* Create new shared memory object */
    fd = shm_open("test1", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1){
        qDebug() << "Function shm_open failed!\n";
    }

    /* Set sh. mem. segment size */
    if (ftruncate(fd, segSize) == -1){
        qDebug() << "Truncating shared memory failed!\n";
    }

    /* Map shared memory object to process virtual address space */
    data = (unsigned int*)mmap(NULL, segSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        qDebug() << "Memory mapping failed!\n";
    }
}

void MainWindow::pinValue()
{
    /* Determine which button was clicked */
    QObject* button = QObject::sender();
    /* Auxiliary variables needed for changing GPIO pin value */
    unsigned int mask, bit;

    /* String which will be printed if debugging */
    QString outString;

    //qDebug() << "Before:" << *data;

    /* Determine mask value */
    if (button == ui->pushButton)
    {
        outString = "LED1";
        mask = 1 << 0;
    }
    else if (button == ui->pushButton_2)
    {
        outString = "LED2";
        mask = 1 << 1;
    }
    else if (button == ui->pushButton_3)
    {
        outString = "LED3";
        mask = 1 << 2;
    }
    else
    {
        outString = "LED4";
        mask = 1 << 3;
    }

    /* Get data bit value and toggle it */
    bit = *data & mask;

    if (bit){
        *data -= mask;
    }
    else{
        *data += mask;
    }

    //qDebug() << "After:" << *data;
}

