#ifndef __ROBOTBASE_H__
#define __ROBOTBASE_H__

#include <string>
#include <iostream>


class RadarObj 
{
public:
    char m_type;  // 'R', 'M', 'F', 'P' (Robot, Mound, Flamethrower, Pit)
    int m_row;    // Row of the object
    int m_col;    // Column of the object

    RadarObj() {}
    RadarObj(char type, int row, int col) : m_type(type), m_row(row), m_col(col) {}
};

constexpr std::pair<int, int> directions[] = {
    {0, 0},   // Placeholder for index 0 - if you return 0, no movement will occur
    {-1, 0},  // 1: Up
    {-1, 1},  // 2: Up-right
    {0, 1},   // 3: Right
    {1, 1},   // 4: Down-right
    {1, 0},   // 5: Down
    {1, -1},  // 6: Down-left
    {0, -1},  // 7: Left
    {-1, -1}  // 8: Up-left
};

enum WeaponType { flamethrower, railgun, grenade, hammer };

class RobotBase {
private:
    int m_health;
    int m_armor;
    int m_move;
    WeaponType m_weapon;
    bool radar_ok;
    int m_location_row;
    int m_location_col;

public:

    int m_board_row_max;
    int m_board_col_max;
    std::string m_name;

    RobotBase(int move_in, int armor_in, WeaponType weapon_in);

    int get_health();
    int get_armor();
    int get_move();
    WeaponType get_weapon();
    void set_boundaries(int row_max, int col_max);

    // Final methods (cannot be overridden)
    virtual void get_current_location(int& current_row, int& current_col) final;
    virtual void disable_radar() final;
    virtual int take_damage(int damage_in) final;
    virtual void move_to(int new_row, int new_col) final;
    virtual void disable_movement() final;
    virtual void reduce_armor(int amount);
    virtual bool radar_enabled() final;
    virtual void print_stats() final;

    // Pure virtual methods (must be implemented by derived classes)
    virtual void get_radar_direction(int& radar_direction) = 0;
    virtual void process_radar_results(const std::vector<RadarObj>& radar_results) = 0;
    virtual bool get_shot_location(int& shot_row, int& shot_col) = 0;
    virtual void get_movement(int &direction,int &distance) = 0;

    // Virtual destructor
    virtual ~RobotBase();
};

#endif

