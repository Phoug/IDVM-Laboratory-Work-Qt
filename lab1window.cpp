#include "lab1window.h"
#include <QApplication>
#include <QDebug>
#include <QOperatingSystemVersion>
#include <QProcess>
#include <QSysInfo>
#include <QHBoxLayout>
#include <QFontDatabase>
#include <QFont>
#include <QPalette>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

Lab1Window::Lab1Window(QWidget *parent): QWidget(parent)
{
    setWindowTitle("Лабораторная 1: Энергопитание");
    resize(600, 400);

    // === Подключение Kosko Bold ===
    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/KoskoBold-Bold.ttf");
    QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont koskoFont(fontFamily, 20, QFont::Bold);

    // === Палитра для белого текста ===
    QPalette whiteText;
    whiteText.setColor(QPalette::WindowText, Qt::black);

    // Фон
    QLabel *backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(":/images/other/lab1_background.svg"));
    backgroundLabel->setScaledContents(true);
    backgroundLabel->setGeometry(0, 0, width(), height());
    backgroundLabel->lower();

    // Основной layout
    QHBoxLayout *layout = new QHBoxLayout(this);

    // --- Персонаж слева ---
    characterLabel = new QLabel(this);
    characterLabel->setFixedSize(60, 82);
    characterLabel->setScaledContents(true);
    layout->addWidget(characterLabel);

    // --- Информационный блок справа ---
    QVBoxLayout *infoLayout = new QVBoxLayout();

    powerTypeLabel = new QLabel(this);
    batteryTypeLabel = new QLabel(this);
    batteryLevelLabel = new QLabel(this);
    powerSavingModeLabel = new QLabel(this);
    batteryTimeLabel = new QLabel(this);
    batteryTimeLabel->setFixedWidth(350);
    batteryIconLabel = new QLabel(this);

    // Применяем Kosko Bold + белый текст ко всем QLabel
    QList<QLabel*> labels = { powerTypeLabel, batteryTypeLabel, batteryLevelLabel,
                              powerSavingModeLabel, batteryTimeLabel };
    for (QLabel *lbl : labels) {
        lbl->setFont(koskoFont);
        lbl->setPalette(whiteText);
        lbl->setWordWrap(true);
    }

    infoLayout->addWidget(powerTypeLabel);
    infoLayout->addWidget(batteryTypeLabel);
    infoLayout->addWidget(batteryLevelLabel);
    infoLayout->addWidget(powerSavingModeLabel);
    infoLayout->addWidget(batteryTimeLabel);
    infoLayout->addWidget(batteryIconLabel);

    layout->addLayout(infoLayout);

    // Таймер для обновления батареи
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &Lab1Window::updateBatteryInfo);
    updateTimer->start(1000);

    // Таймер для моргания персонажа
    QTimer *blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, [this]() {
        static bool blinkState = false;
        blinkState = !blinkState;

        if (isCharge) {
            characterLabel->setPixmap(QPixmap(blinkState ? ":/morty/battery/charge_blink.png"
                                                         : ":/morty/battery/charge_calm.png"));
        } else {
            characterLabel->setPixmap(QPixmap(blinkState ? ":/morty/battery/no_charge_blink.png"
                                                         : ":/morty/battery/no_charge_calm.png"));
        }
    });
    blinkTimer->start(500);

    updateBatteryInfo();
}

void Lab1Window::closeEvent(QCloseEvent *event)
{
    emit windowClosed(); // испускаем сигнал при закрытии окна
    QWidget::closeEvent(event); // вызываем базовый обработчик
}

void Lab1Window::updateBatteryInfo()
{
#ifdef Q_OS_WIN
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps)) {
        isCharge = (sps.ACLineStatus == 1);

        powerTypeLabel->setText(QString("Источник питания: %1").arg(isCharge ? "Сеть" : "Батарея"));
        batteryLevelLabel->setText(QString("Уровень заряда: %1%").arg(sps.BatteryLifePercent));
        powerSavingModeLabel->setText(QString("Режим энергосбережения: %1").arg(sps.SystemStatusFlag ? "Вкл" : "Выкл"));

        // Время работы батареи
        if (isCharge && sps.BatteryLifeTime == 0xFFFFFFFF) {
            batteryTimeLabel->setText("Оставшееся время работы батареи: Ноутбук заряжается");
        } else if (sps.BatteryLifeTime == 0xFFFFFFFF){
            batteryTimeLabel->setText("Оставшееся время работы батареи: Расчёт...");
        }else {
            batteryTimeLabel->setText(QString("Оставшееся время работы батареи: %1 мин").arg(sps.BatteryLifeTime / 60));
        }

        QString batteryPixmap;
        int level = sps.BatteryLifePercent;

        switch (isCharge) {
        case true:
            if (level > 90) batteryPixmap = ":/images/other/battery_charge1.svg";
            else if (level > 80) batteryPixmap = ":/images/other/battery_charge2.svg";
            else if (level > 70) batteryPixmap = ":/images/other/battery_charge3.svg";
            else if (level > 60) batteryPixmap = ":/images/other/battery_charge4.svg";
            else if (level > 50) batteryPixmap = ":/images/other/battery_charge5.svg";
            else if (level > 40) batteryPixmap = ":/images/other/battery_charge6.svg";
            else if (level > 30) batteryPixmap = ":/images/other/battery_charge7.svg";
            else if (level > 20) batteryPixmap = ":/images/other/battery_charge8.svg";
            else if (level > 10) batteryPixmap = ":/images/other/battery_charge9.svg";
            else batteryPixmap = ":/images/other/battery_charge10.svg";

            break;
        case false:
            if (level > 90) batteryPixmap = ":/images/other/battery1.svg";
            else if (level > 80) batteryPixmap = ":/images/other/battery2.svg";
            else if (level > 70) batteryPixmap = ":/images/other/battery3.svg";
            else if (level > 60) batteryPixmap = ":/images/other/battery4.svg";
            else if (level > 50) batteryPixmap = ":/images/other/battery5.svg";
            else if (level > 40) batteryPixmap = ":/images/other/battery6.svg";
            else if (level > 30) batteryPixmap = ":/images/other/battery7.svg";
            else if (level > 20) batteryPixmap = ":/images/other/battery8.svg";
            else if (level > 10) batteryPixmap = ":/images/other/battery9.svg";
            else batteryPixmap = ":/images/other/battery10.svg";
            break;
        }

        batteryIconLabel->setPixmap(QPixmap(batteryPixmap));
        batteryIconLabel->setAlignment(Qt::AlignCenter);
        batteryIconLabel->setScaledContents(true);
        batteryIconLabel->setFixedSize(250, 100);
    }
#else
    powerTypeLabel->setText("Информация недоступна на этой ОС");
    batteryTimeLabel->setText("");
    isCharge = true;
#endif
}

Lab1Window::~Lab1Window() {}
