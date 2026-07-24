#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <SQLiteCpp/SQLiteCpp.h>

#include "models/body-metrics.hpp"
#include "models/user-data.hpp"

class DataBase{
public:
    DataBase(const std::string& db_path);
    ~DataBase() = default;

    // getters
    BodyMetrics getLastBodyMetrics(int64_t user_id) const;
    BodyMetrics getBodyMetricsById(int64_t id) const;
    BodyMetrics getBodyMetricsByUserIdAndDate(int64_t user_id, const std::string& date) const;
    std::vector<BodyMetrics> getBodyMetricsHistory(int64_t user_id) const;
    UserData getUserData(int64_t user_id) const;

    // modifiers
    void addUser(int64_t user_id, const UserData& user_data);
    void addBodyMetrics(const BodyMetrics& body_metrics);
    void editUserData(int64_t user_id, const UserData& user_data);
    // edit by id of the record in the database, not by user id
    void editBodyMetricsById(int64_t id, const BodyMetrics& body_metrics);
    void editBodyMetricsByUserIdAndDate(int64_t user_id, const std::string& date, const BodyMetrics& body_metrics);
    void deleteBodyMetricsById(int64_t id);
    void deleteBodyMetricsByUserIdAndDate(int64_t user_id, const std::string& date);
    void deleteUser(int64_t user_id);

private:
    SQLite::Database db;
};