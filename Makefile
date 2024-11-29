# Object files common to all targets
ALL_THE_OS = RadarObj.o Arena.o RobotBase.o
THE_DOT_HS = RadarObj.h Arena.h RobotBase.h

# The default target to build all executables
all: RobotWarz test_robot

# Compile all .cpp files into .o files
%.o: %.cpp
	g++ -g -std=c++20 -Wall -Wpedantic -Wextra -Werror -Wno-c++11-extensions -c $<

# Link object files to create RobotWarz executable
RobotWarz: RobotWarz.o $(ALL_THE_OS)
	g++ -g -o RobotWarz RobotWarz.o $(ALL_THE_OS)

# Link object files to create test_robot executable
test_robot: test_robot.o $(ALL_THE_OS)
	g++ -g -o test_robot test_robot.o $(ALL_THE_OS) -ldl

# Clean up all object files and executables
clean:
	rm -f *.o RobotWarz test_robot libtest_robot.so