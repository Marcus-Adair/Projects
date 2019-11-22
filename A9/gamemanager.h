#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "gamemap.h"
#include "player.h"
#include <QObject>

class GameManager : public QObject
{
    Q_OBJECT

    Player player;
    GameMap level;

public:
    GameManager();

    void loadLevel(int levelNum);
    bool isLevelCompleted();

    Q_INVOKABLE void moveUp(int spaces = 0);
    Q_INVOKABLE void moveDown(int spaces = 0);
    Q_INVOKABLE void moveLeft(int spaces = 0);
    Q_INVOKABLE void moveRight(int spaces = 0);

    // takes in a list of coordinates that a move would propose, sets players coordinates to where the player should be
    // at, like if they run into a wall for example
    void checkPathSetActualSpot(std::vector<std::tuple<int, int>> tryingPath);


public slots:


signals:
    void movePlayer(int x, int y, bool gameOver);
};

#endif // GAMEMANAGER_H
