#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , codeEditor(new CodeEditor(this))
    , codeManager(new CodeManager())
{
    ui->setupUi(this);
    ui->codeEditlLayout->addWidget(codeEditor);

    ui->debugLeftButton->setEnabled(false);
    ui->debugRightButton->setEnabled(false);


    //set icon
    ui->debugButton->setIcon(QIcon (QPixmap (":/debug.png")));             //debugger
    ui->debugButton->setIconSize(QSize(33,33));
    ui->debugButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    ui->debugLeftButton->setIcon(QIcon (QPixmap (":/debugLeft.png")));             //debugLeft
    ui->debugLeftButton->setIconSize(QSize(33,33));
    ui->debugLeftButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    ui->debugRightButton->setIcon(QIcon (QPixmap (":/debugRight.png")));             //debugRight
    ui->debugRightButton->setIconSize(QSize(33,33));
    ui->debugRightButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    ui->debugStopButton->setIcon(QIcon (QPixmap (":/debugStop.png")));             //debugRight
    ui->debugStopButton->setIconSize(QSize(40,40));
    ui->debugStopButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");


}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::movePlayerUp(int increment)
{
    int x = ui->playerLabel->x();
    int y = ui->playerLabel->y() - ui->playerLabel->height() * increment;
    ui->playerLabel->setGeometry(x, y, ui->playerLabel->width(), ui->playerLabel->height());
}

void MainWindow::movePlayerDown(int increment)
{
    int x = ui->playerLabel->x();
    int y = ui->playerLabel->y() + ui->playerLabel->height() * increment;
    ui->playerLabel->setGeometry(x, y, ui->playerLabel->width(), ui->playerLabel->height());
}

void MainWindow::movePlayerLeft(int increment)
{
    int x = ui->playerLabel->x() - ui->playerLabel->width() * increment;
    int y = ui->playerLabel->y();
    ui->playerLabel->setGeometry(x, y, ui->playerLabel->width(), ui->playerLabel->height());
}

void MainWindow::movePlayerRight(int increment)
{
    int x = ui->playerLabel->x() + ui->playerLabel->width() * increment;
    int y = ui->playerLabel->y();
    ui->playerLabel->setGeometry(x, y, ui->playerLabel->width(), ui->playerLabel->height());
}

void MainWindow::on_goButton_clicked()
{


    codeManager->run(codeEditor->toPlainText());
//    QString input = ui->inputTextBox->toPlainText();
//    if(input[4] == 'U') {
//            int increment = input.mid(7, input.length()-9).toInt();
//            movePlayerUp(increment);
//    }
//    else if(input[4] == 'D') {
//        int increment = input.mid(9, input.length()-11).toInt();
//        movePlayerDown(increment);
//    }
//    else if(input[4] == 'L') {
//        int increment = input.mid(9, input.length()-11).toInt();
//        movePlayerLeft(increment);
//    }
//    else if(input[4] == 'R') {
//        int increment = input.mid(10, input.length()-12).toInt();
//        movePlayerRight(increment);
//    }
//    ui->inputTextBox->clear();

//    QString xString = "x: ";
//    xString.append(QString::number((ui->playerLabel->x()-ui->playField->x())/64));
//    QString yString = "y: ";
//    yString.append(QString::number((ui->playerLabel->y()-ui->playField->y())/64));
//    ui->xLabel->setText(xString);
//    ui->yLabel->setText(yString);
}

void MainWindow::on_debugButton_clicked()
{
    ui->debugLeftButton->setEnabled(true);
    ui->debugRightButton->setEnabled(true);
    ui->debugButton->setEnabled(false);

}

void MainWindow::on_debugLeftButton_clicked()
{

}

void MainWindow::on_debugRightButton_clicked()
{

}

void MainWindow::on_debugStopButton_clicked()
{
    ui->debugLeftButton->setEnabled(false);
    ui->debugRightButton->setEnabled(false);
    ui->debugButton->setEnabled(true);
    ui->debugStopButton->setFocus();

}
