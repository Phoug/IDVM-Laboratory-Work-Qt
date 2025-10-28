#ifndef LAB5WINDOW_H
#define LAB5WINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <windows.h>
#include <dbt.h>
#include <setupapi.h>
#include <devpkey.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <QMap>
#include <QTimer>
#include <QStringList>

DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);

class Lab5Window : public QMainWindow {
    Q_OBJECT

public:
    explicit Lab5Window(QWidget *parent = nullptr);
    ~Lab5Window();
    void listUsbDevices();
    bool ejectUsbDevice(const QString &deviceId);

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void windowClosed();
    void resized();

private slots:
    void onEjectClicked();
    void onRefreshClicked();
    void onDeviceSelected();
    void advanceAnimation();

private:
    void registerUsbNotifications();
    void logMessage(const QString &message);
    QString getDeviceIdFromPath(const wchar_t *path);
    void playAnimation(const QStringList &frames);
    QListWidget *deviceList;
    QPushButton *ejectButton;
    QPushButton *refreshButton;
    QTextEdit *logText;
    QLabel *statusLabel;
    QMap<QString, QString> deviceIdToName;
    QLabel *characterLabel;
    QTimer *animationTimer;
    QStringList currentAnimationFrames;
    int currentFrame;
    QStringList connectFrames;
    QStringList calmConnectFrames;
    QStringList disconnectFrames;
    QStringList calmDisconnectFrames;
};

#endif // LAB5WINDOW_H
