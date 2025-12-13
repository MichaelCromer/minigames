#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>

/*
 *  title screen
 *  1p mode
 *  2p mode (?)
 *
 *  player can 'jump' to whack ball
 *
 *  generically :
 *      input_parse
 *          captures keypresses etc, parses/interprets it, and stores data
 *      state_update
 *          updates global state available to all
 *      player_update, namespace_update etc
 *          use captured input data to decide updates
 *
 *  is that too complex for such a simpe game?
 *
 */


const int window_w = 800;
const int window_h = 600;

const int paddle_h = 90;
const int paddle_w = 6;
const float paddle_speed = 360.0f;

enum PLAYER_MOVE { NONE, UP, DOWN };
enum PLAYER_MOVE player_move = NONE;

Vector2 player_p = { 0.0f, 0.0f };

Vector2 ball_p = { 0.0f, 0.0f };
Vector2 ball_v = { 0.0f, 0.0f };
float ball_speed = 360.0f;
float ball_r = 6.0f;

bool ball_is_out = false;


/* update fns */


void update_player(float dt)
{
    switch (player_move) {
        case UP:
            player_p.y -= paddle_speed * dt;
            break;
        case DOWN:
            player_p.y += paddle_speed * dt;
            break;
        default:
            break;
    }
    player_p.y = Clamp(player_p.y, 0, window_h - paddle_h);
}


void update_ball(float dt)
{
    ball_p = Vector2Add(ball_p, Vector2Scale(ball_v, dt));

    if ((ball_p.y < 0) || (ball_p.y > window_h)) ball_v.y *= -1;
    if ((ball_p.x < 0) || (ball_p.x > window_w)) ball_is_out = true;
}


void update_collisions(void)
{
    if ((ball_p.y < player_p.y) || (ball_p.y > player_p.y + paddle_h)) return;
    if (ball_p.x > player_p.x + paddle_w + ball_r) return;
    ball_v.x *= -1;
}


/* draw fns */


void draw_paddle(Vector2 p)
{
    DrawRectangle(p.x, p.y, paddle_w, paddle_h, WHITE);
}


void draw_ball(void)
{
    DrawCircle(ball_p.x, ball_p.y, ball_r, WHITE);
}


/* pong fns */


void pong_reset(void)
{
    static int seed = 0;
    ball_is_out = false;

    player_p = (Vector2) { 6.0f, (window_h - paddle_h) / 2 };
    ball_p = (Vector2){ window_w / 2, window_h / 2 };

    SetRandomSeed(seed++);
    float theta = GetRandomValue(0, 360);
    ball_v = (Vector2){
        ball_speed * cosf(theta),
        ball_speed * sinf(theta)
    };
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
    update_player(dt);
    update_ball(dt);

    update_collisions();
    if (ball_is_out) pong_reset();
}


void pong_draw(void)
{
    BeginDrawing();
    ClearBackground(SKYBLUE);

    draw_paddle(player_p);
    draw_ball();

    EndDrawing();
}


void pong_initialise(void)
{
    InitWindow(window_w, window_h, "pong");
    SetExitKey(KEY_Q);
    pong_reset();
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
