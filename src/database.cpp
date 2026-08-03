#include "database/database.hpp"

#include "database/sql-queries.hpp"

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
                                const BodyMetrics& body_metrics) {
    query.bind(1, body_metrics.user_id);
    query.bind(2, body_metrics.date);
    query.bind(3, body_metrics.weight);
    query.bind(4, body_metrics.height);
    query.bind(5, body_metrics.age);
    query.bind(6, body_metrics.muscle_mass);
    query.bind(7, body_metrics.fat_mass);

    if (body_metrics.water_mass.has_value()) {
        query.bind(8, body_metrics.water_mass.value());
    } else {
        query.bind(8); // bind NULL
    }

    if (body_metrics.bone_mass.has_value()) {
        query.bind(9, body_metrics.bone_mass.value());
    } else {
        query.bind(9); // bind NULL
    }

    if (body_metrics.visceral_fat.has_value()) {
        query.bind(10, static_cast<int>(body_metrics.visceral_fat.value()));
    } else {
        query.bind(10); // bind NULL
    }

    if (body_metrics.protein_mass.has_value()) {
        query.bind(11, body_metrics.protein_mass.value());
    } else {
        query.bind(11); // bind NULL
    }

    if (body_metrics.segment_muscle_mass.has_value()) {
        const SegmentMass& muscleSegments =
            body_metrics.segment_muscle_mass.value();
        query.bind(12, muscleSegments.left_arm.value());
        query.bind(13, muscleSegments.right_arm.value());
        query.bind(14, muscleSegments.left_leg.value());
        query.bind(15, muscleSegments.right_leg.value());
        query.bind(16, muscleSegments.trunk.value());
    } else {
        for (int i = 12; i <= 16; ++i) {
            query.bind(i); // bind NULL
        }
    }

    if (body_metrics.segment_fat_mass.has_value()) {
        const SegmentMass& segment_fat_mass =
            body_metrics.segment_fat_mass.value();
        query.bind(17, segment_fat_mass.left_arm.value());
        query.bind(18, segment_fat_mass.right_arm.value());
        query.bind(19, segment_fat_mass.left_leg.value());
        query.bind(20, segment_fat_mass.right_leg.value());
        query.bind(21, segment_fat_mass.trunk.value());
    } else {
        for (int i = 17; i <= 21; ++i) {
            query.bind(i); // bind NULL
        }
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
    bindBodyMetricsToStatement(query, body_metrics);
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
    bindBodyMetricsToStatement(query, body_metrics);
    query.bind(22, id); // Bind the ID for the WHERE clause
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
    bindBodyMetricsToStatement(query, body_metrics);
    query.bind(22, user_id); // Bind the user_id for the WHERE clause
    query.bind(23, date);    // Bind the date for the WHERE clause
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