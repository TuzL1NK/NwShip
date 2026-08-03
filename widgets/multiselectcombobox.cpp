#include "multiselectcombobox.h"

#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QAbstractItemView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QApplication>
#include <QResizeEvent>
#include <QBrush>
#include <QFont>
#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QSettings>

TagWidget::TagWidget(const QString &text, const QColor &accent, QWidget *parent)
    : QFrame(parent)
{
    setStyleSheet(QString(
        "QFrame { background-color: %1; border-radius: 12px; }"
        "QLabel { color: white; font-size: 12px; background: transparent; padding: 0; }"
        "QPushButton#tagRemoveBtn {"
        "  background: transparent; color: #e53935; border: none;"
        "  font-weight: bold; font-size: 15px; min-width: 18px; max-width: 18px;"
        "  padding: 0;"
        "}"
        "QPushButton#tagRemoveBtn:hover {"
        "  color: #ff5252; background: rgba(229, 57, 53, 0.18); border-radius: 9px;"
        "}"
    ).arg(accent.name()));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 4, 4, 4);
    layout->setSpacing(2);

    auto *label = new QLabel(text, this);
    auto *closeBtn = new QPushButton(QStringLiteral("×"), this);
    closeBtn->setObjectName(QStringLiteral("tagRemoveBtn"));
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setFixedSize(18, 18);
    closeBtn->setToolTip(QStringLiteral("移除"));

    layout->addWidget(label);
    layout->addWidget(closeBtn);
    adjustSize();

    connect(closeBtn, &QPushButton::clicked, this, [this, text]() {
        emit removeRequested(text);
    });
}

CustomMultiComboBox::CustomMultiComboBox(QWidget *parent)
    : QComboBox(parent)
{
}

void CustomMultiComboBox::showPopup()
{
    QComboBox::showPopup();
    if (auto *popup = findChild<QFrame *>()) {
        popup->setAttribute(Qt::WA_ShowWithoutActivating, true);
    }
}

MultiSelectComboBox::MultiSelectComboBox(QWidget *parent)
    : QWidget(parent)
    , m_accentColor(QStringLiteral("#3d8bfd"))
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(8);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setFrameShape(QFrame::StyledPanel);
    m_scrollArea->setFixedHeight(120);

    m_tagContainer = new QWidget(m_scrollArea);
    m_tagContainer->setMinimumHeight(0);
    m_scrollArea->setWidget(m_tagContainer);

    m_comboBox = new CustomMultiComboBox(this);
    m_comboBox->setModel(new QStandardItemModel(m_comboBox));
    m_comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_areaButton = new QPushButton(this);
    m_areaButton->setText(QStringLiteral("海域"));
    m_areaButton->setFixedWidth(48);
    m_areaButton->setCursor(Qt::PointingHandCursor);
    m_areaButton->setToolTip(QStringLiteral("按海域批量选择"));

    m_clearButton = new QPushButton(this);
    m_clearButton->setText(QStringLiteral("清除"));
    m_clearButton->setFixedWidth(48);
    m_clearButton->setCursor(Qt::PointingHandCursor);
    m_clearButton->setToolTip(QStringLiteral("一键清除所有选择"));

    auto *bottomRow = new QHBoxLayout();
    bottomRow->setContentsMargins(0, 0, 0, 0);
    bottomRow->setSpacing(6);
    bottomRow->addWidget(m_comboBox, 1);
    bottomRow->addWidget(m_areaButton, 0);
    bottomRow->addWidget(m_clearButton, 0);

    m_comboBox->view()->viewport()->installEventFilter(this);
    qApp->installEventFilter(this);

    m_mainLayout->addWidget(m_scrollArea, 1);
    m_mainLayout->addLayout(bottomRow);

    connect(m_comboBox->model(), &QAbstractItemModel::dataChanged,
            this, &MultiSelectComboBox::onSelectionChanged);
    connect(m_areaButton, &QPushButton::clicked, this, &MultiSelectComboBox::refreshAreaMenu);
    connect(m_clearButton, &QPushButton::clicked, this, &MultiSelectComboBox::clearAll);

    styleComboBox();
}

MultiSelectComboBox::~MultiSelectComboBox()
{
    qApp->removeEventFilter(this);
}

void MultiSelectComboBox::setPlaceholderText(const QString &text)
{
    m_comboBox->setPlaceholderText(text);
}

void MultiSelectComboBox::setAccentColor(const QColor &color)
{
    m_accentColor = color;
    reflowTags();
}

void MultiSelectComboBox::styleComboBox()
{
    applyTheme(m_darkTheme);
}

void MultiSelectComboBox::applyTheme(bool dark)
{
    m_darkTheme = dark;
    const QString bg = dark ? QStringLiteral("#2b2d30") : QStringLiteral("#ffffff");
    const QString border = dark ? QStringLiteral("#45484c") : QStringLiteral("#d0d4da");
    const QString text = dark ? QStringLiteral("#e8e8e8") : QStringLiteral("#222222");
    const QString muted = dark ? QStringLiteral("#9aa4b2") : QStringLiteral("#64748b");
    const QString accent = dark ? QStringLiteral("#3d8bfd") : QStringLiteral("#2563eb");

    m_scrollArea->setStyleSheet(QString(
        "QScrollArea { background: %1; border: 1px solid %2; border-radius: 6px; }"
        "QScrollBar:vertical { width: 6px; background: transparent; margin: 2px; }"
        "QScrollBar::handle:vertical { background: %3; border-radius: 3px; min-height: 24px; }"
        "QScrollBar::handle:vertical:hover { background: %4; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    ).arg(bg).arg(border).arg(dark ? QStringLiteral("#5a5d61") : QStringLiteral("#cbd5e1")).arg(dark ? QStringLiteral("#7a7d81") : QStringLiteral("#94a3b8")));

    m_comboBox->setStyleSheet(QString(
        "QComboBox {"
        "  background: %1; color: %2; border: 1px solid %3;"
        "  border-radius: 6px; padding: 6px 10px; min-height: 28px;"
        "}"
        "QComboBox:hover { border-color: %4; }"
        "QComboBox::drop-down { border: none; width: 24px; }"
        "QComboBox::down-arrow {"
        "  image: none;"
        "  border-left: 5px solid transparent;"
        "  border-right: 5px solid transparent;"
        "  border-top: 6px solid %5;"
        "  margin-right: 8px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background: %1; color: %2; border: 1px solid %3;"
        "  selection-background-color: %4; outline: none; padding: 4px;"
        "}"
    ).arg(bg).arg(text).arg(border).arg(accent).arg(muted));

    m_areaButton->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1; color: %2; border: 1px solid %3;"
        "  border-radius: 6px; padding: 4px 8px; font-size: 12px;"
        "}"
        "QPushButton:hover { border-color: %4; background: %5; }"
    ).arg(bg).arg(text).arg(border).arg(accent).arg(dark ? QStringLiteral("#3a3d42") : QStringLiteral("#f1f5f9")));

    m_clearButton->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1; color: %2; border: 1px solid %3;"
        "  border-radius: 6px; padding: 4px 8px; font-size: 12px;"
        "}"
        "QPushButton:hover { color: #e53935; border-color: #e53935; background: %4; }"
    ).arg(bg).arg(text).arg(border).arg(dark ? QStringLiteral("#3a1f1f") : QStringLiteral("#fef2f2")));

    if (!m_cachedSelection.isEmpty()) {
        reflowTags();
    }
}

void MultiSelectComboBox::appendMapSeparator(int mapId)
{
    auto *model = qobject_cast<QStandardItemModel *>(m_comboBox->model());
    if (!model) {
        return;
    }

    auto *separator = new QStandardItem(
        QStringLiteral("—— %1 ——").arg(mapDisplayName(mapId)));
    separator->setSelectable(false);
    separator->setEnabled(false);
    separator->setCheckable(false);
    separator->setFlags(Qt::ItemIsEnabled);
    separator->setBackground(QBrush(QColor(QStringLiteral("#1e2023"))));
    separator->setForeground(QBrush(QColor(QStringLiteral("#9ecbff"))));

    QFont font = separator->font();
    font.setBold(true);
    separator->setFont(font);

    model->appendRow(separator);
}

bool MultiSelectComboBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_comboBox->view()->viewport()
        && event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        const QModelIndex index = m_comboBox->view()->indexAt(mouseEvent->pos());
        if (index.isValid()) {
            auto *model = qobject_cast<QStandardItemModel *>(m_comboBox->model());
            if (QStandardItem *item = model->itemFromIndex(index)) {
                if (item->isCheckable() && item->isEnabled()) {
                    const bool shiftHeld = mouseEvent->modifiers() & Qt::ShiftModifier;
                    if (shiftHeld && m_lastClickedIndex.isValid()) {
                        // Shift range selection: check all items between last and current
                        const int from = qMin(m_lastClickedIndex.row(), index.row());
                        const int to = qMax(m_lastClickedIndex.row(), index.row());
                        model->blockSignals(true);
                        for (int i = from; i <= to; ++i) {
                            QStandardItem *rangeItem = model->item(i);
                            if (rangeItem && rangeItem->isCheckable()) {
                                rangeItem->setCheckState(Qt::Checked);
                            }
                        }
                        model->blockSignals(false);
                        // Trigger onSelectionChanged to refresh tags and save
                        m_cachedSelection = getSelectedItems();
                        reflowTags();
                        emit selectionChanged(m_cachedSelection);
                    } else {
                        item->setCheckState(item->checkState() == Qt::Checked
                                                ? Qt::Unchecked
                                                : Qt::Checked);
                        m_lastClickedIndex = index;
                    }
                    return true;
                }
            }
        }
    }

    if (event->type() == QEvent::Wheel && m_comboBox->view()->isVisible()) {
        auto *wheelEvent = static_cast<QWheelEvent *>(event);
        const QPoint localPos = m_scrollArea->mapFromGlobal(
            wheelEvent->globalPosition().toPoint());
        if (m_scrollArea->rect().contains(localPos)) {
            QCoreApplication::sendEvent(m_scrollArea->verticalScrollBar(), event);
            return true;
        }
    }

    if (event->type() == QEvent::MouseButtonPress && m_comboBox->view()->isVisible()) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        const QList<TagWidget *> tags = m_tagContainer->findChildren<TagWidget *>();
        for (TagWidget *tag : tags) {
            if (auto *btn = tag->findChild<QPushButton *>(QStringLiteral("tagRemoveBtn"))) {
                const QPoint btnLocalPos = btn->mapFromGlobal(
                    mouseEvent->globalPosition().toPoint());
                if (btn->rect().contains(btnLocalPos)) {
                    btn->click();
                    return true;
                }
            }
        }
    }

    return false;
}

void MultiSelectComboBox::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!m_cachedSelection.isEmpty()) {
        reflowTags();
    }
}

void MultiSelectComboBox::setExplorationPoints(const QList<ExplorationPoint> &points)
{
    auto *model = qobject_cast<QStandardItemModel *>(m_comboBox->model());
    if (!model) {
        return;
    }

    model->clear();
    m_mapIds.clear();
    model->blockSignals(true);

    int currentMap = -1;
    for (const ExplorationPoint &point : points) {
        if (point.startingPoint || point.name.isEmpty()) {
            continue;
        }
        if (point.mapId != currentMap) {
            appendMapSeparator(point.mapId);
            currentMap = point.mapId;
            if (!m_mapIds.contains(point.mapId)) {
                m_mapIds.append(point.mapId);
            }
        }

        auto *item = new QStandardItem(point.name);
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        item->setData(point.mapId, Qt::UserRole);
        model->appendRow(item);
    }

    model->blockSignals(false);
}

QStringList MultiSelectComboBox::getSelectedItems() const
{
    QStringList selected;
    const auto *model = qobject_cast<QStandardItemModel *>(m_comboBox->model());
    if (!model) {
        return selected;
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        const QStandardItem *item = model->item(i);
        if (item && item->isCheckable() && item->checkState() == Qt::Checked) {
            selected.append(item->text());
        }
    }
    return selected;
}

void MultiSelectComboBox::reflowTags()
{
    const QList<QWidget *> children = m_tagContainer->findChildren<QWidget *>(
        QString(), Qt::FindDirectChildrenOnly);
    for (QWidget *child : children) {
        child->setParent(nullptr);
        child->deleteLater();
    }

    constexpr int leftMargin = 6;
    constexpr int topMargin = 6;
    constexpr int hSpacing = 6;
    constexpr int vSpacing = 6;

    int currentX = leftMargin;
    int currentY = topMargin;
    int lineHeight = 0;

    int maxWidth = m_tagContainer->width() - leftMargin * 2;
    if (maxWidth < 120) {
        maxWidth = qMax(120, width() - 24);
    }

    for (const QString &text : m_cachedSelection) {
        auto *tag = new TagWidget(text, m_accentColor, m_tagContainer);
        tag->show();

        const int tagWidth = tag->sizeHint().width();
        const int tagHeight = tag->sizeHint().height();
        lineHeight = qMax(lineHeight, tagHeight);

        if (currentX + tagWidth > maxWidth && currentX > leftMargin) {
            currentX = leftMargin;
            currentY += lineHeight + vSpacing;
            lineHeight = tagHeight;
        }

        tag->setGeometry(currentX, currentY, tagWidth, tagHeight);
        currentX += tagWidth + hSpacing;

        connect(tag, &TagWidget::removeRequested,
                this, &MultiSelectComboBox::onTagRemoveRequested);
    }

    const int totalHeight = currentY + (m_cachedSelection.isEmpty() ? 0 : lineHeight) + topMargin;
    m_tagContainer->setMinimumHeight(qMax(totalHeight, 0));
}

void MultiSelectComboBox::onSelectionChanged()
{
    m_comboBox->model()->blockSignals(true);
    m_cachedSelection = getSelectedItems();
    reflowTags();
    m_comboBox->model()->blockSignals(false);
    emit selectionChanged(m_cachedSelection);
}

void MultiSelectComboBox::onTagRemoveRequested(const QString &text)
{
    auto *model = qobject_cast<QStandardItemModel *>(m_comboBox->model());
    if (!model) {
        return;
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *item = model->item(i);
        if (item && item->isCheckable() && item->text() == text) {
            item->setCheckState(Qt::Unchecked);
            break;
        }
    }
}

void MultiSelectComboBox::selectByMapId(int mapId)
{
    auto *model = qobject_cast<QStandardItemModel *>(m_comboBox->model());
    if (!model) {
        return;
    }

    model->blockSignals(true);
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *item = model->item(i);
        if (!item || !item->isCheckable()) {
            continue;
        }
        if (item->data(Qt::UserRole).toInt() == mapId) {
            item->setCheckState(Qt::Checked);
        }
    }
    model->blockSignals(false);

    m_lastClickedIndex = QPersistentModelIndex();
    m_cachedSelection = getSelectedItems();
    reflowTags();
    emit selectionChanged(m_cachedSelection);
}

void MultiSelectComboBox::clearAll()
{
    auto *model = qobject_cast<QStandardItemModel *>(m_comboBox->model());
    if (!model) {
        return;
    }

    model->blockSignals(true);
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *item = model->item(i);
        if (item && item->isCheckable()) {
            item->setCheckState(Qt::Unchecked);
        }
    }
    model->blockSignals(false);

    m_lastClickedIndex = QPersistentModelIndex();
    m_cachedSelection.clear();
    reflowTags();
    emit selectionChanged(m_cachedSelection);
}

void MultiSelectComboBox::refreshAreaMenu()
{
    QMenu menu;
    for (int mapId : m_mapIds) {
        QAction *action = menu.addAction(mapDisplayName(mapId));
        action->setData(mapId);
    }

    QAction *chosen = menu.exec(m_areaButton->mapToGlobal(QPoint(0, m_areaButton->height())));
    if (chosen) {
        selectByMapId(chosen->data().toInt());
    }
}

static QString settingsFilePath()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/NwShip.ini");
}

void MultiSelectComboBox::saveSelection(const QString &key) const
{
    const QStringList selected = getSelectedItems();
    const QJsonArray arr = QJsonArray::fromStringList(selected);
    const QByteArray json = QJsonDocument(arr).toJson(QJsonDocument::Compact);

    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    settings.setValue(key, QString::fromUtf8(json));
}

void MultiSelectComboBox::restoreSelection(const QString &key)
{
    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    const QString raw = settings.value(key).toString();
    if (raw.isEmpty()) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
    if (!doc.isArray()) {
        return;
    }

    QStringList names;
    const QJsonArray arr = doc.array();
    for (const QJsonValue &val : arr) {
        names.append(val.toString());
    }
    if (names.isEmpty()) {
        return;
    }

    auto *model = qobject_cast<QStandardItemModel *>(m_comboBox->model());
    if (!model) {
        return;
    }

    model->blockSignals(true);

    // Uncheck all first, then check only those in the saved list
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *item = model->item(i);
        if (!item || !item->isCheckable()) {
            continue;
        }
        item->setCheckState(names.contains(item->text()) ? Qt::Checked : Qt::Unchecked);
    }

    model->blockSignals(false);

    m_lastClickedIndex = QPersistentModelIndex();
    m_cachedSelection = getSelectedItems();
    reflowTags();
    emit selectionChanged(m_cachedSelection);
}
