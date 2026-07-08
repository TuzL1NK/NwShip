#include "data_loader.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <algorithm>
#include <cmath>

namespace {

constexpr char kDataPackageMagic[] = "NWSH";
constexpr quint8 kDataPackageVersion = 1;
constexpr char kDataPackageKey[] = "NwShipDataPack2026";

QStringList dataFileCandidates(const QString &fileName)
{
    return {
        QCoreApplication::applicationDirPath() + QDir::separator() + fileName,
        QCoreApplication::applicationDirPath() + QStringLiteral("/../../") + fileName,
        QDir::currentPath() + QDir::separator() + fileName,
        QCoreApplication::applicationDirPath() + QStringLiteral("/../../../") + fileName,
    };
}

bool parseStartingPoint(const QString &value)
{
    return value.trimmed().compare(QStringLiteral("True"), Qt::CaseInsensitive) == 0;
}

QStringList parseCsvLine(const QString &line)
{
    QStringList fields;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        const QChar ch = line.at(i);
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line.at(i + 1) == '"') {
                current += '"';
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            fields << current;
            current.clear();
        } else {
            current += ch;
        }
    }

    fields << current;
    return fields;
}

int parseIntField(const QStringList &fields, int index, int defaultValue = 0)
{
    bool ok = false;
    const int value = fields.value(index).trimmed().toInt(&ok);
    return ok ? value : defaultValue;
}

double parseDoubleField(const QStringList &fields, int index, double defaultValue = 0.0)
{
    bool ok = false;
    const double value = fields.value(index).trimmed().toDouble(&ok);
    return ok ? value : defaultValue;
}

QByteArray xorTransform(const QByteArray &input)
{
    QByteArray output = input;
    for (int i = 0; i < output.size(); ++i) {
        output[i] = static_cast<char>(output[i] ^ kDataPackageKey[i % (sizeof(kDataPackageKey) - 1)]);
    }
    return output;
}

bool readTextFile(const QString &path, QByteArray &data)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    data = file.readAll();
    return true;
}

bool readExactBytes(QFile &file, QByteArray &data, quint32 size)
{
    data.resize(size);
    const qint64 readBytes = file.read(data.data(), size);
    return readBytes == size;
}

bool writeDataPackage(const QString &packagePath,
                      const QString &explorationPath,
                      const QString &partsPath,
                      const QString &ranksPath)
{
    QByteArray explorationCsv;
    QByteArray partsCsv;
    QByteArray ranksCsv;
    if (!readTextFile(explorationPath, explorationCsv) ||
        !readTextFile(partsPath, partsCsv) ||
        !readTextFile(ranksPath, ranksCsv)) {
        return false;
    }

    QFile file(packagePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    const QByteArray encodedExploration = xorTransform(explorationCsv);
    const QByteArray encodedParts = xorTransform(partsCsv);
    const QByteArray encodedRanks = xorTransform(ranksCsv);

    const quint32 explorationSize = static_cast<quint32>(encodedExploration.size());
    const quint32 partsSize = static_cast<quint32>(encodedParts.size());
    const quint32 ranksSize = static_cast<quint32>(encodedRanks.size());

    if (file.write(kDataPackageMagic, sizeof(kDataPackageMagic) - 1) != sizeof(kDataPackageMagic) - 1 ||
        file.putChar(kDataPackageVersion) == -1 ||
        file.write(reinterpret_cast<const char *>(&explorationSize), sizeof(explorationSize)) != sizeof(explorationSize) ||
        file.write(reinterpret_cast<const char *>(&partsSize), sizeof(partsSize)) != sizeof(partsSize) ||
        file.write(reinterpret_cast<const char *>(&ranksSize), sizeof(ranksSize)) != sizeof(ranksSize) ||
        file.write(encodedExploration) != encodedExploration.size() ||
        file.write(encodedParts) != encodedParts.size() ||
        file.write(encodedRanks) != encodedRanks.size()) {
        return false;
    }

    return file.flush();
}

bool readDataPackage(const QString &packagePath,
                     QByteArray &explorationCsv,
                     QByteArray &partsCsv,
                     QByteArray &ranksCsv)
{
    QFile file(packagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    quint8 version = 0;
    QByteArray magic(4, '\0');
    if (file.read(magic.data(), 4) != 4 || magic != QByteArray(kDataPackageMagic) || file.getChar(reinterpret_cast<char *>(&version)) == -1) {
        return false;
    }

    quint32 explorationSize = 0;
    quint32 partsSize = 0;
    quint32 ranksSize = 0;
    if (file.read(reinterpret_cast<char *>(&explorationSize), sizeof(explorationSize)) != sizeof(explorationSize) ||
        file.read(reinterpret_cast<char *>(&partsSize), sizeof(partsSize)) != sizeof(partsSize) ||
        file.read(reinterpret_cast<char *>(&ranksSize), sizeof(ranksSize)) != sizeof(ranksSize)) {
        return false;
    }

    QByteArray encodedExploration;
    QByteArray encodedParts;
    QByteArray encodedRanks;

    if (!readExactBytes(file, encodedExploration, explorationSize) ||
        !readExactBytes(file, encodedParts, partsSize) ||
        !readExactBytes(file, encodedRanks, ranksSize)) {
        return false;
    }

    explorationCsv = xorTransform(encodedExploration);
    partsCsv = xorTransform(encodedParts);
    ranksCsv = xorTransform(encodedRanks);
    return true;
}

QList<ExplorationPoint> parseExplorationPoints(const QString &csvText)
{
    QList<ExplorationPoint> points;
    QTextStream in(csvText.toUtf8());
    in.readLine();

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const QStringList fields = parseCsvLine(line);
        if (fields.size() < 14) {
            continue;
        }

        ExplorationPoint point;
        point.id = parseIntField(fields, 0);
        point.nameEn = fields.value(1).trimmed();
        point.nameCn = fields.value(14).trimmed();
        point.name = point.nameCn.isEmpty() ? point.nameEn : point.nameCn;
        point.location = fields.value(2).trimmed();
        point.expReward = parseIntField(fields, 3);
        point.surveyDurationMin = parseIntField(fields, 4);
        point.x = parseDoubleField(fields, 5);
        point.y = parseDoubleField(fields, 6);
        point.z = parseDoubleField(fields, 7);
        point.mapId = parseIntField(fields, 8);
        point.rankReq = parseIntField(fields, 10);
        point.surveyDistance = parseIntField(fields, 12);
        point.startingPoint = parseStartingPoint(fields.value(13));

        if (point.startingPoint || point.name.isEmpty()) {
            if (point.startingPoint) {
                points.append(point);
            }
            continue;
        }

        points.append(point);
    }

    return points;
}

QList<SubmarinePart> parseSubmarineParts(const QString &csvText)
{
    QList<SubmarinePart> parts;
    QTextStream in(csvText.toUtf8());
    in.readLine();

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const QStringList fields = parseCsvLine(line);
        if (fields.size() < 13) {
            continue;
        }

        const int id = parseIntField(fields, 0);
        const int slot = parseIntField(fields, 7);
        const int rank = parseIntField(fields, 8);
        const QString nameCn = fields.value(11).trimmed();
        const QString nameEn = fields.value(12).trimmed();
        const QString displayName = nameCn.isEmpty() ? nameEn : nameCn;

        if (id <= 0 || displayName.isEmpty() || slot < 0 || slot > 3) {
            continue;
        }

        SubmarinePart part;
        part.id = id;
        part.name = displayName;
        part.nameCn = nameCn;
        part.nameEn = nameEn;
        part.slot = slot;
        part.rank = rank;
        part.surveillance = parseIntField(fields, 2);
        part.retrieval = parseIntField(fields, 3);
        part.speed = parseIntField(fields, 4);
        part.range = parseIntField(fields, 5);
        part.favor = parseIntField(fields, 6);
        part.components = parseIntField(fields, 9);
        part.repairMaterials = parseIntField(fields, 10);
        parts.append(part);
    }

    std::sort(parts.begin(), parts.end(),
              [](const SubmarinePart &a, const SubmarinePart &b) {
                  if (a.rank != b.rank) {
                      return a.rank < b.rank;
                  }
                  return a.name < b.name;
              });

    return parts;
}

QVector<SubmarineRank> parseSubmarineRanks(const QString &csvText)
{
    QVector<SubmarineRank> ranks;
    QTextStream in(csvText.toUtf8());
    in.readLine();

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const QStringList fields = parseCsvLine(line);
        if (fields.size() < 8) {
            continue;
        }

        const int level = parseIntField(fields, 0);
        if (level <= 0) {
            continue;
        }

        SubmarineRank rank;
        rank.level = level;
        rank.capacity = parseIntField(fields, 2);
        rank.surveillanceBonus = parseIntField(fields, 3);
        rank.retrievalBonus = parseIntField(fields, 4);
        rank.speedBonus = parseIntField(fields, 5);
        rank.rangeBonus = parseIntField(fields, 6);
        rank.favorBonus = parseIntField(fields, 7);
        ranks.append(rank);
    }

    std::sort(ranks.begin(), ranks.end(),
              [](const SubmarineRank &a, const SubmarineRank &b) {
                  return a.level < b.level;
              });

    return ranks;
}

} // namespace

QString resolveDataFile(const QString &fileName)
{
    // Prefer assets/ folder inside application dir or project root, then default candidates
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString assetsInApp = appDir + QDir::separator() + QStringLiteral("assets") + QDir::separator() + fileName;
    if (QFileInfo::exists(assetsInApp)) {
        return QFileInfo(assetsInApp).absoluteFilePath();
    }

    const QString assetsInRoot = QDir::currentPath() + QDir::separator() + QStringLiteral("assets") + QDir::separator() + fileName;
    if (QFileInfo::exists(assetsInRoot)) {
        return QFileInfo(assetsInRoot).absoluteFilePath();
    }

    for (const QString &path : dataFileCandidates(fileName)) {
        if (QFileInfo::exists(path)) {
            return QFileInfo(path).absoluteFilePath();
        }
    }
    return dataFileCandidates(fileName).constFirst();
}

QString mapDisplayName(int mapId)
{
    switch (mapId) {
    case 1: return QStringLiteral("溺没海");
    case 2: return QStringLiteral("灰海");
    case 3: return QStringLiteral("翠浪海");
    case 4: return QStringLiteral("妖歌海");
    case 5: return QStringLiteral("紫礁海");
    case 6: return QStringLiteral("南苍茫洋");
    case 7: return QStringLiteral("北洋");
    default: return QStringLiteral("地图 %1").arg(mapId);
    }
}

ApplicationDataBundle loadApplicationData(const QString &packagePath)
{
    ApplicationDataBundle bundle;

    const QString resolvedPackagePath = packagePath.isEmpty()
                                           ? resolveDataFile(QStringLiteral("data.bin"))
                                           : packagePath;
    const QString explorationPath = resolveDataFile(QStringLiteral("SubmarineExploration.csv"));
    const QString partsPath = resolveDataFile(QStringLiteral("SubmarinePart.csv"));
    const QString ranksPath = resolveDataFile(QStringLiteral("SubmarineRank.csv"));

    // Simple in-memory cache to avoid reparsing during the same run
    static std::optional<ApplicationDataBundle> s_cache;
    if (s_cache.has_value() && packagePath.isEmpty()) {
        return *s_cache;
    }

    // If a data package exists (release build typically ships only data.bin), prefer it.
    QByteArray explorationCsv;
    QByteArray partsCsv;
    QByteArray ranksCsv;
    if (!resolvedPackagePath.isEmpty() && QFileInfo::exists(resolvedPackagePath)) {
        if (readDataPackage(resolvedPackagePath, explorationCsv, partsCsv, ranksCsv)) {
            bundle.explorationPoints = parseExplorationPoints(QString::fromUtf8(explorationCsv));
            bundle.parts = parseSubmarineParts(QString::fromUtf8(partsCsv));
            bundle.ranks = parseSubmarineRanks(QString::fromUtf8(ranksCsv));
            s_cache = bundle;
            return bundle;
        } else {
            qWarning("Failed to read data package: %s", qPrintable(resolvedPackagePath));
        }
    }

    // Otherwise, try reading CSVs (development mode), prefer CSVs if they are present
    const bool hasCsvData = QFileInfo::exists(explorationPath) && QFileInfo::exists(partsPath) && QFileInfo::exists(ranksPath);
    if (hasCsvData) {
        bundle.explorationPoints = loadExplorationPoints(explorationPath);
        bundle.parts = loadSubmarineParts(partsPath);
        bundle.ranks = loadSubmarineRanks(ranksPath);
        s_cache = bundle;
        return bundle;
    }

    // Final fallback: attempt to read CSVs individually (load* returns empty lists on failure)
    bundle.explorationPoints = loadExplorationPoints(explorationPath);
    bundle.parts = loadSubmarineParts(partsPath);
    bundle.ranks = loadSubmarineRanks(ranksPath);
    s_cache = bundle;
    return bundle;
}

QList<ExplorationPoint> loadExplorationPoints(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream in(&file);
    QString csvText;
    while (!in.atEnd()) {
        csvText += in.readLine() + QStringLiteral("\n");
    }

    return parseExplorationPoints(csvText);
}

QList<SubmarinePart> loadSubmarineParts(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream in(&file);
    QString csvText;
    while (!in.atEnd()) {
        csvText += in.readLine() + QStringLiteral("\n");
    }

    return parseSubmarineParts(csvText);
}

QVector<SubmarineRank> loadSubmarineRanks(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream in(&file);
    QString csvText;
    while (!in.atEnd()) {
        csvText += in.readLine() + QStringLiteral("\n");
    }

    return parseSubmarineRanks(csvText);
}

std::optional<SubmarineRank> rankForLevel(const QVector<SubmarineRank> &ranks, int level)
{
    for (const SubmarineRank &rank : ranks) {
        if (rank.level == level) {
            return rank;
        }
    }
    return std::nullopt;
}

std::optional<SubmarinePart> partById(const QList<SubmarinePart> &parts, int id)
{
    if (id <= 0) {
        return std::nullopt;
    }
    for (const SubmarinePart &part : parts) {
        if (part.id == id) {
            return part;
        }
    }
    return std::nullopt;
}

SubmarineStats computeSubmarineStats(int level,
                                     const SubmarineRank &rank,
                                     const SubmarinePart *hull,
                                     const SubmarinePart *stern,
                                     const SubmarinePart *bow,
                                     const SubmarinePart *bridge)
{
    SubmarineStats stats;
    stats.level = level;
    stats.capacity = rank.capacity;

    const SubmarinePart *selected[] = {hull, stern, bow, bridge};
    const QString slotNames[] = {
        QStringLiteral("船体"),
        QStringLiteral("船尾"),
        QStringLiteral("船首"),
        QStringLiteral("舰桥"),
    };

    QStringList errors;
    for (int i = 0; i < 4; ++i) {
        if (!selected[i]) {
            errors << QStringLiteral("未选择%1").arg(slotNames[i]);
            continue;
        }

        const SubmarinePart &part = *selected[i];
        stats.surveillance += part.surveillance;
        stats.retrieval += part.retrieval;
        stats.speed += part.speed;
        stats.range += part.range;
        stats.favor += part.favor;
        stats.totalWeight += part.components;

        if (level < part.rank) {
            errors << QStringLiteral("%1 需要等级 %2").arg(part.name).arg(part.rank);
        }
    }

    stats.surveillance += rank.surveillanceBonus;
    stats.retrieval += rank.retrievalBonus;
    stats.speed += rank.speedBonus;
    stats.range += rank.rangeBonus;
    stats.favor += rank.favorBonus;

    if (stats.totalWeight > stats.capacity) {
        errors << QStringLiteral("载重不足（%1/%2）").arg(stats.totalWeight).arg(stats.capacity);
    }
    if (stats.speed <= 0) {
        errors << QStringLiteral("航速无效");
    }

    stats.departMessage = errors.join(QStringLiteral("；"));
    stats.canDepart = errors.isEmpty();
    return stats;
}
