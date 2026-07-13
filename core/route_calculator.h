#ifndef ROUTE_CALCULATOR_H
#define ROUTE_CALCULATOR_H

#include "data/data_loader.h"

#include <QString>
#include <QVector>
#include <QStringList>
#include <QMap>

enum class RouteSortMode
{
    Efficiency,
    TotalExp,
    ShortestTime,
};

enum class RouteWhitelistMode
{
    Preferred,
    Strict,
};

struct RouteLeg
{
    int pointId = 0;
    QString name;
};

struct RoutePlan
{
    QVector<RouteLeg> legs;
    double totalExp = 0;
    double travelDistance = 0;
    double rangeRequired = 0;
    double remainingRange = 0;
    double totalTimeMin = 0;
    double efficiency = 0;
    int whitelistMatchCount = 0;
    QStringList whitelistMatches;
};

struct RouteSearchOptions
{
    int level = 1;
    SubmarineStats stats;
    ExplorationPoint homePort;
    QMap<int, ExplorationPoint> mapHomePorts;
    QVector<ExplorationPoint> candidates;
    QStringList whitelist;
    RouteWhitelistMode whitelistMode = RouteWhitelistMode::Preferred;
    int maxPoints = 5;
    int maxCandidatePool = 28;
    int maxResults = 200;
};

QVector<ExplorationPoint> buildRouteCandidates(const QList<ExplorationPoint> &allPoints,
                                               int level,
                                               const QStringList &whitelist,
                                               const QStringList &blacklist,
                                               int maxPool,
                                               RouteWhitelistMode mode);

QVector<RoutePlan> findBestRoutes(const RouteSearchOptions &options);

void sortRoutes(QVector<RoutePlan> &routes, RouteSortMode mode);

QString formatDurationMinutes(double minutes);

#endif // ROUTE_CALCULATOR_H
