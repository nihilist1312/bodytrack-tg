#pragma once

namespace SQLQuery {
    // create tables
    constexpr auto createUsersTable = R"(
CREATE TABLE IF NOT EXISTS users (
    user_id   INTEGER PRIMARY KEY,
    user_name TEXT NOT NULL,
    age       INTEGER NOT NULL,
    sex       INTEGER NOT NULL
);
)";

    constexpr auto createBodyMetricsTable = R"(
CREATE TABLE IF NOT EXISTS body_metrics (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    date TEXT NOT NULL,

    weight REAL NOT NULL,
    height REAL NOT NULL,
    age INTEGER NOT NULL,
    muscle_mass REAL NOT NULL,
    body_fat_mass REAL NOT NULL,

    water_mass REAL,
    bone_mass REAL,
    visceral_fat INTEGER,
    protein_mass REAL,

    muscle_left_arm REAL,
    muscle_right_arm REAL,
    muscle_left_leg REAL,
    muscle_right_leg REAL,
    muscle_trunk REAL,

    fat_left_arm REAL,
    fat_right_arm REAL,
    fat_left_leg REAL,
    fat_right_leg REAL,
    fat_trunk REAL,

    FOREIGN KEY (user_id) REFERENCES users(user_id)
);
)";

    // getters
    constexpr auto getUserName = "SELECT user_name FROM users WHERE user_id = ?;";
    constexpr auto getLastBodyMetrics = "SELECT * FROM body_metrics WHERE user_id = ? ORDER BY date DESC LIMIT 1";
    constexpr auto getBodyMetricsHistory = "SELECT * FROM body_metrics WHERE user_id = ? ORDER BY date DESC";
    constexpr auto getUserData = "SELECT * FROM users WHERE user_id = ?;";
    constexpr auto getBodyMetricsById = "SELECT * FROM body_metrics WHERE id = ?;";
    constexpr auto getBodyMetricsByUserIdAndDate = "SELECT * FROM body_metrics WHERE user_id = ? AND date = ?;";

    // modifiers
    constexpr auto addUser = "INSERT INTO users (user_id, user_name, age, sex) VALUES (?, ?, ?, ?);";
    constexpr auto addBodyMetrics = R"(
INSERT INTO body_metrics (
    user_id, date, weight, height, age, muscle_mass, body_fat_mass,
    water_mass, bone_mass, visceral_fat, protein_mass,
    muscle_left_arm, muscle_right_arm, muscle_left_leg, muscle_right_leg, muscle_trunk,
    fat_left_arm, fat_right_arm, fat_left_leg, fat_right_leg, fat_trunk
) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
)";
    constexpr auto editUserData = "UPDATE users SET user_name = ?, age = ?, sex = ? WHERE user_id = ?;";
    constexpr auto editBodyMetricsById = R"(
UPDATE body_metrics SET
    weight = ?, height = ?, age = ?, muscle_mass = ?, body_fat_mass = ?,
    water_mass = ?, bone_mass = ?, visceral_fat = ?, protein_mass = ?,
    muscle_left_arm = ?, muscle_right_arm = ?, muscle_left_leg = ?, muscle_right_leg = ?, muscle_trunk = ?,
    fat_left_arm = ?, fat_right_arm = ?, fat_left_leg = ?, fat_right_leg = ?, fat_trunk = ?
WHERE id = ?;
)";
    constexpr auto editBodyMetricsByUserIdAndDate = R"(
UPDATE body_metrics SET
    weight = ?, height = ?, age = ?, muscle_mass = ?, body_fat_mass = ?,
    water_mass = ?, bone_mass = ?, visceral_fat = ?, protein_mass = ?,
    muscle_left_arm = ?, muscle_right_arm = ?, muscle_left_leg = ?, muscle_right_leg = ?, muscle_trunk = ?,
    fat_left_arm = ?, fat_right_arm = ?, fat_left_leg = ?, fat_right_leg = ?, fat_trunk = ?
WHERE user_id = ? AND date = ?;
)";
    constexpr auto deleteBodyMetricsById = "DELETE FROM body_metrics WHERE id = ?;";
    constexpr auto deleteBodyMetricsByUserIdAndDate = "DELETE FROM body_metrics WHERE user_id = ? AND date = ?;";
    constexpr auto deleteUser = "DELETE FROM users WHERE user_id = ?;";
}