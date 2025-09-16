#include "lab1window.h"

Lab1Window::Lab1Window(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Лабораторная 1: Энергопитание");
    resize(LAB1_SCREEN_WEIGHT, LAB1_SCREEN_HEIGHT);

    // Шрифт Kosko Bold
    int fontId = QFontDatabase::addApplicationFont(":/fonts/fonts/KoskoBold-Bold.ttf");
    QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont koskoFont(fontFamily, 20, QFont::Bold);

    QPalette blackText;
    blackText.setColor(QPalette::WindowText, Qt::black);

    // Фоновый лейбл
    QLabel *backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(":/images/other/lab1_background.svg"));
    backgroundLabel->setScaledContents(true);
    backgroundLabel->setGeometry(0, 0, width(), height());
    backgroundLabel->lower();

    // Основной layout
    QHBoxLayout *layout = new QHBoxLayout(this);

    // Персонаж
    characterLabel = new QLabel(this);
    characterLabel->setFixedSize(120, 164);
    characterLabel->setScaledContents(true);
    layout->addStretch();
    layout->addWidget(characterLabel);

    // Информационный блок
    QVBoxLayout *infoLayout = new QVBoxLayout();
    powerTypeLabel = new QLabel(this);
    batteryTypeLabel = new QLabel(this);
    batteryLevelLabel = new QLabel(this);
    powerSavingModeLabel = new QLabel(this);
    batteryTimeLabel = new QLabel(this);
    batteryTimeLabel->setFixedWidth(350);
    batteryIconLabel = new QLabel(this);

    QList<QLabel*> infoLabels = { powerTypeLabel, batteryTypeLabel, batteryLevelLabel,
                                  powerSavingModeLabel, batteryTimeLabel };
    for (QLabel *lbl : infoLabels) {
        lbl->setFont(koskoFont);
        lbl->setPalette(blackText);
        lbl->setWordWrap(true);
    }

    // Кнопки спящий режим и гибернация
    auto createButton = [koskoFont](const QString &text) {
        QPushButton *btn = new QPushButton(text);
        btn->setFont(koskoFont);
        btn->setStyleSheet(
            "QPushButton { border: 5px solid black; border-radius: 6px; padding: 6px; background-color: white; color: black; }"
            "QPushButton:hover { background-color: lightgray; }"
            "QPushButton:pressed { background-color: black; border: 5px solid white; color: white; }"
            );
        return btn;
    };

    QPushButton *sleepButton = createButton("Спящий режим");
    QPushButton *hibernateButton = createButton("Гибернация");

    connect(sleepButton, &QPushButton::clicked, this, &Lab1Window::sleepMode);
    connect(hibernateButton, &QPushButton::clicked, this, &Lab1Window::hibernateMode);

    // Добавление виджетов в layout
    infoLayout->addWidget(powerTypeLabel);
    infoLayout->addWidget(batteryTypeLabel);
    infoLayout->addWidget(batteryLevelLabel);
    infoLayout->addWidget(powerSavingModeLabel);
    infoLayout->addWidget(batteryTimeLabel);
    infoLayout->addWidget(sleepButton);
    infoLayout->addWidget(hibernateButton);
    infoLayout->addWidget(batteryIconLabel, 0, Qt::AlignCenter);

    layout->addStretch();
    layout->addLayout(infoLayout);
    layout->setContentsMargins(10, 10, 10, 10);
    infoLayout->setContentsMargins(0, 0, 0, 0);

    // Таймер обновления батареи
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &Lab1Window::updateBatteryInfo);
    updateTimer->start(1000);

    // Таймер моргания персонажа
    QTimer *blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, [this]() {
        static bool blink = false;
        blink = !blink;
        const QString basePath = ":/morty/battery/";
        characterLabel->setPixmap(QPixmap(isCharge ?
                                              (blink ? basePath + "charge_blink.png" : basePath + "charge_calm.png") :
                                              (blink ? basePath + "no_charge_blink.png" : basePath + "no_charge_calm.png")));
    });
    blinkTimer->start(500);

    updateBatteryInfo();
}

void Lab1Window::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    QWidget::closeEvent(event);
}

void Lab1Window::updateBatteryInfo()
{
    SYSTEM_POWER_STATUS sps;
    if (!GetSystemPowerStatus(&sps)) return;

    isCharge = (sps.ACLineStatus == 1);
    powerTypeLabel->setText(QString("Источник питания: %1").arg(isCharge ? "Сеть" : "Батарея"));
    batteryLevelLabel->setText(QString("Уровень заряда: %1%").arg(sps.BatteryLifePercent));
    powerSavingModeLabel->setText(QString("Режим энергосбережения: %1").arg(sps.SystemStatusFlag ? "Вкл" : "Выкл"));

    // Тип батареи через WMIC
    auto getBatteryType = []() -> QString {
        QProcess process;
        process.start("cmd.exe", { "/c", "wmic PATH Win32_Battery Get Chemistry /value" });
        process.waitForFinished(-1);
        QString output = process.readAllStandardOutput();
        for (const QString &line : output.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts)) {
            if (line.startsWith("Chemistry=")) {
                QString code = line.section('=', 1, 1).trimmed();
                if (code == "3") return "Lead Acid";
                if (code == "4") return "NiCd";
                if (code == "5") return "NiMH";
                if (code == "6") return "Li-ion";
                if (code == "7") return "Zinc air";
                if (code == "8") return "Li-Polymer";
                return "Другое";
            }
        }
        return "Неизвестно";
    };
    batteryTypeLabel->setText("Тип батареи: " + getBatteryType());

    // Время работы батареи
    if (sps.BatteryLifeTime == 0xFFFFFFFF) {
        batteryTimeLabel->setText(isCharge ? "Оставшееся время работы батареи: Ноутбук заряжается" :
                                      "Оставшееся время работы батареи: Расчёт...");
    } else {
        batteryTimeLabel->setText(QString("Оставшееся время работы батареи: %1 мин").arg(sps.BatteryLifeTime / 60));
    }

    // Определяем функцию получения пути к иконке батареи
    auto getBatteryIcon = [this, sps]() {
        int level = sps.BatteryLifePercent;
        level = qBound(0, level, 100); // ограничиваем 0-100%
        int n = 10 - level / 10;       // 0-9 -> 1-10
        n = qBound(1, n, 10);

        QString base = ":/images/other/";
        QString suffix = isCharge ? "_charge" : "";
        return QString("%1battery%2%3.svg").arg(base).arg(suffix).arg(n);
    };

    // Устанавливаем иконку в QLabel
    QPixmap pix(getBatteryIcon());
    batteryIconLabel->setPixmap(pix);
    batteryIconLabel->setAlignment(Qt::AlignCenter);
    batteryIconLabel->setScaledContents(true);
    batteryIconLabel->setFixedSize(250, 100);
}

void Lab1Window::sleepMode() {
#ifdef Q_OS_WIN
    SetSuspendState(FALSE, FALSE, FALSE); // Hibernate, ForceCritical, DisableWakeEvent
#endif
}

void Lab1Window::hibernateMode() {
#ifdef Q_OS_WIN
    SetSuspendState(TRUE, FALSE, FALSE);
#endif
}

Lab1Window::~Lab1Window() {}
