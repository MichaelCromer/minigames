#include <raylib.h>

#include "geometry.c"

const int win_width = 800;
const int win_height = 600;
const char* win_title = "Physics Simulation Test";

void draw(void)
{
    BeginDrawing();
    {

    }
    EndDrawing();
}

void update(void)
{
    return;
}

void initialise(void)
{
    InitWindow(win_width, win_height, win_title);
}

void deinitialise(void)
{
    CloseWindow();
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
