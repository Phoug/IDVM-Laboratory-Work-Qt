#include "lab1window.h"
#include <QApplication>
#include <QDebug>
#include <QOperatingSystemVersion>
#include <QProcess>
#include <QSysInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

Lab1Window::Lab1Window(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Лабораторная 1: Энергопитание");
    resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    powerTypeLabel = new QLabel(this);
    batteryTypeLabel = new QLabel(this);
    batteryLevelLabel = new QLabel(this);
    powerSavingModeLabel = new QLabel(this);
    batteryTimeLabel = new QLabel(this);

    layout->addWidget(powerTypeLabel);
    layout->addWidget(batteryTypeLabel);
    layout->addWidget(batteryLevelLabel);
    layout->addWidget(powerSavingModeLabel);
    layout->addWidget(batteryTimeLabel);

    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &Lab1Window::updateBatteryInfo);
    updateTimer->start(1000);

    updateBatteryInfo();
}

void Lab1Window::updateBatteryInfo()
{
#ifdef Q_OS_WIN
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps)) {
        powerTypeLabel->setText(QString("Источник питания: %1").arg(sps.ACLineStatus == 1 ? "Сеть" : "Батарея"));
        batteryLevelLabel->setText(QString("Уровень заряда: %1%").arg(sps.BatteryLifePercent));
        powerSavingModeLabel->setText(QString("Режим энергосбережения: %1").arg(sps.SystemStatusFlag ? "Вкл" : "Выкл"));
        batteryTimeLabel->setText(QString("Оставшееся время работы батареи: %1 мин").arg(sps.BatteryLifeTime / 60));
    }
#else
    // Для Linux/Mac можно добавить команды через upower или pmset
    powerTypeLabel->setText("Информация недоступна на этой ОС");
#endif
}
