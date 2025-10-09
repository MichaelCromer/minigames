#include <raylib.h>
#include <raymath.h>

struct State
{
    struct Player *player;
    struct BulletQueue *bullets;
    struct AsteroidQueue *asteroids;
};


void state_destroy(struct State *state)
{
    if (!state) return;
    if (state->player) player_destroy(state->player);
    if (state->bullets) bulletqueue_destroy(state->bullets);
    if (state->asteroids) asteroidqueue_destroy(state->asteroids);
    free(state);
}


struct State *state_create(void)
{
    struct State *state = malloc(sizeof(struct State));
    if (!state) return NULL;

    state->player = player_create();
    state->bullets = bulletqueue_create(BULLETQUEUE_LEN_MAX);
    state->asteroids = asteroidqueue_create(ASTEROIDQUEUE_LEN_MAX);

    if (!state->player || !state->bullets || !state->asteroids) {
        state_destroy(state);
        return NULL;
    }

    return state;
}


void state_initialise(struct State *state)
{
    *(state->player) = (struct Player) { 
        .position = (Vector2){ WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2},
        .rotation = 0,
        .mass = 0.33,
        .engine = 100,
        .drag = 0.003
    };

    size_t N = 24;
    for (size_t i = 0; i < N; i++) {
        struct Asteroid *a = state->asteroids->asteroids + i;
        asteroid_randomise(a);
        state->asteroids->len = N;
    }
}


void state_draw(struct State *state)
{
    asteroidqueue_draw(state->asteroids);
    bulletqueue_draw(state->bullets);
    player_draw(state->player);
}


void state_update(struct State *state, float dt)
{
    asteroidqueue_update(state->asteroids, dt);
    bulletqueue_update(state->bullets, dt);
    player_update(state->player, dt);

    if (IsKeyDown(KEY_SPACE) && player_can_fire(state->player)) {
        struct Player *p = state->player;
        struct Bullet b = {
            .position = player_barrel(p),
            .velocity = (Vector2) { 
                BULLET_VELOCITY * cos(p->rotation) + p->velocity.x,
                BULLET_VELOCITY * sin(p->rotation) + p->velocity.y
            },
            .lifetime = BULLET_LIFTIME,
        };

        bulletqueue_insert(state->bullets, b);
        p->reload += 0.66;
    }
}
