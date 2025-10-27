#include "lab4window.h"
#include <QFileDialog>
#include <QDebug>
#include <QTransform>
#include <QProcess>
#include <QFontDatabase>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDesktopServices>

Lab4Window::Lab4Window(QWidget *parent)
    : QMainWindow(parent)
{
    pendingPhotoPath = "";
    setupUI();
    showCameraInfo();
    resize(1350, 1095);
}

Lab4Window::~Lab4Window()
{
    stopCamera();
}

void Lab4Window::setupUI()
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // --- Fonts ---
    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/KoskoBold-Bold.ttf");
    QFont koskoFont;
    if (fontId != -1) {
        koskoFont = QFont(QFontDatabase::applicationFontFamilies(fontId).at(0), 12);
        this->setFont(koskoFont);
    }

    // --- GraphicsView ---
    graphicsView = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    videoItem = new QGraphicsVideoItem();
    noVideoItem = new QGraphicsPixmapItem(QPixmap(":/images/other/no_video_png.png"));
    scene->addItem(videoItem);
    graphicsView->setScene(scene);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QTransform transform;
    transform.scale(-1, 1);
    videoItem->setTransform(transform);

    mainLayout->addWidget(graphicsView, 1);

    // --- Character Label ---
    characterLabel = new QLabel(graphicsView);
    characterLabel->setFixedSize(79, 150);
    characterLabel->move(50, 165);
    characterLabel->setAttribute(Qt::WA_TranslucentBackground);
    characterLabel->raise();
    characterLabel->show();

    // --- Background ---
    QPalette palette;
    palette.setBrush(QPalette::Window,
                     QBrush(QPixmap(":/images/other/lab4_background.svg")
                                .scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
    central->setAutoFillBackground(true);
    central->setPalette(palette);

    // Update background on resize
    connect(this, &Lab4Window::resized, this, [this, central]() {
        QPalette p = central->palette();
        p.setBrush(QPalette::Window,
                   QBrush(QPixmap(":/images/other/lab4_background.svg")
                              .scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
        central->setPalette(p);
    });

    // --- Sprite lists ---
    spriteListCrying = { ":/morty/crying/crying1.png",
                        ":/morty/crying/crying2.png",
                        ":/morty/crying/crying3.png" };

    spriteListLoving = { ":/morty/loving/loving1.png",
                        ":/morty/loving/loving2.png" };

    currentSprites = &spriteListCrying;
    currentFrame = 0;

    // --- Animation Timer ---
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        characterLabel->setPixmap(QPixmap((*currentSprites)[currentFrame]).scaled(
            characterLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        currentFrame = (currentFrame + 1) % currentSprites->size();
    });
    timer->start(300);

    // --- Buttons ---
    auto createButton = [koskoFont](const QString &text) -> QPushButton* {
        QPushButton *btn = new QPushButton(text);
        btn->setFont(koskoFont);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton {"
            "   border: 3px solid black;"
            "   border-radius: 8px;"
            "   padding: 8px 14px;"
            "   background-color: white;"
            "   color: black;"
            "   font-weight: bold;"
            "}"
            "QPushButton:hover:!disabled {"
            "   background-color: #e6e6e6;"
            "   border-color: #222;"
            "}"
            "QPushButton:pressed {"
            "   background-color: black;"
            "   color: white;"
            "   border: 3px solid white;"
            "}"
            "QPushButton:disabled {"
            "   background-color: #bfbfbf;"
            "   color: #666;"
            "   border: 3px solid #999;"
            "}"
            );
        return btn;
    };

    btnStart      = createButton("Включить камеру");
    btnPhoto      = createButton("Сделать фото");
    btnRecord     = createButton("Начать запись");
    btnHideToTray = createButton("Скрытая съёмка");
    btnOpenFolder = createButton("Открыть папку");

    btnPhoto->setEnabled(false);
    btnRecord->setEnabled(false);
    btnHideToTray->setEnabled(false);

    labelInfo = new QLabel(this);
    labelInfo->setWordWrap(true);
    labelInfo->setAlignment(Qt::AlignCenter);

    QWidget *controlsWidget = new QWidget(this);
    QHBoxLayout *controlsLayout = new QHBoxLayout(controlsWidget);
    controlsLayout->addWidget(btnStart);
    controlsLayout->addWidget(btnPhoto);
    controlsLayout->addWidget(btnRecord);
    controlsLayout->addWidget(btnHideToTray);
    controlsLayout->addWidget(btnOpenFolder);
    controlsLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(controlsWidget, 0);
    mainLayout->addWidget(labelInfo, 0);

    setCentralWidget(central);
    setWindowTitle("Лабораторная 4: Веб-камера");
    setFixedSize(900, 700);

    // --- Record indicator ---
    recordIndicator = new QLabel("● REC", this);
    recordIndicator->setStyleSheet("color: red; font-weight: bold; font-size: 18px;");
    recordIndicator->move(width() - 70, 10);
    recordIndicator->hide();

    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, [this]() {
        recordIndicator->setVisible(!recordIndicator->isVisible());
    });

    initTray();

    connect(btnStart, &QPushButton::clicked, this, &Lab4Window::handleCameraToggle);
    connect(btnPhoto, &QPushButton::clicked, this, &Lab4Window::capturePhoto);
    connect(btnRecord, &QPushButton::clicked, this, &Lab4Window::handleVideoToggle);
    connect(btnHideToTray, &QPushButton::clicked, this, &Lab4Window::hideAndRecord5s);
    connect(btnOpenFolder, &QPushButton::clicked, this, &Lab4Window::openSaveFolder);
}

void Lab4Window::openSaveFolder()
{
    QString saveBaseDir = "C:/Users/Phoug/Pictures/lab04";
    QDesktopServices::openUrl(QUrl::fromLocalFile(saveBaseDir));
}

void Lab4Window::hideAndRecord5s()
{
    this->hide();
    emit hideRequested();

    if (!camera || !mediaRecorder)
        return;

    prepareVideoPaths();
    mediaRecorder->setOutputLocation(QUrl::fromLocalFile(currentTempVideo));
    mediaRecorder->record();

    QTimer::singleShot(5000, this, [this]() {
        stopRecording();
    });
}

void Lab4Window::initTray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "System tray not available on this system.";
        return;
    }

    trayIcon = new QSystemTrayIcon(QIcon("C:/Users/Phoug/Pictures/Icon/morty_icon"), this);
    trayMenu = new QMenu();

    trayShowAction = new QAction("Показать окно", this);
    trayExitAction = new QAction("Выйти", this);

    trayMenu->addAction(trayShowAction);
    trayMenu->addSeparator();
    trayMenu->addAction(trayExitAction);

    trayIcon->setContextMenu(trayMenu);

    connect(trayShowAction, &QAction::triggered, this, [this]() {
        this->show();
        this->raise();
        this->activateWindow();
    });

    connect(trayExitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    connect(trayIcon, &QSystemTrayIcon::activated, this, &Lab4Window::trayIconActivated);

    trayIcon->show();
}

void Lab4Window::toggleHideToTray()
{
    if (!trayIcon) {
        initTray();
        if (!trayIcon) return;
    }

    if (isVisible()) {
        this->hide();
    } else {
        this->show();
        this->raise();
        this->activateWindow();
    }
}

void Lab4Window::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (!isVisible()) {
            this->show();
            this->raise();
            this->activateWindow();

            emit showRequested();
        } else {
            this->hide();
        }
    }
}

void Lab4Window::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    emit resized();
    updateVideoItemSize();
}

void Lab4Window::updateVideoItemSize()
{
    if (!graphicsView) return;
    QSizeF viewSize = graphicsView->viewport()->size();

    if (videoItem->isVisible()) {
        QSizeF native = videoItem->nativeSize();
        if (native.isEmpty()) return;

        qreal scale = qMin(viewSize.width() / native.width(),
                           viewSize.height() / native.height());
        QSizeF scaledSize(native.width() * scale, native.height() * scale);

        videoItem->setSize(scaledSize);
        videoItem->setPos((viewSize.width() - scaledSize.width()) / 2,
                          (viewSize.height() - scaledSize.height()) / 2);
    } else if (noVideoItem->isVisible()) {
        QRectF bounding = noVideoItem->boundingRect();
        if (bounding.isEmpty()) return;

        qreal scale = qMin(viewSize.width() / bounding.width(),
                           viewSize.height() / bounding.height());
        noVideoItem->setScale(scale);
        noVideoItem->setPos((viewSize.width() - bounding.width() * scale) / 2,
                            (viewSize.height() - bounding.height() * scale) / 2);
    }
}

void Lab4Window::showCameraInfo()
{
    QString info;
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();

    if (cameras.isEmpty()) {
        info = "Нет доступных устройств.";
    } else {
        info = "Доступные устройства видеозахвата:\n";
        for (const QCameraDevice &cam : cameras)
            info += QString("%1 (%2)\n").arg(cam.description(), cam.id());

        selectedCamera = cameras.first();
        info += QString("\nИспользуется сейчас: %1").arg(selectedCamera.description());
    }

    labelInfo->setText(info);
}

void Lab4Window::handleCameraToggle()
{
    if (btnStart->text() == "Включить камеру") {
        startCamera();
        btnStart->setText("Выключить камеру");
        btnPhoto->setEnabled(true);
        btnRecord->setEnabled(true);
        btnHideToTray->setEnabled(true);

        noVideoItem->setVisible(false);
        videoItem->setVisible(true);

        currentSprites = &spriteListLoving;
        currentFrame = 0;
    } else {
        stopCamera();
        btnStart->setText("Включить камеру");
        btnPhoto->setEnabled(false);
        btnRecord->setEnabled(false);
        btnHideToTray->setEnabled(false);

        noVideoItem->setVisible(true);
        videoItem->setVisible(false);

        currentSprites = &spriteListCrying;
        currentFrame = 0;
    }

    updateVideoItemSize();
}

void Lab4Window::startCamera()
{
    if (camera) return;

    camera = new QCamera(selectedCamera, this);
    imageCapture = new QImageCapture(this);
    mediaRecorder = new QMediaRecorder(this);

    captureSession.setCamera(camera);
    captureSession.setImageCapture(imageCapture);
    captureSession.setRecorder(mediaRecorder);
    captureSession.setVideoOutput(videoItem);

    connect(imageCapture, &QImageCapture::imageCaptured, this, &Lab4Window::onImageCaptured);
    connect(videoItem, &QGraphicsVideoItem::nativeSizeChanged, this, &Lab4Window::updateVideoItemSize);

    graphicsView->show();
    camera->start();

    // Delay update to allow native size to be set
    QTimer::singleShot(100, this, &Lab4Window::updateVideoItemSize);
    qDebug() << "Camera started.";
}

void Lab4Window::stopCamera()
{
    if (!camera) return;

    if (mediaRecorder && mediaRecorder->recorderState() == QMediaRecorder::RecordingState) {
        mediaRecorder->stop();
    }

    camera->stop();

    delete mediaRecorder;
    delete imageCapture;
    delete camera;

    mediaRecorder = nullptr;
    imageCapture = nullptr;
    camera = nullptr;

    qDebug() << "Camera stopped.";
}

void Lab4Window::capturePhoto()
{
    if (!camera || !imageCapture) return;

    QString saveDir = "C:/Users/Phoug/Pictures/lab04/photo";
    QDir dir(saveDir);
    if (!dir.exists()) dir.mkpath(".");

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    pendingPhotoPath = dir.filePath(QString("photo_%1.jpg").arg(timestamp));
    imageCapture->capture();
}

void Lab4Window::onImageCaptured(int id, const QImage &image)
{
    Q_UNUSED(id);

    QWidget *flash = new QWidget(this);
    flash->setStyleSheet("background-color: white;");
    flash->setGeometry(0, 0, width(), height());
    flash->show();

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(flash);
    flash->setGraphicsEffect(effect);

    QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(400);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, flash, &QWidget::deleteLater);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    if (pendingPhotoPath.isEmpty()) return;

    QImage mirroredImage = image.mirrored(true, false);
    if (mirroredImage.save(pendingPhotoPath)) {
        qDebug() << "Photo saved to:" << pendingPhotoPath;
    } else {
        qDebug() << "Failed to save photo.";
    }

    pendingPhotoPath.clear();
}

void Lab4Window::handleVideoToggle()
{
    if (btnRecord->text() == "Начать запись") {
        startRecording();
        btnRecord->setText("Выключить запись");
    } else {
        stopRecording();
        btnRecord->setText("Начать запись");
    }
}

void Lab4Window::prepareVideoPaths()
{
    QString saveDir = "C:/Users/Phoug/Pictures/lab04/video";
    QDir dir(saveDir);
    if (!dir.exists()) dir.mkpath(".");

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    currentTempVideo = dir.filePath(QString("temp_%1.mp4").arg(timestamp));
    currentFinalVideo = dir.filePath(QString("video_%1.mp4").arg(timestamp));
}

void Lab4Window::startRecording()
{
    if (!camera || !mediaRecorder) return;

    prepareVideoPaths();
    recordIndicator->show();
    blinkTimer->start(500);

    mediaRecorder->setOutputLocation(QUrl::fromLocalFile(currentTempVideo));
    mediaRecorder->record();

    qDebug() << "Recording started:" << currentTempVideo;
}

void Lab4Window::stopRecording()
{
    if (!mediaRecorder || mediaRecorder->recorderState() != QMediaRecorder::RecordingState) return;

    mediaRecorder->stop();
    qDebug() << "Recording stopped:" << currentTempVideo;

    if (currentTempVideo.isEmpty() || currentFinalVideo.isEmpty()) return;

    blinkTimer->stop();
    recordIndicator->hide();

    // Prepare ffmpeg arguments
    QStringList args;
    args << "-i" << currentTempVideo
         << "-vf" << "hflip"
         << "-c:a" << "copy"
         << currentFinalVideo;

    QProcess *ffmpegProcess = new QProcess(this);

    // Delete temp file after ffmpeg finishes
    connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, ffmpegProcess](int, QProcess::ExitStatus) {
                QFile::remove(currentTempVideo);
                qDebug() << "Temp video deleted:" << currentTempVideo;
                ffmpegProcess->deleteLater();
            });

    ffmpegProcess->start("ffmpeg", args);

    qDebug() << "Flipping started, output:" << currentFinalVideo;
}

void Lab4Window::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    QMainWindow::closeEvent(event);
}
