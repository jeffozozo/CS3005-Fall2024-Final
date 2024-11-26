#include "RobotBase.h"

// Constructor
RobotBase::RobotBase(int move_in, int armor_in, WeaponType weapon_in)
    : m_weapon(weapon_in), m_health(100), m_name("Default"), radar_ok(true) {
    
    // Validate move input
    m_move = move_in;

    if (move_in < 0) 
    {
        m_move = 0;
    }
    
    if(move_in > 4) 
    {
        m_move = 4;
    }
    
    // Total move + armor is 7. Armor can be traded for more move, up to move = 4.
    int max_armor = 7 - m_move;

    // Validate armor input
    if (armor_in < 0) 
    {
        m_armor = 0;
    } 
    else if (armor_in > max_armor) 
    {
        m_armor = max_armor;
    } 
    else 
    {
        m_armor = armor_in;
    }

    // Initialize location (to be set later by the arena)
    m_location_row = 0;
    m_location_col = 0;
}

int RobotBase::get_health() 
{
    return m_health;
}

int RobotBase::get_armor() 
{
    return m_armor;
}

int RobotBase::get_move() 
{
    return m_move;
}

WeaponType RobotBase::get_weapon() 
{
    return m_weapon;
}


// Final methods (to be implemented later)
void RobotBase::get_current_location(int& current_row, int& current_col) 
{
    current_row = m_location_row;
    current_col = m_location_col;
}

void RobotBase::disable_radar() 
{
    radar_ok = false;
}

bool RobotBase::radar_enabled()
{
    return radar_ok;
}


int RobotBase::take_damage(int damage_in) 
{
    m_health -= damage_in;

    return 0;
}

void RobotBase::set_next_location(int new_row, int new_col)
{
    m_location_row = new_row;
    m_location_col = new_col;
}

void RobotBase::disable_movement()
{
    m_move = 0;
}

// Pure virtual methods 
void RobotBase::get_radar_location(int& radar_row, int& radar_col) 
{
    // Should be implemented by derived classes
    // you should make your robot think about where it wants to use
    // its radar to "look"
}

void RobotBase::set_radar_results(RadarObj& radar_results) 
{
    // Should be implemented by derived classes
    // you should think carefully about what you want to do with
    // the information you just got make your robot react.
}

bool RobotBase::get_shot_location(int& shot_row, int& shot_col) 
{
    // if you choose to shoot this round, set the location of your shot 
    // in the row and col and then return true.
    // return false if you do NOT want to shoot.
    // returning true will mean that the get_move_direction function will NOT
    // get called this round. Returning false will cause the Arena to call
    // your get_move_direction function. 

    return false; 
}

bool RobotBase::get_move_direction(int& move_row, int& move_col) 
{
    // if you returned false from your shoot function the Arena will call 
    // this move function. Set the information in row and column and 
    // return true.
    // returning false will make you not move.

    return false; 
}

// Destructor
RobotBase::~RobotBase() 
{
    // Cleanup code, if any
}