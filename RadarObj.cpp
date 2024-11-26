#include "RadarObj.h"

// Clears radar data
void RadarObj::set_empty() 
{
    m_enemies.clear();
    m_obstacles.clear();
    obstacles_found = false;
    enemies_found = false;
}

// Checks if enemies were found
bool RadarObj::found_enemies() const 
{
    return !m_enemies.empty();
}

// Checks if obstacles were found
bool RadarObj::found_obstacles() const 
{
    return !m_obstacles.empty();
}