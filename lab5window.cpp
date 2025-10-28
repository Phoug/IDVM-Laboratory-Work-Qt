#include "lab5window.h"
#include <QCloseEvent>
#include <QResizeEvent>
#include <QFontDatabase>
#include <QGuiApplication>
#include <initguid.h>
#include <devpropdef.h>
#include <QDateTime>
#include <QMap>
#include <QDebug>
#include <dbt.h>
#include <setupapi.h>

DEFINE_DEVPROPKEY(DEVPKEY_Device_FriendlyName, 0xa45c254e, 0xdf1c, 0x4efd,
                  0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 14);
DEFINE_DEVPROPKEY(DEVPKEY_Device_DeviceDesc, 0xa45c254e, 0xdf1c, 0x4efd,
                  0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 2);

Lab5Window::Lab5Window(QWidget *parent) : QMainWindow(parent) {
    // --- Fonts ---
    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/KoskoBold-Bold.ttf");
    QFont koskoFont;
    if (fontId != -1) {
        koskoFont = QFont(QFontDatabase::applicationFontFamilies(fontId).at(0), 12);
        this->setFont(koskoFont);
    }

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    statusLabel = new QLabel("Менеджер USB-устройств", this);
    statusLabel->setFont(koskoFont);
    layout->addWidget(statusLabel);

    deviceList = new QListWidget(this);
    deviceList->setWordWrap(true);
    layout->addWidget(deviceList);
    connect(deviceList, &QListWidget::itemSelectionChanged, this, &Lab5Window::onDeviceSelected);

    refreshButton = new QPushButton("Обновить список", this);
    connect(refreshButton, &QPushButton::clicked, this, &Lab5Window::onRefreshClicked);

    ejectButton = new QPushButton("Безопасно извлечь выбранное", this);
    ejectButton->setEnabled(false);
    connect(ejectButton, &QPushButton::clicked, this, &Lab5Window::onEjectClicked);

    // Добавляем персонажа
    characterLabel = new QLabel(this);
    characterLabel->setAlignment(Qt::AlignCenter);
    characterLabel->setScaledContents(true);
    characterLabel->setFixedSize(150, 109);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(characterLabel);
    buttonLayout->addStretch();
    buttonLayout->addWidget(ejectButton);
    layout->addLayout(buttonLayout);

    logText = new QTextEdit(this);
    logText->setReadOnly(true);
    QLabel *logLabel = new QLabel("Журнал событий:", this);
    logLabel->setFont(koskoFont);
    layout->addWidget(logLabel);
    layout->addWidget(logText);

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &Lab5Window::advanceAnimation);

    // Инициализация списков кадров
    calmConnectFrames = {
        ":/morty/connect/calm_connect1.png",
        ":/morty/connect/calm_connect2.png",
        ":/morty/connect/calm_connect3.png"
    };

    connectFrames = {
        ":/morty/connect/connect1.png",
        ":/morty/connect/connect2.png",
        ":/morty/connect/connect3.png",
        ":/morty/connect/connect4.png",
        ":/morty/connect/connect5.png",
        ":/morty/connect/connect6.png",
        ":/morty/connect/connect7.png",
        ":/morty/connect/connect8.png"
    };

    calmDisconnectFrames = {
        ":/morty/disconnect/calm_disconnect1.png",
        ":/morty/disconnect/calm_disconnect2.png",
        ":/morty/disconnect/calm_disconnect3.png"
    };

    disconnectFrames = {
        ":/morty/disconnect/disconnect1.png",
        ":/morty/disconnect/disconnect2.png",
        ":/morty/disconnect/disconnect3.png",
        ":/morty/disconnect/disconnect4.png",
        ":/morty/disconnect/disconnect5.png",
        ":/morty/disconnect/disconnect6.png",
        ":/morty/disconnect/disconnect7.png",
        ":/morty/disconnect/disconnect8.png"
    };

    // --- Button Styles ---
    auto styleButtons = [koskoFont](QPushButton *btn) {
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
    };

    styleButtons(refreshButton);
    styleButtons(ejectButton);

    characterLabel->setPixmap(QPixmap(":/morty/disconnect/calm_disconnect1.png"));

    auto styleWidgets = [](QWidget *widget) {
        QString className = widget->metaObject()->className();
        QFont f = widget->font();

        QString style = QString(
                            "%1 {"
                            "   border: 3px solid black;"
                            "   border-radius: 8px;"
                            "   background-color: white;"
                            "   color: black;"
                            "   font-weight: bold;"
                            "   font-family: \"%2\";"
                            "   font-size: %3pt;"
                            "   selection-background-color: black;"
                            "   selection-color: white;"
                            "}"
                            ).arg(className)
                            .arg(f.family())
                            .arg(f.pointSize());

        widget->setStyleSheet(style);
    };

    styleWidgets(deviceList);
    styleWidgets(logText);

    // --- Background ---
    QPalette palette;
    palette.setBrush(QPalette::Window,
                     QBrush(QPixmap(":/images/other/lab4_background.svg")
                                .scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
    centralWidget->setAutoFillBackground(true);
    centralWidget->setPalette(palette);

    connect(this, &Lab5Window::resized, this, [this, centralWidget]() {
        QPalette p = centralWidget->palette();
        p.setBrush(QPalette::Window,
                   QBrush(QPixmap(":/images/other/lab4_background.svg")
                              .scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
        centralWidget->setPalette(p);
    });

    setCentralWidget(centralWidget);
    setWindowTitle("Лабораторная №5 - USB-устройства");

    resize(1000, 600);
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    registerUsbNotifications();
    listUsbDevices(); // Initial list
}

Lab5Window::~Lab5Window() {}

void Lab5Window::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    emit resized();
}

void Lab5Window::registerUsbNotifications() {
    DEV_BROADCAST_DEVICEINTERFACE filter = {};
    filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;
    HDEVNOTIFY notify = RegisterDeviceNotification((HANDLE)winId(), &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (!notify) {
        logMessage("Не удалось зарегистрироваться для уведомлений USB");
    }
}

QString Lab5Window::getDeviceIdFromPath(const wchar_t *path) {
    QString str = QString::fromWCharArray(path);
    if (str.startsWith("\\\\?\\")) {
        str = str.mid(4);
    }
    int guidStart = str.lastIndexOf('#');
    if (guidStart == -1) {
        return "";
    }
    str = str.left(guidStart);
    str.replace('#', '\\');
    return str;
}

bool Lab5Window::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
    if (eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_DEVICECHANGE) {
            PDEV_BROADCAST_HDR hdr = reinterpret_cast<PDEV_BROADCAST_HDR>(msg->lParam);
            if (hdr) {
                if (hdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                    PDEV_BROADCAST_DEVICEINTERFACE dbi = reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE>(hdr);
                    QString devId = getDeviceIdFromPath(dbi->dbcc_name);
                    QString name = deviceIdToName.value(devId, "Неизвестное устройство");

                    switch (msg->wParam) {
                    case DBT_DEVICEARRIVAL:
                        listUsbDevices();
                        name = deviceIdToName.value(devId, "Неизвестное устройство"); // Update after list
                        logMessage("Новое USB-устройство подключено: " + name + " (" + devId + ")");
                        playAnimation(connectFrames); // Анимация подключения
                        break;
                    case DBT_DEVICEREMOVECOMPLETE:
                        logMessage("Устройство извлечено безопасно: " + name + " (" + devId + ")");
                        listUsbDevices();
                        playAnimation(calmDisconnectFrames); // Спокойная анимация безопасного отключения
                        break;
                    case DBT_DEVICEREMOVEPENDING:
                        logMessage("Безопасное извлечение в процессе для: " + name + " (" + devId + ")");
                        break;
                    case DBT_DEVICEQUERYREMOVEFAILED:
                        logMessage("Безопасное извлечение отклонено для: " + name + " (" + devId + ")");
                        listUsbDevices();
                        playAnimation(disconnectFrames); // Анимация отказа (возбужденная)
                        break;
                    }
                } else if (hdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                    if (msg->wParam == DBT_DEVICEREMOVECOMPLETE) {
                        PDEV_BROADCAST_VOLUME dbv = reinterpret_cast<PDEV_BROADCAST_VOLUME>(hdr);
                        QString drives;
                        DWORD mask = dbv->dbcv_unitmask;
                        for (int i = 0; i < 26; ++i) {
                            if (mask & (1 << i)) {
                                drives += QString(QChar('A' + i)) + ":/ ";
                            }
                        }
                        logMessage("Устройство извлечено (возможно небезопасно): " + drives.trimmed());
                        listUsbDevices();
                        playAnimation(disconnectFrames); // Анимация небезопасного отключения
                    }
                }
            }
            *result = 0;
            return true;
        }
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

void Lab5Window::listUsbDevices() {
    deviceList->clear();

    HDEVINFO devInfo = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (devInfo == INVALID_HANDLE_VALUE) {
        logMessage("Не удалось получить информацию об устройствах");
        return;
    }

    QSet<QString> uniqueIds;
    QList<QListWidgetItem*> nonRemovableItems;
    QList<QListWidgetItem*> removableItems;

    SP_DEVINFO_DATA devData = {};
    devData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (DWORD i = 0; SetupDiEnumDeviceInfo(devInfo, i, &devData); ++i) {
        ULONG bufSize = 0;
        CM_Get_Device_ID_Size(&bufSize, devData.DevInst, 0);
        QByteArray idBuf((bufSize + 1) * sizeof(wchar_t), 0);
        CM_Get_Device_IDW(devData.DevInst, reinterpret_cast<PWCHAR>(idBuf.data()), bufSize + 1, 0);
        QString deviceId = QString::fromWCharArray(reinterpret_cast<wchar_t*>(idBuf.data()));

        QString upperId = deviceId.toUpper();
        if (!upperId.startsWith("USB\\") && !upperId.startsWith("USBSTOR\\")) continue;

        if (uniqueIds.contains(deviceId)) continue;
        uniqueIds.insert(deviceId);

        // Get friendly name
        DWORD size = 0;
        SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_FRIENDLYNAME, NULL, NULL, 0, &size);
        QString name;
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && size > 0) {
            QByteArray buffer(size, 0);
            if (SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)buffer.data(), size, NULL)) {
                name = QString::fromUtf16(reinterpret_cast<const ushort*>(buffer.constData()));
            }
        }

        // If friendly name is empty, get device description
        if (name.isEmpty()) {
            size = 0;
            SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_DEVICEDESC, NULL, NULL, 0, &size);
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && size > 0) {
                QByteArray buffer(size, 0);
                if (SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_DEVICEDESC, NULL, (PBYTE)buffer.data(), size, NULL)) {
                    name = QString::fromUtf16(reinterpret_cast<const ushort*>(buffer.constData()));
                }
            }
        }

        // Fallback if still empty
        if (name.isEmpty()) {
            name = "Неизвестное устройство";
        }

        deviceIdToName[deviceId] = name; // Update cache

        // Get device class
        size = 0;
        SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_CLASS, NULL, NULL, 0, &size);
        QString deviceClass;
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && size > 0) {
            QByteArray classBuffer(size, 0);
            if (SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_CLASS, NULL, (PBYTE)classBuffer.data(), size, NULL)) {
                deviceClass = QString::fromUtf16(reinterpret_cast<const ushort*>(classBuffer.constData()));
            }
        }

        bool isRemovable = false;
        if (deviceClass == "DiskDrive" || deviceClass == "CdRom" || deviceClass == "WPD" || deviceClass == "Volume" || deviceClass == "PortableDevice") {
            isRemovable = true;
        }
        if (deviceClass == "USB") {
            // Get service
            size = 0;
            SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_SERVICE, NULL, NULL, 0, &size);
            QString service;
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && size > 0) {
                QByteArray buffer(size, 0);
                if (SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_SERVICE, NULL, (PBYTE)buffer.data(), size, NULL)) {
                    service = QString::fromUtf16(reinterpret_cast<const ushort*>(buffer.constData()));
                }
            }
            if (service.toLower() == "usbstor") {
                isRemovable = true;
            }
        }

        QString typeStr;
        if (deviceClass == "Mouse" || (deviceClass == "HIDClass" && name.toLower().contains("mouse"))) {
            typeStr = "Мышь";
        } else if (deviceClass == "DiskDrive") {
            typeStr = "Флэшка или внешний диск";
        } else if (deviceClass == "Camera") {
            typeStr = "Камера";
        } else if (deviceClass == "CdRom") {
            typeStr = "CD-ROM";
        } else if (deviceClass == "WPD") {
            typeStr = "Портативное устройство (MTP)";
        } else {
            typeStr = deviceClass;
        }

        QString listText = name + " (ID: " + deviceId + " - Тип: " + typeStr + ")";
        if (isRemovable) {
            listText += " - Извлекаемое";
        }
        QListWidgetItem *item = new QListWidgetItem(listText);
        item->setData(Qt::UserRole, deviceId);
        item->setData(Qt::UserRole + 1, isRemovable);

        if (isRemovable) {
            removableItems.append(item);
        } else {
            nonRemovableItems.append(item);
        }

        qDebug() << "USB Device:" << name << "ID:" << deviceId << "Class:" << deviceClass << "Removable:" << isRemovable;
    }
    SetupDiDestroyDeviceInfoList(devInfo);

    // Add section headers and items
    if (!nonRemovableItems.isEmpty()) {
        QListWidgetItem *header = new QListWidgetItem("Неизвлекаемые устройства:");
        QFont boldFont = header->font();
        boldFont.setBold(true);
        header->setFont(boldFont);
        header->setFlags(header->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        deviceList->addItem(header);

        for (auto item : nonRemovableItems) {
            deviceList->addItem(item);
        }
    }

    if (!removableItems.isEmpty()) {
        QListWidgetItem *header = new QListWidgetItem("Извлекаемые устройства:");
        QFont boldFont = header->font();
        boldFont.setBold(true);
        header->setFont(boldFont);
        header->setFlags(header->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        deviceList->addItem(header);

        for (auto item : removableItems) {
            deviceList->addItem(item);
        }
    }
}

bool Lab5Window::ejectUsbDevice(const QString &deviceId) {
    DEVINST devInst;
    CONFIGRET cr = CM_Locate_DevNodeW(&devInst, (DEVINSTID_W)deviceId.toStdWString().c_str(), CM_LOCATE_DEVNODE_NORMAL);
    if (cr != CR_SUCCESS) {
        logMessage("Не удалось найти устройство: " + deviceId);
        return false;
    }

    PNP_VETO_TYPE vetoType;
    cr = CM_Request_Device_Eject(devInst, &vetoType, NULL, 0, 0);
    if (cr != CR_SUCCESS) {
        QString vetoStr;
        switch (vetoType) {
        case PNP_VetoTypeUnknown: vetoStr = "Неизвестно"; break;
        case PNP_VetoLegacyDevice: vetoStr = "Устаревшее устройство"; break;
        case PNP_VetoPendingClose: vetoStr = "Ожидание закрытия"; break;
        case PNP_VetoWindowsApp: vetoStr = "Приложение Windows"; break;
        case PNP_VetoWindowsService: vetoStr = "Служба Windows"; break;
        case PNP_VetoOutstandingOpen: vetoStr = "Открытые дескрипторы"; break;
        case PNP_VetoDevice: vetoStr = "Устройство"; break;
        case PNP_VetoDriver: vetoStr = "Драйвер"; break;
        case PNP_VetoIllegalDeviceRequest: vetoStr = "Недопустимый запрос"; break;
        case PNP_VetoInsufficientPower: vetoStr = "Недостаточно питания"; break;
        case PNP_VetoNonDisableable: vetoStr = "Неотключаемое"; break;
        case PNP_VetoLegacyDriver: vetoStr = "Устаревший драйвер"; break;
        case PNP_VetoInsufficientRights: vetoStr = "Недостаточно прав"; break;
        default: vetoStr = "Неопределено"; break;
        }
        logMessage("Извлечение не удалось для " + deviceId + ": код " + QString::number(cr) + ", Veto: " + vetoStr);
        return false;
    }
    return true;
}

void Lab5Window::playAnimation(const QStringList &frames) {
    if (frames.isEmpty()) return;
    currentAnimationFrames = frames;
    currentFrame = 0;
    animationTimer->start(100); // 100 мс на кадр, подстройте под нужную скорость
}

void Lab5Window::advanceAnimation() {
    if (currentFrame < currentAnimationFrames.size()) {
        characterLabel->setPixmap(QPixmap(currentAnimationFrames[currentFrame]));
        currentFrame++;
    } else {
        animationTimer->stop();
        // Опционально: установить статичное изображение после анимации
        // characterLabel->setPixmap(QPixmap(":/morty/idle.png")); // Если есть idle
    }
}

void Lab5Window::onEjectClicked() {
    QListWidgetItem *selected = deviceList->currentItem();
    if (selected) {
        QString deviceId = selected->data(Qt::UserRole).toString();
        bool isRemovable = selected->data(Qt::UserRole + 1).toBool();
        if (isRemovable) {
            ejectUsbDevice(deviceId);
            // Анимация будет запущена по событию системы
        } else {
            logMessage("Выбранное устройство не извлекаемое");
        }
    }
}

void Lab5Window::onRefreshClicked() {
    listUsbDevices();
}

void Lab5Window::onDeviceSelected() {
    QListWidgetItem *selected = deviceList->currentItem();
    if (selected) {
        bool isRemovable = selected->data(Qt::UserRole + 1).toBool();
        ejectButton->setEnabled(isRemovable);
    } else {
        ejectButton->setEnabled(false);
    }
}

void Lab5Window::logMessage(const QString &message) {
    QString timedMsg = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + message;
    QString currentText = logText->toPlainText();
    if (!currentText.isEmpty()) {
        QStringList lines = currentText.split('\n');
        QString lastLine = lines.last();
        // To avoid duplicates, compare message without time
        QString lastMsg = lastLine.mid(lastLine.indexOf(' ') + 1);
        if (lastMsg == message) {
            return;
        }
    }
    logText->append(timedMsg);
    qDebug() << timedMsg;
}

void Lab5Window::closeEvent(QCloseEvent *event) {
    emit windowClosed();
    QMainWindow::closeEvent(event);
}
