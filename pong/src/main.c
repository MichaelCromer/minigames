#include <time.h>
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
 *      pong_input
 *          captures keypresses etc, parses/interprets it, and stores data
 *      state_update
 *          updates global state available to all
 *      player1_update, namespace_update etc
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
struct Player { Vector2 pos; enum PLAYER_MOVE dir; };
struct Player player1 = { .pos = { 0.0f, 0.0f }, .dir = NONE };
struct Player player2 = { .pos = { 0.0f, 0.0f }, .dir = NONE };

float ai_lookahead_sec = 0.3f;
int ai_key_press = 0;
float ai_keypress_duration_min = 0.1f;

Vector2 ball_p = { 0.0f, 0.0f };
Vector2 ball_v = { 0.0f, 0.0f };
float ball_speed = 360.0f;
float ball_r = 6.0f;

bool ball_is_out = false;


/* update fns */


void update_player(struct Player *player, float dt)
{
    switch (player->dir) {
        case UP:
            player->pos.y -= paddle_speed * dt;
            break;
        case DOWN:
            player->pos.y += paddle_speed * dt;
            break;
        default:
            break;
    }
    player->pos.y = Clamp(player->pos.y, 0, window_h - paddle_h);
}


void update_ball(float dt)
{
    ball_p = Vector2Add(ball_p, Vector2Scale(ball_v, dt));

    if ((ball_p.y < 0) || (ball_p.y > window_h)) ball_v.y *= -1;
    if ((ball_p.x < ball_r) || (ball_p.x > window_w - ball_r)) ball_is_out = true;
}


void update_collisions(void)
{
    if (ball_p.x <= player1.pos.x + paddle_w + ball_r) {
        if ((ball_p.y < player1.pos.y) || (ball_p.y > player1.pos.y + paddle_h)) return;
        ball_v.x *= -1;
    }
    if (ball_p.x >= player2.pos.x - ball_r) {
        if ((ball_p.y < player2.pos.y) || (ball_p.y > player2.pos.y + paddle_h)) return;
        ball_v.x *= -1;
    }

    ball_p.x = Clamp(
            ball_p.x,
            player1.pos.x + paddle_w + ball_r + EPSILON,
            player2.pos.x - ball_r - EPSILON
    );
}


/* draw fns */


void draw_player(struct Player *player)
{
    DrawRectangle(player->pos.x, player->pos.y, paddle_w, paddle_h, WHITE);
}


void draw_ball(void)
{
    DrawCircle(ball_p.x, ball_p.y, ball_r, WHITE);
}


/* pong fns */


void pong_reset(void)
{
    ball_is_out = false;

    player1 = (struct Player) {
        .pos = (Vector2) { 6.0f, (window_h - paddle_h) / 2 },
        .dir = NONE
    };
    player2 = (struct Player) {
        .pos = (Vector2) { (window_w - 6.0f - paddle_w), (window_h - paddle_h) / 2 },
        .dir = NONE
    };

    ball_p = (Vector2){ window_w / 2, window_h / 2 };

    float theta = GetRandomValue(0, 360);
    ball_v = (Vector2){
        ball_speed * cosf(theta),
        ball_speed * sinf(theta)
    };
}


void pong_input(void)
{
    bool down = IsKeyDown(KEY_DOWN), up = IsKeyDown(KEY_UP);

    if ((down && up) || (!down && !up)) {
        player1.dir = NONE;
    } else {
        player1.dir = (down) ? DOWN : UP;
    }
}


void pong_ai(void)
{
    if ((ball_v.x < 0) || ((window_w - ball_p.x) / ball_v.x > ai_lookahead_sec)) {
        player2.dir = NONE;
        return;
    }

    if ((player2.pos.y + 2*paddle_h/3) < ball_p.y) {
        player2.dir = DOWN;
        return;
    }
    if ((player2.pos.y + paddle_h/3) > ball_p.y) {
        player2.dir = UP;
        return;
    }

    player2.dir = NONE;
}


void pong_update()
{
    float dt = GetFrameTime();
    pong_input();
    pong_ai();
    update_player(&player1, dt);
    update_player(&player2, dt);
    update_ball(dt);

    update_collisions();
    if (ball_is_out) pong_reset();
}


void pong_draw(void)
{
    BeginDrawing();
    ClearBackground(SKYBLUE);

    draw_player(&player1);
    draw_player(&player2);
    draw_ball();

    EndDrawing();
}


void pong_initialise(void)
{
    InitWindow(window_w, window_h, "pong");
    SetExitKey(KEY_Q);
    SetRandomSeed(1 + GetMouseX()*GetMouseX() + GetMouseY()*GetMouseY());
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
