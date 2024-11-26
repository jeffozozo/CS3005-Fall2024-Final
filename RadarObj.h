#ifndef __RADAR_OBJ__
#define __RADAR_OBJ__

#include <vector>

class RadarObj {
public:
    // Format: row|col|row|col|row|col...
    std::vector<int> m_enemies;

    // Format: obstacle|row|col|obstacle|row|col
    // obstacle is a char - F, M, P
    std::vector<int> m_obstacles;

    bool obstacles_found;
    bool enemies_found;

    void set_empty();
    bool found_enemies() const;
    bool found_obstacles() const;
};

#endif