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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override; // ловим глобальные события

private slots:
    void updateAnimation();

private:
    enum Direction { None, Back, Front, Left, Right };
    Direction currentDirection;
    bool isWalking;
    int frameIndex;
    int idleCounter;

    QLabel *characterLabel;
    QTimer *animTimer;
    QMap<QString, QPixmap> sprites;

    QLabel *frameLab1;
    QLabel *frameLab2;
    QLabel *frameLab3;
    QLabel *frameLab4;
    QLabel *frameLab5;
    QLabel *frameLab6;
    QRect frameRects[NUMBERS_OF_LAB_FRAMES];
    QLabel *buttonELabel;

    Lab1Window *lab1Window;
};

#endif // MAINWINDOW_H
