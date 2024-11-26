# Object files common to all targets
ALL_THE_OS = RadarObj.o Arena.o RobotBase.o
THE_DOT_HS = RadarObj.h Arena.h RobotBase.h

# The default target to build all executables
all: RobotWarz

# Compile all .cpp files into .o files
%.o: %.cpp
	g++ -g -std=c++20 -Wall -Wpedantic -Wextra -Werror -Wno-c++11-extensions -c $<

# Link object files to create the executable
RobotWarz: RobotWarz.o $(ALL_THE_OS)
	g++ -g -o RobotWarz RobotWarz.o $(ALL_THE_OS)

# Clean up all object files and executables
clean:
	rm -f *.o RobotWarz