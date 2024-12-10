#include "TestArena.h"
#include <iomanip> // For std::setw

void TestArena::print_test_result(const std::string& test_name, bool condition) {
    const std::string green = "\033[32m";  // ANSI escape code for green
    const std::string red = "\033[31m";    // ANSI escape code for red
    const std::string reset = "\033[0m";   // ANSI escape code to reset color
    const int result_column_width = 50;    // Adjust this width as needed for alignment

    std::cout << "  Test: " << std::left << std::setw(result_column_width) << test_name;
    if (condition) {
        std::cout << green << "[ok]" << reset << "\n";
    } else {
        std::cout << red << "[failed]" << reset << "\n";
    }
}

void TestArena::test_initialize_board() {
    std::cout << "Testing initialize_board...\n";
    Arena arena(10, 10);
    arena.initialize_board();

    // Valid characters on the board after initialization
    std::set<char> valid_cells = {'.', 'M', 'P', 'F'};
    bool board_initialized = true;

    // Check that all cells are one of the valid characters
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            char cell = arena.m_board[row][col];
            if (valid_cells.find(cell) == valid_cells.end()) {
                board_initialized = false;
                std::cout << "  [failed] Board contains '" << cell 
                          << "' at (" << row << "," << col << "), which is not in the list ('.', 'M', 'P', 'F').\n";
                break;
            }
        }
        if (!board_initialized) {
            break;
        }
    }

    if (board_initialized) {
        std::cout << "  [ok] Board contains only valid characters ('.', 'M', 'P', 'F').\n";
    }
}


// Test handle_move

void TestArena::test_handle_move() {
    std::cout << "\nTesting handle_move...\n";
    Arena arena1(10, 10);
    arena1.initialize_board(true); //empty board

    // **Test 1: Boundary conditions with RobotOutOfBounds**
    std::cout << "*** testing out of bounds: \n" << std::endl;
    RobotOutOfBounds robotOutOfBounds;
    robotOutOfBounds.move_to(5, 5); // Start the robot at (5, 5)
    robotOutOfBounds.set_boundaries(10, 10);

    struct TestCase {
        int start_row, start_col;
        int expected_row, expected_col;
        std::string description;
    };

    std::vector<TestCase> boundary_tests = {
        {5, 5, 7, 7, "Boundary movement case 1 (valid move to (7,7))"},
        {9, 9, 9, 9, "Boundary movement case 2 (move out of bounds right to (9,9))"},
        {9, 9, 9, 9, "Boundary movement case 3 (move out of bounds down to (9,9))"},
        {0, 0, 0, 0, "Boundary movement case 4 (move out of bounds up to (0,0))"},
        {0, 0, 0, 0, "Boundary movement case 5 (move out of bounds left to (0,0))"}
    };

    for (size_t i = 0; i < boundary_tests.size(); ++i) {
        const auto& test_case = boundary_tests[i];

        // Move the robot to the starting location
        robotOutOfBounds.move_to(test_case.start_row, test_case.start_col);

        // Run the move logic
        arena1.handle_move(&robotOutOfBounds);

        // Get the robot's resulting location
        int result_row, result_col;
        robotOutOfBounds.get_current_location(result_row, result_col);

        // Print the test details and results
        std::cout << "  Test: " << test_case.description << "\n";
        std::cout << "    Start location: (" << test_case.start_row << ", " << test_case.start_col << ")\n";
        std::cout << "    Expected location: (" << test_case.expected_row << ", " << test_case.expected_col << ")\n";
        std::cout << "    Result location: (" << result_row << ", " << result_col << ")\n";
        print_test_result(test_case.description, (result_row == test_case.expected_row && result_col == test_case.expected_col));
    }

    // **Test 2: BadMovesRobot scenarios**
    Arena arena2(10,10);
    arena2.initialize_board(true); //empty board
    std::cout << "\n\n*** testing bad move parameters: \n";
    BadMovesRobot badMovesRobot;
    badMovesRobot.set_boundaries(10, 10);

    struct BadMovesTestCase {
        int expected_row, expected_col;
        std::string description;
    };

    std::vector<BadMovesTestCase> bad_moves_tests = 
    {
        {5, 7, "Move too far horizontally (distance > speed)"},
        {5, 5, "Negative distance"},
        {5, 5, "Invalid direction (> 8)"},
        {5, 5, "Invalid direction (0)"}
    };

    for (size_t i = 0; i < bad_moves_tests.size(); ++i) {
        const auto& test_case = bad_moves_tests[i];
        std::cout << "test case: " << test_case.description << std::endl;
        badMovesRobot.move_to(5, 5); // Start the robot at (5, 5)

        // Run the move logic
        arena2.handle_move(&badMovesRobot);

        // Get the robot's resulting location
        int result_row, result_col;
        badMovesRobot.get_current_location(result_row, result_col);

        // Print the test details and results
        std::cout << "    Expected location: (" << test_case.expected_row << ", " << test_case.expected_col << ")\n";
        std::cout << "    Result location: (" << result_row << ", " << result_col << ")\n";
        print_test_result(test_case.description, (result_row == test_case.expected_row && result_col == test_case.expected_col));

    }

    // **Test 3: JumperRobot obstacle tests**
Arena arena3(10, 10);
arena3.initialize_board(true); // empty board
std::cout << "\n*** testing obstacles: \n";
std::unique_ptr<JumperRobot> jumperBot = std::make_unique<JumperRobot>();
jumperBot->move_to(4, 1); // Start the robot at (4, 1)
jumperBot->set_boundaries(10, 10);

struct ObstacleTestCase {
    char obstacle;
    int obstacle_row, obstacle_col;
    int expected_row, expected_col;
    std::string description;
};

std::vector<ObstacleTestCase> obstacle_tests = {
    {'M', 4, 2, 4, 1, "Obstacle 'M' (stop before obstacle)"},
    {'X', 4, 3, 4, 2, "Obstacle 'X' (stop before obstacle)"},
    {'R', 4, 4, 4, 3, "Obstacle 'R' (stop before obstacle)"},
    {'P', 4, 5, 4, 4, "Obstacle 'P' (stop before obstacle)"},
    {'F', 4, 6, 4, 5, "Obstacle 'F' (stop before obstacle)"},
    {'X', 4, 9, 4, 6, "Obstacle 'X' (not hit obstacle)"}
};

for (size_t i = 0; i < obstacle_tests.size(); ++i) {
    const auto& test_case = obstacle_tests[i];
    std::cout << "  Test: " << test_case.description << "\n";

    // Set up the board with obstacle and robot in correct place.
    arena3.initialize_board(true);
    jumperBot->move_to(4, 1);

    // Add obstacle
    arena3.m_board[test_case.obstacle_row][test_case.obstacle_col] = test_case.obstacle;

    // Run the move logic
    std::cout << "trying to move..." << std::endl;
    arena3.handle_move(jumperBot.get());

    // Get the robot's resulting location
    int result_row, result_col;
    jumperBot->get_current_location(result_row, result_col);

    // Print the test details and results
    std::cout << "    Obstacle was at: (" << test_case.obstacle_row << ", " << test_case.obstacle_col << ")\n";
    std::cout << "    Expected stopping location: (" << test_case.expected_row << ", " << test_case.expected_col << ")\n";
    std::cout << "    Result stopping location: (" << result_row << ", " << result_col << ")\n";
    print_test_result(test_case.description, (result_row == test_case.expected_row && result_col == test_case.expected_col));

    // Reset the robot's position and clear the obstacle
    arena3.m_board[test_case.obstacle_row][test_case.obstacle_col] = '.';
    arena3.m_board[result_row][result_col] = '.';

    // Reinitialize the robot if movement is disabled
    if (jumperBot->get_move() == 0) 
    {  
        jumperBot = std::make_unique<JumperRobot>();
        jumperBot->move_to(4, 1);
        jumperBot->set_boundaries(10, 10);
    }
}
    // **Test 4: Disabled movement**
    Arena arena4(10,10);
    arena4.initialize_board(true); //empty board
    std::cout << "\n*** Testing disabled movement: \n";
    JumperRobot disabledRobot;
    disabledRobot.move_to(6, 6); // Start at (6, 6)
    disabledRobot.set_boundaries(10, 10);

    // Test normal movement
    arena4.handle_move(&disabledRobot);
    int before_disable_row, before_disable_col;
    disabledRobot.get_current_location(before_disable_row, before_disable_col);
    print_test_result("Normal movement before disabling", (before_disable_row != 6 || before_disable_col != 6));

    // Disable movement and test
    disabledRobot.disable_movement();
    arena4.handle_move(&disabledRobot);
    int after_disable_row, after_disable_col;
    disabledRobot.get_current_location(after_disable_row, after_disable_col);
    print_test_result("No movement after disabling", (after_disable_row == before_disable_row && after_disable_col == before_disable_col));

    std::cout << "*** move testing complete ***\n\n";
}



// Test handle_collision
void TestArena::test_handle_collision() {
    std::cout << "\nTesting handle_collision...\n";
    Arena arena(10, 10);
    arena.initialize_board();
    TestRobot robot(5, 3, flamethrower, "CollisionBot");

    // Test collision with mound
    arena.m_board[4][4] = 'M';
    robot.move_to(4, 4);
    arena.handle_collision(&robot, 'M', 4, 4);
    print_test_result("Collision with mound", true);

    // Test collision with pit
    arena.m_board[3][3] = 'P';
    robot.move_to(3, 3);
    arena.handle_collision(&robot, 'P', 3, 3);
    print_test_result("Collision with pit", !robot.get_move());

    // Test collision with another robot
    arena.m_board[2][2] = 'R';
    robot.move_to(2, 2);
    arena.handle_collision(&robot, 'R', 2, 2);
    print_test_result("Collision with robot", true);
}

// Test handle_shot with fake radar
void TestArena::test_handle_shot_with_fake_radar() {
    std::cout << "\nTesting handle_shot with fake radar...\n";
    Arena arena(10, 10);
    arena.initialize_board();
    TestRobot shooter(5, 3, flamethrower, "ShooterBot");
    shooter.move_to(5, 5);

    // Create fake radar results
    std::vector<RadarObj> radar_results;
    radar_results.emplace_back('R', 6, 6); // Robot target
    radar_results.emplace_back('.', 7, 7); // Empty cell

    // Test handle_shot with flamethrower
    arena.handle_shot(&shooter, 6, 6); // Target should take damage
    print_test_result("Flamethrower shot at target", true);

    arena.handle_shot(&shooter, 7, 7); // Empty cell, no damage expected
    print_test_result("Flamethrower shot at empty cell", true);
}

// Test RobotBase creation
void TestArena::test_robot_creation() {
    std::cout << "\nTesting RobotBase creation...\n";

    TestRobot excessiveBot1(10, 10, flamethrower, "ExcessiveBot - 10,10");
    print_test_result("ExcessiveBot move clamped at 5", excessiveBot1.get_move() == 5);
    print_test_result("ExcessiveBot armor clamped at 2", excessiveBot1.get_armor() == 2);

    TestRobot excessiveBot2(0, 10, flamethrower, "ExcessiveBot - 0,10");
    std::cout << "bot2 move:" << excessiveBot2.get_move() << " armor:" << excessiveBot2.get_armor() << std::endl;
    print_test_result("ExcessiveBot move clamped at 2", excessiveBot2.get_move() == 2);
    print_test_result("ExcessiveBot armor clamped at 5", excessiveBot2.get_armor() == 5);


    TestRobot negativeBot(-1, -1, flamethrower, "NegativeBot");
    print_test_result("NegativeBot move clamped", negativeBot.get_move() == 2);
    print_test_result("NegativeBot armor clamped", negativeBot.get_armor() == 0);
}


// Test BadRobot with all weapons
void TestArena::test_bad_robot_with_all_weapons() 
{
    WeaponType weapons[] = {flamethrower, railgun, grenade, hammer};

    struct ShotTestCase 
    {
    int weapon;
    int in_range_row, in_range_col;
    std::string description;
    };

    std::vector<ShotTestCase> weapon_tests = 
    {
        {flamethrower, 3, 3, "flamethrower test, range 2,2"},
        {railgun, 5,5, "railgun test, 5,5"},
        {grenade, 7,6, "grenade test, 7,6"},
        {hammer, 2,2, "hammer test, 1,1"}
    };

    for (WeaponType weapon : weapons) 
    {
        std::cout << "\nTesting: " << weapon_tests[weapon].description << "\n";

        // **Valid Target Test**
        std::cout << "*** Testing valid target...\n";
        Arena arena(20, 20); // Create a 20x20 arena
        arena.initialize_board(true); // Empty board

        ShooterRobot shooter(weapon, "ShooterBot");
        TestRobot target(5, 3, hammer, "TargetBot");
        shooter.set_boundaries(20, 20);
        target.set_boundaries(20, 20);

        arena.m_robots.clear();
        arena.m_robots.push_back(&shooter);
        arena.m_robots.push_back(&target);

        arena.m_board[1][1] = 'R';
        shooter.move_to(1, 1);
        std::cout << "Shooter at (1,1)\n";

        int target_row = weapon_tests[weapon].in_range_row;
        int target_col = weapon_tests[weapon].in_range_col;
        arena.m_board[target_row][target_col] = 'R';
        target.move_to(target_row, target_col); // Position the target within range based on the weapon
        std::cout << "Target Robot at (" << target_row << "," << target_col << ")" << std::endl;


        // Construct radar_results with the target
        std::vector<RadarObj> radar_results;
        radar_results.emplace_back('R', target_row, target_col);

        shooter.process_radar_results(radar_results);
        int shot_row, shot_col;

        if (shooter.get_shot_location(shot_row, shot_col)) 
        {
            std::cout << "Shot location specified: (" << shot_row << "," << shot_col << ")\n";
            int target_health_before = target.get_health();
            int target_armor_before = target.get_armor();

            std::cout << "calling handle shot...\n";
            arena.handle_shot(&shooter, shot_row, shot_col);

            int target_health_after = target.get_health();
            int target_armor_after = target.get_armor();

            std::cout << "Before: h:" << target_health_before << " a:" << target_armor_before << std::endl;
            std::cout << "After: h: " << target_health_after << " a: " << target_armor_after << std::endl;

            bool valid_shot = (target_health_after < target_health_before && target_armor_after < target_armor_before);
            print_test_result("Valid shot: target takes damage", valid_shot);
        } 
        else 
        {
            std::cout << "Shooter did not take the shot.\n"; 
            print_test_result("Valid shot: shooter did not shoot", false);
        }

        // ** Invalid Target Test ************************************
        std::cout << "*** Testing out of range\n";
        ShooterRobot Nextshooter(weapon, "ShooterBot");
        TestRobot Nexttarget(5, 3, hammer, "TargetBot");
        Nextshooter.set_boundaries(20, 20);
        Nexttarget.set_boundaries(20, 20);

        arena.m_robots.clear();
        arena.m_robots.push_back(&Nextshooter);
        arena.m_robots.push_back(&Nexttarget);

        arena.m_board[2][2] = 'R';
        Nextshooter.move_to(2, 2);
        std::cout << "Shooter at (2,2)\n";

        // out of range target
        target_row = 18;
        target_col = 18;
        arena.m_board[target_row][target_col] = 'R';
        target.move_to(target_row, target_col); // Position the target within range based on the weapon
        std::cout << "Target Robot at (" << target_row << "," << target_col << ")" << std::endl;

        // Construct radar_results with the target
        radar_results.emplace_back('R', target_row, target_col);

        Nextshooter.process_radar_results(radar_results);
        Nexttarget.move_to(18, 18);

        // Update radar_results with out of range location
        radar_results.clear();
        int row,col;
        Nexttarget.get_current_location(row,col);
        radar_results.emplace_back('R', row, col);
        Nextshooter.process_radar_results(radar_results);

        //take the shot, even if target is out of range
        Nextshooter.get_shot_location(shot_row, shot_col); 
        int target_health_before = Nexttarget.get_health();
        arena.handle_shot(&Nextshooter, shot_row, shot_col);

        int target_health_after = Nexttarget.get_health();
        bool no_damage = (target_health_after == target_health_before);

        //special case railgun - it is never out of range.
        if(weapon == railgun)
            if(no_damage)
                print_test_result("railgun did not hit the target. No damage dealt", no_damage);
            else
                print_test_result("railgun hits.", true);
        else
            print_test_result("target was not hit. No damage dealt: ",no_damage);
            

    }
}