The project we are doing for our final exam is called 

*******************
 R O B O T W A R Z
*******************

You will build a robot combat simulator in which robots (programs) will battle it out
to see which robot is the winner.

The way the game works is this:

1. The arena is a X by Y grid - the robots occupy a single location. Min size is 10 x 10
2. The robots are placed inside the arena and the 'game loop' calls functions on the robots to make them do their things.
3. The robots will move around the arena, react to the surroundings and try to kill the other robots. 
4. The last robot still operating is the winner. 
5. The player can watch each turn unfold, or they can run the entire simulation and print the results to a file. The 
    turns are just printed out ... There is no visual representation (i.e. there isn't a grid that shows the robots
    positions)
6. Robots are just an instance of a c++ class. 
7. Robots can move, check their radar and shoot. 
8. There are obstacles in the arena - randomly placed. (a pit, a mound, and a flame-thrower)
9. Robots can have different configurations for weapons and armor
    * 4 armor plates. (e.g. you could have 2 in the front, one on each side and none in the back)
    * flamethrower - short range but very damaging
    * sniper - long range, very fast but not too damaging
    * grenade - explodes at the end of it's trajectory

A sample run might look like this:

Robot 1 - Skullface - at location: 27,10 - health 100 - reads radar  - enemy at 10,10 - shoots sniper - robot Ratboy hit - armor - 10 pts damage! End turn
Robot 2 - Ratboy - at location: 10,10 - health 90 - moves to 30,30 - end turn
Robot 3 - Blipzo - at location 99,99 - health 100 - reads radar - no enemies - end turn


The Arena works like this:

has a list of robots
for each robot calls get_radar_location, set_radar_results, get_shot_location, get_move_direction, set_next_location. 
The robot can decide if it wants to do anything with the radar, the shot, or move. If it returns None, then the arena knows not to perform a move or shot.
If the shot_location registers a hit, the arena calls get_armor() and then take_damage() on the target.  
Radar returns a targeted grid location. The arena looks up the location and calls set_radar_results with the info.

Robots inherit from a base class. The base class has some private members that make it so that the robots can't be OP hacked at runtime.

base robot class

private members
armor plate config - set on construction 
front - all 4 plates in front
sides - two plates on sides none on front or back
even - one on each side
front/right - two on front, two on right

Orientation - which way the robot is facing. up, right, down left


weapon type - set on construction
flame
sniper
grenade 


health - all robots start with 100 
at 20% they can no longer move
at 10% they can no longer shoot
Radar has a percent chance to be damaged on every hit



functions
get_radar_location 
set_radar_results 
get_shot_location 
get_move_direction
set_next_location







