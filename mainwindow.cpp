#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <algorithm>
#include <QMovie>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setFixedSize(1280, 720);
    setWindowTitle("Основное окно");
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    central->setStyleSheet("background-image: url(:/images/other/main_background.svg); background-repeat: no-repeat;");

    characterLabel = new QLabel(central);
    characterLabel->setGeometry(610, 220, 60, 82);
    characterLabel->setScaledContents(true);
    characterLabel->setStyleSheet("background: transparent;");
    characterLabel->show();

    loadSpritesFromFolder("simple", {"back_calm","back_walk1","back_walk2","back_walk3",
                                     "front_calm","front_walk1","front_walk2","front_walk3",
                                     "left_calm","left_walk1","left_walk2","left_walk3",
                                     "right_calm","right_walk1","right_walk2","right_walk3"});
    loadSpritesFromFolder("asleepy", {"front_asleepy_calm","back_asleepy_calm","left_asleepy_calm","right_asleepy_calm"});
    loadSpritesFromFolder("sleepy",  {"front_sleepy_calm","back_sleepy_calm","left_sleepy_calm","right_sleepy_calm"});


    characterLabel->setPixmap(sprites["front_sleepy"]);

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &MainWindow::updateAnimation);
    animTimer->start(35);

    QStringList frames = {"frame_lab1","frame_lab2","frame_lab3","frame_lab4","frame_lab5","frame_lab6"};
    QList<QRect> geometries = {QRect(5,5,250,180), QRect(5,190,250,180), QRect(5,380,250,180),
                               QRect(1020,5,250,180), QRect(1020,190,250,180), QRect(1020,380,250,180)};
    for (int i = 0; i < frames.size(); ++i) {
        frameLab[i] = new QLabel(central);
        frameLab[i]->setPixmap(QPixmap(":/images/other/" + frames[i] + ".svg"));
        frameLab[i]->setGeometry(geometries[i]);
        frameLab[i]->setScaledContents(true);
        frameLab[i]->setStyleSheet("background: transparent");
        frameRects[i] = frameLab[i]->geometry();
    }

    buttonELabel = new QLabel(central);
    buttonELabel->setPixmap(QPixmap(":/images/other/button_e.svg"));
    buttonELabel->setScaledContents(true);
    buttonELabel->setGeometry(590, 605, 100, 100);
    buttonELabel->hide();

    characterLabel->raise();
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
    case Qt::Key_E:
        for (int i=0; i < NUMBERS_OF_LAB_FRAMES; ++i) {
            if (characterLabel->geometry().intersects(frameRects[i])) {
                if (i == 0) openLabWindow(frameRects[i], lab1Window);
                if (i == 1)
                {
                    if (charState == Idle || charState == Walking) {
                        charState = Jumping;
                        jumpStep = 0;
                    }
                }
                if (i == 3)
                {
                    charState = Portal;
                }
            }
        }
        break;
    }
    idleCounter = 0;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_W: case Qt::Key_S: case Qt::Key_A: case Qt::Key_D:
        isWalking = false;
        break;
    default: QMainWindow::keyReleaseEvent(event);
    }
}

void MainWindow::updateAnimation()
{
    if (charState == Jumping) {
        currentDirection = Front;
        QPoint pos = characterLabel->pos();
        if (jumpStep < 10) pos.setY(pos.y() - 6);
        else if (jumpStep < 20) pos.setY(pos.y() + 6);
        characterLabel->move(pos);

        jumpStep++;
        if (jumpStep >= 20) {
            charState = Escaping;
            escapeStep = 0;
        }
    }
    else if (charState == Escaping) {
        QPoint pos = characterLabel->pos();
        pos.setX(pos.x() - 8);
        characterLabel->move(pos);

        if (escapeStep % escapeFrameInterval == 0) {
            escapeFrameIndex = (escapeFrameIndex + 1) % 3;
            QString key = QString("left_walk%1").arg(escapeFrameIndex + 1);
            if (sprites.contains(key))
                characterLabel->setPixmap(sprites[key]);
        }

        escapeStep++;
        if (escapeStep > 40) {
            charState = OpeningLab;
            openLabWindow(frameRects[1], lab2Window);
        }
    }
    else if (charState == Portal) {
        characterLabel->hide();

        QLabel *gifLabel = new QLabel(this);
        gifLabel->setScaledContents(true);
        gifLabel->setGeometry(characterLabel->x() - 45, characterLabel->y() - 45, 150, 150);
        gifLabel->setAttribute(Qt::WA_TranslucentBackground);
        gifLabel->setStyleSheet("background: transparent;");

        // Загружаем кадры портала один раз
        static QList<QPixmap> portalFrames;
        if (portalFrames.isEmpty()) {
            for (int i = 1; i <= 11; ++i) {
                portalFrames.append(QPixmap(QString(":/morty/portal/portal%1.png").arg(i)));
            }
        }

        int totalFrames = portalFrames.size();
        int frameDurationMs = 3000 / totalFrames;

        // Устанавливаем первый кадр сразу
        gifLabel->setPixmap(portalFrames[0]);
        gifLabel->show();

        // Создаём таймер для анимации
        QTimer *portalTimer = new QTimer(this);
        int *currentFrameIndex = new int(1);  // Начинаем с 1, так как 0 уже установлен

        connect(portalTimer, &QTimer::timeout, this, [gifLabel, portalTimer, currentFrameIndex]() mutable {
            if (*currentFrameIndex >= portalFrames.size()) {
                portalTimer->stop();
                gifLabel->deleteLater();
                delete currentFrameIndex;

                // Открываем окно после анимации
                MainWindow *mainWin = qobject_cast<MainWindow*>(gifLabel->parent());
                if (mainWin) {
                    mainWin->openLabWindow(mainWin->frameRects[3], mainWin->lab4Window);
                }
                return;
            }

            gifLabel->setPixmap(portalFrames[*currentFrameIndex]);
            ++(*currentFrameIndex);
        });

        portalTimer->start(frameDurationMs);

        // Устанавливаем состояние в Idle сразу, чтобы предотвратить повторный вход
        charState = Idle;
    }

    else {
        QString key;
        if (isWalking && charState != Jumping) {
            idleCounter = 0;
            frameIndex = (frameIndex + 1) % 3;
            switch (currentDirection) {
            case Back:  key = QString("back_walk%1").arg(frameIndex+1); break;
            case Front: key = QString("front_walk%1").arg(frameIndex+1); break;
            case Left:  key = QString("left_walk%1").arg(frameIndex+1); break;
            case Right: key = QString("right_walk%1").arg(frameIndex+1); break;
            default: key = "front_calm"; break;
            }
        } else {
            idleCounter++;
            if (idleCounter < 45) key = getCalmSprite();
            else if (idleCounter < 120) key = getAsleepySprite();
            else key = getSleepySprite();
        }

        if (!key.isEmpty() && sprites.contains(key))
            characterLabel->setPixmap(sprites[key]);

        moveCharacter();
        updateButtonE();
    }
}


QString MainWindow::getCalmSprite() const
{
    switch (currentDirection) {
    case Back: return "back_calm";
    case Front: return "front_calm";
    case Left: return "left_calm";
    case Right: return "right_calm";
    default: return "front_calm";
    }
}

QString MainWindow::getAsleepySprite() const
{
    bool blink = (idleCounter % 40 >= 20);
    switch (currentDirection) {
    case Back:  return blink ? "back_asleepy_calm"  : "back_calm";
    case Front: return blink ? "front_asleepy_calm" : "front_calm";
    case Left:  return blink ? "left_asleepy_calm"  : "left_calm";
    case Right: return blink ? "right_asleepy_calm" : "right_calm";
    default:    return blink ? "front_asleepy_calm" : "front_calm"; // fallback
    }
}

QString MainWindow::getSleepySprite() const
{
    switch (currentDirection) {
    case Back:  return "back_sleepy_calm";
    case Front: return "front_sleepy_calm";
    case Left:  return "left_sleepy_calm";
    case Right: return "right_sleepy_calm";
    default:    return "front_sleepy_calm"; // fallback
    }
}


void MainWindow::moveCharacter()
{
    if (!isWalking) return;

    int speed = 7;
    QPoint pos = characterLabel->pos();
    switch (currentDirection) {
    case Back:  pos.setY(pos.y()-speed); break;
    case Front: pos.setY(pos.y()+speed); break;
    case Left:  pos.setX(pos.x()-speed); break;
    case Right: pos.setX(pos.x()+speed); break;
    default: break;
    }
    QRect bounds(0, 0, width()-characterLabel->width(), height()-characterLabel->height());
    pos.setX(std::clamp(pos.x(), bounds.left(), bounds.right()));
    pos.setY(std::clamp(pos.y(), bounds.top(), bounds.bottom()));
    characterLabel->move(pos);
}

void MainWindow::updateButtonE()
{
    QRect charRect = characterLabel->geometry();
    bool insideFrame = false;
    for (int i=0; i<NUMBERS_OF_LAB_FRAMES; ++i) {
        if (charRect.intersects(frameRects[i])) { insideFrame = true; break; }
    }
    buttonELabel->setVisible(insideFrame);
}

void MainWindow::loadSpritesFromFolder(const QString &folder, const QStringList &keys)
{
    for (const auto &k : keys)
        sprites[k] = QPixmap(QString(":/morty/%1/%2.png").arg(folder, k));
}

template<typename T>
void MainWindow::openLabWindow(const QRect &frameRect, T *&labWindow)
{
    if (!labWindow) {
        labWindow = new T(nullptr);

        if constexpr (std::is_base_of<Lab4Window, T>::value) {
            connect(labWindow, &T::hideRequested, this, [this, &labWindow]() {
                this->hide();
                if (labWindow) labWindow->hide();
            });
        }

        if constexpr (std::is_base_of<Lab4Window, T>::value) {
            connect(labWindow, &T::showRequested, this, [this, &labWindow]() {
                labWindow->hide();
                this->show();
                if (labWindow) labWindow->show();
            });
        }

        connect(labWindow, &T::windowClosed, this, [this, &labWindow]() {
            characterLabel->show();
            characterLabel->setGeometry(610, 220, 60, 82);
            charState = Idle;
            if (labWindow) { labWindow->deleteLater(); labWindow = nullptr; }
        });
    }
    characterLabel->hide();
    labWindow->show();
    labWindow->raise();
    labWindow->activateWindow();
}
