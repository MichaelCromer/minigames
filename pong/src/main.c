#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>

/*
 *  title screen
 *  1p mode
 *  2p mode
 *
 *  generically :
 *      input_parse
 *          captures keypresses etc, parses/interprets it, and stores data
 *      state_update
 *          updates global state available to all
 *      player_update, namespace_update etc
 *          use captured input data to decide updates
 *
 *
 */


const int window_w = 800;
const int window_h = 600;


const int paddle_height = 60;
const int paddle_width = 6;
const float paddle_speed = 360.0f;


enum PLAYER_MOVE { NONE, UP, DOWN };
enum PLAYER_MOVE player_move = NONE;
float player_y = 0.0f;

Vector2 ball_pos = { 0.0f, 0.0f };
Vector2 ball_velocity = { 0.0f, 0.0f };
float ball_radius = 6.0f;


/* player fns */


void player_update(float dt)
{
    switch (player_move) {
        case UP:
            player_y -= paddle_speed * dt;
            break;
        case DOWN:
            player_y += paddle_speed * dt;
            break;
        default:
            break;
    }
    player_y = Clamp(player_y, 0, window_h - paddle_height);
}


/* draw fns */


void draw_paddle(int x, int y)
{
    DrawRectangle(x, y, paddle_width, paddle_height, WHITE);
}


void draw_ball(Vector2 pos)
{
    DrawCircle(pos.x, pos.y, ball_radius, WHITE);
}


/* pong fns */


void pong_initialise(void)
{
    player_y = (window_h - paddle_height) / 2;
    ball_pos = (Vector2){ window_w / 2, window_h / 2 };

    InitWindow(window_w, window_h, "pong");
    SetExitKey(KEY_Q);
}


void input_parse(void)
{
    bool down = IsKeyDown(KEY_DOWN), up = IsKeyDown(KEY_UP);

    if ((down && up) || (!down && !up)) {
        player_move = NONE;
    } else {
        player_move = (down) ? DOWN : UP;
    }
}


void pong_update()
{
    float dt = GetFrameTime();
    input_parse();
    player_update(dt);
}


void pong_draw(void)
{
    BeginDrawing();
    ClearBackground(SKYBLUE);

    draw_paddle(6, player_y);
    draw_ball(ball_pos);

    EndDrawing();
}


void pong_deinitialise(void)
{
    CloseWindow();
}


int main(void)
{
    pong_initialise();

    while (!WindowShouldClose()) {
        pong_draw();
        pong_update();
    }

    pong_deinitialise();
    return 0;
}
