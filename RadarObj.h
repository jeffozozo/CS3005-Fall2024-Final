#ifndef __RADAR_OBJ__
#define __RADAR_OBJ__

#include <vector>

class RadarObj
{
private:
    int m_rows; // Number of rows in the radar
    int m_cols; // Number of columns in the radar
    std::vector<std::vector<char>> m_board; // Radar board data

public:
    // Constructor
    RadarObj(int rows = 0, int cols = 0);

    // Clears the radar board
    void set_empty();

    // Access a specific row of the radar board (const version)
    const std::vector<char>& operator[](int row) const;

    // Access a specific row of the radar board (non-const version)
    std::vector<char>& operator[](int row);

    // Get the number of rows in the radar board
    int get_rows() const;

    // Get the number of columns in the radar board
    int get_cols() const;
};

#endif // __RADAR_OBJ__