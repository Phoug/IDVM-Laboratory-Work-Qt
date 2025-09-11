#ifndef LAB1WINDOW_H
#define LAB1WINDOW_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QCloseEvent>

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

private:
    QLabel *characterLabel;

    QLabel *powerTypeLabel;
    QLabel *batteryTypeLabel;
    QLabel *batteryLevelLabel;
    QLabel *powerSavingModeLabel;
    QLabel *batteryTimeLabel;

    QTimer *updateTimer;

    bool isCharge;
    QLabel *batteryIconLabel;
};

#endif // LAB1WINDOW_H
