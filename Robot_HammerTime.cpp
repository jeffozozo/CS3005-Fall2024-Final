#include "RobotBase.h"
#include <vector>
#include <cmath> // For abs()

class Robot_HammerTime : public RobotBase 
{
private:
    int last_direction; // The last direction HammerTime moved or scanned

    // Check if a target is within one square of HammerTime's location
    bool is_target_within_one(const std::vector<RadarObj>& radar_results, int current_row, int current_col, int& target_row, int& target_col)
    {
        for (const auto& obj : radar_results)
        {
            if (obj.m_type == 'R' &&
                std::abs(obj.m_row - current_row) <= 1 &&
                std::abs(obj.m_col - current_col) <= 1)
            {
                target_row = obj.m_row;
                target_col = obj.m_col;
                return true;
            }
        }
        return false;
    }

public:
    Robot_HammerTime() : RobotBase(2, 5, hammer), last_direction(3) {} // Initialize with move=2, armor=5, hammer weapon, default direction right

    // Determine radar direction (same as the movement direction)
    virtual void get_radar_direction(int& radar_direction) override
    {
        radar_direction = last_direction; // Always radar in the movement direction
    }

    // Process radar results (nothing special for HammerTime)
    virtual void process_radar_results(const std::vector<RadarObj>& radar_results) override
    {
        // No persistent state from radar needed for HammerTime
    }

    // Determine the shot location
    virtual bool get_shot_location(int& shot_row, int& shot_col) override
    {
        int current_row, current_col;
        get_current_location(current_row, current_col);

        std::vector<RadarObj> radar_results;
        get_radar_direction(last_direction); // Make sure radar aligns with movement
        process_radar_results(radar_results);

        // Check if there's a robot within one space
        if (is_target_within_one(radar_results, current_row, current_col, shot_row, shot_col))
        {
            return true; // Hammer swing at the robot
        }

        return false; // No target to attack
    }

    // Handle movement in a spiral pattern
    virtual void get_movement(int& move_direction, int& move_distance) override
    {
        int current_row, current_col;
        get_current_location(current_row, current_col);

        static const std::vector<int> spiral_directions = {3, 5, 7, 1}; // Right, Down, Left, Up
        int attempts = 0;

        while (attempts < spiral_directions.size())
        {
            int direction = spiral_directions[(last_direction - 1 + attempts) % spiral_directions.size()]; // Try next direction
            int delta_row = directions[direction].first;
            int delta_col = directions[direction].second;

            int new_row = current_row + delta_row;
            int new_col = current_col + delta_col;

            // Check if the move is within bounds and not blocked
            if (new_row >= 0 && new_row < m_board_row_max &&
                new_col >= 0 && new_col < m_board_col_max)
            {
                move_direction = direction;
                move_distance = 1; // Always move one step at a time
                last_direction = direction; // Update the last direction
                return;
            }

            attempts++;
        }

        // If no valid move found, stay in place
        move_direction = 0;
        move_distance = 0;
    }
};

// Factory function to create Robot_HammerTime
extern "C" RobotBase* create_robot()
{
    return new Robot_HammerTime();
}