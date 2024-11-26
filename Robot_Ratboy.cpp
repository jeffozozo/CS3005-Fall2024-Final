#include "RobotBase.h"

class Robot_Ratboy : public RobotBase {
private:
    int radar_col = 0; // Tracks the radar scanning column
    bool target_found = false;
    bool moving_down = true; // Tracks vertical movement direction

public:
    Robot_Ratboy() : RobotBase(3, 4, railgun) {} // Initialize with 3 movement, 4 armor, railgun

    virtual void get_radar_location(int& radar_row, int& radar_col) override {
        // Use get_current_location to retrieve the robot's current position
        int current_row, current_col;
        get_current_location(current_row, current_col);

        radar_row = current_row; // Radar scans the robot's current row
        radar_col = this->radar_col;

        // Advance radar column by 3, ensuring it doesn't exceed arena boundaries
        if (this->radar_col + 3 < m_arena_col_max) {
            this->radar_col += 3;
        } else {
            this->radar_col = 0; // Reset radar column after reaching max
        }
    }

    virtual void set_radar_results(RadarObj& radar_results) override {
        // If radar found enemies, set target_found to true
        target_found = radar_results.found_enemies();
    }

    virtual bool get_shot_location(int& shot_row, int& shot_col) override {
        // If the radar found a target, shoot toward the scanned column
        if (target_found) {
            int current_row, current_col;
            get_current_location(current_row, current_col);

            shot_row = current_row;   // Shoot along the same row
            shot_col = radar_col - 3; // Adjust to the last scanned position
            target_found = false;     // Reset target flag
            return true;              // Shoot
        }
        return false; // No target to shoot
    }

    virtual bool get_move_direction(int& move_row, int& move_col) override {
        int current_row, current_col;
        get_current_location(current_row, current_col);

        // If not yet at the left side, move left
        if (current_col > 0) {
            move_row = 0;
            move_col = -1;
            return true;
        }

        // If at the left side, adjust vertical position after scanning all rows
        if (radar_col == 0) { // Completed a horizontal scan
            radar_col = 0; // Reset radar column

            // Move down or up by 3 rows depending on current direction
            if (moving_down) {
                if (current_row + 3 < m_arena_row_max) {
                    move_row = 3;
                    move_col = 0;
                    return true;
                } else {
                    moving_down = false; // Reverse direction if at the bottom
                }
            }

            if (!moving_down) {
                if (current_row - 3 >= 0) {
                    move_row = -3;
                    move_col = 0;
                    return true;
                } else {
                    moving_down = true; // Reverse direction if at the top
                }
            }
        }

        // No movement if scanning or at boundaries
        return false;
    }
};

// Factory function to create Robot_Ratboy
extern "C" RobotBase* create_robot() {
    return new Robot_Ratboy();
}