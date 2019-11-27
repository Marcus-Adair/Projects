#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <queue>

#include "codeeditor.h"
#include "codemanager.h"
#include "Box2D/Box2D.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    int targetX = 0;
    int targetY = 0;
    int xStep = 0;
    int yStep = 0;
    std::queue<int> xTargets;
    std::queue<int> yTargets;
    bool gameOver = false;

public:
    MainWindow(QWidget *parent = nullptr, GameManager *gameEngine = nullptr);
    ~MainWindow();

private slots:
    void on_goButton_clicked();

    void on_debugButton_clicked();

    void on_debugRightButton_clicked();

    void on_debugStopButton_clicked();

    void movePlayer(int x = 0, int y = 0, bool mainCommand = false, bool gameOver = false);

    //CodeManager
    void onDebugLineChanged(int currentLine);
    void onDebugException(const QString errorMessage);
    void onRunningFinsih();

    //Physics Engine
    void onPhysicsUpdate();
    void onPlayerDead(int deadPosX, int deadPosY);


private:
    Ui::MainWindow *ui;

    CodeEditor *codeEditor;
    CodeManager *codeManager;
    GameManager *gameEngine;
    QCompleter *completer;

    b2World* world;
    QTimer* physicsTimer;

    void addBloodParticles(int deadPosX, int deadPosY, int amount);
    int generateRandomNumber(int low, int high);

    QAbstractItemModel *modelFromFile(const QString& fileName);
};
#endif // MAINWINDOW_H
