#include <raylib.h>
#include <raymath.h>


enum ASTEROID_LEVEL
{
    LEVEL0,
    LEVEL1,
    LEVEL2,
    NUM_ASTEROID_LEVELS
};


static const size_t ASTEROIDLEVEL_NUM_CORNERS[NUM_ASTEROID_LEVELS] = { 3, 4, 6 };
static const Color  ASTEROIDLEVEL_COLOUR[NUM_ASTEROID_LEVELS] = {
    { 255, 255, 255, 255 },
    { 255, 255, 255, 255 },
    { 255, 255, 255, 255 }
};
static const float ASTEROIDLEVEL_RADIUS[NUM_ASTEROID_LEVELS] = { 12.0f, 18.0f, 24.0f };
static const float ASTEROIDLEVEL_MASS[NUM_ASTEROID_LEVELS] = { 144.0f, 324.0f, 576.0f };
static const float ASTEROIDLEVEL_MOI[NUM_ASTEROID_LEVELS] = {
    12.0f * 12.0f * 144.0f / (3 + 1),
    18.0f * 18.0f * 324.0f / (4 + 1),
    24.0f * 24.0f * 324.0f / (6 + 1)
};


struct Asteroid
{
    Vector2 corners[ASTEROID_VERTICES_MAX];
    Vector2 centre;
    Vector2 radius;
    Vector2 velocity;
    float mass;
    float moi;
    float rotation;
    float spin;
    float hitpoints;
    enum ASTEROID_LEVEL level;
    bool collision;
    Color colour;
};


struct AsteroidQueue
{
    struct Asteroid *asteroids;
    size_t len;
    size_t max;
};


void asteroid_clear(struct Asteroid *ast)
{
    if (!ast) return;

    for (size_t i = 0; i < ASTEROID_VERTICES_MAX; i++) {
        ast->corners[i] = (Vector2) { 0, 0 };
    }
    ast->centre= (Vector2) { 0, 0 };
    ast->radius = (Vector2) { 0, 0 };
    ast->velocity = (Vector2) { 0, 0 };
    ast->mass = 0;
    ast->moi = 0;
    ast->rotation = 0;
    ast->spin = 0;
    ast->hitpoints = 0;
    ast->level = LEVEL0;

    ast->collision = false;
    ast->colour = WHITE;
}


float asteroid_radius(struct Asteroid *ast)
{
    return ASTEROIDLEVEL_RADIUS[ast->level];
}


Color asteroid_colour(struct Asteroid *ast)
{
    return ast->colour;
}


bool asteroid_alive(struct Asteroid *ast)
{
    if (!ast) return false;
    return (0 < ast->hitpoints);
}


void asteroid_initialise(struct Asteroid *ast, Vector2 *vertices, size_t n)
{
    if (!ast || !vertices || !n) return;

    /* centroid and effective positions of corners */
    Vector2 centre = polygon_area_moment_1(vertices, n);
    for (size_t i = 0; i < n; i++) {
        ast->corners[i] = Vector2Subtract(vertices[i], centre);
    }

    /* area and mass */
    float area = polygon_area_moment_0(ast->corners, n);
    ast->mass = area * ASTEROID_DENSITY;

    /* moment of inertia */
    ast->moi = polygon_area_moment_2(ast->corners, n) * ASTEROID_DENSITY;
}


void asteroid_randomise(struct Asteroid *ast)
{
    if (!ast) return;

    asteroid_clear(ast);
    ast->level = random() % NUM_ASTEROID_LEVELS;

    size_t num_corners = ASTEROIDLEVEL_NUM_CORNERS[ast->level];
    Vector2 corners[num_corners];

    float radius = asteroid_radius(ast);
    float angle_step = (2 * PI) / num_corners;
    float angle_delta = 0;
    for (size_t i = 0; i < num_corners; i++) {
        angle_delta = (random()%(2*num_corners)) * angle_step / (2.0f*num_corners);
        corners[i] = (Vector2) {
            radius * cos(i*angle_step + angle_delta),
            radius * sin(i*angle_step + angle_delta)
        };
    }
    asteroid_initialise(ast, corners, num_corners);

    ast->centre= (Vector2) { random() % WINDOW_WIDTH, random() % WINDOW_HEIGHT };
    ast->velocity = (Vector2) {
        3*((random()%12) - (random()%6)),
        3*((random()%12) - (random()%6))
    };
    ast->rotation = (random() % 360) * (2 * PI) / 360;
    //ast->spin = 1.0f * (random() % 360) * (2 * PI) / ((ast->level + 1) * 360);
    ast->hitpoints = 100 * (ast->level + 1) * (ast->level + 1);
}


/* test if any of the vertices of ast1 lie within any of the triangles of ast2 */
bool asteroid_is_intersect(struct Asteroid *ast1, struct Asteroid *ast2)
{
    float d = Vector2Length(Vector2Subtract(ast1->centre, ast2->centre));
    if (d > (asteroid_radius(ast1) + asteroid_radius(ast2))) {
        return false;
    }

    /* now do the separating axis theorem for each face of each polygon */

    size_t n1 = ASTEROIDLEVEL_NUM_CORNERS[ast1->level];
    size_t n2 = ASTEROIDLEVEL_NUM_CORNERS[ast2->level];

    Vector2 curr = { 0 }, next = ast1->corners[0];
    Vector2 perp;

    float base, min, max;

    for (size_t i = 0; i < n1; i++) {
        curr = next, next = ast1->corners[(i+1) % n1];
        perp = vector2_perp(Vector2Subtract(next, curr));

        base = vector2_dot(Vector2Add(curr, ast1->centre));
        for (size_t j = 0; j < n2;
    }
}


void asteroid_collide(struct Asteroid *ast1, struct Asteroid *ast2)
{
    Vector2 ds = Vector2Subtract(ast2->centre, ast1->centre);
    Vector2 dv = Vector2Subtract(ast2->velocity, ast1->velocity);

    float factor = (
        2*vector2_dot(ds, dv) / ((ast1->mass + ast2->mass) * Vector2LengthSqr(ds))
    );

    Vector2 dv1 = Vector2Scale(ds, ast2->mass * factor);
    Vector2 dv2 = Vector2Scale(ds, -1.0f * ast1->mass * factor);

    ast1->velocity = Vector2Add(ast1->velocity, dv1);
    ast2->velocity = Vector2Add(ast2->velocity, dv2);
}


void asteroid_draw(struct Asteroid *ast)
{
    if (!ast || !asteroid_alive(ast)) return;

    size_t num_corners = ASTEROIDLEVEL_NUM_CORNERS[ast->level];
    Vector2 vertex0 = ast->centre;
    Vector2 vertex1 = { 0, 0 }, vertex2 = Vector2Add(vertex0, ast->corners[0]);

    DrawCircle(vertex0.x, vertex0.y, 1, RED);

    for (size_t i = 0; i < num_corners; i++) {
        vertex1 = vertex2;
        vertex2 = Vector2Add(vertex0, ast->corners[(i + 1) % num_corners]);
        DrawLineV(vertex1, vertex2, asteroid_colour(ast));
    }

    DrawLineV(vertex0, Vector2Add(vertex0, ast->velocity), BLUE);
}


void asteroid_update(struct Asteroid *ast, float dt)
{
    ast->colour = WHITE;
    ast->collision = false;

    size_t num_corners = ASTEROIDLEVEL_NUM_CORNERS[ast->level];
    float delta_angle = ast->spin * dt;
    float buffer = asteroid_radius(ast);

    for (size_t i = 0; i < num_corners; i++) {
        ast->corners[i] = Vector2Rotate(ast->corners[i], delta_angle);
    }

    ast->rotation += delta_angle;
    ast->centre= vector2_wrap(
        Vector2Add(ast->centre, Vector2Scale(ast->velocity, dt)),
        (const Vector2) {
            -1.0f * buffer,
            -1.0f * buffer
        },
        (const Vector2) {
            WINDOW_WIDTH + buffer,
            WINDOW_HEIGHT + buffer
        }
    );
}


void asteroidqueue_destroy(struct AsteroidQueue *aq)
{
    if (!aq) return;
    if (aq->asteroids) free(aq->asteroids);
    free(aq);
}


struct AsteroidQueue *asteroidqueue_create(size_t max)
{
    struct AsteroidQueue *aq = malloc(sizeof(struct AsteroidQueue));
    if (!aq) return NULL;

    aq->asteroids = malloc(max * sizeof(struct Asteroid));
    if (!aq->asteroids) {
        asteroidqueue_destroy(aq);
        return NULL;
    }

    for (size_t i = 0; i < max; i++) asteroid_clear(aq->asteroids + i);

    aq->len = 0;
    aq->max = max;

    return aq;
}


void asteroidqueue_apply
(
    struct AsteroidQueue *aq,
    void (*func)(struct Asteroid *)
)
{
    if (!aq || !func) return;
    for (size_t i = 0; i < aq->len; i++) func(aq->asteroids + i);
}


void asteroidqueue_insert(struct AsteroidQueue *aq, struct Asteroid a)
{
    if (!aq || (aq->len >= aq->max)) return;
    aq->asteroids[aq->len++] = a;
}


void asteroidqueue_remove(struct AsteroidQueue *aq, size_t i)
{
    if (!aq || i >= aq->len) return;
    aq->asteroids[i] = aq->asteroids[--aq->len];
}


void asteroidqueue_draw(struct AsteroidQueue *aq)
{
    asteroidqueue_apply(aq, asteroid_draw);
}


void asteroidqueue_update(struct AsteroidQueue *aq, float dt)
{
    size_t i = 0;
    struct Asteroid *curr = aq->asteroids + 0;
    struct Asteroid *next = NULL;
    while ((i < aq->len) && asteroid_alive(curr)) {
        asteroid_update(curr, dt);

        if (!asteroid_alive(aq->asteroids + i)) asteroidqueue_remove(aq, i);
        else curr = aq->asteroids + (++i);
    }

    for (size_t i = 0; i < aq->len; i++) {
        curr = aq->asteroids + i;
        for (size_t j = i+1; j < aq->len; j++) {
            next = aq->asteroids + j;
            if (!asteroid_is_intersect(curr, next)) continue;
            curr->collision = true, next->collision = true;
            curr->colour = RED, next->colour = RED;
            //asteroid_collide(curr, next);
        }
    }
}
