#include "lab2window.h"
#include <QRandomGenerator>
#include <QTableWidgetItem>
#include <algorithm>

Lab2Window::Lab2Window(QWidget *parent)
    : QMainWindow(parent)
{

    table = new QTableWidget(this);
    setCentralWidget(table);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"VendorID", "DeviceID", "Friendly name", "Instance ID"});
    table->horizontalHeader()->setStretchLastSection(true);

    // Шрифт таблицы
    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/KoskoBold-Bold.ttf");
    if (fontId != -1) {
        QFont font(QFontDatabase::applicationFontFamilies(fontId).at(0), 12);
        table->setFont(font);
        table->horizontalHeader()->setFont(font);
        table->verticalHeader()->setFont(font);
    }

    populateTable(enumeratePciDevices());
    setWindowTitle("Лабораторная 2: Конфигурационное пространство PCI");
    resize(1280, 571);

    characterLabel = new QLabel(this);
    characterLabel->setGeometry(0, 360, 60, 82);
    characterLabel->setScaledContents(true);
    characterLabel->show();

    leftWalkSprites = {
        QPixmap(":/morty/simple/left_walk1.png"),
        QPixmap(":/morty/simple/left_walk2.png"),
        QPixmap(":/morty/simple/left_walk3.png")
    };
    rightWalkSprites = {
        QPixmap(":/morty/simple/right_walk1.png"),
        QPixmap(":/morty/simple/right_walk2.png"),
        QPixmap(":/morty/simple/right_walk3.png")
    };

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &Lab2Window::updateAnimation);
    animTimer->start(50);
}

Lab2Window::~Lab2Window() {}

void Lab2Window::closeEvent(QCloseEvent *event) {
    emit windowClosed();
    QMainWindow::closeEvent(event);
}

void Lab2Window::updateAnimation() {
    int speed = 5;
    QPoint pos = characterLabel->pos();

    if (walkDirection == -1) {
        pos.setX(pos.x() - speed);
        characterLabel->setPixmap(leftWalkSprites[frameIndex]);
    } else {
        pos.setX(pos.x() + speed);
        characterLabel->setPixmap(rightWalkSprites[frameIndex]);
    }

    frameIndex = (frameIndex + 1) % leftWalkSprites.size();

    // Проверка границ
    if (pos.x() < -characterLabel->width() || pos.x() > width()) {
        walkDirection *= -1;
        int deltaY = QRandomGenerator::global()->bounded(-150, 150);
        pos.setY(std::clamp(pos.y() + deltaY, 0, height() - characterLabel->height()));
    }

    characterLabel->move(pos);
}

static std::vector<std::string> splitMultiSz(const std::vector<char>& buf) {
    std::vector<std::string> out;
    size_t i = 0;
    while (i < buf.size() && buf[i] != '\0') {
        std::string s(&buf[i]);
        out.push_back(s);
        i += s.size() + 1;
    }
    return out;
}

// Получение PCI устройств
QVector<PciDeviceInfo> Lab2Window::enumeratePciDevices() {
    QVector<PciDeviceInfo> list;

#ifdef Q_OS_WIN

    // Дескрипторы устройтсв
    HDEVINFO devInfo = SetupDiGetClassDevsA(nullptr, nullptr, nullptr, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (devInfo == INVALID_HANDLE_VALUE) return list;

    //
    SP_DEVINFO_DATA devData;
    devData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD index = 0; SetupDiEnumDeviceInfo(devInfo, index, &devData); ++index) {
        DWORD required = 0;
        std::vector<char> buffer(4096);
        if (!SetupDiGetDeviceRegistryPropertyA(devInfo, &devData, SPDRP_HARDWAREID, nullptr,
                                               (PBYTE)buffer.data(), (DWORD)buffer.size(), &required)) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                buffer.resize(required);
                if (!SetupDiGetDeviceRegistryPropertyA(devInfo, &devData, SPDRP_HARDWAREID, nullptr,
                                                       (PBYTE)buffer.data(), (DWORD)buffer.size(), &required)) continue;
            } else continue;
        }

        auto hwids = splitMultiSz(buffer);
        QString vendor, device;
        bool foundPCI = false;
        std::regex pci_re(R"(PCI\\VEN_([0-9A-Fa-f]{4})&DEV_([0-9A-Fa-f]{4}))");

        for (const auto& s : hwids) {
            std::smatch m;
            if (std::regex_search(s, m, pci_re) && m.size() >= 3) {
                vendor = QString::fromStdString(m[1]);
                device = QString::fromStdString(m[2]);
                foundPCI = true;
                break;
            }
        }
        if (!foundPCI) continue;

        char instanceIdBuf[512] = {};
        SetupDiGetDeviceInstanceIdA(devInfo, &devData, instanceIdBuf, sizeof(instanceIdBuf), nullptr);

        char friendly[512] = {};
        if (!SetupDiGetDeviceRegistryPropertyA(devInfo, &devData, SPDRP_FRIENDLYNAME, nullptr,
                                               (PBYTE)friendly, sizeof(friendly), nullptr)) {
            SetupDiGetDeviceRegistryPropertyA(devInfo, &devData, SPDRP_DEVICEDESC, nullptr,
                                              (PBYTE)friendly, sizeof(friendly), nullptr);
        }

        list.push_back({vendor, device, QString::fromLocal8Bit(instanceIdBuf),
                        QString::fromLocal8Bit(friendly)});
    }
    SetupDiDestroyDeviceInfoList(devInfo);
#endif
    return list;
}

// Заполнение таблицы
void Lab2Window::populateTable(const QVector<PciDeviceInfo> &devices) {
    table->setRowCount(devices.size());
    for (int i = 0; i < devices.size(); ++i) {
        const auto &d = devices[i];
        table->setItem(i, 0, new QTableWidgetItem(d.vendor));
        table->setItem(i, 1, new QTableWidgetItem(d.device));
        table->setItem(i, 2, new QTableWidgetItem(d.friendlyName.isEmpty() ? "-" : d.friendlyName));
        table->setItem(i, 3, new QTableWidgetItem(d.instanceId.isEmpty() ? "-" : d.instanceId));
    }
    table->resizeColumnsToContents();
}
