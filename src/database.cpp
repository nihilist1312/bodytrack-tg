#include "database/database.hpp"

#include "database/sql-queries.hpp"

#include <cstddef>
#include <optional>

#include <spdlog/spdlog.h>

BodyMetrics getBodyMetricsFromRow(const SQLite::Statement& query) {
    BodyMetrics metrics;
    metrics.id = query.getColumn("id").getInt64();
    metrics.user_id = query.getColumn("user_id").getInt64();
    metrics.date = query.getColumn("date").getString();
    metrics.weight = query.getColumn("weight").getDouble();
    metrics.height = query.getColumn("height").getDouble();
    metrics.age = query.getColumn("age").getInt();
    metrics.muscle_mass = query.getColumn("muscle_mass").getDouble();
    metrics.fat_mass = query.getColumn("body_fat_mass").getDouble();

    if (!query.getColumn("water_mass").isNull()) {
        metrics.water_mass = query.getColumn("water_mass").getDouble();
    }
    if (!query.getColumn("bone_mass").isNull()) {
        metrics.bone_mass = query.getColumn("bone_mass").getDouble();
    }
    if (!query.getColumn("visceral_fat").isNull()) {
        metrics.visceral_fat = query.getColumn("visceral_fat").getInt();
    }
    if (!query.getColumn("protein_mass").isNull()) {
        metrics.protein_mass = query.getColumn("protein_mass").getDouble();
    }

    if (!query.getColumn("muscle_left_arm").isNull()) {
        SegmentMass muscleSegments;
        muscleSegments.left_arm =
            query.getColumn("muscle_left_arm").getDouble();
        muscleSegments.right_arm =
            query.getColumn("muscle_right_arm").getDouble();
        muscleSegments.left_leg =
            query.getColumn("muscle_left_leg").getDouble();
        muscleSegments.right_leg =
            query.getColumn("muscle_right_leg").getDouble();
        muscleSegments.trunk = query.getColumn("muscle_trunk").getDouble();
        metrics.segment_muscle_mass = muscleSegments;
    }

    if (!query.getColumn("fat_left_arm").isNull()) {
        SegmentMass segment_fat_mass;
        segment_fat_mass.left_arm = query.getColumn("fat_left_arm").getDouble();
        segment_fat_mass.right_arm =
            query.getColumn("fat_right_arm").getDouble();
        segment_fat_mass.left_leg = query.getColumn("fat_left_leg").getDouble();
        segment_fat_mass.right_leg =
            query.getColumn("fat_right_leg").getDouble();
        segment_fat_mass.trunk = query.getColumn("fat_trunk").getDouble();
        metrics.segment_fat_mass = segment_fat_mass;
    }
    return metrics;
}

UserData getUserDataFromRow(const SQLite::Statement& query) {
    UserData user_data;
    user_data.user_id = query.getColumn("user_id").getInt64();
    user_data.user_name = query.getColumn("user_name").getString();
    user_data.age = query.getColumn("age").getInt();
    user_data.sex = static_cast<Sex>(query.getColumn("sex").getInt());

    return user_data;
}

void bindBodyMetricsToStatement(SQLite::Statement& query,
                                const BodyMetrics& body_metrics,
                                int first_index = 1) {
    int i = first_index;

    query.bind(i++, body_metrics.weight);
    query.bind(i++, body_metrics.height);
    query.bind(i++, body_metrics.age);
    query.bind(i++, body_metrics.muscle_mass);
    query.bind(i++, body_metrics.fat_mass);

    if (body_metrics.water_mass.has_value())
        query.bind(i++, *body_metrics.water_mass);
    else
        query.bind(i++);

    if (body_metrics.bone_mass.has_value())
        query.bind(i++, *body_metrics.bone_mass);
    else
        query.bind(i++);

    if (body_metrics.visceral_fat.has_value())
        query.bind(i++, static_cast<int>(*body_metrics.visceral_fat));
    else
        query.bind(i++);

    if (body_metrics.protein_mass.has_value())
        query.bind(i++, *body_metrics.protein_mass);
    else
        query.bind(i++);

    if (body_metrics.segment_muscle_mass.has_value()) {
        const auto& segments = *body_metrics.segment_muscle_mass;

        query.bind(i++, segments.left_arm.value());
        query.bind(i++, segments.right_arm.value());
        query.bind(i++, segments.left_leg.value());
        query.bind(i++, segments.right_leg.value());
        query.bind(i++, segments.trunk.value());
    } else {
        for (int j = 0; j < 5; ++j)
            query.bind(i++);
    }

    if (body_metrics.segment_fat_mass.has_value()) {
        const auto& segments = *body_metrics.segment_fat_mass;

        query.bind(i++, segments.left_arm.value());
        query.bind(i++, segments.right_arm.value());
        query.bind(i++, segments.left_leg.value());
        query.bind(i++, segments.right_leg.value());
        query.bind(i++, segments.trunk.value());
    } else {
        for (int j = 0; j < 5; ++j)
            query.bind(i++);
    }
}

Database::Database(const std::string& db_path)
    : db(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    spdlog::info("Open database: {}", db_path);
    // create users table if it doesn't exist
    db.exec(SQLQuery::createUsersTable);
    if (db.getChanges() != 0) {
        spdlog::info("Create User table");
    }

    // create body_metrics table if it doesn't exist
    db.exec(SQLQuery::createBodyMetricsTable);
    if (db.getChanges() != 0) {
        spdlog::info("Create Metrics table");
    }
}

// getters
std::optional<BodyMetrics> Database::getLastBodyMetrics(int64_t user_id) const {
    spdlog::debug("Database. Get user data for {}", user_id);
    SQLite::Statement query(db, SQLQuery::getLastBodyMetrics);
    query.bind(1, user_id);

    if (query.executeStep()) {
        return getBodyMetricsFromRow(query);
    } else {
        spdlog::debug("Database. User {} not found", user_id);
        return std::nullopt;
    }
}

std::optional<BodyMetrics> Database::getBodyMetricsById(int64_t id) const {
    spdlog::debug("Database. Get user metric by id {}", id);
    SQLite::Statement query(db, SQLQuery::getBodyMetricsById);
    query.bind(1, id);

    if (query.executeStep()) {
        return getBodyMetricsFromRow(query);
    } else {
        spdlog::debug("Database. Metric {} not found", id);
        return std::nullopt;
    }
}

std::vector<BodyMetrics>
Database::getBodyMetricsHistory(int64_t user_id, int offset, int limit) const {
    spdlog::debug(
        "Database. Get body metrics history for user {} (limit={}, offset={})",
        user_id, limit, offset);
    SQLite::Statement query(db, SQLQuery::getBodyMetricsHistoryPage);
    query.bind(1, user_id);
    query.bind(2, limit);
    query.bind(3, offset);

    std::vector<BodyMetrics> result;
    while (query.executeStep()) {
        result.push_back(getBodyMetricsFromRow(query));
    }

    spdlog::debug("Database. Found {} metrics for user {}", result.size(),
                  user_id);
    return result;
}

size_t Database::getBodyMetricsCount(int64_t user_id) const {
    spdlog::debug("Database. Get body metrics count for user {}", user_id);
    SQLite::Statement query(db, SQLQuery::getBodyMetricsCount);
    query.bind(1, user_id);

    if (query.executeStep()) {
        return query.getColumn(0).getInt64();
    }
    return 0;
}

std::optional<BodyMetrics>
Database::getBodyMetricsByUserIdAndDate(int64_t user_id,
                                        const std::string& date) const {
    spdlog::debug("Database. Get user metric by user ID {} and date {}",
                  user_id, date);
    SQLite::Statement query(db, SQLQuery::getBodyMetricsByUserIdAndDate);
    query.bind(1, user_id);
    query.bind(2, date);

    if (query.executeStep()) {
        return getBodyMetricsFromRow(query);
    } else {
        spdlog::debug("Database. Metric not found. User {}, date {}", user_id,
                      date);
        return std::nullopt;
    }
}

std::vector<BodyMetrics>
Database::getBodyMetricsHistory(int64_t user_id) const {
    spdlog::debug("Database. Get user metric history. User ID {}", user_id);
    SQLite::Statement query(db, SQLQuery::getBodyMetricsHistory);
    query.bind(1, user_id);

    std::vector<BodyMetrics> history;
    while (query.executeStep()) {
        BodyMetrics metrics = getBodyMetricsFromRow(query);
        history.push_back(metrics);
    }
    return history;
}

std::optional<UserData> Database::getUserData(int64_t user_id) const {
    spdlog::debug("Database. Get user data: {}", user_id);
    SQLite::Statement query(db, SQLQuery::getUserData);
    query.bind(1, user_id);

    if (query.executeStep()) {
        UserData user_data = getUserDataFromRow(query);
        return user_data;
    } else {
        spdlog::debug("Database. User not found {}", user_id);
        return std::nullopt;
    }
}

// modifiers
bool Database::addUser(int64_t user_id, const UserData& user_data) {
    spdlog::debug("Database. Add user {}", user_id);
    SQLite::Statement query(db, SQLQuery::addUser);
    query.bind(1, user_id);
    query.bind(2, user_data.user_name);
    query.bind(3, user_data.age);
    query.bind(4, static_cast<int>(user_data.sex));

    query.exec();

    if (db.getChanges() == 0) {
        spdlog::debug("Database. User {} already exist", user_id);
        return false;
    } else
        return true;
}

bool Database::addBodyMetrics(const BodyMetrics& body_metrics) {
    spdlog::debug("Database. Add metric with user {}, date {}",
                  body_metrics.user_id, body_metrics.date);
    SQLite::Statement query(db, SQLQuery::addBodyMetrics);
    query.bind(1, body_metrics.user_id);
    query.bind(2, body_metrics.date);

    bindBodyMetricsToStatement(query, body_metrics, 3);
    query.exec();

    if (db.getChanges() == 0) {
        spdlog::debug("Database. Metric with user {}, date {} already exist",
                      body_metrics.user_id, body_metrics.date);
        return false;
    } else
        return true;
}

bool Database::editUserData(int64_t user_id, const UserData& user_data) {
    spdlog::debug("Database. Edit user data. User ID {}", user_id);
    SQLite::Statement query(db, SQLQuery::editUserData);
    query.bind(1, user_data.user_name);
    query.bind(2, user_data.age);
    query.bind(3, static_cast<int>(user_data.sex));
    query.bind(4, user_id);
    query.exec();

    if (db.getChanges() == 0) {
        spdlog::debug("Database. User {} data not modifield", user_id);
        return false;
    } else
        return true;
}

bool Database::editBodyMetricsById(int64_t id,
                                   const BodyMetrics& body_metrics) {
    spdlog::debug("Database. Edit Metric by id {}", id);
    SQLite::Statement query(db, SQLQuery::editBodyMetricsById);
    query.bind(1, body_metrics.date);
    bindBodyMetricsToStatement(query, body_metrics, 2);
    query.bind(21, id);
    query.exec();

    if (db.getChanges() == 0) {
        spdlog::debug("Database. Metric {} not modifield", id);
        return false;
    } else
        return true;
}

bool Database::editBodyMetricsByUserIdAndDate(int64_t user_id,
                                              const std::string& date,
                                              const BodyMetrics& body_metrics) {
    spdlog::debug("Database. Edit Metric by User ID {} and Date {}", user_id,
                  date);
    SQLite::Statement query(db, SQLQuery::editBodyMetricsByUserIdAndDate);
    bindBodyMetricsToStatement(query, body_metrics, 1);
    query.bind(20, body_metrics.user_id);
    query.bind(21, body_metrics.date);
    query.exec();

    if (db.getChanges() == 0) {
        spdlog::debug("Database. Metric with User {} and Date {} not modifield",
                      user_id, date);
        return false;
    } else
        return true;
}

bool Database::deleteBodyMetricsById(int64_t id) {
    spdlog::debug("Database. Delete Metric by ID {}", id);
    SQLite::Statement query(db, SQLQuery::deleteBodyMetricsById);
    query.bind(1, id);
    query.exec();
    if (db.getChanges() == 0) {
        spdlog::debug("Database. Metric {} does exist", id);
        return false;
    } else
        return true;
}

bool Database::deleteBodyMetricsByUserIdAndDate(int64_t user_id,
                                                const std::string& date) {
    spdlog::debug("Database. Delete Metriv by User ID {} and Date {}", user_id,
                  date);
    SQLite::Statement query(db, SQLQuery::deleteBodyMetricsByUserIdAndDate);
    query.bind(1, user_id);
    query.bind(2, date);
    query.exec();
    if (db.getChanges() == 0) {
        spdlog::debug("Database. Metric with User ID {} and Date {} does exist",
                      user_id, date);
        return false;
    } else
        return true;
}

bool Database::deleteUser(int64_t user_id) {
    spdlog::debug("Database. Delete User {}", user_id);
    SQLite::Statement query(db, SQLQuery::deleteUser);
    query.bind(1, user_id);
    query.exec();
    if (db.getChanges() == 0) {
        spdlog::debug("Database. User {} does exist", user_id);
        return false;
    } else
        return true;
}