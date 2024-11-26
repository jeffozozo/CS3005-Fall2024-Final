#include "Arena.h"

// Set the size of the arena
void Arena::set_size(int row_in, int col_in) {
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
    try {
        // Scan the current directory for Robot_<name>.cpp files
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();

                // Check if the file matches the naming pattern Robot_<name>.cpp
                if (filename.rfind("Robot_", 0) == 0 && filename.substr(filename.size() - 4) == ".cpp") {
                    std::string robot_name = filename.substr(6, filename.size() - 10); // Extract <name>

                    // Compile the file into a shared library
                    std::string shared_lib = "lib" + robot_name + ".so";
                    std::string compile_cmd = "g++ -shared -fPIC -o " + shared_lib + " " + filename + " -I. -std=c++17";
                    if (std::system(compile_cmd.c_str()) != 0) {
                        std::cerr << "Failed to compile " << filename << std::endl;
                        continue;
                    }

                    // Load the shared library dynamically
                    void* handle = dlopen(shared_lib.c_str(), RTLD_LAZY);
                    if (!handle) {
                        std::cerr << "Failed to load " << shared_lib << ": " << dlerror() << std::endl;
                        continue;
                    }

                    // Locate the factory function to create the robot
                    using RobotFactory = RobotBase* (*)();
                    RobotFactory create_robot = (RobotFactory)dlsym(handle, "create_robot");
                    if (!create_robot) {
                        std::cerr << "Failed to find create_robot in " << shared_lib << ": " << dlerror() << std::endl;
                        dlclose(handle);
                        continue;
                    }

                    // Instantiate the robot and add it to the m_robots list
                    RobotBase* robot = create_robot();
                    if (robot) {
                        m_robots.push_back(robot);
                        std::cout << "Loaded robot: " << robot_name << std::endl;
                    } else {
                        std::cerr << "Failed to create robot from " << shared_lib << std::endl;
                    }
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }

    return !m_robots.empty(); // Return true if at least one robot was loaded
}

// Get radar results for a robot
void Arena::get_radar_results(int radar_row, int radar_col, RadarObj &radar_results) {
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



void Arena::apply_damage_to_robot(int row, int col, WeaponType weapon, std::set<RobotBase*>& robots_hit) {
    if (row >= 0 && row < size_row && col >= 0 && col < size_col && m_board[row][col] == 'R') {
        for (auto* target : m_robots) {
            if (target->m_location_x == row && target->m_location_y == col && robots_hit.find(target) == robots_hit.end()) {
                target->take_damage(calculate_damage(weapon, target));
                robots_hit.insert(target); // Mark the robot as hit
            }
        }
    }
}

// Calculate damage based on weapon type
int Arena::calculate_damage(WeaponType weapon, const RobotBase* bot) {
    // Implement damage calculation logic here
    return 0;
}

void Arena::handle_flame_shot(RobotBase* robot, int shot_row, int shot_col) {
    int current_row, current_col;
    robot->get_current_location(current_row, current_col);

    int delta_row = shot_row - current_row;
    int delta_col = shot_col - current_col;

    double steps = 3; // Flamethrower range
    double slope_row = static_cast<double>(delta_row) / steps;
    double slope_col = static_cast<double>(delta_col) / steps;

    double r = current_row, c = current_col;

    for (int step = 1; step <= steps; ++step) {
        r += slope_row;
        c += slope_col;

        int new_row = static_cast<int>(std::round(r));
        int new_col = static_cast<int>(std::round(c));

        // Main path square
        apply_damage_to_robot(new_row, new_col, flamethrower);

        // Adjacent squares
        apply_damage_to_robot(new_row - 1, new_col, flamethrower);
        apply_damage_to_robot(new_row + 1, new_col, flamethrower);
        apply_damage_to_robot(new_row, new_col - 1, flamethrower);
        apply_damage_to_robot(new_row, new_col + 1, flamethrower);
    }
}




// Handle robot movement
void Arena::handle_move(RobotBase* robot, int move_row, int move_col) {
    // (Insert your refactored handle_move function here)
}

// Run the simulation
void Arena::run_simulation() {

    if(m_robots.size() == 0)
    {
        std::cout << "Robot list did not load.";
        exit(-1);
    }

    for (auto* robot : m_robots) 
    {
        int radar_row = 0, radar_col = 0;
        robot->get_radar_location(radar_row, radar_col);

        RadarObj radar_results(radar_row, radar_col);
        get_radar_results(radar_row, radar_col, radar_results);
        robot->set_radar_results(radar_results);

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