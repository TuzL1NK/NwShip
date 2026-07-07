#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <QString>
#include <QList>
#include <QVector>
#include <optional>

struct ExplorationPoint
{
    int id = 0;
    QString name;
    QString nameCn;
    QString nameEn;
    QString location;
    int expReward = 0;
    int surveyDurationMin = 0;
    double x = 0;
    double y = 0;
    double z = 0;
    int mapId = 0;
    int rankReq = 0;
    int surveyDistance = 0;
    bool startingPoint = false;
};

struct SubmarinePart
{
    int id = 0;
    QString name;
    QString nameCn;
    QString nameEn;
    int slot = -1;
    int rank = 0;
    int surveillance = 0;
    int retrieval = 0;
    int speed = 0;
    int range = 0;
    int favor = 0;
    int components = 0;
    int repairMaterials = 0;
};

struct SubmarineRank
{
    int level = 0;
    int capacity = 0;
    int surveillanceBonus = 0;
    int retrievalBonus = 0;
    int speedBonus = 0;
    int rangeBonus = 0;
    int favorBonus = 0;
};

struct SubmarineStats
{
    int level = 0;
    int capacity = 0;
    int surveillance = 0;
    int retrieval = 0;
    int speed = 0;
    int range = 0;
    int favor = 0;
    int totalWeight = 0;
    bool canDepart = false;
    QString departMessage;
};

struct ApplicationDataBundle
{
    QList<ExplorationPoint> explorationPoints;
    QList<SubmarinePart> parts;
    QVector<SubmarineRank> ranks;
};

QString resolveDataFile(const QString &fileName);
QString mapDisplayName(int mapId);

ApplicationDataBundle loadApplicationData(const QString &packagePath = QString());
QList<ExplorationPoint> loadExplorationPoints(const QString &path);
QList<SubmarinePart> loadSubmarineParts(const QString &path);
QVector<SubmarineRank> loadSubmarineRanks(const QString &path);

std::optional<SubmarineRank> rankForLevel(const QVector<SubmarineRank> &ranks, int level);
std::optional<SubmarinePart> partById(const QList<SubmarinePart> &parts, int id);

SubmarineStats computeSubmarineStats(int level,
                                     const SubmarineRank &rank,
                                     const SubmarinePart *hull,
                                     const SubmarinePart *stern,
                                     const SubmarinePart *bow,
                                     const SubmarinePart *bridge);

#endif // DATA_LOADER_H
