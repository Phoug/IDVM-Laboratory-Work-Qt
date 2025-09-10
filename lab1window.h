#ifndef LAB1WINDOW_H
#define LAB1WINDOW_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

class Lab1Window : public QWidget
{
    Q_OBJECT
public:
    explicit Lab1Window(QWidget *parent = nullptr);

private slots:
    void updateBatteryInfo();

private:
    QLabel *powerTypeLabel;
    QLabel *batteryTypeLabel;
    QLabel *batteryLevelLabel;
    QLabel *powerSavingModeLabel;
    QLabel *batteryTimeLabel;

    QTimer *updateTimer;
};

#endif // LAB1WINDOW_H
