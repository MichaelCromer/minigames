#include <float.h>
#include <raylib.h>
#include <raymath.h>

enum ASTEROID_LEVEL
{
    LEVEL0 = 0,
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
    Vector2 velocity;
    float radius;
    float mass;
    float inv_mass;
    float moi;
    float inv_moi;
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
    ast->velocity = (Vector2) { 0, 0 };
    ast->radius = 0;
    ast->mass = 0;
    ast->inv_mass = 0;
    ast->moi = 0;
    ast->inv_moi = 0;
    ast->rotation = 0;
    ast->spin = 0;
    ast->hitpoints = 0;
    ast->level = LEVEL0;

    ast->collision = false;
    ast->colour = WHITE;
}


size_t asteroid_num_corners(struct Asteroid *ast)
{
    return ASTEROIDLEVEL_NUM_CORNERS[ast->level];
}


float asteroid_radius(struct Asteroid *ast)
{
    return ast->radius;
}


/* world position of ast's ith corner */
Vector2 asteroid_vertex(struct Asteroid *ast, size_t i)
{
    return Vector2Add(
        ast->centre,
        Vector2Rotate(ast->corners[i % asteroid_num_corners(ast)], ast->rotation)
    );
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
        ast->corners[i] = vector2_diff(vertices[i], centre);
        if (Vector2Length(ast->corners[i]) > ast->radius) {
            ast->radius = Vector2Length(ast->corners[i]);
        }
    }

    /* area and mass */
    float area = polygon_area_moment_0(ast->corners, n);
    ast->mass = area * ASTEROID_DENSITY;
    ast->inv_mass = 1 / ast->mass;

    /* moment of inertia */
    ast->moi = polygon_area_moment_2(ast->corners, n) * ASTEROID_DENSITY;
    ast->inv_moi = 1 / ast->moi;
}


void asteroid_randomise(struct Asteroid *ast)
{
    if (!ast) return;

    asteroid_clear(ast);
    ast->level = random() % NUM_ASTEROID_LEVELS;

    size_t num_corners = ASTEROIDLEVEL_NUM_CORNERS[ast->level];
    Vector2 corners[num_corners];

    float radius = ASTEROIDLEVEL_RADIUS[ast->level];
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


/* calculate
 *      the collision axis (one of the normals to an edge) in local coordinates
 *          given directed from ast2 to ast1
 *      the collision point in global coordinates
 */
void asteroid_collision_data
(
    struct Asteroid *ast1, struct Asteroid *ast2,
    struct Vector2 *axis, struct Vector2 *point
)
{
    Vector2 v0, v1, pt;
    Vector2 dr = Vector2Subtract(ast2->centre, ast1->centre);
    *axis = Vector2Zero(), *point = Vector2Zero();

    v1 = asteroid_vertex(ast1, 0);
    for (size_t i = 0; i < asteroid_num_corners(ast1); i++) {
        v0 = v1;
        v1 = asteroid_vertex(ast1, i + 1);
        for (size_t j = 0; j < asteroid_num_corners(ast2); j++) {
            pt = asteroid_vertex(ast2, j);
            if (!point_on_segment(pt, v0, v1)) continue;

            *axis = Vector2Normalize(vector2_perp(Vector2Subtract(v0, v1)));
            if (vector2_dot(*axis, dr)) *axis = Vector2Scale(*axis, -1);
            *point = pt;
            return;
        }
    }

    v1 = asteroid_vertex(ast2, 0);
    for (size_t j = 0; j < asteroid_num_corners(ast2); j++) {
        v0 = v1;
        v1 = asteroid_vertex(ast2, j + 1);
        for (size_t i = 0; i < asteroid_num_corners(ast1); i++) {
            pt = asteroid_vertex(ast1, i);
            if (!point_on_segment(pt, v0, v1)) continue;

            *axis = Vector2Normalize(vector2_perp(Vector2Subtract(v0, v1)));
            if (vector2_dot(*axis, dr)) *axis = Vector2Scale(*axis, -1);
            *point = pt;
            return;
        }
    }
}


void asteroid_collide(struct Asteroid *ast1, struct Asteroid *ast2)
{
    /* early exit if too far apart */
    float dr = Vector2Length(Vector2Subtract(ast1->centre, ast2->centre));
    if (dr > (asteroid_radius(ast1) + asteroid_radius(ast2))*(1 + EPSILON)) return;

    /* calculate collision axis and point */
    Vector2 n, P;
    asteroid_collision_data(ast1, ast2, &n, &P);

    /* early exit if no collision axis */
    if (Vector2Equals(n, Vector2Zero())) return;

    /* distances from centres to collision point */
    Vector2 r1_P = Vector2Subtract(P, ast1->centre);
    Vector2 r2_P = Vector2Subtract(P, ast2->centre);

    /* tangential vector at collision point */
    Vector2 t1_P = vector2_perp(r1_P);
    Vector2 t2_P = vector2_perp(r2_P);

    /* rotational velocity */
    Vector2 w1_P = Vector2Scale(t1_P, ast1->spin);
    Vector2 w2_P = Vector2Scale(t2_P, ast2->spin);

    /* net velocities at collision point */
    Vector2 v1_P = Vector2Add(w1_P, ast1->velocity);
    Vector2 v2_P = Vector2Add(w2_P, ast2->velocity);

    /* relative velocity at collision points */
    Vector2 v_12 = Vector2Subtract(v2_P, v1_P);

    /* early exit if velocity on collision axis is negative */
    if (vector2_dot(n, v_12) <= 0) return;

    ast1->collision = true, ast2->collision = true;
    ast1->colour = GREEN, ast2->colour = RED;

    /* j is the magic scalar */
    float j_numer = -2 * vector2_dot(n, v_12);
    float j_denom = (
        vector2_dot(n, n) * (ast1->inv_mass + ast2->inv_mass) +
        (ast1->inv_moi) * vector2_dot(t1_P, n) * vector2_dot(t1_P, n) +
        (ast2->inv_moi) * vector2_dot(t2_P, n) * vector2_dot(t2_P, n)
    );
    float j = (iszero(j_denom)) ? 0 : j_numer / j_denom;

    ast1->velocity = Vector2Add(ast1->velocity, Vector2Scale(n, j * ast1->inv_mass));
    ast2->velocity = Vector2Add(ast2->velocity, Vector2Scale(n, -j * ast2->inv_mass));

    /*
     *
     *  firstly if the relative normal velocity is positive, they are separating and do
     *  not collide.
     *
     *  secondly the correct equations for the linear and angular velocities v, w are
     *  given in terms of the impulse j, calculated from inverse mass and m.o.i
     *
     *  v_f,A = v_i,A + (j / M_A)n
     *  v_f,B = v_i,B - (j / M_B)n
     *
     *  w_f,A = w_i,A + (r*_AP) . (jn) / I
     *  w_f,B = w_i,A - (r*_BP) . (jn) / I
     *
     *  j = -(1+e) v_i,AB.n / (n.n( 1/M_A + 1/M_B) + (r*_AP.n)^2/I_A + (r*_BP.n)/I_B)
     *
     */
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
            asteroid_collide(curr, next);
        }
    }
}
