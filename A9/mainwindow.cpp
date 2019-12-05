#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore>
#include <QDebug>
#include <QPixmap>
#include <QMovie>


MainWindow::MainWindow(QWidget *parent, GameManager *_gameEngine)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    gameEngine = _gameEngine;
    ui->setupUi(this);

    // Make a seed for random number generater.
    QDateTime cd = QDateTime::currentDateTime();
    qsrand(cd.toTime_t());

    // Init Code editor
    codeEditor = new CodeEditor(this);
    ui->codeEditlLayout->addWidget(codeEditor);

    // Init Code completer.
    completer = new QCompleter(this);
    completer->setModel(modelFromFile(":/command.txt"));
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    codeEditor->setCompleter(completer);

    // Init debugging
    ui->debugRightButton->setEnabled(false);
    ui->debugStopButton->setEnabled(false);

    // Connect all things for gameEngine
    QObject::connect(gameEngine, SIGNAL(movePlayer(int,int,bool,bool)), this, SLOT(movePlayer(int,int,bool,bool)));
    QObject::connect(gameEngine, SIGNAL(updateLevelCount(int)), this, SLOT(updateLevelCount(int)));
    QObject::connect(gameEngine, SIGNAL(resetSignal()), this, SLOT(resetBoard()));
    QObject::connect(this, SIGNAL(signalGameOver()), gameEngine, SLOT(checkLevelCompletionReset()));
    QObject::connect(gameEngine, SIGNAL(useKeySignal()), this, SLOT(usedKey()));
    QObject::connect(gameEngine, SIGNAL(useWeaponSignal()), this, SLOT(usedWeapon()));
    QObject::connect(gameEngine, SIGNAL(updateInventory(int, bool)), this, SLOT(updateInventory(int, bool)));
    QObject::connect(gameEngine, SIGNAL(deadSignal(int, int)), this, SLOT(onPlayerDead(int, int)));
    QObject::connect(gameEngine, SIGNAL(tutorial(int)), this, SLOT(tutorial(int)));
    QObject::connect(gameEngine, SIGNAL(spellCastSignal(int)), this, SLOT(onPlayerCastSpell(int)));
    QObject::connect(gameEngine, SIGNAL(toggleEnemyState(int)), this, SLOT(setEnemyState(int)));
    QObject::connect(gameEngine, SIGNAL(turnPlayer(int)), this, SLOT(turnPlayer(int)));

    tutorial(1);


    // Init code manager
    codeManager = new CodeManager(gameEngine);
    connect(codeManager, SIGNAL(signalLineChanged(int)), this, SLOT(onDebugLineChanged(int)));
    connect(codeManager, SIGNAL(signalException(const QString)), this, SLOT(onDebugException(const QString)));
    connect(codeManager, SIGNAL(signalFinish()), this, SLOT(onRunningFinsih()));

    //set icon
    ui->debugButton->setIcon(QIcon (QPixmap (":/debug.png")));             //debugger
    ui->debugButton->setIconSize(QSize(33,33));
    ui->debugButton->setStyleSheet("background-color: rgba(255, 255, 255, 20);");
    ui->debugRightButton->setIcon(QIcon (QPixmap (":/debugRight.png")));             //debugRight
    ui->debugRightButton->setIconSize(QSize(33,33));
    ui->debugRightButton->setStyleSheet("background-color: rgba(255, 255, 255, 20);");
    ui->debugStopButton->setIcon(QIcon (QPixmap (":/debugStop.png")));             //debugRight
    ui->debugStopButton->setIconSize(QSize(40,40));
    ui->debugStopButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    ui->debugStopButton->setIconSize(QSize(33,33));
    ui->debugStopButton->setStyleSheet("background-color: rgba(255, 255, 255, 20);");

    //set console
    ui->console->setTextInteractionFlags(Qt::TextInteractionFlag::NoTextInteraction);

    //Hide map section.
    ui->mapSection->setStyleSheet("QWidget {border-style: none;}");

    //Init Physics Engine
    b2Vec2 gravity = b2Vec2(0.0f, 0.0f);
    world = new b2World(gravity);
    world->SetAllowSleeping(true);
    world->SetContinuousPhysics(true);

    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0, 0);

    b2EdgeShape groundEdge;
    b2FixtureDef boxShapeDef;
    boxShapeDef.shape = &groundEdge;

    physicsTimer = new QTimer(this);
    connect(physicsTimer, SIGNAL(timeout()), this, SLOT(onPhysicsUpdate()));
    physicsTimer->setInterval(1);

    ui->doorLabel->setVisible(false);
    ui->goldKeyLabel->setVisible(false);
    ui->enemyLabel->setVisible(false);

    //ready
    resetBoard();
    ui->swordLabel->setVisible(false);

    QMovie *play = new QMovie(":/player_down_idle.gif");
    QMovie *play_top = new QMovie(":/player_down_idle_top.gif");
    play->start();
    play_top->start();
    ui->playerLabel->setAttribute(Qt::WA_NoSystemBackground);
    ui->playerLabel->setMovie(play);
    ui->playerLabel->setScaledContents(true);
    ui->playerTopLabel->setAttribute(Qt::WA_NoSystemBackground);
    ui->playerTopLabel->setMovie(play_top);
    ui->playerTopLabel->setScaledContents(true);
}

MainWindow::~MainWindow()
{
    delete world;
    delete physicsTimer;
    delete completer;
    delete codeManager;
    delete codeEditor;
    delete ui;
    delete music;
    delete playlist;
}

void MainWindow::movePlayer(int _x, int _y, bool mainCommand, bool _gameOver) {
    if(mainCommand) {
        if(gameOver){
            return;
        }
        else if(_gameOver){
            gameOver = _gameOver;
        }
        xTargets.push(_x);
        yTargets.push(_y);

        if(xTargets.size()>1) {
            return;
        }

        targetX = xTargets.front();
        targetY = yTargets.front();

        int xOff = xTargets.front()-((ui->playerLabel->x()-ui->playField->x())/ui->playerLabel->width());
        int yOff = yTargets.front()-((ui->playerLabel->y()+ui->playerLabel->height()/2-ui->playField->y())/ui->playerLabel->width());

        xStep = 0;
        yStep = 0;

        if(xOff != 0) {
            xStep = xOff/std::abs(xOff);
            xStep = 2 * xStep;
        }
        if(yOff != 0) {
            yStep = yOff/std::abs(yOff);
            yStep = 2 * yStep;
        }
    }

    // Move actual player sprite
    int x = ui->playerLabel->x() + xStep;
    int y = ui->playerLabel->y() + yStep;
    ui->playerLabel->setGeometry(x, y, ui->playerLabel->width(), ui->playerLabel->height());
    ui->playerTopLabel->setGeometry(x, y, ui->playerTopLabel->width(), ui->playerTopLabel->height());
    ui->shadowLabel->setGeometry(x, y, ui->shadowLabel->width(), ui->shadowLabel->height());

    // Update player variable labels
    updateCoordinateLabels();

    // If the player is not in the right spot yet...
    if(ui->playerLabel->y()+ui->playerLabel->height()/2 != ui->playField->y()+ui->playerLabel->width()*targetY || ui->playerLabel->x() != ui->playField->x()+ui->playerLabel->width()*targetX) {
        QTimer::singleShot(5, this, SLOT(movePlayer()));
        return;
    }
    // Else move on to next target position
    else {
        idlePlayer();
        QTimer::singleShot(100, codeManager, SLOT(onAnimationFinished()));

        if(ui->debugStopButton->isEnabled())
            ui->debugRightButton->setEnabled(true);

        xTargets.pop();
        yTargets.pop();

        if(gameOver)
        {
            emit signalGameOver();
            QTimer::singleShot(0, codeManager, SLOT(onInterrupted()));
        }
    }
}

void MainWindow::turnPlayer(int direction) {
    playerFacing = direction;
    QMovie *play;
    QMovie *play_top;
    switch (direction) {
        case 0: {
            play = new QMovie(":/player_right.gif");
            play_top = new QMovie(":/player_right_top.gif");
            break;
        }
        case 1: {
            play = new QMovie(":/player_up.gif");
            play_top = new QMovie(":/player_up_top.gif");
            break;
        }
        case 2: {
            play = new QMovie(":/player_left.gif");
            play_top = new QMovie(":/player_left_top.gif");
            break;
        }
        case 3: {
            play = new QMovie(":/player_down.gif");
            play_top = new QMovie(":/player_down_top.gif");
            break;
        }
    }
    play->start();
    play_top->start();
    ui->playerLabel->setMovie(play);
    ui->playerTopLabel->setMovie(play_top);
}

void MainWindow::idlePlayer() {
    QMovie *idle;
    QMovie *idle_top;
    switch (playerFacing) {
        case 0: {
            idle = new QMovie(":/player_right_idle.gif");
            idle_top = new QMovie(":/player_right_idle_top.gif");
            break;
        }
        case 1: {
            idle = new QMovie(":/player_up_idle.gif");
            idle_top = new QMovie(":/player_up_idle_top.gif");
            break;
        }
        case 2: {
            idle = new QMovie(":/player_left_idle.gif");
            idle_top = new QMovie(":/player_left_idle_top.gif");
            break;
        }
        case 3: {
            idle = new QMovie(":/player_down_idle.gif");
            idle_top = new QMovie(":/player_down_idle_top.gif");
            break;
        }
    }
    idle->start();
    idle_top->start();
    ui->playerLabel->setMovie(idle);
    ui->playerTopLabel->setMovie(idle_top);
}

void MainWindow::updateLevelCount(int level)
{
    QString levelString = "Level: ";
    levelString.append(QString::number(level));
    ui->levelLabel->setText(levelString);

    tutorial(level);
}


void MainWindow::usedKey()
{
    ui->doorLabel->setVisible(false);
}

void MainWindow::usedWeapon()
{
    ui->enemyLabel->setVisible(false);
    addBloodParticles(ui->enemyLabel->x()+ ui->enemyLabel->width()/2, ui->enemyLabel->y()+ ui->enemyLabel->height()/2, 100);
}

void MainWindow::updateInventory(int pickup, bool status)
{
    QString item;
    switch(pickup)
    {
        case 0:
            item = "Has Key: ";
            item.append(QVariant(status).toString());
            ui->keyLabel->setText(item);
            ui->goldKeyLabel->setVisible(false);
            break;
        case 1:
            item = "Has Weapon: ";
            item.append(QVariant(status).toString());
            ui->weaponLabel->setText(item);
            ui->swordLabel->setVisible(false);
            break;
        default:
            break;
    }
}

void MainWindow::setEnemyState(int state) {
    if(state==1) {
        QPixmap pixmap = QPixmap(":/enemy_awake.png");
        ui->enemyLabel->setPixmap(pixmap);
    }
    else {
        QPixmap pixmap = QPixmap(":/enemy_sleep.png");
        ui->enemyLabel->setPixmap(pixmap);
    }
}

void MainWindow::updateCoordinateLabels(){
    QString xString = "x: ";
    xString.append(QString::number((ui->playerLabel->x()-ui->playField->x())/ui->playerLabel->width()));
    QString yString = "y: ";
    yString.append(QString::number((ui->playerLabel->y()+ui->playerLabel->height()/2-ui->playField->y())/ui->playerLabel->width()));
    ui->xLabel->setText(xString);
    ui->yLabel->setText(yString);
}


void MainWindow::resetBoard() {
    gameEngine->loadLevel(gameEngine->getLevelCount());
    gameEngine->emitGameOverSignals();

    ui->playerLabel->setVisible(true);
    ui->playerTopLabel->setVisible(true);
    ui->shadowLabel->setVisible(true);
    gameOver = false;
    targetX = 0;
    targetY = 0;
    xStep = 0;
    yStep = 0;
    gameEngine->resetPlayer();
    ui->goButton->setEnabled(true);
    ui->playerLabel->setGeometry(ui->playField->x()+gameEngine->getPlayerX()*ui->playerLabel->width(), ui->playField->y()-ui->playerLabel->height()/2+gameEngine->getPlayerY()*ui->playerLabel->width(), ui->playerLabel->width(), ui->playerLabel->height());
    ui->playerTopLabel->setGeometry(ui->playerLabel->x(), ui->playerLabel->y(), ui->playerTopLabel->width(), ui->playerTopLabel->height());
    ui->shadowLabel->setGeometry(ui->playerLabel->x(), ui->playerLabel->y(), ui->shadowLabel->width(), ui->shadowLabel->height());
    int numTargets = xTargets.size();

    for(int i = 0; i < numTargets; i++) {
        xTargets.pop();
        yTargets.pop();
    }

    targetX = 0;
    targetY = 0;
    xStep = 0;
    yStep = 0;
    updateCoordinateLabels();

    ui->doorLabel->setVisible(false);
    ui->goldKeyLabel->setVisible(false);
    ui->enemyLabel->setVisible(false);
    ui->swordLabel->setVisible(false);

    if(std::get<0>(gameEngine->getDoorCoords()) != -1) {
        int x1 = ui->playField->x() + std::get<0>(gameEngine->getDoorCoords()) * ui->doorLabel->width();
        int y1 = ui->playField->y() + std::get<1>(gameEngine->getDoorCoords()) * ui->doorLabel->width() - ui->doorLabel->height()/3;
        ui->doorLabel->setGeometry(x1,y1, ui->doorLabel->width(), ui->doorLabel->height());
        ui->doorLabel->setVisible(true);
        int x2 = ui->playField->x() + std::get<0>(gameEngine->getKeyCoords()) * ui->goldKeyLabel->width();
        int y2 = ui->playField->y() + std::get<1>(gameEngine->getKeyCoords()) * ui->goldKeyLabel->width() - ui->goldKeyLabel->height()/3;
        ui->goldKeyLabel->setGeometry(x2,y2, ui->goldKeyLabel->width(), ui->goldKeyLabel->height());
        ui->goldKeyLabel->setVisible(true);
    }

    if(std::get<0>(gameEngine->getEnemyCoords()) != -1) {
        int x1 = ui->playField->x() + std::get<0>(gameEngine->getEnemyCoords()) * ui->enemyLabel->width();
        int y1 = ui->playField->y() + std::get<1>(gameEngine->getEnemyCoords()) * ui->enemyLabel->width() - ui->enemyLabel->height()/3;
        ui->enemyLabel->setGeometry(x1,y1, ui->enemyLabel->width(), ui->enemyLabel->height());
        ui->enemyLabel->setVisible(true);
        int x2 = ui->playField->x() + std::get<0>(gameEngine->getSwordCoords()) * ui->swordLabel->width();
        int y2 = ui->playField->y() + std::get<1>(gameEngine->getSwordCoords()) * ui->swordLabel->width() - ui->swordLabel->height()/3;
        ui->swordLabel->setGeometry(x2,y2, ui->swordLabel->width(), ui->swordLabel->height());
        ui->swordLabel->setVisible(true);
    }

    int x1 = ui->playField->x() + std::get<0>(gameEngine->getEnd()) * ui->finishLabel->width();
    int y1 = ui->playField->y() + std::get<1>(gameEngine->getEnd()) * ui->finishLabel->width() - 8;
    ui->finishLabel->setGeometry(x1,y1, ui->finishLabel->width(), ui->finishLabel->height());

    int x2 = ui->playField->x() + std::get<0>(gameEngine->getStart()) * ui->startLabel->width();
    int y2 = ui->playField->y() + std::get<1>(gameEngine->getStart()) * ui->startLabel->width() - 8;
    ui->startLabel->setGeometry(x2,y2, ui->startLabel->width(), ui->startLabel->height());

    QPixmap pixmap = QPixmap(":/level_" + QString::number(gameEngine->getLevelCount()) + ".png");
    ui->level1Label->setPixmap(pixmap);
    setEnemyState(1);
    playerFacing = 3;
    idlePlayer();
}

void MainWindow::on_goButton_clicked()
{
    resetBoard();

    ui->goButton->setEnabled(false);
    ui->debugStopButton->setEnabled(false);
    ui->debugButton->setEnabled(false);
    ui->debugRightButton->setEnabled(false);

    this->codeEditor->setTextInteractionFlags(Qt::TextInteractionFlag::NoTextInteraction);

    codeManager->run(codeEditor->toPlainText(), 1000);
}

void MainWindow::on_debugButton_clicked()
{
    resetBoard();

    ui->goButton->setEnabled(false);
    ui->debugRightButton->setEnabled(true);
    ui->debugButton->setEnabled(false);
    ui->debugStopButton->setEnabled(true);

    this->codeEditor->setTextInteractionFlags(Qt::TextInteractionFlag::NoTextInteraction);

    codeManager->debug(codeEditor->toPlainText());
}

void MainWindow::on_debugRightButton_clicked()
{
    codeManager->moveNextLine();
    ui->debugRightButton->setEnabled(false);
}

void MainWindow::on_debugStopButton_clicked()
{
    ui->debugStopButton->setEnabled(false);
    ui->debugRightButton->setEnabled(false);
    ui->debugButton->setEnabled(true);

    this->codeEditor->setTextInteractionFlags(Qt::TextInteractionFlag::TextEditorInteraction);

    //Finish the game.
    emit signalGameOver();
    QTimer::singleShot(0, codeManager, SLOT(onInterrupted()));
}

void MainWindow::onDebugLineChanged(int currentLine)
{
    qDebug() << "[Main] [onDebugLineChanged] Line : " << currentLine;

    codeEditor->lineHighlighter(currentLine);
}

void MainWindow::onDebugException(const QString errorMessage)
{
    qDebug() << "[Main] [onDebugException] " << errorMessage;

    ui->console->append(errorMessage);

    this->codeEditor->setTextInteractionFlags(Qt::TextInteractionFlag::TextEditorInteraction);

    ui->goButton->setEnabled(true);
    ui->debugButton->setEnabled(true);
    ui->debugRightButton->setEnabled(false);
    ui->debugStopButton->setEnabled(false);
}

void MainWindow::onRunningFinsih()
{
    qDebug() << "[Main] [onRunningFinish] Finish Debugging";

    this->codeEditor->setTextInteractionFlags(Qt::TextInteractionFlag::TextEditorInteraction);

    ui->goButton->setEnabled(true);
    ui->debugButton->setEnabled(true);
    ui->debugRightButton->setEnabled(false);
    ui->debugStopButton->setEnabled(false);
}

void MainWindow::onPhysicsUpdate()
{
    int velocityIterations = 8;
    int positionIterations = 3;

    // Simiulate
    world->Step(1, velocityIterations, positionIterations);

    // Update all objects.
    for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
    {
        int x = b->GetPosition().x;
        int y = b->GetPosition().y;

        int mapWidth = ui->mapSection->geometry().width();
        int mapHeight = ui->mapSection->geometry().height();

        if (b->GetUserData() != nullptr) {
            QLabel* spriteData = (QLabel *)b->GetUserData();
            spriteData->raise();

            //If it goes out of map, delete.
            if(x < -1 || y < -1 || mapWidth < x || mapHeight < y)
            {
                 world->DestroyBody(b);
                 delete spriteData;
            }
            else
            {
                spriteData->setGeometry(x, y, spriteData->width(), spriteData->height());
            }
        }
    }

    if(world->GetBodyCount() == 0)
        physicsTimer->stop();
}

void MainWindow::onPlayerDead(int deadPosX, int deadPosY)
{
    ui->goButton->setEnabled(true);
    int posPlayerX = ui->playerLabel->x();
    int posPlayerY = ui->playerLabel->y();

    int posX = posPlayerX + (ui->playerLabel->width() / 2);
    int posY = posPlayerY + (ui->playerLabel->height() / 2);

    qDebug() << "[Main] [onPlayerDead] x :" << posX << " / y : " << posY;

    ui->playerLabel->setVisible(false);
    ui->playerTopLabel->setVisible(false);
    ui->shadowLabel->setVisible(false);

    addBloodParticles(posX, posY, 100);
}

void MainWindow::onPlayerCastSpell(int spellCastPhase)
{
    //int posX = posBookX + (ui->bookLabel->width() / 2);
    //int posy = posBookY + (ui->bookLabel->height() / 2);

    //need to add book position for label
    //addGoldParticles(posX, posY, 20);

}

void MainWindow::addBloodParticles(int deadPosX, int deadPosY, int amount)
{
    qDebug() << "[Main] [addBloodParticles] x :" << deadPosX << " / y : " << deadPosY;

    while(amount-- > 0)
    {
        QLabel* qSprite = new QLabel(this);
        qSprite->setGeometry(deadPosX, deadPosY, 16, 16);

        // Pick random blood particle
        int randomBlood = qrand()%5 + 1;
        QPixmap pixmap = QPixmap(":/blood_" + QString::number(randomBlood) + ".png");
        pixmap = pixmap.scaled(qSprite->width(), qSprite->height(), Qt::KeepAspectRatio);
        qSprite->setPixmap(pixmap);
        qSprite->raise();
        qSprite->show();

        // Set body position
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(deadPosX, deadPosY);
        bodyDef.userData = qSprite;
        b2Body* body = world->CreateBody(&bodyDef);

        int vX = qrand()%100 + 5;
        int vY = qrand()%100 + 5;
        if(qrand()%2 == 0) {
            vX = -vX;
        }
        if(qrand()%2 == 0) {
            vY = -vY;
        }

        body->SetLinearVelocity(b2Vec2(vX, vY));

        b2CircleShape circle;
        circle.m_radius = 0.55f;

        // Set fixture for object
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &circle;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.2f;
        fixtureDef.restitution = 0.9f;
        body->CreateFixture(&fixtureDef);
    }
    physicsTimer->start();
}

void MainWindow::addGoldParticles(int bookPosX, int bookPosY, int amount)
{

}


QAbstractItemModel *MainWindow::modelFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << line.trimmed();
    }

#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif
    return new QStringListModel(words, completer);
}

void MainWindow::tutorial(int level) {

    if(currentLevel == level)
        return;
    else
        currentLevel = level;

    QString text;
    switch (level) {

    //left, right, up and down
    case 1:
        text.append("//Level 1 : Use only moveRight, moveLeft, moveUp, moveDown to complete\n\n");
        text.append("//the player can move up\n");
        text.append("player.moveUp()\n\n");
        text.append("//the player can move up\n");
        text.append("player.moveRight()\n\n");
        text.append("//the player can move down\n");
        text.append("player.moveDown()\n\n");
        text.append("//the player can move right\n");
        text.append("player.moveRight()\n\n");

        break;

    //
    case 2:
        text.append("//Level 2 : Try to experiment with parameters. e.g. player.moveUp(1) \n\n");
        text.append("//the player can move up\n");
        text.append("player.moveUp()\n\n");
        text.append("//the player can move up\n");
        text.append("player.moveRight()\n\n");
        text.append("//the player can move down\n");
        text.append("player.moveDown()\n\n");
        text.append("//the player can move right\n");
        text.append("player.moveRight()\n\n");

        break;

    case 3:
        text.append("// Some methods have return types, one is called a bool which can either return a true or false value. \n");
        text.append("// Loops like this while-loop can be used to continuously do stuff until variables change. \n");
        text.append("// Try using the checkGuardIsAwake() method together with the while-loop. \n");

        text.append("// Hint: use the player.wait() method \n\n");
        text.append("while (    ) { \n");
        text.append("\n");
        text.append("}");
        break;
    }

    codeEditor->setPlainText(text);
}
