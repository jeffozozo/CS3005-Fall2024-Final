#include "RobotBase.h"
#include <cstdlib>
#include <ctime>
#include <map>
#include <set>
#include <cmath>
#include <limits>

class Robot_Flame_e_o : public RobotBase {
private:
    bool target_found = false;
    int target_row = -1;
    int target_col = -1;

    // Directions for the 8 radar scans around the robot (row, col offsets)
    const std::pair<int, int> radar_directions[8] = {
        {-1, -1}, {-1, 0}, {-1, 1}, // Top-left, Top-center, Top-right
        {0, -1},           {0, 1},  // Left-center, Right-center
        {1, -1}, {1, 0}, {1, 1}     // Bottom-left, Bottom-center, Bottom-right
    };

    int current_scan_index = 0; // Tracks which direction the radar is scanning

    // Memory of obstacles
    std::set<std::pair<int, int>> obstacles_memory;

    bool is_close_to_target(int current_row, int current_col) const {
        return std::abs(current_row - target_row) <= 1 && std::abs(current_col - target_col) <= 1;
    }

    void find_closest_enemy(const RadarObj& radar_results, int current_row, int current_col) {
        target_found = false;
        double closest_distance = std::numeric_limits<double>::max();

        for (int r = 0; r < radar_results.get_rows(); ++r) {
            for (int c = 0; c < radar_results.get_cols(); ++c) {
                if (radar_results[r][c] == 'R') { // Enemy robot
                    double distance = std::sqrt(std::pow(r - current_row, 2) + std::pow(c - current_col, 2));
                    if (distance < closest_distance) {
                        closest_distance = distance;
                        target_row = r;
                        target_col = c;
                        target_found = true;
                    }
                }
            }
        }
    }

    bool is_passable(int row, int col) const {
        // Check if the cell is within memory of obstacles
        if (obstacles_memory.count({row, col}) > 0) {
            return false; // Obstacle is in memory
        }
        return true; // Passable if not in memory
    }

    void update_obstacle_memory(const RadarObj& radar_results) {
        for (int r = 0; r < radar_results.get_rows(); ++r) {
            for (int c = 0; c < radar_results.get_cols(); ++c) {
                char cell = radar_results[r][c];
                if (cell == 'M' || cell == 'P' || cell == 'F') {
                    obstacles_memory.insert({r, c}); // Add obstacle to memory
                }
            }
        }
    }

public:
    Robot_Flame_e_o() : RobotBase(2, 5, flamethrower) {
        std::srand(static_cast<unsigned int>(std::time(nullptr))); // Seed for random movement
    }

    void get_radar_location(int &radar_row, int &radar_col) override
    {
        // Directions for a "clockwise sweep"
        static const std::pair<int, int> directions[] = {
            {-1, 0},  // 12:00 - straight up
            {-1, 1},  // 1:30 - up-right
            {0, 1},   // 3:00 - right
            {1, 1},   // 4:30 - down-right
            {1, 0},   // 6:00 - down
            {1, -1},  // 7:30 - down-left
            {0, -1},  // 9:00 - left
            {-1, -1}  // 10:30 - up-left
        };

        static int radar_direction_index = 0; // Keep track of current direction

        // Get the current location of the robot
        int current_row, current_col;
        get_current_location(current_row, current_col);

        // Adjust the radar direction index if needed to avoid edges
        bool valid_direction = false;
        while (!valid_direction) {
            int dir_row = directions[radar_direction_index].first;
            int dir_col = directions[radar_direction_index].second;

            // Calculate the radar location
            radar_row = current_row + dir_row;
            radar_col = current_col + dir_col;

            // Check if the radar location is within bounds
            if (radar_row >= 0 && radar_row < m_arena_row_max &&
                radar_col >= 0 && radar_col < m_arena_col_max) {
                valid_direction = true; // Valid direction found
            } else {
                // Move to the next direction, looping back to the start
                radar_direction_index = (radar_direction_index + 1) % 8;
            }
        }

        // Advance to the next direction for the next call
        radar_direction_index = (radar_direction_index + 1) % 8;
    }

    virtual void process_radar_results(RadarObj& radar_results) override {
        int current_row, current_col;
        get_current_location(current_row, current_col);

        // Update obstacle memory
        update_obstacle_memory(radar_results);

        // Look for the closest enemy in the radar results
        find_closest_enemy(radar_results, current_row, current_col);
    }

    virtual bool get_shot_location(int& shot_row, int& shot_col) override {
        if (target_found && is_close_to_target(shot_row, shot_col)) {
            shot_row = target_row;
            shot_col = target_col;
            target_found = false; // Reset after firing
            return true;          // Shoot
        }
        return false; // No valid target to shoot
    }

    virtual bool get_move_direction(int& move_row, int& move_col) override {
        int current_row, current_col;
        get_current_location(current_row, current_col);

        if (target_found) {
            // Try to move toward the target while avoiding obstacles
            int row_step = (target_row > current_row) ? 1 : (target_row < current_row) ? -1 : 0;
            int col_step = (target_col > current_col) ? 1 : (target_col < current_col) ? -1 : 0;

            // Check if the target direction is passable
            if (is_passable(current_row + row_step, current_col)) {
                move_row = row_step;
                move_col = 0;
            } else if (is_passable(current_row, current_col + col_step)) {
                move_row = 0;
                move_col = col_step;
            } else if (is_passable(current_row + row_step, current_col + col_step)) {
                move_row = row_step;
                move_col = col_step;
            } else {
                move_row = 0;
                move_col = 0; // Stay in place if all options are blocked
            }

            return true;
        }

        // Random movement if no target is found
        int directions[3] = {-1, 0, 1};
        move_row = directions[std::rand() % 3];
        move_col = directions[std::rand() % 3];

        // Ensure movement stays within bounds
        if (current_row + move_row < 0 || current_row + move_row >= m_arena_row_max) {
            move_row = 0;
        }
        if (current_col + move_col < 0 || current_col + move_col >= m_arena_col_max) {
            move_col = 0;
        }

        return true; // Always attempt to move
    }
};

// Factory function to create Robot_Flame_e_o
extern "C" RobotBase* create_robot() {
    return new Robot_Flame_e_o();
}