#include "RobotBase.h"
#include <cstdlib> // For std::rand and std::srand
#include <ctime>   // For seeding std::rand

class Robot_Flame_e_o : public RobotBase {
private:
    int current_scan_index = 0; // Tracks which of the 8 directions is being scanned
    bool target_found = false;

    // Directions for the 8 radar scans around the robot (row, col offsets)
    const std::pair<int, int> radar_directions[8] = {
        {-1, -1}, {-1, 0}, {-1, 1}, // Top-left, Top-center, Top-right
        {0, -1},           {0, 1},  // Left-center, Right-center
        {1, -1}, {1, 0}, {1, 1}     // Bottom-left, Bottom-center, Bottom-right
    };

public:
    Robot_Flame_e_o() : RobotBase(2, 5, flamethrower) {
        std::srand(static_cast<unsigned int>(std::time(nullptr))); // Seed for random movement
    }

    virtual void get_radar_location(int& radar_row, int& radar_col) override {
        // Use get_current_location to retrieve the robot's current position
        int current_row, current_col;
        get_current_location(current_row, current_col);

        // Determine the direction for the current scan
        radar_row = current_row + radar_directions[current_scan_index].first * 3;
        radar_col = current_col + radar_directions[current_scan_index].second * 3;

        // Wrap the radar scan index for the next turn
        current_scan_index = (current_scan_index + 1) % 8;
    }

    virtual void set_radar_results(RadarObj& radar_results) override {
        // If radar found enemies, set target_found to true
        target_found = radar_results.found_enemies();
    }

    virtual bool get_shot_location(int& shot_row, int& shot_col) override {
        // If the radar found a target, fire the flamethrower
        if (target_found) {
            int current_row, current_col;
            get_current_location(current_row, current_col);

            // Aim at the center of the current radar scan
            shot_row = current_row + radar_directions[(current_scan_index + 7) % 8].first * 3;
            shot_col = current_col + radar_directions[(current_scan_index + 7) % 8].second * 3;

            target_found = false; // Reset target flag
            return true;          // Fire the flamethrower
        }
        return false; // No target to shoot
    }

    virtual bool get_move_direction(int& move_row, int& move_col) override {
        // Random movement logic
        int directions[3] = {-1, 0, 1}; // Possible movement directions: -1, 0, 1

        // Randomly choose row and column movement
        move_row = directions[std::rand() % 3];
        move_col = directions[std::rand() % 3];

        // Prevent movement out of bounds
        int current_row, current_col;
        get_current_location(current_row, current_col);

        if (current_row + move_row < 0 || current_row + move_row >= m_arena_row_max) {
            move_row = 0; // Stay in bounds vertically
        }

        if (current_col + move_col < 0 || current_col + move_col >= m_arena_col_max) {
            move_col = 0; // Stay in bounds horizontally
        }

        return true; // Always move randomly
    }
};

// Factory function to create Robot_Flame_e_o
extern "C" RobotBase* create_robot() {
    return new Robot_Flame_e_o();
}