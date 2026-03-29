#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "../src/arena.c"


const int win_width = 800;
const int win_height = 600;
const char* win_title = "Physics Simulation Test : Rigid Body";


const size_t N_BODY = 1;
struct RigidBodyArena *rba = NULL;

struct Body *b = NULL;


void body_randominse(void)
{

}


void initialise(void)
{
    rba = rigidbodyarena_create(N_BODY);
}


void deinitialise(void)
{
    rigidbodyarena_destroy(rba);
}


void draw(void)
{
    BeginDrawing();
    {
        /* stuff */
    }
    EndDrawing();
}


void update(void)
{
    return;
}


int main(void)
{
    initialise();

    while (!WindowShouldClose()) {
        draw();
        update();
    }

    deinitialise();
    return 0;
}
