#ifndef LAB1WINDOW_H
#define LAB1WINDOW_H

#include <QRegularExpression>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCloseEvent>
#include <QPalette>
#include <QProcess>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QFont>

#ifdef Q_OS_WIN
#include <windows.h>
#include <powrprof.h>
#endif

#include "define.h"

class Lab1Window : public QWidget
{
    Q_OBJECT

public:
    explicit Lab1Window(QWidget *parent = nullptr);
    ~Lab1Window();

signals:
    void windowClosed();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateBatteryInfo();
    void sleepMode();
    void hibernateMode();

private:
    void setupUi();
    void setupLabels();
    void setupButtons();
    void setupTimers();
    void blinkCharacter();
    QString getBatteryType() const;
    QString getBatteryIcon(int level) const;

private:
    QLabel *characterLabel;
    QLabel *powerTypeLabel;
    QLabel *batteryTypeLabel;
    QLabel *batteryLevelLabel;
    QLabel *powerSavingModeLabel;
    QLabel *batteryTimeLabel;
    QLabel *batteryIconLabel;

    QTimer *updateTimer;
    QTimer *blinkTimer;

    bool isCharge;
};

#endif // LAB1WINDOW_H
