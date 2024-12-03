#include "RobotBase.h"
#include <cstdlib>
#include <ctime>
#include <map>
#include <set>
#include <cmath>
#include <limits>

class Robot_Flame_e_o : public RobotBase 
{
private:
    bool target_found = false;
    int target_row = -1;
    int target_col = -1;

    bool shot_last_turn = false; // Tracks if a shot was fired last turn
    int last_radar_direction = 1; // Tracks the radar direction used in the last turn
    int radar_direction = 1; // Tracks which direction the radar is scanning (1-8)
    
    std::set<std::pair<int, int>> obstacles_memory; // Memory of obstacles

    // Check if the robot is close to its target
    bool is_close_to_target(int current_row, int current_col) const 
    {
        return std::abs(current_row - target_row) <= 1 && std::abs(current_col - target_col) <= 1;
    }

    // Find the closest enemy from the radar results
    void find_closest_enemy(const std::vector<RadarObj>& radar_results, int current_row, int current_col) 
    {
        target_found = false;
        double closest_distance = std::numeric_limits<double>::max();

        for (const auto& obj : radar_results) 
        {
            if (obj.m_type == 'R') // Enemy robot
            {
                double distance = std::sqrt(std::pow(obj.m_row - current_row, 2) + std::pow(obj.m_col - current_col, 2));
                if (distance < closest_distance) 
                {
                    closest_distance = distance;
                    target_row = obj.m_row;
                    target_col = obj.m_col;
                    target_found = true;
                }
            }
        }
    }

    // Update the memory of obstacles
    void update_obstacle_memory(const std::vector<RadarObj>& radar_results) 
    {
        for (const auto& obj : radar_results) 
        {
            if (obj.m_type == 'M' || obj.m_type == 'P' || obj.m_type == 'F') 
            {
                obstacles_memory.insert({obj.m_row, obj.m_col});
            }
        }
    }

    // Check if a cell is passable
    bool is_passable(int row, int col) const 
    {
        return obstacles_memory.find({row, col}) == obstacles_memory.end();
    }

public:
    Robot_Flame_e_o() : RobotBase(2, 5, flamethrower) 
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr))); // Seed for random movement
    }

    // Set the radar direction for scanning
    virtual void get_radar_direction(int& radar_direction_out) override 
    {
        if (shot_last_turn) 
        {
            // Scan the same direction as last turn if a shot was fired
            radar_direction_out = last_radar_direction;
        } 
        else 
        {
            // Increment radar direction for the next scan (clockwise)
            radar_direction_out = radar_direction;
            last_radar_direction = radar_direction; // Store the new direction
            radar_direction = (radar_direction % 8) + 1; // Move to the next direction
        }
    }

    // Process the radar results and update the target and obstacles
    virtual void process_radar_results(const std::vector<RadarObj>& radar_results) override 
    {
        target_found = false; // Reset target state at the start
        int current_row, current_col;
        get_current_location(current_row, current_col);

        // Update obstacle memory
        update_obstacle_memory(radar_results);

        // Look for the closest enemy in the radar results
        find_closest_enemy(radar_results, current_row, current_col);

        if (!target_found && shot_last_turn) 
        {
            // If no target is found and a shot was fired last turn, reset shot state
            shot_last_turn = false;
        }
    }

    // Get the location for shooting, if applicable
    virtual bool get_shot_location(int& shot_row, int& shot_col) override 
    {
        if (target_found) 
        {
            // If a target is found, prepare to shoot and set shot state
            shot_row = target_row;
            shot_col = target_col;
            shot_last_turn = true; // Remember that a shot was fired
            target_found = false;  // Reset target for next turn
            return true;
        }
        else 
        {
            shot_last_turn = false; // Reset shot state if no target
        }

        return false; // No valid target to shoot
    }

    // Get the movement direction and distance
    virtual void get_movement(int& move_direction, int& move_distance) override 
    {
        int current_row, current_col;
        get_current_location(current_row, current_col);

        if (target_found) 
        {
            // Move toward the target while avoiding obstacles
            int row_step = (target_row > current_row) ? 1 : (target_row < current_row) ? -1 : 0;
            int col_step = (target_col > current_col) ? 1 : (target_col < current_col) ? -1 : 0;

            if (is_passable(current_row + row_step, current_col)) 
            {
                move_direction = (row_step > 0) ? 5 : 1; // Down or Up
                move_distance = 1;
            } 
            else if (is_passable(current_row, current_col + col_step)) 
            {
                move_direction = (col_step > 0) ? 3 : 7; // Right or Left
                move_distance = 1;
            } 
            else if (is_passable(current_row + row_step, current_col + col_step)) 
            {
                move_direction = (row_step > 0) ? ((col_step > 0) ? 4 : 6) : ((col_step > 0) ? 2 : 8);
                move_distance = 1; // Diagonal
            } 
            else 
            {
                move_direction = 0; // Stay in place
                move_distance = 0;
            }

            return;
        }

        // Random movement if no target is found
        move_direction = (std::rand() % 8) + 1; // Random direction (1-8)
        move_distance = 1; // Move 1 space
    }
};

// Factory function to create Robot_Flame_e_o
extern "C" RobotBase* create_robot() 
{
    return new Robot_Flame_e_o();
}