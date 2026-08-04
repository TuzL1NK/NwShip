#ifndef MULTISELECTCOMBOBOX_H
#define MULTISELECTCOMBOBOX_H

#include <QWidget>
#include <QComboBox>
#include <QFrame>
#include <QScrollArea>
#include <QColor>
#include <QVBoxLayout>
#include <QPushButton>
#include <QList>
#include <QPersistentModelIndex>

#include "data/data_loader.h"

class TagWidget : public QFrame
{
    Q_OBJECT
public:
    explicit TagWidget(const QString &text, const QColor &accent, QWidget *parent = nullptr);

signals:
    void removeRequested(const QString &text);
};

class CustomMultiComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CustomMultiComboBox(QWidget *parent = nullptr);

protected:
    void showPopup() override;
};

class MultiSelectComboBox : public QWidget
{
    Q_OBJECT
public:
    explicit MultiSelectComboBox(QWidget *parent = nullptr);
    ~MultiSelectComboBox() override;

    void setExplorationPoints(const QList<ExplorationPoint> &points);
    QStringList getSelectedItems() const;
    void setPlaceholderText(const QString &text);
    void setAccentColor(const QColor &color);
    void applyTheme(bool dark);
    void setLanguage(bool chinese);

    void saveSelection(const QString &key) const;
    void restoreSelection(const QString &key);
    void selectByMapId(int mapId);
    void clearAll();

signals:
    void selectionChanged(const QStringList &selected);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onSelectionChanged();
    void onTagRemoveRequested(const QString &text);

private:
    void reflowTags();
    void styleComboBox();
    void appendMapSeparator(int mapId);
    void refreshAreaMenu();

    QVBoxLayout *m_mainLayout;
    QWidget *m_tagContainer;
    CustomMultiComboBox *m_comboBox;
    QPushButton *m_areaButton;
    QPushButton *m_clearButton;
    QScrollArea *m_scrollArea;
    QColor m_accentColor;
    QStringList m_cachedSelection;
    QList<int> m_mapIds;
    QPersistentModelIndex m_lastClickedIndex;
    bool m_darkTheme = true;
    bool m_chinese = true;
};

#endif // MULTISELECTCOMBOBOX_H
