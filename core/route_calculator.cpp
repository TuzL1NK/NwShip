#include "route_calculator.h"

#include <QSet>
#include <QtGlobal>
#include <QMap>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace {

constexpr double kDistanceRangeFactor = 0.035;
constexpr double kDistanceTimeFactor = 39.90;
constexpr double kBaseHours = 12.0;

struct Vec3
{
    double x = 0;
    double y = 0;
    double z = 0;
};

Vec3 toVec3(const ExplorationPoint &point)
{
    return {point.x, point.y, point.z};
}

double euclideanDistance(const Vec3 &a, const Vec3 &b)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    const double dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

double routeTravelDistance(const ExplorationPoint &home,
                           const QVector<const ExplorationPoint *> &route)
{
    Vec3 previous = toVec3(home);
    double distance = 0;
    for (const ExplorationPoint *point : route) {
        const Vec3 current = toVec3(*point);
        distance += euclideanDistance(previous, current);
        previous = current;
    }
    return distance;
}

int routeRangeRequired(const ExplorationPoint &home,
                       const QVector<const ExplorationPoint *> &route)
{
    Vec3 previous = toVec3(home);
    int total = 0;
    for (const ExplorationPoint *point : route) {
        const Vec3 current = toVec3(*point);
        const double segmentDistance = euclideanDistance(previous, current);
        total += static_cast<int>(std::floor(segmentDistance * kDistanceRangeFactor));
        previous = current;
    }
    total += std::accumulate(route.begin(), route.end(), 0,
                             [](int acc, const ExplorationPoint *point) {
                                 return acc + point->surveyDistance;
                             });
    return total;
}

int sumSurveyDistance(const QVector<const ExplorationPoint *> &route)
{
    int total = 0;
    for (const ExplorationPoint *point : route) {
        total += point->surveyDistance;
    }
    return total;
}

int sumSurveyDuration(const QVector<const ExplorationPoint *> &route)
{
    int total = 0;
    for (const ExplorationPoint *point : route) {
        total += point->surveyDurationMin;
    }
    return total;
}

int sumExpReward(const QVector<const ExplorationPoint *> &route)
{
    int total = 0;
    for (const ExplorationPoint *point : route) {
        total += point->expReward;
    }
    return total;
}

std::optional<RoutePlan> evaluateRoute(const ExplorationPoint &home,
                                       const QVector<const ExplorationPoint *> &route,
                                       const SubmarineStats &stats)
{
    if (route.isEmpty() || stats.speed <= 0) {
        return std::nullopt;
    }

    const int firstMapId = route.first()->mapId;
    for (const ExplorationPoint *p : route) {
        if (p->mapId != firstMapId) {
            return std::nullopt;
        }
    }

    const double travelDistance = routeTravelDistance(home, route);
    const int rangeRequired = routeRangeRequired(home, route);
    if (stats.range < rangeRequired) {
        return std::nullopt;
    }

    RoutePlan plan;
    plan.travelDistance = travelDistance;
    plan.rangeRequired = static_cast<double>(rangeRequired);
    plan.remainingRange = static_cast<double>(stats.range - rangeRequired);
    plan.totalExp = sumExpReward(route);
    plan.totalTimeMin = (travelDistance * kDistanceTimeFactor / stats.speed)
                        + sumSurveyDuration(route) * 70.0 / stats.speed
                        + kBaseHours * 60.0;
    plan.efficiency = plan.totalExp / plan.totalTimeMin;

    plan.legs.reserve(route.size());
    for (const ExplorationPoint *point : route) {
        plan.legs.append({point->id, point->name});
    }
    return plan;
}

bool containsName(const QStringList &names, const QString &name)
{
    return names.contains(name, Qt::CaseInsensitive);
}

QStringList collectWhitelistMatches(const QVector<RouteLeg> &legs, const QStringList &whitelist)
{
    if (whitelist.isEmpty()) {
        return {};
    }

    QSet<QString> whitelistNames;
    whitelistNames.reserve(whitelist.size());
    for (const QString &name : whitelist) {
        whitelistNames.insert(name.toLower());
    }

    QStringList matches;
    for (const RouteLeg &leg : legs) {
        if (whitelistNames.contains(leg.name.toLower())
            && !matches.contains(leg.name, Qt::CaseInsensitive)) {
            matches.append(leg.name);
        }
    }
    return matches;
}

void applyWhitelistPreference(QVector<RoutePlan> &routes,
                               const QStringList &whitelist,
                               RouteWhitelistMode mode)
{
    if (whitelist.isEmpty()) {
        return;
    }

    QVector<RoutePlan> filtered;
    filtered.reserve(routes.size());
    for (RoutePlan &route : routes) {
        route.whitelistMatches = collectWhitelistMatches(route.legs, whitelist);
        route.whitelistMatchCount = route.whitelistMatches.size();
        if (mode == RouteWhitelistMode::Strict) {
            if (route.whitelistMatchCount > 0) {
                filtered.append(route);
            }
        } else {
            filtered.append(route);
        }
    }

    if (mode == RouteWhitelistMode::Strict && !filtered.isEmpty()) {
        routes = filtered;
    } else if (mode == RouteWhitelistMode::Preferred) {
        routes = filtered;
    }
}

QString routeKey(const RoutePlan &plan)
{
    QStringList ids;
    ids.reserve(plan.legs.size());
    for (const RouteLeg &leg : plan.legs) {
        ids << QString::number(leg.pointId);
    }
    return ids.join('|');
}

void evaluatePermutations(const ExplorationPoint &home,
                          const QVector<const ExplorationPoint *> &combo,
                          const SubmarineStats &stats,
                          RoutePlan &bestPlan,
                          bool &found)
{
    QVector<const ExplorationPoint *> order = combo;
    std::sort(order.begin(), order.end(),
              [](const ExplorationPoint *a, const ExplorationPoint *b) {
                  return a->id < b->id;
              });

    do {
        if (const std::optional<RoutePlan> plan = evaluateRoute(home, order, stats)) {
            if (!found || plan->totalTimeMin < bestPlan.totalTimeMin
                || (qFuzzyCompare(plan->totalTimeMin, bestPlan.totalTimeMin)
                    && plan->efficiency > bestPlan.efficiency)) {
                bestPlan = *plan;
                found = true;
            }
        }
    } while (std::next_permutation(order.begin(), order.end(),
                                   [](const ExplorationPoint *a, const ExplorationPoint *b) {
                                       return a->id < b->id;
                                   }));
}

void collectCombinations(const RouteSearchOptions &options,
                         int index,
                         int remaining,
                         QVector<const ExplorationPoint *> &current,
                         QVector<RoutePlan> &results,
                         QSet<QString> &seen)
{
    if (remaining == 0) {
        if (current.isEmpty()) {
            return;
        }

        RoutePlan bestPlan;
        bool found = false;
        evaluatePermutations(options.homePort, current, options.stats, bestPlan, found);
        if (!found) {
            return;
        }

        const QString key = routeKey(bestPlan);
        if (seen.contains(key)) {
            return;
        }
        seen.insert(key);
        results.append(bestPlan);
        return;
    }

    if (index >= options.candidates.size()) {
        return;
    }

    collectCombinations(options, index + 1, remaining, current, results, seen);

    current.append(&options.candidates.at(index));
    collectCombinations(options, index + 1, remaining - 1, current, results, seen);
    current.removeLast();
}

} // namespace

QVector<ExplorationPoint> buildRouteCandidates(const QList<ExplorationPoint> &allPoints,
                                               int level,
                                               const QStringList &whitelist,
                                               const QStringList &blacklist,
                                               int maxPool,
                                               RouteWhitelistMode mode)
{
    const bool strictWhitelist = mode == RouteWhitelistMode::Strict && !whitelist.isEmpty();

    QVector<ExplorationPoint> filtered;
    filtered.reserve(allPoints.size());

    for (const ExplorationPoint &point : allPoints) {
        if (point.startingPoint || point.name.isEmpty()) {
            continue;
        }
        if (point.rankReq > level) {
            continue;
        }
        if (containsName(blacklist, point.name)) {
            continue;
        }
        if (strictWhitelist && !containsName(whitelist, point.name)) {
            continue;
        }
        filtered.append(point);
    }

    if (strictWhitelist || filtered.size() <= maxPool) {
        return filtered;
    }

    QVector<ExplorationPoint> mandatory;
    mandatory.reserve(whitelist.size());
    for (const ExplorationPoint &point : filtered) {
        if (containsName(whitelist, point.name)) {
            mandatory.append(point);
        }
    }

    std::sort(filtered.begin(), filtered.end(),
              [](const ExplorationPoint &a, const ExplorationPoint &b) {
                  return a.expReward > b.expReward;
              });

    QVector<ExplorationPoint> pool = mandatory;
    QSet<int> usedIds;
    for (const ExplorationPoint &point : pool) {
        usedIds.insert(point.id);
    }

    for (const ExplorationPoint &point : filtered) {
        if (pool.size() >= maxPool) {
            break;
        }
        if (usedIds.contains(point.id)) {
            continue;
        }
        pool.append(point);
        usedIds.insert(point.id);
    }

    return pool;
}

QVector<RoutePlan> findBestRoutes(const RouteSearchOptions &options)
{
    QVector<RoutePlan> results;
    if (!options.stats.canDepart || options.candidates.isEmpty()) {
        return results;
    }

    QMap<int, QVector<ExplorationPoint>> groups;
    for (const ExplorationPoint &p : options.candidates) {
        groups[p.mapId].append(p);
    }

    QSet<QString> seen;

    for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
        const int mapId = it.key();
        const QVector<ExplorationPoint> groupCandidates = it.value();
        if (groupCandidates.isEmpty()) {
            continue;
        }

        RouteSearchOptions localOptions = options;
        localOptions.candidates = groupCandidates;
        localOptions.homePort = options.mapHomePorts.value(mapId, options.homePort);

        const int maxPoints = qMin(localOptions.maxPoints, localOptions.candidates.size());
        for (int count = 1; count <= maxPoints; ++count) {
            QVector<const ExplorationPoint *> current;
            current.reserve(count);
            collectCombinations(localOptions, 0, count, current, results, seen);
        }
    }

    applyWhitelistPreference(results, options.whitelist, options.whitelistMode);
    sortRoutes(results, RouteSortMode::Efficiency);

    if (results.size() > options.maxResults) {
        results.resize(options.maxResults);
    }

    return results;
}

void sortRoutes(QVector<RoutePlan> &routes, RouteSortMode mode)
{
    auto cmp = [mode](const RoutePlan &a, const RoutePlan &b) {
        if (a.whitelistMatchCount != b.whitelistMatchCount) {
            return a.whitelistMatchCount > b.whitelistMatchCount;
        }

        switch (mode) {
        case RouteSortMode::TotalExp:
            if (!qFuzzyCompare(a.totalExp, b.totalExp)) {
                return a.totalExp > b.totalExp;
            }
            return a.totalTimeMin < b.totalTimeMin;
        case RouteSortMode::ShortestTime:
            if (!qFuzzyCompare(a.totalTimeMin, b.totalTimeMin)) {
                return a.totalTimeMin < b.totalTimeMin;
            }
            return a.totalExp > b.totalExp;
        case RouteSortMode::Efficiency:
        default:
            if (!qFuzzyCompare(a.efficiency, b.efficiency)) {
                return a.efficiency > b.efficiency;
            }
            if (!qFuzzyCompare(a.totalExp, b.totalExp)) {
                return a.totalExp > b.totalExp;
            }
            return a.totalTimeMin < b.totalTimeMin;
        }
    };

    std::sort(routes.begin(), routes.end(), cmp);
}

QString formatDurationMinutes(double minutes)
{
    const int total = static_cast<int>(std::lround(minutes));
    const int hours = total / 60;
    const int mins = total % 60;
    return QStringLiteral("%1小时%2分").arg(hours).arg(mins);
}
