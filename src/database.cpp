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
    metrics.age = static_cast<uint8_t>(query.getColumn("age").getInt());
    metrics.muscleMass = query.getColumn("muscle_mass").getDouble();
    metrics.fatMass = query.getColumn("body_fat_mass").getDouble();

    if (!query.getColumn("water_mass").isNull()) {
        metrics.waterMass = query.getColumn("water_mass").getDouble();
    }
    if (!query.getColumn("bone_mass").isNull()) {
        metrics.boneMass = query.getColumn("bone_mass").getDouble();
    }
    if (!query.getColumn("visceral_fat").isNull()) {
        metrics.visceralFat =
            static_cast<uint8_t>(query.getColumn("visceral_fat").getInt());
    }
    if (!query.getColumn("protein_mass").isNull()) {
        metrics.proteinMass = query.getColumn("protein_mass").getDouble();
    }

    if (!query.getColumn("muscle_left_arm").isNull()) {
        SegmentMass muscleSegments;
        muscleSegments.leftArm = query.getColumn("muscle_left_arm").getDouble();
        muscleSegments.rightArm =
            query.getColumn("muscle_right_arm").getDouble();
        muscleSegments.leftLeg = query.getColumn("muscle_left_leg").getDouble();
        muscleSegments.rightLeg =
            query.getColumn("muscle_right_leg").getDouble();
        muscleSegments.trunk = query.getColumn("muscle_trunk").getDouble();
        metrics.muscleMassSegments = muscleSegments;
    }

    if (!query.getColumn("fat_left_arm").isNull()) {
        SegmentMass fatSegments;
        fatSegments.leftArm = query.getColumn("fat_left_arm").getDouble();
        fatSegments.rightArm = query.getColumn("fat_right_arm").getDouble();
        fatSegments.leftLeg = query.getColumn("fat_left_leg").getDouble();
        fatSegments.rightLeg = query.getColumn("fat_right_leg").getDouble();
        fatSegments.trunk = query.getColumn("fat_trunk").getDouble();
        metrics.fatMassSegments = fatSegments;
    }
    return metrics;
}

UserData getUserDataFromRow(const SQLite::Statement& query) {
    UserData user_data;
    user_data.user_id = query.getColumn("user_id").getInt64();
    user_data.user_name = query.getColumn("user_name").getString();
    user_data.age = static_cast<uint8_t>(query.getColumn("age").getInt());
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
    query.bind(6, body_metrics.muscleMass);
    query.bind(7, body_metrics.fatMass);

    if (body_metrics.waterMass.has_value()) {
        query.bind(8, body_metrics.waterMass.value());
    } else {
        query.bind(8); // bind NULL
    }

    if (body_metrics.boneMass.has_value()) {
        query.bind(9, body_metrics.boneMass.value());
    } else {
        query.bind(9); // bind NULL
    }

    if (body_metrics.visceralFat.has_value()) {
        query.bind(10, static_cast<int>(body_metrics.visceralFat.value()));
    } else {
        query.bind(10); // bind NULL
    }

    if (body_metrics.proteinMass.has_value()) {
        query.bind(11, body_metrics.proteinMass.value());
    } else {
        query.bind(11); // bind NULL
    }

    if (body_metrics.muscleMassSegments.has_value()) {
        const SegmentMass& muscleSegments =
            body_metrics.muscleMassSegments.value();
        query.bind(12, muscleSegments.leftArm);
        query.bind(13, muscleSegments.rightArm);
        query.bind(14, muscleSegments.leftLeg);
        query.bind(15, muscleSegments.rightLeg);
        query.bind(16, muscleSegments.trunk);
    } else {
        for (int i = 12; i <= 16; ++i) {
            query.bind(i); // bind NULL
        }
    }

    if (body_metrics.fatMassSegments.has_value()) {
        const SegmentMass& fatSegments = body_metrics.fatMassSegments.value();
        query.bind(17, fatSegments.leftArm);
        query.bind(18, fatSegments.rightArm);
        query.bind(19, fatSegments.leftLeg);
        query.bind(20, fatSegments.rightLeg);
        query.bind(21, fatSegments.trunk);
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