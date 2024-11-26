#ifndef __ROBOTBASE_H__
#define __ROBOTBASE_H__

#include "RadarObj.h"
#include <string>


enum WeaponType { flamethrower, railgun, grenade, hammer, emp };

class RobotBase {
private:
    int m_health;
    int m_armor;
    int m_move;
    WeaponType m_weapon;
    std::string m_name;
    bool radar_ok;
    int m_location_row;
    int m_location_col;

public:

    int m_arena_row_max;
    int m_arena_col_max;

    RobotBase(int move_in, int armor_in, WeaponType weapon_in);

    int get_health();
    int get_armor();
    int get_move();
    WeaponType get_weapon();

    void set_arena_size(int row_max, int col_max);

    // Final methods (cannot be overridden)
    virtual void get_current_location(int& current_row, int& current_col) final;
    virtual void disable_radar() final;
    virtual int take_damage(int damage_in) final;
    virtual void set_next_location(int new_row, int new_col) final;
    virtual void disable_movement() final;
    virtual bool radar_enabled() final;

    // Pure virtual methods (must be implemented by derived classes)
    virtual void get_radar_location(int& radar_row, int& radar_col) = 0;
    virtual void set_radar_results(RadarObj& radar_results) = 0;
    virtual bool get_shot_location(int& shot_row, int& shot_col) = 0;
    virtual bool get_move_direction(int& move_row, int& move_col) = 0;

    // Virtual destructor
    virtual ~RobotBase();
};

#endif

