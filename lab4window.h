#ifndef LAB4WINDOW_H
#define LAB4WINDOW_H
#include <QMainWindow>
#include <QCamera>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QImageCapture>
#include <QMediaRecorder>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QGraphicsPixmapItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCloseEvent>
#include <QImage>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

class Lab4Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Lab4Window(QWidget *parent = nullptr);
    ~Lab4Window();

signals:
    void windowClosed();
    void hideRequested();
    void showRequested();
    void resized();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void hideAndRecord5s();
    void handleCameraToggle();
    void startCamera();
    void stopCamera();
    void capturePhoto();
    void onImageCaptured(int id, const QImage &image);
    void handleVideoToggle();
    void startRecording();
    void stopRecording();
    void updateVideoItemSize();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void setupUI();
    void showCameraInfo();
    void openSaveFolder();
    void prepareVideoPaths();
    QCamera *camera = nullptr;
    QCameraDevice selectedCamera;
    QImageCapture *imageCapture = nullptr;
    QMediaRecorder *mediaRecorder = nullptr;
    QMediaCaptureSession captureSession;
    QGraphicsView *graphicsView = nullptr;
    QGraphicsScene *scene = nullptr;
    QGraphicsVideoItem *videoItem = nullptr;
    QGraphicsPixmapItem *noVideoItem = nullptr;
    QPushButton *btnStart;
    QPushButton *btnPhoto;
    QPushButton *btnRecord;
    QLabel *labelInfo;
    QString pendingPhotoPath;

    QLabel *characterLabel;

    QLabel *recordIndicator = nullptr;
    QTimer *blinkTimer = nullptr;

    QString currentTempVideo;   // временный файл записи
    QString currentFinalVideo;  // итоговый (отзеркаленный) файл

    QSystemTrayIcon *trayIcon = nullptr;
    QMenu *trayMenu = nullptr;
    QAction *trayShowAction = nullptr;
    QAction *trayExitAction = nullptr;
    QPushButton *btnHideToTray = nullptr;
    QPushButton* btnOpenFolder = nullptr;

    QStringList spriteListCrying;
    QStringList spriteListLoving;
    QStringList *currentSprites;
    int currentFrame;

    void initTray();
    void toggleHideToTray();
};
#endif // LAB4WINDOW_H
