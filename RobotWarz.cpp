#include "Arena.h"
#include <ctime>

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    Arena the_arena(20,20);
    the_arena.load_robots();
    the_arena.run_simulation();

}