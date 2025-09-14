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

    Lab1Window *lab1Window = nullptr;
    Lab2Window *lab2Window = nullptr;

    // --- функции для анимации ---
    QString getCalmSprite() const;
    QString getAsleepySprite() const;
    QString getSleepySprite() const;
    void moveCharacter();
    void updateButtonE();
    void loadSpritesFromFolder(const QString &folder, const QStringList &keys);

    template<typename T>
    void openLabWindow(const QRect &frameRect, T *&labWindow);
};

#endif // MAINWINDOW_H
