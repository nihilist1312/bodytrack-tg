#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

#include <SQLiteCpp/SQLiteCpp.h>

#include "models/body-metrics.hpp"
#include "models/user-data.hpp"

class Database{
public:
    Database(const std::string& db_path);
    ~Database() = default;

    // getters
    std::optional<BodyMetrics> getLastBodyMetrics(int64_t user_id) const;
    std::optional<BodyMetrics> getBodyMetricsById(int64_t id) const;
    std::optional<BodyMetrics> getBodyMetricsByUserIdAndDate(int64_t user_id, const std::string& date) const;
    std::vector<BodyMetrics> getBodyMetricsHistory(int64_t user_id) const;
    std::optional<UserData> getUserData(int64_t user_id) const;

    // modifiers
    bool addUser(int64_t user_id, const UserData& user_data);
    bool addBodyMetrics(const BodyMetrics& body_metrics);
    bool editUserData(int64_t user_id, const UserData& user_data);
    // edit by id of the record in the Database, not by user id
    bool editBodyMetricsById(int64_t id, const BodyMetrics& body_metrics);
    bool editBodyMetricsByUserIdAndDate(int64_t user_id, const std::string& date, const BodyMetrics& body_metrics);
    bool deleteBodyMetricsById(int64_t id);
    bool deleteBodyMetricsByUserIdAndDate(int64_t user_id, const std::string& date);
    bool deleteUser(int64_t user_id);

private:
    SQLite::Database db;
};