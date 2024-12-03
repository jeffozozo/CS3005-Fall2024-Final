#include "RobotBase.h"
#include <vector>
#include <iostream>
#include <algorithm> // For std::find_if

class Robot_Ratboy : public RobotBase 
{
private:
    bool m_moving_down = true; // Tracks vertical movement direction
    int to_shoot_row = -1;   // Tracks the row of the next target to shoot
    int to_shoot_col = -1;   // Tracks the column of the next target to shoot
    
    std::vector<RadarObj> known_obstacles; // Permanent obstacle list

    // Helper function to determine if a cell is an obstacle
    bool is_obstacle(int row, int col) const 
    {
        return std::any_of(known_obstacles.begin(), known_obstacles.end(), 
                           [&](const RadarObj& obj) {
                               return obj.m_row == row && obj.m_col == col;
                           });
    }

    // Clears the target when no enemy is found
    void clear_target() 
    {
        to_shoot_row = -1;
        to_shoot_col = -1;
    }

    // Helper function to add an obstacle to the list if it's not already there
    void add_obstacle(const RadarObj& obj) 
    {
        if (obj.m_type == 'M' || obj.m_type == 'P' || obj.m_type == 'F' && 
            !is_obstacle(obj.m_row, obj.m_col)) 
        {
            known_obstacles.push_back(obj);
        }
    }

public:
    Robot_Ratboy() : RobotBase(3, 4, railgun) {} // Initialize with 3 movement, 4 armor, railgun

    // Radar location for scanning in one of the 8 directions
    virtual void get_radar_direction(int& radar_direction) override 
    {
        int current_row, current_col;
        get_current_location(current_row, current_col);

        // Decide radar direction
        radar_direction = (current_col > 0) ? 7 : 3; // Left or Right
    }

    // Processes radar results and updates known obstacles and target
    virtual void process_radar_results(const std::vector<RadarObj>& radar_results) override 
    {
        clear_target();

        for (const auto& obj : radar_results) 
        {
            // Add static obstacles to the obstacle list
            add_obstacle(obj);

            // Identify the first enemy found as the target
            if (obj.m_type == 'R' && to_shoot_row == -1 && to_shoot_col == -1) 
            {
                to_shoot_row = obj.m_row;
                to_shoot_col = obj.m_col;
            }
        }
    }

    // Determines the next shot location
    virtual bool get_shot_location(int& shot_row, int& shot_col) override 
    {
        if (to_shoot_row != -1 && to_shoot_col != -1) 
        {
            shot_row = to_shoot_row;
            shot_col = to_shoot_col;
            clear_target(); // Clear target after shooting
            return true;
        }
        return false;
    }

    // Determines the next movement direction
    virtual void get_movement(int& move_direction, int& move_distance) override 
    {
        int current_row, current_col;
        int move = get_move();
        get_current_location(current_row, current_col);

        if(current_col > 0)
        {
            move_direction = 7;
            move_distance = move;
            return;
        }

        if(m_moving_down)
        {
            if((current_row + move) > m_board_row_max)
            {
                std::cerr << "swapping to up." << current_row + move << " m_board_row_max: " << m_board_row_max << std::endl;
                m_moving_down = false;
                move_direction = 5;
                move_distance = 1;
                return;
            }

            move_direction = 5;
            move_distance = move;
            return;
        }
        else
        {
            if(current_row - move < 0)
            {
                std::cerr << "swapping to down." << std::endl;
                m_moving_down = true;
                move_direction = 1;
                move_distance = 1;
                return;
            }
        }

        move_direction = 1;
        move_distance = move;
    }

};

// Factory function to create Robot_Ratboy
extern "C" RobotBase* create_robot() 
{
    return new Robot_Ratboy();
}