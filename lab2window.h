#ifndef LAB2WINDOW_H
#define LAB2WINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <QVector>
#include <QString>
#include <QFontDatabase>
#include <QHeaderView>
#include <vector>
#include <regex>

#ifdef Q_OS_WIN
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#endif

struct PciDeviceInfo {
    QString vendor;
    QString device;
    QString instanceId;
    QString friendlyName;
};

class Lab2Window : public QMainWindow
{
    Q_OBJECT
public:
    explicit Lab2Window(QWidget *parent = nullptr);
    ~Lab2Window();

signals:
    void windowClosed();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QVector<PciDeviceInfo> enumeratePciDevices();
    void populateTable(const QVector<PciDeviceInfo> &devices);
    void updateAnimation();

private:
    QTableWidget *table;

    QLabel *characterLabel;

    QTimer *animTimer;

    QVector<QPixmap> leftWalkSprites;
    QVector<QPixmap> rightWalkSprites;

    int walkDirection = 1;
    int frameIndex = 0;
};

#endif // LAB2WINDOW_H
