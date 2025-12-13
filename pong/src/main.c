#include <time.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>

const int WINDOW_W = 800;
const int WINDOW_H = 600;
const float BALL_SPEED = 360.0f;
const float BALL_RADIUS = 6.0f;
const int PADDLE_H = 90;
const int PADDLE_W = 6;
const float PADDLE_SPEED = 360.0f;
const float AI_LOOKAHEAD_SEC = 0.3f;

enum PLAYER_MOVE { NONE, UP, DOWN };
struct Player { Vector2 pos; enum PLAYER_MOVE dir; };

struct Player player1 = { .pos = { 0.0f, 0.0f }, .dir = NONE };
struct Player player2 = { .pos = { 0.0f, 0.0f }, .dir = NONE };

Vector2 ball_p = { 0.0f, 0.0f };
Vector2 ball_v = { 0.0f, 0.0f };
bool ball_is_out = false;


/* update fns */


void update_player(struct Player *player, float dt)
{
    switch (player->dir) {
        case UP:
            player->pos.y -= PADDLE_SPEED * dt;
            break;
        case DOWN:
            player->pos.y += PADDLE_SPEED * dt;
            break;
        default:
            break;
    }
    player->pos.y = Clamp(player->pos.y, 0, WINDOW_H - PADDLE_H);
}


void update_ball(float dt)
{
    ball_p = Vector2Add(ball_p, Vector2Scale(ball_v, dt));

    if ((ball_p.y < 0) || (ball_p.y > WINDOW_H)) ball_v.y *= -1;
    if (
        (ball_p.x < BALL_RADIUS) || (ball_p.x > WINDOW_W - BALL_RADIUS)
    ) ball_is_out = true;
}


void update_collisions(void)
{
    if (ball_p.x <= player1.pos.x + PADDLE_W + BALL_RADIUS) {
        if ((ball_p.y < player1.pos.y) || (ball_p.y > player1.pos.y + PADDLE_H)) return;
        ball_v.x *= -1;
    }
    if (ball_p.x >= player2.pos.x - BALL_RADIUS) {
        if ((ball_p.y < player2.pos.y) || (ball_p.y > player2.pos.y + PADDLE_H)) return;
        ball_v.x *= -1;
    }

    ball_p.x = Clamp(
            ball_p.x,
            player1.pos.x + PADDLE_W + BALL_RADIUS + EPSILON,
            player2.pos.x - BALL_RADIUS - EPSILON
    );
}


/* draw fns */


void draw_player(struct Player *player)
{
    DrawRectangle(player->pos.x, player->pos.y, PADDLE_W, PADDLE_H, WHITE);
}


void draw_ball(void)
{
    DrawCircle(ball_p.x, ball_p.y, BALL_RADIUS, WHITE);
}


/* pong fns */


void pong_reset(void)
{
    ball_is_out = false;

    player1 = (struct Player) {
        .pos = (Vector2) { 6.0f, (WINDOW_H - PADDLE_H) / 2 },
        .dir = NONE
    };
    player2 = (struct Player) {
        .pos = (Vector2) { (WINDOW_W - 6.0f - PADDLE_W), (WINDOW_H - PADDLE_H) / 2 },
        .dir = NONE
    };

    ball_p = (Vector2){ WINDOW_W / 2, WINDOW_H / 2 };

    float theta = GetRandomValue(0, 360);
    ball_v = (Vector2){
        BALL_SPEED * cosf(theta),
        BALL_SPEED * sinf(theta)
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
    if ((ball_v.x < 0) || ((WINDOW_W - ball_p.x) / ball_v.x > AI_LOOKAHEAD_SEC)) {
        player2.dir = NONE;
        return;
    }

    if ((player2.pos.y + 2*PADDLE_H/3) < ball_p.y) {
        player2.dir = DOWN;
        return;
    }
    if ((player2.pos.y + PADDLE_H/3) > ball_p.y) {
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
    InitWindow(WINDOW_W, WINDOW_H, "pong");
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
