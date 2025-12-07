#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>


#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define BULLET_LIFTIME 1
#define BULLET_VELOCITY 300
#define BULLETQUEUE_LEN_MAX 100
#define ASTEROIDQUEUE_LEN_MAX 100

#define ASTEROID_VERTICES_MAX 6
#define ASTEROID_DENSITY 1


static inline Vector2 vector2_wrap(Vector2 vec, const Vector2 min, const Vector2 max)
{
    Vector2 res = vec;
    if (res.x < min.x) res.x = max.x;
    if (res.y < min.y) res.y = max.y;
    if (res.x > max.x) res.x = min.x;
    if (res.y > max.y) res.y = min.y;
    return res;
}


static inline float vector2_dot(Vector2 v1, Vector2 v2)
{
    return (v1.x*v2.x) + (v1.y*v2.y);
}


static inline float vector2_cross(Vector2 v1, Vector2 v2)
{
    return (v1.x*v2.y) - (v1.y*v2.x);
}


static inline Vector2 vector2_perp(Vector2 v)
{
    return (Vector2) { -v.y, v.x };
}


static inline Vector2 vector2_diff(Vector2 v1, Vector2 v2)
{
    return (Vector2) { v1.x - v2.x, v1.y - v2.y };
}


static inline bool point_on_triangle(Vector2 p, Vector2 v0, Vector2 v1, Vector2 v2)
{
    /* inverts the matrix implied by the wi = (vi - v0)
     * checks for image in the unit simplex
     */

    Vector2 w1 = Vector2Subtract(v1, v0);
    Vector2 w2 = Vector2Subtract(v2, v0);
    Vector2 q = Vector2Subtract(p, v0);

    float det = (w1.x * w2.y) - (w2.x * w1.y);
    if (fabsf(det) < EPSILON) return false;

    float invdet = 1/det;
    float x = ((w2.y*q.x) - (w2.x*q.y)) * invdet;
    float y = ((w1.x*q.y) - (w1.y*q.x)) * invdet;

    return (x >= 0) && (y >= 0) && ((x + y) <= 1);
}


static inline bool point_on_segment(Vector2 p, Vector2 v0, Vector2 v1)
{
    /* t is projection of q = (p - v0) onto dv = (v1 - v0)
     * s is area of rectangle q dv 
     * check for t in [0, |dv|^2]
     * check for s^2 in [0, |dv|^2] (implies tolerance of 1px on either side)
     */

    Vector2 dv = Vector2Subtract(v1, v0);
    Vector2 q = Vector2Subtract(p, v0);
    float norm_dv2 = Vector2LengthSqr(dv);
    float t = vector2_dot(q, dv);
    float s = vector2_cross(q, dv);

    return (t >= 0) && (t <= norm_dv2) && (s*s <= norm_dv2*(1 + EPSILON));
}


static inline bool segment_on_segment(Vector2 p0, Vector2 p1, Vector2 q0, Vector2 q1)
{
    /* solve the system (K : p0 + a*dp) == (L : q0 + b*dq)
     * check that the solution has a, b in [0, 1]
     */

    Vector2 dp = Vector2Subtract(p1, p0);
    Vector2 dq = Vector2Subtract(q1, q0);
    Vector2 r = Vector2Subtract(q0, p0);

    float s = vector2_cross(r, dq);
    float t = vector2_cross(r, dp);
    float u = vector2_cross(dp, dq);
    
    if (u < 0) { s = -s, t = -t, u = -u; }
    return ((s >= 0) && (t >= 0) && (s <= u) && (t <= u));
}


static inline bool polygon_is_axis_separate
(
    Vector2 *vertices1, size_t n1, Vector2 *vertices2, size_t n2
)
{    
    /*  separating axis theorem
     *      two polygons intersect iff every edge-normal projection intersects
     */
    Vector2 curr, next, norm;
    float min, max, base, proj;
    bool intersect;

    curr = (Vector2) { 0, 0 }, next = vertices1[0];
    for (size_t i = 0; i < n1; i++) {
        intersect = false; 

        curr = next, next = vertices1[(i+1) % n1];
        norm = vector2_perp(Vector2Subtract(next, curr));

        /* base axis *min* assuming anticlockwise points */
        base = vector2_dot(norm, curr);

        proj = vector2_dot(norm, vertices2[0]);
        min = proj, max = proj;
        for (size_t j = 0; j < n2; j++) {
            proj = vector2_dot(norm, vertices2[j]);

            if (proj < min) min = proj;
            if (proj > max) max = proj;

            /* we know intersection as soon as max and min are on opposite sides */
            if (((min < base) && (max > base)) || ((min > base) && (max < base))) {
                intersect = true;
                break;
            }
        }

        if (!intersect) return false;
    }
    return true;
}


static inline bool polygon_on_polygon
(
    Vector2 *vertices1, size_t n1, Vector2 *vertices2, size_t n2
)
{
    return (
        polygon_is_axis_separate(vertices1, n1, vertices2, n2) ||
        polygon_is_axis_separate(vertices2, n2, vertices1, n1)
    );
}


static inline float polygon_area_moment_0(Vector2 *vertices, size_t n)
{
    if (!vertices || !n) return 0;

    float area = 0;
    Vector2 curr = { 0 }, next = vertices[0];
    for (size_t i = 0; i < n; i++) {
        curr = next, next = vertices[(i+1) % n];
        area += vector2_cross(curr, next);
    }

    return fabsf(area);
}


static inline Vector2 polygon_area_moment_1(Vector2 *vertices, size_t n)
{
    if (!vertices || !n) return (Vector2) { 0, 0 };

    Vector2 moment_1 = { 0 };
    float factor = 0, denominator = 0;
    Vector2 curr = { 0 }, next = vertices[0];

    for (size_t i = 0; i < n; i++) {
        curr = next, next = vertices[(i+1) % n];
        factor = vector2_cross(curr, next);
        moment_1.x += (curr.x + next.x) * factor;
        moment_1.y += (curr.y + next.y) * factor;
        denominator += factor;
    }

    return Vector2Scale(moment_1, 1/(3*denominator));
}


static inline float polygon_area_moment_2(Vector2 *vertices, size_t n)
{
    if (!vertices || !n) return 0;

    float moment_2 = 0;
    float factor = 0, denominator = 0;
    Vector2 curr = { 0 }, next = vertices[0];

    for (size_t i = 0; i < n; i++) {
        curr = next, next = vertices[(i+1) % n];
        factor = vector2_cross(curr, next);
        moment_2 += factor * (
            Vector2LengthSqr(curr) + vector2_dot(curr, next) + Vector2LengthSqr(next)
        );
        denominator += factor;
    }
    
    return moment_2 / (6*denominator);
}


#include "asteroid.c"
#include "bullet.c"
#include "player.c"
#include "state.c"


int main(void)
{
    bool paused = false;

    struct State *state = state_create();
    state_initialise(state);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "hey hey hey");

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(SKYBLUE);
            state_draw(state);
        }
        EndDrawing();

        if (IsKeyPressed(KEY_P)) paused = !paused;
        if (paused) continue;

        state_update(state, GetFrameTime());
    }

    CloseWindow();

    state_destroy(state);
    return 0;
}
