#ifndef __ARENA_H__
#define __ARENA_H__

#include "RobotBase.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <set>


class Arena {
private:
    int m_size_row, m_size_col;
    std::vector<std::vector<char>> m_board;
    std::vector<RobotBase*> m_robots;

    // radar
    void get_radar_results(RobotBase* robot, int radar_direction, std::vector<RadarObj>& radar_results); 
    void check_radar_location(int& radar_row, int& radar_col, int robot_row, int robot_col);
 
    // shot
    void handle_shot(RobotBase* robot, int shot_row, int shot_col);
    void handle_flame_shot(RobotBase* robot, int shot_row, int shot_col);
    void handle_railgun_shot(RobotBase* robot, int shot_row, int shot_col);
    void handle_grenade_shot(RobotBase* robot, int shot_row, int shot_col);
    void handle_hammer_shot(RobotBase* robot, int shot_row, int shot_col);
    void handle_emp_shot(RobotBase* robot);
    int calculate_damage(WeaponType weapon, int armor_level);
    void apply_damage_to_robot(RobotBase* robot, WeaponType weapon);

    //move 
    void handle_move(RobotBase* robot);
    void handle_collision(RobotBase* robot, char cell, int row, int col);
 
    bool winner();

public:

    Arena(int row_in, int col_in);
    bool load_robots();
    void initialize_board();
    void print_board() const; 
    void run_simulation();

};

#endif