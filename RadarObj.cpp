#include "RadarObj.h"

// Constructor to initialize the radar board
RadarObj::RadarObj(int rows, int cols)
    : m_rows(rows), m_cols(cols), m_board(rows, std::vector<char>(cols, ' '))
{
}

// Clears the radar board by resetting it to empty spaces
void RadarObj::set_empty()
{
    for (int r = 0; r < m_rows; ++r)
    {
        for (int c = 0; c < m_cols; ++c)
        {
            m_board[r][c] = ' ';
        }
    }
}

// Access a specific row of the radar board (const version)
const std::vector<char>& RadarObj::operator[](int row) const
{
    return m_board[row];
}

// Access a specific row of the radar board (non-const version)
std::vector<char>& RadarObj::operator[](int row)
{
    return m_board[row];
}

// Get the number of rows in the radar board
int RadarObj::get_rows() const
{
    return m_rows;
}

// Get the number of columns in the radar board
int RadarObj::get_cols() const
{
    return m_cols;
}