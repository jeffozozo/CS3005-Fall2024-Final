#ifndef __ARENA_H__
#define __ARENA_H__

#include "RobotBase.h"
#include "RadarObj.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <set>


class Arena {
private:
    int size_row, size_col;
    std::vector<std::vector<char>> m_board;
    std::vector<RobotBase*> m_robots;

    void get_radar_results(RobotBase *robot, int radar_row, int radar_col, RadarObj& radar_results);
    void check_radar_location(int& radar_row, int& radar_col, int robot_row, int robot_col);
    void handle_shot(RobotBase* robot, int shot_row, int shot_col);
    void handle_flame_shot(RobotBase* robot, int shot_row, int shot_col);
    void handle_railgun_shot(RobotBase* robot, int shot_row, int shot_col);
    void handle_grenade_shot(RobotBase* robot, int shot_row, int shot_col);
    void handle_hammer_shot(RobotBase* robot, int shot_row, int shot_col);
    void handle_emp_shot(RobotBase* robot);
    void apply_damage_to_robot(int row, int col, WeaponType weapon, std::set<RobotBase*>& robots_hit);
    int calculate_damage(WeaponType weapon, int armor_level); 
    void calculate_increment(const int start_row, const int start_col, const int end_row, const int end_col, double &inc_row, double &inc_col);
    void handle_move(RobotBase* robot, int move_row, int move_col);
    bool winner();

public:

    Arena(int row_in, int col_in);
    bool load_robots();
    void initialize_board();
    void print_board() const; 
    void run_simulation();

};

#endif