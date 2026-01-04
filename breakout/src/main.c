#include <raylib.h>
#include <raymath.h>


const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;


void breakout_draw(void)
{
    BeginDrawing();

    ClearBackground(SKYBLUE);

    EndDrawing();
}


void breakout_update(void)
{
    return;
}


void breakout_initialise(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Breakout");
}


void breakout_deinitialise(void)
{
    CloseWindow();
}


int main(void)
{
    breakout_initialise();

    while (!WindowShouldClose()) {
        breakout_draw();
        breakout_update();
    }

    breakout_deinitialise();

    return 0;
}
