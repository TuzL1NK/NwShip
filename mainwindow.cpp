#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "widgets/multiselectcombobox.h"

#include <QApplication>
#include <QComboBox>
#include <QIcon>
#include <QFrame>
#include <QLabel>
#include <QPalette>
#include <QSizePolicy>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    if (ui->filterLayout) {
        ui->filterLayout->setContentsMargins(0, 0, 0, 0);
        ui->filterLayout->setColumnStretch(0, 1);
        ui->filterLayout->setColumnStretch(1, 1);
    }
    if (ui->comboBox_whitelistMode) {
        ui->comboBox_whitelistMode->setMinimumWidth(150);
        ui->comboBox_whitelistMode->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    if (ui->widget_whitelist) {
        ui->widget_whitelist->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }
    if (ui->widget_blacklist) {
        ui->widget_blacklist->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }
    setWindowIcon(QIcon(QStringLiteral(":/icons/submarine.ico")));
    setWindowTitle(localizedText(QStringLiteral("NwShip — 潜艇探索助手"), QStringLiteral("NwShip — Submarine Exploration Assistant")));

    applyTheme(m_darkTheme);
    connect(ui->actionEnglish, &QAction::triggered, this, &MainWindow::switchToEnglish);
    connect(ui->actionChinese, &QAction::triggered, this, &MainWindow::switchToChinese);
    loadData();
    applyLanguage(m_language);

    connect(ui->widget_whitelist, &MultiSelectComboBox::selectionChanged,
            this, &MainWindow::onSelectionChanged);
    connect(ui->widget_blacklist, &MultiSelectComboBox::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    connect(ui->widget_whitelist, &MultiSelectComboBox::selectionChanged,
            this, [this]() { ui->widget_whitelist->saveSelection(QStringLiteral("whitelist")); });
    connect(ui->widget_blacklist, &MultiSelectComboBox::selectionChanged,
            this, [this]() { ui->widget_blacklist->saveSelection(QStringLiteral("blacklist")); });

    connect(ui->comboBox_hull, &QComboBox::currentIndexChanged,
            this, &MainWindow::onPartSelectionChanged);
    connect(ui->comboBox_stern, &QComboBox::currentIndexChanged,
            this, &MainWindow::onPartSelectionChanged);
    connect(ui->comboBox_bow, &QComboBox::currentIndexChanged,
            this, &MainWindow::onPartSelectionChanged);
    connect(ui->comboBox_bridge, &QComboBox::currentIndexChanged,
            this, &MainWindow::onPartSelectionChanged);
    connect(ui->spinBox_level, &QSpinBox::editingFinished,
            this, [this]() {
                populatePartCombos();
                updateConfigLabel();
            });

    connect(ui->actionLight_Dark, &QAction::triggered, this, &MainWindow::toggleTheme);
    connect(ui->actionclose_2, &QAction::triggered, this, &QWidget::close);
    connect(ui->pushButton_reset, &QPushButton::clicked, this, &MainWindow::resetFilters);
    connect(ui->pushButton_calculate, &QPushButton::clicked, this, &MainWindow::calculateRoutes);

    ui->widget_whitelist->setPlaceholderText(localizedText(QStringLiteral("选择白名单航点…"), QStringLiteral("Select preferred waypoints…")));
    ui->widget_whitelist->setAccentColor(QColor(QStringLiteral("#2ea86a")));
    ui->widget_blacklist->setPlaceholderText(localizedText(QStringLiteral("选择黑名单航点…"), QStringLiteral("Select blacklisted waypoints…")));
    ui->widget_blacklist->setAccentColor(QColor(QStringLiteral("#c94c4c")));

    m_sortCombo = new QComboBox(this);
    ui->rightLayout->insertWidget(2, m_sortCombo);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSortModeChanged);

    m_resultsScrollArea = new QScrollArea(this);
    m_resultsScrollArea->setWidgetResizable(true);
    m_resultsScrollArea->setFrameShape(QFrame::NoFrame);
    m_resultsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resultsContent = new QWidget(m_resultsScrollArea);
    m_resultsLayout = new QVBoxLayout(m_resultsContent);
    m_resultsLayout->setContentsMargins(0, 0, 0, 0);
    m_resultsLayout->setSpacing(8);
    m_resultsScrollArea->setWidget(m_resultsContent);

    m_resultsEmptyLabel = new QLabel(localizedText(QStringLiteral("还没有路线结果，先点击“开始计算”吧。"), QStringLiteral("No route results yet. Click Calculate first.")), m_resultsContent);
    m_resultsEmptyLabel->setWordWrap(true);
    m_resultsEmptyLabel->setAlignment(Qt::AlignCenter);
    m_resultsEmptyLabel->setStyleSheet("color: #7a7f87; padding: 16px; background: transparent;");

    ui->rightLayout->removeWidget(ui->tableView);
    ui->tableView->hide();
    ui->rightLayout->addWidget(m_resultsScrollArea, 1);

    refreshUiTexts();
    updateConfigLabel();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadData()
{
    const ApplicationDataBundle bundle = loadApplicationData();
    m_explorationPoints = bundle.explorationPoints;
    m_parts = bundle.parts;
    m_ranks = bundle.ranks;

    if (m_explorationPoints.isEmpty()) {
        setStatusMessage(
            QStringLiteral("未能加载航点数据，请将 SubmarineExploration.csv 放在程序目录或项目根目录"),
            QStringLiteral("Failed to load waypoints. Place SubmarineExploration.csv in the app directory or project root."));
    } else {
        ui->widget_whitelist->setExplorationPoints(m_explorationPoints);
        ui->widget_blacklist->setExplorationPoints(m_explorationPoints);
        ui->widget_whitelist->restoreSelection(QStringLiteral("whitelist"));
        ui->widget_blacklist->restoreSelection(QStringLiteral("blacklist"));
    }

    populatePartCombos();

    QStringList messages;
    if (!m_explorationPoints.isEmpty()) {
        messages << localizedText(QStringLiteral("航点 %1 个").arg(m_explorationPoints.size()), QStringLiteral("%1 waypoints").arg(m_explorationPoints.size()));
    }
    if (!m_parts.isEmpty()) {
        messages << localizedText(QStringLiteral("部件 %1 个").arg(m_parts.size()), QStringLiteral("%1 parts").arg(m_parts.size()));
    }
    if (!m_ranks.isEmpty()) {
        messages << localizedText(QStringLiteral("军衔 %1 级").arg(m_ranks.size()), QStringLiteral("%1 ranks").arg(m_ranks.size()));
    }
    if (!messages.isEmpty()) {
        setStatusMessage(messages.join(QStringLiteral("  ·  ")), messages.join(QStringLiteral("  ·  ")));
    } else {
        setStatusMessage(QStringLiteral("未能加载数据文件"), QStringLiteral("Failed to load data files"));
    }
}

void MainWindow::populatePartCombos()
{
    const int level = ui->spinBox_level->value();

    struct SlotTarget {
        int slot;
        QComboBox *combo;
    };

    const SlotTarget targets[] = {
        {0, ui->comboBox_hull},
        {1, ui->comboBox_stern},
        {2, ui->comboBox_bow},
        {3, ui->comboBox_bridge},
    };

    for (const SlotTarget &target : targets) {
        QComboBox *combo = target.combo;
        const int previousId = combo->currentData().toInt();

        combo->blockSignals(true);
        combo->clear();
        combo->addItem(localizedText(QStringLiteral("未选择"), QStringLiteral("Unselected")), 0);

        for (const SubmarinePart &part : m_parts) {
            if (part.slot != target.slot || part.rank > level) {
                continue;
            }
            combo->addItem(partDisplayName(part), part.id);
        }

        const int restoreIndex = combo->findData(previousId);
        combo->setCurrentIndex(restoreIndex >= 0 ? restoreIndex : 0);
        combo->blockSignals(false);
    }
}

void MainWindow::onSelectionChanged()
{
    updateConfigLabel();
}

void MainWindow::onPartSelectionChanged()
{
    updateConfigLabel();
}

void MainWindow::resetFilters()
{
    for (ExplorationPoint &point : m_explorationPoints) {
        point.name = m_language == AppLanguage::English
                         ? (point.nameEn.isEmpty() ? point.nameCn : point.nameEn)
                         : (point.nameCn.isEmpty() ? point.nameEn : point.nameCn);
    }
    ui->widget_whitelist->setExplorationPoints(m_explorationPoints);
    ui->widget_blacklist->setExplorationPoints(m_explorationPoints);
    ui->widget_whitelist->saveSelection(QStringLiteral("whitelist"));
    ui->widget_blacklist->saveSelection(QStringLiteral("blacklist"));
    updateConfigLabel();
}

void MainWindow::updateConfigLabel()
{
    auto partText = [this](QComboBox *combo) -> QString {
        return combo->currentIndex() <= 0 ? localizedText(QStringLiteral("未选"), QStringLiteral("Unselected")) : combo->currentText();
    };

    ui->label_status->setText(localizedText(
        QStringLiteral("白名单 %1 项  ·  黑名单 %2 项  ·  船体：%3 ｜ 船尾：%4 ｜ 船首：%5 ｜ 舰桥：%6")
            .arg(ui->widget_whitelist->getSelectedItems().size())
            .arg(ui->widget_blacklist->getSelectedItems().size())
            .arg(partText(ui->comboBox_hull))
            .arg(partText(ui->comboBox_stern))
            .arg(partText(ui->comboBox_bow))
            .arg(partText(ui->comboBox_bridge)),
        QStringLiteral("Whitelist %1 · Blacklist %2 · Hull: %3 | Stern: %4 | Bow: %5 | Bridge: %6")
            .arg(ui->widget_whitelist->getSelectedItems().size())
            .arg(ui->widget_blacklist->getSelectedItems().size())
            .arg(partText(ui->comboBox_hull))
            .arg(partText(ui->comboBox_stern))
            .arg(partText(ui->comboBox_bow))
            .arg(partText(ui->comboBox_bridge))));
}

void MainWindow::calculateRoutes()
{
    if (m_explorationPoints.isEmpty()) {
        loadData();
    }

    const auto stats = currentSubmarineStats();
    if (!stats) {
        const QString message = localizedText(QStringLiteral("请先完成潜艇配置后再计算航线"), QStringLiteral("Please complete the submarine configuration before calculating routes"));
        setStatusMessage(message, message);
        return;
    }

    if (!stats->canDepart) {
        const QString message = stats->departMessage.isEmpty()
                                    ? localizedText(QStringLiteral("当前配置无法出发"), QStringLiteral("The current configuration cannot depart"))
                                    : stats->departMessage;
        statusBar()->showMessage(message);
        ui->label_status->setText(localizedText(
            QStringLiteral("无法出发：%1  ·  载重 %2/%3  ·  续航 %4")
                .arg(message)
                .arg(stats->totalWeight)
                .arg(stats->capacity)
                .arg(stats->range),
            QStringLiteral("Cannot depart: %1 · Weight %2/%3 · Range %4")
                .arg(message)
                .arg(stats->totalWeight)
                .arg(stats->capacity)
                .arg(stats->range)));
        return;
    }

    const QStringList whitelist = ui->widget_whitelist->getSelectedItems();
    const QStringList blacklist = ui->widget_blacklist->getSelectedItems();
    const RouteWhitelistMode whitelistMode = static_cast<RouteWhitelistMode>(
        ui->comboBox_whitelistMode ? ui->comboBox_whitelistMode->currentData().toInt()
                                  : static_cast<int>(RouteWhitelistMode::Preferred));

    const QVector<ExplorationPoint> candidates = buildRouteCandidates(
        m_explorationPoints,
        ui->spinBox_level->value(),
        whitelist,
        blacklist,
        28,
        whitelistMode);

    RouteSearchOptions options;
    options.level = ui->spinBox_level->value();
    options.stats = *stats;
    options.homePort = homePort();
    for (const ExplorationPoint &point : m_explorationPoints) {
        if (point.startingPoint) {
            options.mapHomePorts.insert(point.mapId, point);
        }
    }
    options.candidates = candidates;
    options.whitelist = whitelist;
    options.whitelistMode = whitelistMode;
    options.maxPoints = 5;
    options.maxResults = 200;

    m_lastRoutes = findBestRoutes(options);

    const int sortModeValue = m_sortCombo ? m_sortCombo->currentData().toInt() : static_cast<int>(RouteSortMode::Efficiency);
    sortRoutes(m_lastRoutes, static_cast<RouteSortMode>(sortModeValue));

    displayRoutes(m_lastRoutes);

    const QString configSummary = localizedText(
        QStringLiteral("可出发 · 等级 %1 · 载重 %2/%3 · 航速 %4 · 续航 %5")
            .arg(stats->level)
            .arg(stats->totalWeight)
            .arg(stats->capacity)
            .arg(stats->speed)
            .arg(stats->range),
        QStringLiteral("Ready to depart · Level %1 · Weight %2/%3 · Speed %4 · Range %5")
            .arg(stats->level)
            .arg(stats->totalWeight)
            .arg(stats->capacity)
            .arg(stats->speed)
            .arg(stats->range));

    if (m_lastRoutes.isEmpty()) {
        const QString message = localizedText(QStringLiteral("未找到满足当前条件的航线"), QStringLiteral("No route matched the current conditions"));
        setStatusMessage(message, message);
        ui->label_status->setText(QStringLiteral("%1  ·  %2").arg(configSummary).arg(message));
    } else {
        const QString message = localizedText(QStringLiteral("已找到 %1 条可行航线"), QStringLiteral("Found %1 viable routes")).arg(m_lastRoutes.size());
        setStatusMessage(message, message);
        ui->label_status->setText(QStringLiteral("%1  ·  %2").arg(configSummary).arg(message));
    }
}

void MainWindow::onSortModeChanged(int index)
{
    if (m_lastRoutes.isEmpty()) {
        return;
    }

    const int modeValue = m_sortCombo ? m_sortCombo->itemData(index).toInt() : 0;
    const RouteSortMode mode = static_cast<RouteSortMode>(modeValue);
    sortRoutes(m_lastRoutes, mode);
    displayRoutes(m_lastRoutes);
}

void MainWindow::displayRoutes(const QVector<RoutePlan> &routes)
{
    if (!m_resultsLayout) {
        return;
    }

    while (QLayoutItem *item = m_resultsLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    if (routes.isEmpty()) {
        m_resultsEmptyLabel->setStyleSheet(QString(
            "color: %1; padding: 16px; background: transparent;"
        ).arg(m_darkTheme ? QStringLiteral("#a8b0bc") : QStringLiteral("#64748b")));
        m_resultsLayout->addWidget(m_resultsEmptyLabel);
        return;
    }

    for (int row = 0; row < routes.size(); ++row) {
        const RoutePlan &route = routes.at(row);
        QString routeText;
        for (int i = 0; i < route.legs.size(); ++i) {
            if (i > 0) {
                routeText += QStringLiteral(" → ");
            }
            routeText += route.legs.at(i).name;
        }

        const bool hasWhitelistMatch = route.whitelistMatchCount > 0;
        const QString accentColor = hasWhitelistMatch
                                         ? (m_darkTheme ? QStringLiteral("#34d399") : QStringLiteral("#16a34a"))
                                         : (m_darkTheme ? QStringLiteral("#5a9cf5") : QStringLiteral("#3d8bfd"));
        const QString cardBackground = hasWhitelistMatch
                                          ? (m_darkTheme ? QStringLiteral("#1f2d24") : QStringLiteral("#f0fdf4"))
                                          : (m_darkTheme ? QStringLiteral("#24262b") : QStringLiteral("#ffffff"));
        const QString cardBorder = hasWhitelistMatch
                                      ? accentColor
                                      : (m_darkTheme ? QStringLiteral("#3b3f46") : QStringLiteral("#d6dbe4"));

        auto *card = new QFrame(m_resultsContent);
        card->setObjectName(QStringLiteral("routeCard"));
        card->setFrameShape(QFrame::StyledPanel);
        card->setCursor(Qt::PointingHandCursor);
        card->setToolTip(routeText);
        card->setStyleSheet(QString(
            "QFrame#routeCard {"
            "  background: %1;"
            "  border: 1px solid %2;"
            "  border-radius: 10px;"
            "  padding: 8px;"
            "}"
            "QFrame#routeCard:hover { border-color: %3; }"
        )
            .arg(cardBackground)
            .arg(cardBorder)
            .arg(accentColor));

        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 10, 12, 10);
        cardLayout->setSpacing(8);

        const QString titleText = localizedText(QStringLiteral("路线 %1").arg(row + 1), QStringLiteral("Route %1").arg(row + 1))
                                      + (hasWhitelistMatch
                                             ? localizedText(QStringLiteral(" · 白名单命中 %1").arg(route.whitelistMatchCount), QStringLiteral(" · Whitelist hit %1").arg(route.whitelistMatchCount))
                                             : QString());
        auto *titleLabel = new QLabel(titleText, card);
        titleLabel->setStyleSheet(QString("font-weight: bold; color: %1;")
                                      .arg(m_darkTheme ? QStringLiteral("#f8fafc") : QStringLiteral("#111827")));

        auto *routeLabel = new QLabel(routeText, card);
        routeLabel->setWordWrap(true);
        routeLabel->setStyleSheet(QString("color: %1; line-height: 1.4;")
                                     .arg(m_darkTheme ? QStringLiteral("#e5e7eb") : QStringLiteral("#0f172a")));
        routeLabel->setToolTip(routeText);

        QString whitelistHint = localizedText(QStringLiteral("未命中白名单"), QStringLiteral("No whitelist hit"));
        if (hasWhitelistMatch) {
            whitelistHint = localizedText(QStringLiteral("白名单点位：%1").arg(route.whitelistMatches.join(QStringLiteral(" / "))), QStringLiteral("Whitelist points: %1").arg(route.whitelistMatches.join(QStringLiteral(" / "))));
        }

        auto *whitelistLabel = new QLabel(whitelistHint, card);
        whitelistLabel->setWordWrap(true);
        whitelistLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600;")
                                           .arg(hasWhitelistMatch
                                                    ? (m_darkTheme ? QStringLiteral("#4ade80") : QStringLiteral("#15803d"))
                                                    : (m_darkTheme ? QStringLiteral("#94a3b8") : QStringLiteral("#64748b"))));

        auto *metricsLayout = new QHBoxLayout();
        metricsLayout->setSpacing(8);
        metricsLayout->addWidget(new QLabel(localizedText(QStringLiteral("经验 %1").arg(qRound(route.totalExp)), QStringLiteral("Exp %1").arg(qRound(route.totalExp))), card));
        metricsLayout->addWidget(new QLabel(localizedText(QStringLiteral("时长 %1").arg(formatDurationMinutes(route.totalTimeMin)), QStringLiteral("Time %1").arg(formatDurationMinutes(route.totalTimeMin))), card));
        metricsLayout->addWidget(new QLabel(localizedText(QStringLiteral("效率 %1").arg(QString::number(route.efficiency, 'f', 2)), QStringLiteral("Efficiency %1").arg(QString::number(route.efficiency, 'f', 2))), card));
        metricsLayout->addWidget(new QLabel(localizedText(QStringLiteral("剩余续航 %1").arg(qRound(route.remainingRange)), QStringLiteral("Remaining range %1").arg(qRound(route.remainingRange))), card));
        metricsLayout->addStretch(1);

        for (int i = 0; i < metricsLayout->count(); ++i) {
            if (QLabel *metricLabel = qobject_cast<QLabel *>(metricsLayout->itemAt(i)->widget())) {
                metricLabel->setStyleSheet(QString("color: %1; font-size: 12px;")
                                               .arg(m_darkTheme ? QStringLiteral("#9aa4b2") : QStringLiteral("#475569")));
            }
        }

        cardLayout->addWidget(titleLabel);
        cardLayout->addWidget(routeLabel);
        cardLayout->addWidget(whitelistLabel);
        cardLayout->addLayout(metricsLayout);
        m_resultsLayout->addWidget(card);
    }
}

std::optional<SubmarineStats> MainWindow::currentSubmarineStats() const
{
    if (m_ranks.isEmpty()) {
        return std::nullopt;
    }

    const auto rank = rankForLevel(m_ranks, ui->spinBox_level->value());
    if (!rank) {
        return std::nullopt;
    }

    const auto hullPart = partById(m_parts, selectedPartId(ui->comboBox_hull));
    const auto sternPart = partById(m_parts, selectedPartId(ui->comboBox_stern));
    const auto bowPart = partById(m_parts, selectedPartId(ui->comboBox_bow));
    const auto bridgePart = partById(m_parts, selectedPartId(ui->comboBox_bridge));

    return computeSubmarineStats(ui->spinBox_level->value(),
                                 *rank,
                                 hullPart ? &*hullPart : nullptr,
                                 sternPart ? &*sternPart : nullptr,
                                 bowPart ? &*bowPart : nullptr,
                                 bridgePart ? &*bridgePart : nullptr);
}

ExplorationPoint MainWindow::homePort() const
{
    for (const ExplorationPoint &point : m_explorationPoints) {
        if (point.startingPoint) {
            return point;
        }
    }
    return {};
}

int MainWindow::selectedPartId(QComboBox *combo) const
{
    return combo ? combo->currentData().toInt() : 0;
}

void MainWindow::toggleTheme()
{
    m_darkTheme = !m_darkTheme;
    applyTheme(m_darkTheme);
}

void MainWindow::switchToChinese()
{
    applyLanguage(AppLanguage::Chinese);
}

void MainWindow::switchToEnglish()
{
    applyLanguage(AppLanguage::English);
}

void MainWindow::applyLanguage(AppLanguage language)
{
    m_language = language;
    refreshUiTexts();
    refreshExplorationNames();
    populatePartCombos();
    if (ui->widget_whitelist) {
        ui->widget_whitelist->setExplorationPoints(m_explorationPoints);
        ui->widget_whitelist->restoreSelection(QStringLiteral("whitelist"));
    }
    if (ui->widget_blacklist) {
        ui->widget_blacklist->setExplorationPoints(m_explorationPoints);
        ui->widget_blacklist->restoreSelection(QStringLiteral("blacklist"));
    }
    updateConfigLabel();
    if (!m_lastRoutes.isEmpty()) {
        displayRoutes(m_lastRoutes);
    }
}

void MainWindow::setStatusMessage(const QString &zh, const QString &en)
{
    m_statusMessageZh = zh;
    m_statusMessageEn = en;
    statusBar()->showMessage(m_language == AppLanguage::English ? m_statusMessageEn : m_statusMessageZh);
}

QString MainWindow::localizedText(const QString &zh, const QString &en) const
{
    return m_language == AppLanguage::English ? en : zh;
}

QString MainWindow::partDisplayName(const SubmarinePart &part) const
{
    if (m_language == AppLanguage::English && !part.nameEn.isEmpty()) {
        return part.nameEn;
    }
    if (!part.nameCn.isEmpty()) {
        return part.nameCn;
    }
    return part.name;
}

void MainWindow::refreshExplorationNames()
{
    for (ExplorationPoint &point : m_explorationPoints) {
        if (m_language == AppLanguage::English) {
            point.name = point.nameEn.isEmpty() ? point.nameCn : point.nameEn;
        } else {
            point.name = point.nameCn.isEmpty() ? point.nameEn : point.nameCn;
        }
    }
}

void MainWindow::refreshUiTexts()
{
    ui->groupBox_config->setTitle(localizedText(QStringLiteral("潜艇配置"), QStringLiteral("Submarine Configuration")));
    ui->label_level->setText(localizedText(QStringLiteral("潜水艇等级"), QStringLiteral("Submarine Level")));
    ui->label_hull->setText(localizedText(QStringLiteral("船体"), QStringLiteral("Hull")));
    ui->label_stern->setText(localizedText(QStringLiteral("船尾"), QStringLiteral("Stern")));
    ui->label_bow->setText(localizedText(QStringLiteral("船首"), QStringLiteral("Bow")));
    ui->label_bridge->setText(localizedText(QStringLiteral("舰桥"), QStringLiteral("Bridge")));
    ui->pushButton_calculate->setText(localizedText(QStringLiteral("开始计算"), QStringLiteral("Calculate")));
    ui->pushButton_reset->setText(localizedText(QStringLiteral("重置"), QStringLiteral("Reset")));
    ui->groupBox_filters->setTitle(localizedText(QStringLiteral("航点筛选"), QStringLiteral("Waypoint Filters")));
    ui->label_whitelist_title->setText(localizedText(QStringLiteral("白名单（优先探索）"), QStringLiteral("Whitelist (Priority)")));
    ui->label_blacklist_title->setText(localizedText(QStringLiteral("黑名单（排除）"), QStringLiteral("Blacklist (Excluded)")));
    ui->label_status->setText(localizedText(QStringLiteral("白名单 0 项  ·  黑名单 0 项"), QStringLiteral("Whitelist 0 · Blacklist 0")));
    ui->menuoptions->setTitle(localizedText(QStringLiteral("选项"), QStringLiteral("Options")));
    ui->menuLanguages->setTitle(m_language == AppLanguage::Chinese ? QStringLiteral("Languages") : QStringLiteral("语言"));
    ui->actionLight_Dark->setText(localizedText(QStringLiteral("切换浅色主题"), QStringLiteral("Switch to light theme")));
    ui->actionEnglish->setText(QStringLiteral("English"));
    ui->actionChinese->setText(QStringLiteral("中文"));
    ui->actionclose_2->setText(localizedText(QStringLiteral("退出"), QStringLiteral("Exit")));
    ui->actionLight_Dark->setText(m_darkTheme ? localizedText(QStringLiteral("切换浅色主题"), QStringLiteral("Switch to light theme"))
                                             : localizedText(QStringLiteral("切换深色主题"), QStringLiteral("Switch to dark theme")));
    setWindowTitle(localizedText(QStringLiteral("NwShip — 潜艇探索助手"), QStringLiteral("NwShip — Submarine Exploration Assistant")));
    if (!m_statusMessageZh.isEmpty() || !m_statusMessageEn.isEmpty()) {
        statusBar()->showMessage(m_language == AppLanguage::English ? m_statusMessageEn : m_statusMessageZh);
    }
    ui->widget_whitelist->setPlaceholderText(localizedText(QStringLiteral("选择白名单航点…"), QStringLiteral("Select preferred waypoints…")));
    ui->widget_blacklist->setPlaceholderText(localizedText(QStringLiteral("选择黑名单航点…"), QStringLiteral("Select blacklisted waypoints…")));
    ui->widget_whitelist->setAccentColor(QColor(QStringLiteral("#2ea86a")));
    ui->widget_blacklist->setAccentColor(QColor(QStringLiteral("#c94c4c")));
    if (ui->comboBox_whitelistMode) {
        ui->comboBox_whitelistMode->setToolTip(localizedText(QStringLiteral("白名单策略：优先模式会保留所有结果并把命中路线排前面；严格模式只保留命中白名单的路线"), QStringLiteral("Whitelist strategy: Priority keeps all results and ranks matched routes first; Strict only keeps routes that hit the whitelist")));
    }

    if (ui->comboBox_whitelistMode) {
        ui->comboBox_whitelistMode->blockSignals(true);
        ui->comboBox_whitelistMode->clear();
        ui->comboBox_whitelistMode->addItem(localizedText(QStringLiteral("优先模式"), QStringLiteral("Priority mode")), static_cast<int>(RouteWhitelistMode::Preferred));
        ui->comboBox_whitelistMode->addItem(localizedText(QStringLiteral("严格模式"), QStringLiteral("Strict mode")), static_cast<int>(RouteWhitelistMode::Strict));
        ui->comboBox_whitelistMode->setCurrentIndex(0);
        ui->comboBox_whitelistMode->blockSignals(false);
    }

    if (m_sortCombo) {
        m_sortCombo->blockSignals(true);
        m_sortCombo->clear();
        m_sortCombo->addItem(localizedText(QStringLiteral("经验效率优先"), QStringLiteral("Experience efficiency first")), static_cast<int>(RouteSortMode::Efficiency));
        m_sortCombo->addItem(localizedText(QStringLiteral("总经验优先"), QStringLiteral("Total experience first")), static_cast<int>(RouteSortMode::TotalExp));
        m_sortCombo->addItem(localizedText(QStringLiteral("时间最短优先"), QStringLiteral("Shortest time first")), static_cast<int>(RouteSortMode::ShortestTime));
        m_sortCombo->setCurrentIndex(0);
        m_sortCombo->blockSignals(false);
    }

    if (m_resultsEmptyLabel) {
        m_resultsEmptyLabel->setText(localizedText(QStringLiteral("还没有路线结果，先点击“开始计算”吧。"), QStringLiteral("No route results yet. Click Calculate first.")));
    }
}

void MainWindow::applyTheme(bool dark)
{
    const QString windowBg = dark ? QStringLiteral("#1e1f22") : QStringLiteral("#f5f6f8");
    const QString panelBg = dark ? QStringLiteral("#25262a") : QStringLiteral("#ffffff");
    const QString borderColor = dark ? QStringLiteral("#3a3d42") : QStringLiteral("#d0d4da");
    const QString textColor = dark ? QStringLiteral("#e8e8e8") : QStringLiteral("#222222");
    const QString mutedText = dark ? QStringLiteral("#b8c0cc") : QStringLiteral("#475569");
    const QString accent = dark ? QStringLiteral("#3d8bfd") : QStringLiteral("#2563eb");
    const QString accentHover = dark ? QStringLiteral("#5a9cf5") : QStringLiteral("#3b82f6");

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(windowBg));
    palette.setColor(QPalette::WindowText, QColor(textColor));
    palette.setColor(QPalette::Base, QColor(panelBg));
    palette.setColor(QPalette::AlternateBase, dark ? QColor("#32353a") : QColor("#f9fafb"));
    palette.setColor(QPalette::Text, QColor(textColor));
    palette.setColor(QPalette::Button, QColor(accent));
    palette.setColor(QPalette::ButtonText, QColor("#ffffff"));
    palette.setColor(QPalette::Highlight, QColor(accent));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    qApp->setPalette(palette);

    setStyleSheet(QString(
        "QMainWindow { background: %1; }"
        "QWidget#centralwidget { background: %1; color: %2; }"
        "QGroupBox {"
        "  font-weight: bold; font-size: 13px; color: %3;"
        "  border: 1px solid %4; border-radius: 8px;"
        "  margin-top: 14px; padding-top: 12px; background: %5;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin; left: 12px; padding: 0 6px; color: %6;"
        "}"
        "QLabel { color: %2; background: transparent; }"
        "QLabel#label_status {"
        "  background: %5; border: 1px solid %4;"
        "  border-radius: 6px; padding: 8px 12px; color: %3;"
        "}"
        "QLabel#label_whitelist_title { color: %7; font-weight: bold; }"
        "QLabel#label_blacklist_title { color: %8; font-weight: bold; }"
        "QSpinBox, QComboBox {"
        "  background: %5; color: %2; border: 1px solid %4;"
        "  border-radius: 6px; padding: 4px 8px; min-height: 26px;"
        "}"
        "QSpinBox:focus, QComboBox:focus { border-color: %9; }"
        "QSpinBox::up-button, QSpinBox::down-button {"
        "  background: %4; border: none; width: 18px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background: %5; color: %2; border: 1px solid %4;"
        "  selection-background-color: %9;"
        "}"
        "QPushButton {"
        "  background: %9; color: white; border: none;"
        "  border-radius: 6px; padding: 8px 16px; font-weight: bold;"
        "}"
        "QPushButton:hover { background: %10; }"
        "QPushButton:pressed { background: %11; }"
        "QPushButton#pushButton_reset {"
        "  background: %4; color: %2;"
        "}"
        "QPushButton#pushButton_reset:hover { background: %12; }"
        "QTableView {"
        "  background: %5; alternate-background-color: %13;"
        "  color: %2; gridline-color: %4;"
        "  border: 1px solid %4; border-radius: 6px;"
        "  selection-background-color: %9;"
        "}"
        "QHeaderView::section {"
        "  background: %14; color: %15; padding: 6px;"
        "  border: none; border-bottom: 1px solid %4; font-weight: bold;"
        "}"
        "QMenuBar { background: %5; color: %2; }"
        "QMenuBar::item:selected { background: %4; }"
        "QMenu { background: %5; color: %2; border: 1px solid %4; }"
        "QMenu::item:selected { background: %9; }"
        "QStatusBar { background: %5; color: %16; }"
    )
        .arg(windowBg)
        .arg(textColor)
        .arg(mutedText)
        .arg(borderColor)
        .arg(panelBg)
        .arg(dark ? QStringLiteral("#9ecbff") : QStringLiteral("#2563eb"))
        .arg(dark ? QStringLiteral("#6fcf97") : QStringLiteral("#059669"))
        .arg(dark ? QStringLiteral("#eb8e8e") : QStringLiteral("#dc2626"))
        .arg(accent)
        .arg(accentHover)
        .arg(dark ? QStringLiteral("#2d7ae0") : QStringLiteral("#1d4ed8"))
        .arg(dark ? QStringLiteral("#4a4d52") : QStringLiteral("#e5e7eb"))
        .arg(dark ? QStringLiteral("#32353a") : QStringLiteral("#f9fafb"))
        .arg(dark ? QStringLiteral("#32353a") : QStringLiteral("#f3f4f6"))
        .arg(dark ? QStringLiteral("#b0b0b0") : QStringLiteral("#6b7280"))
        .arg(dark ? QStringLiteral("#888888") : QStringLiteral("#64748b")));

    if (m_resultsScrollArea) {
        m_resultsScrollArea->setStyleSheet(QString(
            "QScrollArea { background: transparent; border: 1px solid %1; border-radius: 8px; }"
            "QScrollBar:vertical { background: transparent; width: 8px; }"
            "QScrollBar::handle:vertical { background: %2; border-radius: 4px; min-height: 24px; }"
            "QScrollBar::handle:vertical:hover { background: %3; }"
        )
            .arg(borderColor)
            .arg(dark ? QStringLiteral("#5b5f67") : QStringLiteral("#cbd5e1"))
            .arg(dark ? QStringLiteral("#7a7f87") : QStringLiteral("#94a3b8")));
    }

    if (ui->widget_whitelist) {
        ui->widget_whitelist->applyTheme(dark);
    }
    if (ui->widget_blacklist) {
        ui->widget_blacklist->applyTheme(dark);
    }

    if (m_resultsLayout) {
        displayRoutes(m_lastRoutes);
    }

    ui->actionLight_Dark->setText(dark ? QStringLiteral("切换浅色主题")
                                       : QStringLiteral("切换深色主题"));
}
