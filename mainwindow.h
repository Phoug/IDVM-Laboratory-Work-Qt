#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QKeyEvent>
#include <QMap>
#include <QPixmap>

#include "define.h"
#include "lab1window.h"
#include "lab2window.h"
#include "lab4window.h"
#include "lab5window.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QLabel* getCharacterLabel() { return characterLabel; }

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void updateAnimation();

private:
    enum CharacterState { Idle, Walking, Jumping, Escaping, OpeningLab, Portal, Explosion, Exploding };
    enum Direction { None, Back, Front, Left, Right };
    Direction currentDirection = None;
    bool isWalking = false;
    int frameIndex = 0;
    int idleCounter = 0;

    QLabel *characterLabel;
    QTimer *animTimer;
    QMap<QString, QPixmap> sprites;

    QLabel *frameLab[NUMBERS_OF_LAB_FRAMES];
    QRect frameRects[NUMBERS_OF_LAB_FRAMES];
    QLabel *buttonELabel;
    QLabel* skeletonLabel;

    Lab1Window *lab1Window = nullptr;
    Lab2Window *lab2Window = nullptr;
    Lab4Window *lab4Window = nullptr;
    Lab5Window *lab5Window = nullptr;

    CharacterState charState = Idle;

    int jumpStep = 0;
    int escapeStep = 0;
    int escapeFrameIndex = 0;
    int escapeFrameInterval = 2;

    // функции для анимации
    QString getCalmSprite() const;
    QString getAsleepySprite() const;
    QString getSleepySprite() const;
    void moveCharacter();
    void updateButtonE();
    void loadSpritesFromFolder(const QString &folder, const QStringList &keys);

    template<typename T>
    void openLabWindow(T *&labWindow);
};

#endif // MAINWINDOW_H
