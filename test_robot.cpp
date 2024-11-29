#include "RobotBase.h"
#include <iostream>
#include <dlfcn.h>
#include <string>

bool test_robot(const std::string& shared_lib, int arena_rows, int arena_cols) {
    std::cout << "Testing robot..." << std::endl;

    // Dynamically load the shared library
    void* handle = dlopen(shared_lib.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Failed to load " << shared_lib << ": " << dlerror() << std::endl;
        return false;
    }

    // Locate the factory function to create the robot
    using RobotFactory = RobotBase* (*)();
    RobotFactory create_robot = (RobotFactory)dlsym(handle, "create_robot");
    if (!create_robot) {
        std::cerr << "Failed to find create_robot in " << shared_lib << ": " << dlerror() << std::endl;
        dlclose(handle);
        return false;
    }

    // Instantiate the robot
    RobotBase* robot = create_robot();
    if (!robot) {
        std::cerr << "Failed to create robot instance from " << shared_lib << std::endl;
        dlclose(handle);
        return false;
    }

    // Setup the robot
    robot->set_arena_size(arena_rows, arena_cols);
    robot->m_name = "Default";

    // Place the robot in the arena at the top-left corner
    robot->set_next_location(0, 0);

    // Print robot stats
    robot->print_stats();

    // Test get_radar_location
    int radar_row = 0, radar_col = 0;
    robot->get_radar_location(radar_row, radar_col);
    std::cout << "  get_radar_location: (" << radar_row << ", " << radar_col << ")" << std::endl;

    // Check radar location bounds
    if (radar_row < 0 || radar_row >= arena_rows || radar_col < 0 || radar_col >= arena_cols) {
        std::cerr << "Error: Radar location (" << radar_row << ", " << radar_col << ") is out of bounds!" << std::endl;
        dlclose(handle);
        return false;
    }

    // Create a dummy radar object to simulate radar results
    RadarObj radar_results(arena_rows, arena_cols);
    for (int r = 0; r < arena_rows; ++r) {
        for (int c = 0; c < arena_cols; ++c) {
            radar_results[r][c] = '.'; // Initialize as empty
        }
    }

    // Test process_radar_results
    try {
        robot->process_radar_results(radar_results);
        std::cout << "  process_radar_results: Radar processed successfully." << std::endl;
    } catch (...) {
        std::cerr << "Error: process_radar_results caused an exception." << std::endl;
        dlclose(handle);
        return false;
    }

    // Test get_shot_location
    int shot_row = 0, shot_col = 0;
    if (robot->get_shot_location(shot_row, shot_col)) {
        std::cout << "  get_shot_location: Shooting at (" << shot_row << ", " << shot_col << ")" << std::endl;

        // Check shot location bounds
        if (shot_row < 0 || shot_row >= arena_rows || shot_col < 0 || shot_col >= arena_cols) {
            std::cerr << "Error: Shot location (" << shot_row << ", " << shot_col << ") is out of bounds!" << std::endl;
        }
    } else {
        std::cout << "  get_shot_location: No target to shoot." << std::endl;
    }

    // Test get_move_direction
    int move_row = 0, move_col = 0;
    if (robot->get_move_direction(move_row, move_col)) {
        std::cout << "  get_move_direction: Moving by (" << move_row << ", " << move_col << ")" << std::endl;

        // Simulate movement and check bounds
        int current_row, current_col;
        robot->get_current_location(current_row, current_col);
        int target_row = current_row + move_row;
        int target_col = current_col + move_col;

        if (target_row < 0 || target_row >= arena_rows || target_col < 0 || target_col >= arena_cols) {
            std::cerr << "Error: Movement target (" << target_row << ", " << target_col << ") is out of bounds!" << std::endl;
        }
    } else {
        std::cout << "  get_move_direction: No movement required." << std::endl;
    }

    // Cleanup
    delete robot;
    dlclose(handle);

    std::cout << "Robot testing complete." << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <robot_name.cpp>" << std::endl;
        return 1;
    }

    const std::string robot_file = argv[1];
    const std::string shared_lib = "lib" + robot_file.substr(0, robot_file.find(".cpp")) + ".so";

    // Compile the robot into a shared library
    std::string compile_cmd = "g++ -shared -fPIC -o " + shared_lib + " " + robot_file + " RobotBase.o RadarObj.o -I. -std=c++20";
    std::cout << "Compiling " << robot_file << " into " << shared_lib << "..." << std::endl;

    if (std::system(compile_cmd.c_str()) != 0) {
        std::cerr << "Failed to compile " << robot_file << " into " << shared_lib << std::endl;
        return 1;
    }

    // Test the robot
    if (!test_robot(shared_lib, 20, 20)) {
        std::cerr << "Robot test failed." << std::endl;
        return 1;
    }

    return 0;
}