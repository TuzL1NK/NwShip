#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

#include "core/route_calculator.h"
#include "data/data_loader.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSelectionChanged();
    void onPartSelectionChanged();
    void toggleTheme();
    void switchToChinese();
    void switchToEnglish();
    void resetFilters();
    void calculateRoutes();
    void onSortModeChanged(int index);

private:
    enum class AppLanguage {
        Chinese,
        English
    };

    void applyTheme(bool dark);
    void applyLanguage(AppLanguage language);
    void refreshUiTexts();
    void refreshExplorationNames();
    void setStatusMessage(const QString &zh, const QString &en);
    QString localizedText(const QString &zh, const QString &en) const;
    QString partDisplayName(const SubmarinePart &part) const;
    void loadData();
    void populatePartCombos();
    void updateConfigLabel();
    void displayRoutes(const QVector<RoutePlan> &routes);
    std::optional<SubmarineStats> currentSubmarineStats() const;
    ExplorationPoint homePort() const;
    int selectedPartId(QComboBox *combo) const;

    Ui::MainWindow *ui;
    QComboBox *m_sortCombo = nullptr;
    QScrollArea *m_resultsScrollArea = nullptr;
    QWidget *m_resultsContent = nullptr;
    QVBoxLayout *m_resultsLayout = nullptr;
    QLabel *m_resultsEmptyLabel = nullptr;
    QList<ExplorationPoint> m_explorationPoints;
    QList<SubmarinePart> m_parts;
    QVector<SubmarineRank> m_ranks;
    QVector<RoutePlan> m_lastRoutes;
    bool m_darkTheme = true;
    AppLanguage m_language = AppLanguage::Chinese;
    QString m_statusMessageZh;
    QString m_statusMessageEn;
};

#endif // MAINWINDOW_H
