#include "Arena.h"

// Set the size of the arena
Arena::Arena(int row_in, int col_in) {
    size_row = row_in;
    size_col = col_in;
    m_board.resize(size_row, std::vector<char>(size_col, '.'));
}

#include <filesystem>
#include <string>
#include <iostream>
#include <dlfcn.h> // For dynamic library loading

bool Arena::load_robots() 
{
    namespace fs = std::filesystem;
    std::cout << "Loading Robots..." << std::endl;

    try 
    {
        // Scan the current directory for Robot_<name>.cpp files
        for (const auto& entry : fs::directory_iterator(".")) 
        {
            if (entry.is_regular_file()) 
            {
                std::string filename = entry.path().filename().string();

                // Check if the file matches the naming pattern Robot_<name>.cpp
                if (filename.rfind("Robot_", 0) == 0 && filename.substr(filename.size() - 4) == ".cpp") 
                {
                    std::string robot_name = filename.substr(6, filename.size() - 10); // Extract <name>
                    std::string shared_lib = "lib" + robot_name + ".so";

                    // Compile the file into a shared library
                    std::string compile_cmd = "g++ -shared -fPIC -o " + shared_lib + " " + filename + " RobotBase.o RadarObj.o -I. -std=c++20";
                    std::cout << "Compiling " << filename << " to " << shared_lib << "...\n";

                    int compile_result = std::system(compile_cmd.c_str());
                    if (compile_result != 0) 
                    {
                        std::cerr << "Failed to compile " << filename << " with command: " << compile_cmd << std::endl;
                        continue;
                    }

                    // Load the shared library dynamically
                    void* handle = dlopen(shared_lib.c_str(), RTLD_LAZY);
                    if (!handle) 
                    {
                        std::cerr << "Failed to load " << shared_lib << ": " << dlerror() << std::endl;
                        continue;
                    }

                    // Locate the factory function to create the robot
                    using RobotFactory = RobotBase* (*)();
                    RobotFactory create_robot = (RobotFactory)dlsym(handle, "create_robot");
                    if (!create_robot) 
                    {
                        std::cerr << "Failed to find create_robot in " << shared_lib << ": " << dlerror() << std::endl;
                        dlclose(handle);
                        continue;
                    }

                    // Instantiate the robot and add it to the m_robots list
                    RobotBase* robot = create_robot();
                    if (robot) 
                    {
                        //setup the robot
                        robot->set_arena_size(size_row, size_col);
                        robot->m_name = robot_name;

                        // Find a random valid position for the robot
                        int row, col;
                        do 
                        {
                            row = std::rand() % size_row;
                            col = std::rand() % size_col;
                        } while (m_board[row][col] != '.'); // Ensure it's an empty spot

                        robot->set_next_location(row, col);
                        m_board[row][col] = 'R'; // Mark the robot's position on the board
                        m_robots.push_back(robot);

                        std::cout << "Loaded robot: " << robot_name << " at (" << row << ", " << col << ")\n";
                    } 
                    else 
                    {
                        std::cerr << "Failed to create robot from " << shared_lib << std::endl;
                        dlclose(handle);
                    }
                }
            }
        }
    } 
    catch (const fs::filesystem_error& e) 
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }

    return !m_robots.empty(); // Return true if at least one robot was loaded
}


// Given the robot's preference on radar direction, get radar results
void Arena::get_radar_results(RobotBase* robot, int radar_row, int radar_col, RadarObj& radar_results)
{
    // Zero out the radar object
    radar_results.set_empty();

    int current_row, current_col;
    robot->get_current_location(current_row, current_col);

    // Calculate deltas for slope-based movement
    int delta_row = radar_row - current_row;
    int delta_col = radar_col - current_col;

    int steps = std::max(std::abs(delta_row), std::abs(delta_col));
    double slope_row = 0.0;
    double slope_col = 0.0;

    if (steps != 0) 
    {
        slope_row = static_cast<double>(delta_row) / steps;
        slope_col = static_cast<double>(delta_col) / steps;
    }

    double r = current_row, c = current_col;

    // Traverse the ray path step by step to the edge of the board
    while (true)
    {
        // Advance the radar ray
        r += slope_row;
        c += slope_col;

        int ray_row = static_cast<int>(std::round(r));
        int ray_col = static_cast<int>(std::round(c));

        // Stop if out of bounds
        if (ray_row < 0 || ray_row >= size_row || ray_col < 0 || ray_col >= size_col)
        {
            break;
        }

        // Mark the radar path in the RadarObj
        radar_results[ray_row][ray_col] = m_board[ray_row][ray_col];

        // Handle edge cases where radar swath is near the boundaries
        for (int offset = -1; offset <= 1; ++offset)
        {
            int swath_row = ray_row + (delta_col != 0 ? offset : 0);
            int swath_col = ray_col + (delta_row != 0 ? offset : 0);

            // Ensure swath remains within bounds
            if (swath_row >= 0 && swath_row < size_row && swath_col >= 0 && swath_col < size_col)
            {
                radar_results[swath_row][swath_col] = m_board[swath_row][swath_col];
            }
        }

        // If we've reached the edge, stop processing further
        if (ray_row == 0 || ray_row == size_row - 1 || ray_col == 0 || ray_col == size_col - 1)
        {
            break;
        }
    }
}

void Arena::check_radar_location(int& radar_row, int& radar_col, int robot_row, int robot_col) 
{
    // Clamp radar_row within valid bounds
    if (radar_row < 0) radar_row = 0;
    else if (radar_row >= size_row) radar_row = size_row - 1;

    // Clamp radar_col within valid bounds
    if (radar_col < 0) radar_col = 0;
    else if (radar_col >= size_col) radar_col = size_col - 1;

    // Check if radar location matches the robot's location
    if (radar_row == robot_row && radar_col == robot_col) 
    {
        // Default to (0,0) if not already there
        if (robot_row != 0 || robot_col != 0) {
            radar_row = 0;
            radar_col = 0;
        } else {
            // If robot is at (0,0), set radar location to bottom-right corner
            radar_row = size_row - 1;
            radar_col = size_col - 1;
        }
    }
}


// Handle the robot's shot
void Arena::handle_shot(RobotBase* robot, int shot_row, int shot_col) 
{
    WeaponType weapon = robot->get_weapon();
    switch (weapon) 
    {
        case flamethrower:
            std::cout << "flamethrower ";
            handle_flame_shot(robot, shot_row, shot_col);
            break;
        case railgun:
            std::cout << "railgun ";
            handle_railgun_shot(robot, shot_row, shot_col);
            break;
        case grenade:
            std::cout << "grenade ";
            handle_grenade_shot(robot, shot_row, shot_col);
            break;
        case hammer:
            std::cout << "hammer ";
            handle_hammer_shot(robot, shot_row, shot_col);
            break;
        case emp:
            std::cout << "EMP ";
            handle_emp_shot(robot);
            break;
        default:
            break;
    }
}

void Arena::apply_damage_to_robot(int row, int col, WeaponType weapon, std::set<RobotBase*>& robots_hit) 
{
    if (row >= 0 && row < size_row && col >= 0 && col < size_col && m_board[row][col] == 'R') 
    {
        for (auto* target : m_robots) 
        {
            int target_row, target_col;
            target->get_current_location(target_row, target_col);

            if (target_row == row && target_col == col && robots_hit.find(target) == robots_hit.end()) 
            {
                int damage = calculate_damage(weapon, target->get_armor());
                target->take_damage(damage);
                robots_hit.insert(target); // Mark the robot as hit
                std::cout << target->m_name << " at (" << target_row << "," << target_col << ") takes " << damage 
                << " from a " << weapon <<". "<< target->get_health()<< " health remaining.\n";

            }
        }
    }
}



int Arena::calculate_damage(WeaponType weapon, int armor_level) 
{
    int min_damage = 0, max_damage = 0;
    switch (weapon) {
        case flamethrower:
            min_damage = 30;
            max_damage = 50;
            break;
        case railgun:
            min_damage = 10;
            max_damage = 20;
            break;
        case hammer:
            min_damage = 50;
            max_damage = 60;
            break;
        case grenade:
            min_damage = 10;
            max_damage = 40;
            break;
        default:
            return 0; // No damage for unrecognized weapons
    }

    // Generate random damage within the range
    int base_damage = min_damage + (std::rand() % (max_damage - min_damage + 1));

    // Apply armor reduction (10% per armor level, max 40%)
    double armor_multiplier = 1.0 - (0.1 * std::min(armor_level, 4));
    int final_damage = static_cast<int>(base_damage * armor_multiplier);

    return final_damage;
}


void Arena::handle_flame_shot(RobotBase* robot, int shot_row, int shot_col) {
    // Get the current location of the robot firing the flamethrower
    int shooter_row, shooter_col;
    robot->get_current_location(shooter_row, shooter_col);

    // Calculate directional increments
    int delta_row = shot_row - shooter_row;
    int delta_col = shot_col - shooter_col;

    int steps = 3; // Flamethrower range
    double slope_row = static_cast<double>(delta_row) / steps;
    double slope_col = static_cast<double>(delta_col) / steps;

    double r = shooter_row, c = shooter_col;

    // Track robots hit to avoid duplicate damage
    std::set<RobotBase*> robots_hit;

    for (int step = 1; step <= steps; ++step) {
        r += slope_row;
        c += slope_col;

        int path_row = static_cast<int>(std::round(r));
        int path_col = static_cast<int>(std::round(c));

        // Check if the path square is within bounds
        if (path_row >= 0 && path_row < size_row && path_col >= 0 && path_col < size_col) {
            // Damage robots in the path
            apply_damage_to_robot(path_row, path_col, flamethrower, robots_hit);

            // Damage robots adjacent to the path
            for (int row_offset = -1; row_offset <= 1; ++row_offset) {
                for (int col_offset = -1; col_offset <= 1; ++col_offset) {
                    if (row_offset == 0 && col_offset == 0) {
                        continue; // Skip the main path square, already handled
                    }
                    int adj_row = path_row + row_offset;
                    int adj_col = path_col + col_offset;

                    // Check boundaries before applying damage
                    if (adj_row >= 0 && adj_row < size_row && adj_col >= 0 && adj_col < size_col) {
                        apply_damage_to_robot(adj_row, adj_col, flamethrower, robots_hit);
                    }
                }
            }
        }
    }
}


void Arena::handle_railgun_shot(RobotBase* robot, int shot_row, int shot_col) 
{
    int current_row, current_col;
    robot->get_current_location(current_row, current_col);

    int delta_row = shot_row - current_row;
    int delta_col = shot_col - current_col;

    int steps = std::max(std::abs(delta_row), std::abs(delta_col)); 
    double slope_row = steps == 0 ? 0 : static_cast<double>(delta_row) / steps;
    double slope_col = steps == 0 ? 0 : static_cast<double>(delta_col) / steps;

    double r = current_row, c = current_col;

    std::set<RobotBase*> robots_hit;

    for (int step = 1; step <= steps; ++step) {
        r += slope_row;
        c += slope_col;

        int new_row = static_cast<int>(std::round(r));
        int new_col = static_cast<int>(std::round(c));

        if (new_row < 0 || new_row >= size_row || new_col < 0 || new_col >= size_col) {
            break; // Stop if out of bounds
        }

        apply_damage_to_robot(new_row, new_col, railgun, robots_hit);
    }
}

void Arena::handle_grenade_shot(RobotBase* robot, int shot_row, int shot_col) 
{
    int current_row, current_col;
    robot->get_current_location(current_row, current_col);

    int max_distance = 10;
    int distance = std::abs(shot_row - current_row) + std::abs(shot_col - current_col);

    if (distance <= max_distance) {
        std::set<RobotBase*> robots_hit;

        // Explosion area: 3x3 grid around the target location
        for (int r = shot_row - 1; r <= shot_row + 1; ++r) {
            for (int c = shot_col - 1; c <= shot_col + 1; ++c) {
                apply_damage_to_robot(r, c, grenade, robots_hit);
            }
        }
    }
}


void Arena::handle_hammer_shot(RobotBase* robot, int shot_row, int shot_col) 
{
    int current_row, current_col;
    robot->get_current_location(current_row, current_col);

    int delta_row = shot_row - current_row;
    int delta_col = shot_col - current_col;

    if (std::abs(delta_row) <= 1 && std::abs(delta_col) <= 1) {
        int target_row = current_row + delta_row;
        int target_col = current_col + delta_col;

        std::set<RobotBase*> robots_hit;
        apply_damage_to_robot(target_row, target_col, hammer, robots_hit);
    }
}

void Arena::handle_emp_shot(RobotBase* robot) 
{
    int current_row, current_col;
    robot->get_current_location(current_row, current_col);

    std::set<RobotBase*> robots_hit;

    // EMP affects all squares in a 1-square radius
    for (int r = current_row - 1; r <= current_row + 1; ++r) 
    {
        for (int c = current_col - 1; c <= current_col + 1; ++c) 
        {
            apply_damage_to_robot(r, c, emp, robots_hit);
        }
    }
}

void Arena::calculate_increment(const int start_row, const int start_col, const int end_row, const int end_col, double &inc_row, double &inc_col) 
{
    // Calculate the differences in row and column
    int delta_row = end_row - start_row;
    int delta_col = end_col - start_col;

    // Determine the number of steps needed based on the dominant axis
    int steps = std::max(std::abs(delta_row), std::abs(delta_col));

    // Handle the case where the start and destination are the same
    if (steps == 0) 
    {
        inc_row = 0.0;
        inc_col = 0.0;
        return;
    }

    // Calculate the increments
    inc_row = static_cast<double>(delta_row) / steps;
    inc_col = static_cast<double>(delta_col) / steps;
}

void Arena::handle_move(RobotBase* robot, int target_row, int target_col) 
{
    int current_row, current_col;
    robot->get_current_location(current_row, current_col);

    // Remove the robot's marker from its current position
    m_board[current_row][current_col] = '.';

    // Store the maximum movement allowed for the robot
    int max_move = robot->get_move();

    // Ensure the target location is within bounds
    target_row = std::max(0, std::min(target_row, size_row - 1));
    target_col = std::max(0, std::min(target_col, size_col - 1));

    // Calculate increments for movement
    double inc_row, inc_col;
    calculate_increment(current_row, current_col, target_row, target_col, inc_row, inc_col);

    double r = current_row, c = current_col;

    // Traverse the path step by step
    for (int step = 1; step <= max_move; ++step) 
    {
        r += inc_row;
        c += inc_col;

        int intermediate_row = static_cast<int>(std::round(r));
        int intermediate_col = static_cast<int>(std::round(c));

        // Check and adjust boundaries
        if (intermediate_row < 0) 
        {
            intermediate_row = 0;
        } 
        else if (intermediate_row >= size_row) 
        {
            intermediate_row = size_row - 1;
        }

        if (intermediate_col < 0) 
        {
            intermediate_col = 0;
        } 
        else if (intermediate_col >= size_col) 
        {
            intermediate_col = size_col - 1;
        }

        char cell = m_board[intermediate_row][intermediate_col];

        if (cell == 'M') 
        { // Mound
            std::cout << robot->m_name << " is stopped by a mound at (" << intermediate_row << "," << intermediate_col
                      << "). Current health: " << robot->get_health() << std::endl;
            break;
        } 
        else if (cell == 'R') 
        { // Another robot
            std::cout << robot->m_name << " runs into another robot at (" << intermediate_row << "," << intermediate_col
                      << "). Current health: " << robot->get_health() << std::endl;
            break;
        } 
        else if (cell == 'P') 
        { // Pit
            robot->set_next_location(intermediate_row, intermediate_col);
            m_board[intermediate_row][intermediate_col] = 'R'; // Robot now occupies the pit
            robot->disable_movement();
            std::cout << robot->m_name << " gets stuck in a pit at (" << intermediate_row << "," << intermediate_col
                      << "). Movement is disabled. Current health: " << robot->get_health() << std::endl;
            return; 
        } 
        else if (cell == 'F') 
        { // Flamethrower
            std::set<RobotBase*> robots_hit; 
            apply_damage_to_robot(intermediate_row, intermediate_col, flamethrower, robots_hit);
            break;
        }

        // Update the current position
        current_row = intermediate_row;
        current_col = intermediate_col;
    }

    // Update the robot's position to the final valid location
    robot->set_next_location(current_row, current_col);

    // Mark the robot's new location on the board
    m_board[current_row][current_col] = 'R';

    std::cout << robot->m_name << " moves to (" << current_row << "," << current_col
              << "). Current health: " << robot->get_health() << std::endl;
}



void Arena::initialize_board() 
{
    // Resize the board and initialize all cells to '.'
    m_board.resize(size_row, std::vector<char>(size_col, '.'));

    // Determine the maximum number of obstacles based on the size of the board
    int total_cells = size_row * size_col;
    int max_obstacles = (total_cells > 500) ? 10 : std::min(8, total_cells / 100);

    // Obstacle types to place
    std::vector<char> obstacle_types = {'M', 'P', 'F'};

    // Seed the random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    for (char obstacle : obstacle_types) 
    {
        // Random number of obstacles for this type (between 0 and max_obstacles)
        int obstacle_count = std::rand() % (max_obstacles + 1);

        for (int i = 0; i < obstacle_count; ++i) 
        {
            int row, col;
            do 
            {
                // Randomly generate a position within the board
                row = std::rand() % size_row;
                col = std::rand() % size_col;
            } 
            while (m_board[row][col] != '.'); // Ensure the position is empty

            // Place the obstacle
            m_board[row][col] = obstacle;
        }
    }

}


void Arena::print_board() const
{
    // Calculate consistent spacing for the columns
    const int col_width = 3; // Adjust width for even spacing (enough for two-digit numbers and a space)

    // Print column headers with consistent spacing
    std::cout << "   "; // Leading space for row indices
    for (int col = 0; col < size_col; ++col)
    {
        std::cout << std::setw(col_width) << col; // Use std::setw for fixed width
    }
    std::cout << std::endl;

    // Print each row of the arena
    for (int row = 0; row < size_row; ++row)
    {
        // Print row index with consistent spacing
        std::cout << std::setw(2) << row << " "; // Row indices aligned with column headers

        // Print the contents of the row
        for (int col = 0; col < size_col; ++col)
        {
            std::cout << std::setw(col_width) << m_board[row][col];
        }
        std::cout << std::endl;
    }
}


bool Arena::winner()
{
    int num_living_robots = 0;
    for (auto* robot : m_robots)
    {
        if(robot->get_health() > 0)
            num_living_robots++;
    }

    return num_living_robots == 1;

}

// Run the simulation
// assumes robots have been loaded.
void Arena::run_simulation() {

    if(m_robots.size() == 0)
    {
        std::cout << "Robot list did not load.";
        return;
    }

    RadarObj radar_results = RadarObj(size_row, size_col); // Ensure proper size
    radar_results.set_empty();
    int round = 0;

    while(!winner() && round < 1000000)
    {
        std::cout << "=========== starting round " << round << " ==========="<<std::endl;
        print_board();

        for (auto* robot : m_robots) 
        {

            if(robot->get_health() < 0)
            {
                std::cout << robot->m_name << " is out." << std::endl;
                continue;
            }

            std::cout << robot->m_name << " begins turn." << std::endl;
            robot->print_stats();

            if(robot->radar_enabled())
            {
                std::cout << "  checking radar at ";
                int radar_row = 0, radar_col = 0;
                robot->get_radar_location(radar_row, radar_col);

                // Get the robot's current location
                int current_row = 0, current_col = 0;
                robot->get_current_location(current_row, current_col);

                // If radar points to the robot's current position, adjust it
                if (radar_row == current_row && radar_col == current_col) 
                {
                    if(current_row !=0 && current_col !=0)
                        radar_row = radar_col = 0;
                    else
                    {
                        radar_row = size_row-1;
                        radar_col = size_col-1;
                    }    
                }

                std::cout << "(" << radar_row << "," << radar_col << ")\n";

                RadarObj radar_results(size_row, size_col); // Create RadarObj for results
                get_radar_results(robot, radar_row, radar_col, radar_results);
                robot->process_radar_results(radar_results);
            }

            int shot_row = 0, shot_col = 0;
            if (robot->get_shot_location(shot_row, shot_col)) 
            {
                std::cout << "Shooting: " ;
                handle_shot(robot, shot_row, shot_col);
            } 
            else 
            {
                int move_row = 0, move_col = 0;
                if (robot->get_move_direction(move_row, move_col)) 
                {
                    handle_move(robot, move_row, move_col);
                }
            }

        }
        round++;
    }

    std::cout << "game over.";

}