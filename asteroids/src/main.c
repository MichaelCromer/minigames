/*=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%
 *
 *  ASTEROIDS
 *
 *%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=*/

/*==============================================================================
 *  SETUP
 */

/*------------------------------------------------------------------------------
 *  INCLUDE
 */

#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>



/*------------------------------------------------------------------------------
 *  DEFINE
 */

#define WINDOW_WIDTH  (800)
#define WINDOW_HEIGHT (600)

#define BULLET_LIFETIME (1)
#define BULLET_VELOCITY (330)
#define NUM_BULLETS_MAX (100)

#define NUM_ASTEROIDS_MAX (100)
#define ASTEROID_VERTICES_MAX (6)

#define PLAYER_RELOAD (0.66f)


/*------------------------------------------------------------------------------
 *  ENUM
 */

enum ASTEROID_LEVEL
{
    ASTEROID_LEVEL_1 = 1,
    ASTEROID_LEVEL_2,
    ASTEROID_LEVEL_3,
    NUM_ASTEROID_LEVELS,
    ASTEROID_LEVEL_NONE
};



/*------------------------------------------------------------------------------
 *  STRUCT
 */


struct Polygon
{
    Vector2 *vertices[ASTEROID_VERTICES_MAX];
};


struct Asteroids
{
    size_t len;
    enum ASTEROID_LEVEL *level;
    float *rotation;
    float *spin;
    float *hitpoints;
    struct Polygon *polygons;
    Vector2 *position;
    Vector2 *velocity;
};


struct Bullets
{
    size_t len;
    float *lifetime;
    Vector2 *position;
    Vector2 *velocity;
};


struct Player
{
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float mass;
    float thrust;
    float drag;
    float reload;
    float max_reload;
};


/*==============================================================================
 *  DATA
 */

/*------------------------------------------------------------------------------
 *  GLOBAL STATE
 */

struct
{
    bool paused;
    bool quit;
    struct Asteroids asteroids;
    struct Bullets   bullets;
    struct Player    player;
} state = {
    .paused = false,
    .quit = false,
};



/*------------------------------------------------------------------------------
 *  GEOMETRY
 */

const float BOUNDS_ASTEROID_DELTA = 30.0f;

const Rectangle BOUNDS_ASTEROID = {
    .x = -1.0f*BOUNDS_ASTEROID_DELTA,
    .y = -1.0f*BOUNDS_ASTEROID_DELTA,
    .width = WINDOW_WIDTH + BOUNDS_ASTEROID_DELTA,
    .height = WINDOW_HEIGHT + BOUNDS_ASTEROID_DELTA
};

const Rectangle BOUNDS_PLAYER = {
    .x = 0.0f,
    .y = 0.0f,
    .width = WINDOW_WIDTH,
    .height = WINDOW_HEIGHT
};




/*==============================================================================
 *  FUNCTIONS
 */

/*------------------------------------------------------------------------------
 *  UTILITY
 */

static inline Vector2 vector2_wrap(Vector2 vec, const Rectangle rect)
{
    if (vec.x < rect.x) vec.x = rect.x + rect.width;
    if (vec.y < rect.y) vec.y = rect.y + rect.height;
    if (vec.x > rect.x + rect.width) vec.x = rect.x;
    if (vec.y > rect.y + rect.height) vec.y = rect.y;

    return vec;
}

//#include "asteroid.c"

/*------------------------------------------------------------------------------
 *  BULLETS
 */


void bullets_initialise(struct Bullets *bullets)
{
    bullets->len = 0;

    bullets->position = malloc(NUM_BULLETS_MAX * sizeof(Vector2));
    bullets->velocity = malloc(NUM_BULLETS_MAX * sizeof(Vector2));
    bullets->lifetime = malloc(NUM_BULLETS_MAX * sizeof(float));
}


void bullets_add(struct Bullets *bullets, Vector2 pos, Vector2 vel)
{
    if (NUM_BULLETS_MAX <= bullets->len) return;
    bullets->position[bullets->len] = pos;
    bullets->velocity[bullets->len] = vel;
    bullets->lifetime[bullets->len] = BULLET_LIFETIME;
    bullets->len++;
}


void bullets_remove(struct Bullets *bullets, size_t i)
{
    if (NUM_BULLETS_MAX <= i) return;
    bullets->position[i] = bullets->position[bullets->len - 1];
    bullets->velocity[i] = bullets->velocity[bullets->len - 1];
    bullets->lifetime[i] = bullets->lifetime[bullets->len - 1];
    bullets->len--;
}


void bullets_update(struct Bullets *bullets, float dt)
{
    for (size_t i = 0; i < bullets->len; i++) {
        bullets->position[i] = Vector2Add(
            bullets->position[i], Vector2Scale(bullets->velocity[i], dt)
        );
        bullets->lifetime[i] -= dt;
        while (bullets->len && bullets->lifetime[i] < 0) {
            bullets_remove(bullets, i);
        }
    }
}


void bullets_draw(struct Bullets *bullets)
{
    for (size_t i = 0; i < bullets->len; i++) {
        DrawCircleV(bullets->position[i], 2, WHITE);
    }
}


void bullets_deinitialise(struct Bullets *bullets)
{
    free(bullets->position);
    free(bullets->velocity);
    free(bullets->lifetime);
}


/*------------------------------------------------------------------------------
 *  PLAYER
 */


void player_initialise(struct Player *player)
{
    *player = (struct Player) {
        .position = (Vector2){ WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2},
        .rotation = 0,
        .mass = 0.33,
        .thrust = 100,
        .drag = 0.003,
        .reload = 0.0f,
        .max_reload = PLAYER_RELOAD
    };
}


bool player_can_shoot(struct Player *player)
{
    return (0 >= player->reload);
}


void player_update_rotation(struct Player *player, float dt)
{
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) player->rotation -= dt * 6;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) player->rotation += dt * 6;
}


void player_update_position(struct Player *player, float dt)
{
    Vector2 force = { 0, 0 };

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        Vector2 df = { cos(player->rotation), sin(player->rotation) };
        force = Vector2Add(force, df);
    }

    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        Vector2 df = { -0.3 * cos(player->rotation), -0.3 * sin(player->rotation) };
        force = Vector2Add(force, df);
    }

    force = Vector2Subtract(force, Vector2Scale(player->velocity, player->drag));

    player->velocity = Vector2Add(
        player->velocity, Vector2Scale(force, dt * player->thrust / player->mass)
    );

    player->position = vector2_wrap(
        Vector2Add(player->position, Vector2Scale(player->velocity, dt)), BOUNDS_PLAYER
    );
}


void player_update(struct Player *player, float dt)
{
    player_update_position(player, dt);
    player_update_rotation(player, dt);

    if (0 < player->reload) {
        player->reload -= dt;
        if (0 > player->reload) player->reload = 0;
    }
}


void player_draw(struct Player *player)
{
    Vector2 offset1 = Vector2Rotate((Vector2){ 12, 0 }, player->rotation);
    Vector2 offset2 = Vector2Rotate((Vector2){ -6, -6 }, player->rotation);
    Vector2 offset3 = Vector2Rotate((Vector2){ -3, 0 }, player->rotation);
    Vector2 offset4 = Vector2Rotate((Vector2){ -6, 6 }, player->rotation);

    Vector2 ver1 = Vector2Add(player->position, offset1);
    Vector2 ver2 = Vector2Add(player->position, offset2);
    Vector2 ver3 = Vector2Add(player->position, offset3);
    Vector2 ver4 = Vector2Add(player->position, offset4);

    DrawTriangle(ver1, ver2, ver3, WHITE);
    DrawTriangle(ver4, ver1, ver3, WHITE);
}


Vector2 player_barrel(struct Player *player)
{
    return (Vector2) {
        player->position.x + 12*cos(player->rotation),
        player->position.y + 12*sin(player->rotation)
    };
}



/*------------------------------------------------------------------------------
 *  ACTIONS
 */


void action_player_shoot(struct Player *player, struct Bullets *bullets)
{
    if (IsKeyDown(KEY_LEFT_SHIFT) && player_can_shoot(player)) {
        Vector2 pos = player_barrel(player);
        Vector2 vel = (Vector2){
            BULLET_VELOCITY*cos(player->rotation) + player->velocity.x,
            BULLET_VELOCITY*sin(player->rotation) + player->velocity.y
        };

        bullets_add(bullets, pos, vel);
        player->reload = player->max_reload;
    }
}


void action_update(struct Player *player, struct Bullets *bullets, float dt)
{
    (void)dt;
    action_player_shoot(player, bullets);
}


/*------------------------------------------------------------------------------
 *  MAIN LOOP
 */

void initialise(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "hey hey hey");

    player_initialise(&state.player);
    bullets_initialise(&state.bullets);
}


void draw(void)
{
    BeginDrawing();
    {
        ClearBackground(SKYBLUE);

        player_draw(&state.player);
        bullets_draw(&state.bullets);
    }
    EndDrawing();
}


void update(float dt)
{
    if (IsKeyPressed(KEY_Q)) state.quit = true;
    if (IsKeyPressed(KEY_P)) state.paused = !state.paused;
    if (state.paused) return;

    player_update(&state.player, dt);
    bullets_update(&state.bullets, dt);
    action_update(&state.player, &state.bullets, dt);
}


void deinitialise(void)
{
    CloseWindow();

    bullets_deinitialise(&state.bullets);
}


int main(void)
{
    initialise();

    while (!state.quit) {
        draw();
        update(GetFrameTime());
    }

    deinitialise();
    return 0;
}
