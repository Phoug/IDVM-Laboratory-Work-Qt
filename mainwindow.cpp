#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentDirection(None)
    , isWalking(false)
    , frameIndex(0)
    , idleCounter(0)
{
    setFixedSize(1280, 720);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    central->setStyleSheet(
        "background-image: url(:/images/other/main_background.svg);"
        "background-repeat: no-repeat;"
        );

    characterLabel = new QLabel(central);
    characterLabel->setGeometry(640, 180, 60, 82);
    characterLabel->setScaledContents(true);
    characterLabel->setStyleSheet("background: transparent;");
    characterLabel->show();

    // Загружаем спрайты (обычные)
    sprites["back_calm"]   = QPixmap(":/morty/simple/back_calm.png");
    sprites["back_walk1"]  = QPixmap(":/morty/simple/back_walk1.png");
    sprites["back_walk2"]  = QPixmap(":/morty/simple/back_walk2.png");
    sprites["back_walk3"]  = QPixmap(":/morty/simple/back_walk3.png");

    sprites["front_calm"]  = QPixmap(":/morty/simple/front_calm.png");
    sprites["front_walk1"] = QPixmap(":/morty/simple/front_walk1.png");
    sprites["front_walk2"] = QPixmap(":/morty/simple/front_walk2.png");
    sprites["front_walk3"] = QPixmap(":/morty/simple/front_walk3.png");

    sprites["left_calm"]   = QPixmap(":/morty/simple/left_calm.png");
    sprites["left_walk1"]  = QPixmap(":/morty/simple/left_walk1.png");
    sprites["left_walk2"]  = QPixmap(":/morty/simple/left_walk2.png");
    sprites["left_walk3"]  = QPixmap(":/morty/simple/left_walk3.png");

    sprites["right_calm"]  = QPixmap(":/morty/simple/right_calm.png");
    sprites["right_walk1"] = QPixmap(":/morty/simple/right_walk1.png");
    sprites["right_walk2"] = QPixmap(":/morty/simple/right_walk2.png");
    sprites["right_walk3"] = QPixmap(":/morty/simple/right_walk3.png");

    // Idle (моргает и засыпает)
    sprites["front_asleepy"] = QPixmap(":/morty/asleepy/front_аsleepy_calm.png");
    sprites["back_asleepy"]  = QPixmap(":/morty/asleepy/back_аsleepy_calm.png");
    sprites["left_asleepy"]  = QPixmap(":/morty/asleepy/left_аsleepy_calm.png");
    sprites["right_asleepy"] = QPixmap(":/morty/asleepy/right_аsleepy_calm.png");

    sprites["front_sleepy"] = QPixmap(":/morty/sleepy/front_sleepy_calm.png");
    sprites["back_sleepy"]  = QPixmap(":/morty/sleepy/back_sleepy_calm.png");
    sprites["left_sleepy"]  = QPixmap(":/morty/sleepy/left_sleepy_calm.png");
    sprites["right_sleepy"] = QPixmap(":/morty/sleepy/right_sleepy_calm.png");

    characterLabel->setPixmap(sprites["front_calm"]);

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &MainWindow::updateAnimation);
    animTimer->start(50);

    qApp->installEventFilter(this);
}

MainWindow::~MainWindow() {}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        keyPressEvent(static_cast<QKeyEvent*>(event));
        return true;
    }
    if (event->type() == QEvent::KeyRelease) {
        keyReleaseEvent(static_cast<QKeyEvent*>(event));
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_W: currentDirection = Back;  isWalking = true; break;
    case Qt::Key_S: currentDirection = Front; isWalking = true; break;
    case Qt::Key_A: currentDirection = Left;  isWalking = true; break;
    case Qt::Key_D: currentDirection = Right; isWalking = true; break;
    }
    idleCounter = 0; // сбрасываем бездействие
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_S:
    case Qt::Key_A:
    case Qt::Key_D:
        isWalking = false;
        break;
    default:
        QMainWindow::keyReleaseEvent(event);
    }
}

void MainWindow::updateAnimation()
{
    QString key;

    if (isWalking) {
        idleCounter = 0; // сбрасываем счётчик
        frameIndex = (frameIndex + 1) % 3;
        switch (currentDirection) {
        case Back:  key = QString("back_walk%1").arg(frameIndex+1); break;
        case Front: key = QString("front_walk%1").arg(frameIndex+1); break;
        case Left:  key = QString("left_walk%1").arg(frameIndex+1); break;
        case Right: key = QString("right_walk%1").arg(frameIndex+1); break;
        default: break;
        }
    } else {
        idleCounter++;

        // каждые ~20 кадров = 1 сек при таймере 50 мс
        if (idleCounter < 30) {
            // первые 2 секунды calm
            switch (currentDirection) {
            case Back:  key = "back_calm"; break;
            case Front: key = "front_calm"; break;
            case Left:  key = "left_calm"; break;
            case Right: key = "right_calm"; break;
            default:    key = "front_calm"; break;
            }
        } else if (idleCounter < 80) {
            // 2–4 сек → моргаем sleepy
            switch (currentDirection) {
            case Back:  key = (idleCounter % 40 < 20) ? "back_calm" : "back_asleepy"; break;
            case Front: key = (idleCounter % 40 < 20) ? "front_calm" : "front_asleepy"; break;
            case Left:  key = (idleCounter % 40 < 20) ? "left_calm" : "left_asleepy"; break;
            case Right: key = (idleCounter % 40 < 20) ? "right_calm" : "right_asleepy"; break;
            default:    key = "front_calm"; break;
            }
        } else {
            // больше 4 сек → asleepy (засыпает)
            switch (currentDirection) {
            case Back:  key = "back_sleepy"; break;
            case Front: key = "front_sleepy"; break;
            case Left:  key = "left_sleepy"; break;
            case Right: key = "right_sleepy"; break;
            default:    key = "front_sleepy"; break;
            }
        }
    }

    if (!key.isEmpty()) {
        characterLabel->setPixmap(sprites[key]);

        // Движение
        QPoint pos = characterLabel->pos();
        int speed = 7;
        if (isWalking) {
            if (currentDirection == Back)  pos.setY(pos.y() - speed);
            if (currentDirection == Front) pos.setY(pos.y() + speed);
            if (currentDirection == Left)  pos.setX(pos.x() - speed);
            if (currentDirection == Right) pos.setX(pos.x() + speed);

            QRect bounds(0, 0, width() - characterLabel->width(), height() - characterLabel->height());
            pos.setX(std::clamp(pos.x(), bounds.left(), bounds.right()));
            pos.setY(std::clamp(pos.y(), bounds.top(), bounds.bottom()));

            characterLabel->move(pos);
        }
    }
}
