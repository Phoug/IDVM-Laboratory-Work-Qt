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
    QLabel* getCharacterLabel() { return characterLabel; }
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

    Lab1Window *lab1Window = nullptr;
    Lab2Window *lab2Window = nullptr;

    template<typename T>
    void openLabWindow(QRect frameRect, T *&labWindow)
    {
        if (characterLabel->geometry().intersects(frameRect)) {
            if (!labWindow) {
                labWindow = new T(nullptr);
                connect(labWindow, &T::windowClosed, this, [this, &labWindow]() {
                    characterLabel->show();
                    if (labWindow) {
                        labWindow->deleteLater();
                        labWindow = nullptr;
                    }
                });
            }
            characterLabel->hide();
            labWindow->show();
            labWindow->raise();
            labWindow->activateWindow();
        }
    }
};



#endif // MAINWINDOW_H
