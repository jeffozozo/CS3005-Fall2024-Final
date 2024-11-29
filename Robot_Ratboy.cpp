#include "RobotBase.h"
#include <vector>
#include <iostream>
#include <algorithm> // For std::find

class Robot_Ratboy : public RobotBase 
{
private:
    bool target_found = false;
    bool moving_down = true; // Tracks vertical movement direction
    int radar_target_row = 0; // Tracks the row being scanned
    int radar_target_col = 0; // Tracks the column being scanned
    std::vector<std::pair<int, int>> known_obstacles; // Tracks known obstacles

    // Helper function to determine if a cell is an obstacle
    bool is_obstacle(int row, int col) const 
    {
        return std::find(known_obstacles.begin(), known_obstacles.end(), std::make_pair(row, col)) != known_obstacles.end();
    }

    // Check if there is a clear path from the current location to the leftmost column (0)
    bool is_path_clear(int current_row, int current_col, int target_col) const 
    {
        for (int col = current_col - 1; col >= target_col; --col) 
        {
            if (is_obstacle(current_row, col)) 
            {
                return false; // Path is blocked
            }
        }
        return true; // Path is clear
    }

public:
    Robot_Ratboy() : RobotBase(3, 4, railgun) {} // Initialize with 3 movement, 4 armor, railgun

    // Radar location points directly left
    virtual void get_radar_location(int& radar_row, int& radar_col) override 
    {
        int current_row, current_col;
        get_current_location(current_row, current_col);

        radar_row = current_row; // Radar scans in the current row
        radar_col = current_col - 3; // Scan leftward
        radar_col = std::max(0, radar_col); // Ensure within bounds
    }

    // Process radar results to find enemies and obstacles
    virtual void process_radar_results(RadarObj& radar_results) override 
    {
        target_found = false;
        known_obstacles.clear(); // Clear the obstacle memory for this turn

        for (int r = 0; r < radar_results.get_rows(); ++r) 
        {
            for (int c = 0; c < radar_results.get_cols(); ++c) 
            {
                char cell = radar_results[r][c];
                if (cell == 'M' || cell == 'P' || cell == 'F' || cell == 'R') 
                {
                    known_obstacles.emplace_back(r, c); // Store obstacle location
                }
                if (cell == 'E') 
                {
                    // Mark enemy as found
                    target_found = true;
                    radar_target_row = r;
                    radar_target_col = c;
                }
            }
        }
    }

    virtual bool get_shot_location(int& shot_row, int& shot_col) override 
    {
        if (target_found) 
        {
            shot_row = radar_target_row;
            shot_col = radar_target_col;
            target_found = false; // Reset target after firing
            return true;
        }
        return false; // No target to shoot
    }

    virtual bool get_move_direction(int& move_row, int& move_col) override 
    {
        int current_row, current_col;
        get_current_location(current_row, current_col);

        // Check if the path to the leftmost column is clear
        if (current_col > 0 && is_path_clear(current_row, current_col, 0)) 
        {
            move_row = 0;
            move_col = -1; // Move left
            return true;
        }

        // If blocked, adjust vertically to avoid obstacles
        if (!is_path_clear(current_row, current_col, 0)) 
        {
            if (moving_down) 
            {
                if (current_row + 3 < m_arena_row_max) 
                {
                    move_row = 3;
                    move_col = 0;
                    return true;
                } 
                else 
                {
                    moving_down = false; // Reverse direction at the bottom
                }
            } 
            else 
            {
                if (current_row - 3 >= 0) 
                {
                    move_row = -3;
                    move_col = 0;
                    return true;
                } 
                else 
                {
                    moving_down = true; // Reverse direction at the top
                }
            }
        }

        // If at the leftmost column, adjust vertical movement
        if (current_col == 0) 
        {
            if (moving_down) 
            {
                if (current_row + 3 < m_arena_row_max) 
                {
                    move_row = 3;
                    move_col = 0;
                    return true;
                } 
                else 
                {
                    moving_down = false; // Reverse direction at the bottom
                }
            } 
            else 
            {
                if (current_row - 3 >= 0) 
                {
                    move_row = -3;
                    move_col = 0;
                    return true;
                } 
                else 
                {
                    moving_down = true; // Reverse direction at the top
                }
            }
        }

        // If no movement is possible, stay in place
        move_row = 0;
        move_col = 0;
        return false;
    }
};

// Factory function to create Robot_Ratboy
extern "C" RobotBase* create_robot() 
{
    return new Robot_Ratboy();
}