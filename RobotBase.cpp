#include "RobotBase.h"


//overload the << operator to print the weapon
std::ostream& operator<<(std::ostream& os, const WeaponType& weapon)
{
    switch (weapon)
    {
        case flamethrower: os << "flamethrower"; break;
        case railgun:      os << "railgun";      break;
        case grenade:      os << "grenade";      break;
        case hammer:       os << "hammer";       break;
        default:           os << "unknown";      break;
    }

    return os;
}

// Constructor
RobotBase::RobotBase(int move_in, int armor_in, WeaponType weapon_in)
    : m_health(100), m_weapon(weapon_in),  radar_ok(true), m_name("Default")
{
    // Validate move input
    if (move_in < 0)
    {
        m_move = 0;
    }
    else if (move_in > 4)
    {
        m_move = 4;
    }
    else
    {
        m_move = move_in;
    }

    // Calculate maximum armor based on the move value
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

    // Initialize location and arena size
    m_location_row = 0;
    m_location_col = 0;

}

// Get the robot's current health
int RobotBase::get_health()
{
    return m_health;
}

// Get the robot's armor level
int RobotBase::get_armor()
{
    return m_armor;
}

// Get the robot's movement range
int RobotBase::get_move()
{
    return m_move;
}

// Get the robot's weapon type
WeaponType RobotBase::get_weapon()
{
    return m_weapon;
}

// Get the robot's current location
void RobotBase::get_current_location(int& current_row, int& current_col)
{
    current_row = m_location_row;
    current_col = m_location_col;
}

// Disable the robot's radar
void RobotBase::disable_radar()
{
    radar_ok = false;
}

// Check if the radar is enabled
bool RobotBase::radar_enabled()
{
    return radar_ok;
}

// Apply damage to the robot and reduce its health
int RobotBase::take_damage(int damage_in)
{
    m_health -= damage_in;
    m_armor -= 1;
    if (m_health < 0)
    {
        m_health = 0; // Prevent health from going below zero
    }
    return m_health;
}

// Set the robot's next location
void RobotBase::move_to(int new_row, int new_col)
{
    m_location_row = new_row;
    m_location_col = new_col;
}

// Disable the robot's movement
void RobotBase::disable_movement()
{
    m_move = 0;
}

void RobotBase::reduce_armor(int amount)
{
    m_armor = m_armor - amount;
    if(m_armor < 0)
        m_armor = 0;

}

//set the arena size
void RobotBase::set_boundaries(int row_max, int col_max)
{
    m_board_row_max = row_max;
    m_board_col_max = col_max;
}

#include <iostream>
#include <string>
#include "RobotBase.h"

void RobotBase::print_stats() 
{
    // Map weapon types to their names
    std::string weapon_name;
    switch (m_weapon) 
    {
        case flamethrower: weapon_name = "Flamethrower"; break;
        case railgun:      weapon_name = "Railgun";      break;
        case grenade:      weapon_name = "Grenade";      break;
        case hammer:       weapon_name = "Hammer";       break;
        default:           weapon_name = "Unknown";     break;
    }

    // Determine radar status
    std::string radar_status = radar_ok ? "Operational" : "Disabled";

    // Print the robot's statistics
    std::cout << m_name << ":\n";
    std::cout << "  Health: " << m_health << "\n";
    std::cout << "  Weapon: " << weapon_name << "\n";
    std::cout << "  Armor: " << m_armor << "\n";
    std::cout << "  Move Speed: " << m_move << "\n";
    std::cout << "  Radar: " << radar_status << "\n";
    std::cout << "  Location: (" << m_location_row << "," << m_location_col << ")\n";
}


// Destructor
RobotBase::~RobotBase()
{
    // No additional cleanup required
}