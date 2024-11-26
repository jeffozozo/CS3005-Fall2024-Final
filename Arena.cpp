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
                        // Set the arena size for the robot
                        robot->set_arena_size(size_row, size_col);

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


// Get radar results for a robot
void Arena::get_radar_results(int radar_row, int radar_col, RadarObj &radar_results) 
{
    //zero out the radar object.
    radar_results.set_empty();

    for (int r = radar_row - 1; r <= radar_row + 1; ++r) {
        for (int c = radar_col - 1; c <= radar_col + 1; ++c) {
            if (r >= 0 && r < size_row && c >= 0 && c < size_col) {
                char cell = m_board[r][c];
                if (cell == 'E') {
                    radar_results.m_enemies.push_back(r);
                    radar_results.m_enemies.push_back(c);
                    radar_results.enemies_found = true;
                } else if (cell == 'M' || cell == 'P' || cell == 'F') {
                    radar_results.m_obstacles.push_back('M');
                    radar_results.m_obstacles.push_back(r);
                    radar_results.m_obstacles.push_back(c);
                    radar_results.obstacles_found = true;
                }
            }
        }
    }
}

// Handle the robot's shot
void Arena::handle_shot(RobotBase* robot, int shot_row, int shot_col) {
    WeaponType weapon = robot->get_weapon();
    switch (weapon) {
        case flamethrower:
            handle_flame_shot(robot, shot_row, shot_col);
            break;
        case railgun:
            handle_railgun_shot(robot, shot_row, shot_col);
            break;
        case grenade:
            handle_grenade_shot(robot, shot_row, shot_col);
            break;
        case hammer:
            handle_hammer_shot(robot, shot_row, shot_col);
            break;
        case emp:
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
                target->take_damage(calculate_damage(weapon, target->get_armor()));
                robots_hit.insert(target); // Mark the robot as hit
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


void Arena::handle_flame_shot(RobotBase* robot, int shot_row, int shot_col) 
{
    int current_row, current_col;
    robot->get_current_location(current_row, current_col);

    int delta_row = shot_row - current_row;
    int delta_col = shot_col - current_col;

    double steps = 3; // Flamethrower range
    double slope_row = static_cast<double>(delta_row) / steps;
    double slope_col = static_cast<double>(delta_col) / steps;

    double r = current_row, c = current_col;

    // Track robots hit to prevent duplicates where needed
    std::set<RobotBase*> robots_hit;

    for (int step = 1; step <= steps; ++step) {
        r += slope_row;
        c += slope_col;

        int new_row = static_cast<int>(std::round(r));
        int new_col = static_cast<int>(std::round(c));

        // Main path square
        apply_damage_to_robot(new_row, new_col, flamethrower, robots_hit);

        // Adjacent squares
        apply_damage_to_robot(new_row - 1, new_col, flamethrower, robots_hit);
        apply_damage_to_robot(new_row + 1, new_col, flamethrower, robots_hit);
        apply_damage_to_robot(new_row, new_col - 1, flamethrower, robots_hit);
        apply_damage_to_robot(new_row, new_col + 1, flamethrower, robots_hit);
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

void Arena::handle_move(RobotBase* robot, int move_row, int move_col) 
{
    int current_row, current_col;
    robot->get_current_location(current_row, current_col);

    // Store the maximum movement allowed for the robot
    int max_move = robot->get_move();

    // Calculate the target location
    int target_row = current_row + move_row * max_move;
    int target_col = current_col + move_col * max_move;

    // Ensure the target location is within bounds
    target_row = std::max(0, std::min(target_row, size_row - 1));
    target_col = std::max(0, std::min(target_col, size_col - 1));

    // Slope-based movement
    int delta_row = target_row - current_row;
    int delta_col = target_col - current_col;

    int steps = std::max(std::abs(delta_row), std::abs(delta_col)); // Use dominant axis for steps
    double slope_row = steps == 0 ? 0 : static_cast<double>(delta_row) / steps;
    double slope_col = steps == 0 ? 0 : static_cast<double>(delta_col) / steps;

    double r = current_row, c = current_col;

    // Traverse the path
    for (int step = 1; step <= steps; ++step) 
    {
        r += slope_row;
        c += slope_col;

        int intermediate_row = static_cast<int>(std::round(r));
        int intermediate_col = static_cast<int>(std::round(c));

        // Stop if out of bounds
        if (intermediate_row < 0 || intermediate_row >= size_row || 
            intermediate_col < 0 || intermediate_col >= size_col) {
            break;
        }

        char cell = m_board[intermediate_row][intermediate_col];

        if (cell == 'M') { // Mound
            break; // Stop movement before the mound
        } else if (cell == 'R') { // Another robot
            break; // Stop movement before another robot
        } else if (cell == 'P') { // Pit
            // Robot gets stuck in the pit
            robot->set_next_location(intermediate_row, intermediate_col);
            m_board[current_row][current_col] = '.'; // Clear old position
            m_board[intermediate_row][intermediate_col] = 'R'; // Robot now occupies the pit
            robot->disable_movement(); // Disable further movement
            return; // Movement ends here
        } else if (cell == 'F') { // Flamethrower
            // Robot takes damage
            std::set<RobotBase*> robots_hit; // To comply with apply_damage_to_robot signature
            apply_damage_to_robot(intermediate_row, intermediate_col, flamethrower, robots_hit);
            break; // Stop movement after taking damage
        }

        // Stop if we've reached the maximum movement allowed
        if (step == max_move) {
            break;
        }
    }

    // Update the board and robot position
    m_board[current_row][current_col] = '.'; // Clear old position
    m_board[static_cast<int>(std::round(r))][static_cast<int>(std::round(c))] = 'R'; // Mark new position
    robot->set_next_location(static_cast<int>(std::round(r)), static_cast<int>(std::round(c)));
}


void Arena::initialize_board() 
{
    // Resize the board to the specified dimensions
    m_board.resize(size_row, std::vector<char>(size_col, '.'));


    // Define obstacle and enemy types
    const std::vector<char> elements = {'M', 'P', 'F'}; // Mound, Pit, Flamethrower
    const int total_elements = static_cast<int>(elements.size());

    // Fill the board with random elements
    for (int r = 0; r < size_row; ++r) {
        for (int c = 0; c < size_col; ++c) {
            // Randomly decide whether to place an obstacle/enemy or leave it empty
            int random_value = std::rand() % 10; // 10% chance to place an item
            if (random_value < total_elements) {
                m_board[r][c] = elements[random_value];
            }
        }
    }
}



// Run the simulation
// assumes robots have been loaded.
void Arena::run_simulation() {

    if(m_robots.size() == 0)
    {
        std::cout << "Robot list did not load.";
        return;
    }

    for (auto* robot : m_robots) 
    {
        if(robot->get_health() < 0)
            continue;

        if(robot->radar_enabled())
        {
            int radar_row = 0, radar_col = 0;
            robot->get_radar_location(radar_row, radar_col);

            RadarObj radar_results;
            get_radar_results(radar_row, radar_col, radar_results);
            robot->set_radar_results(radar_results);
        }

        int shot_row = 0, shot_col = 0;
        if (robot->get_shot_location(shot_row, shot_col)) 
        {
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
}