#include <vector>

class RadarObj
{
public:
    std::vector<int> m_enemies;
    std::vector<int> m_obstacles;
    bool obstacles_found;
    bool enemies_found;

    RadarObj(int row, int col);
    bool found_enemies();
    bool found_obstacles();

};