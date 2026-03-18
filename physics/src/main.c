#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "geometry.c"

const int win_width = 800;
const int win_height = 600;
const char* win_title = "Physics Simulation Test";

const size_t n_points = 3000;
const size_t n_segments = 1;
const size_t n_triangles = 1;

struct Vector *vv = NULL;
struct Segment *ss = NULL;
struct Triangle *tt = NULL;

Color *points_colour = NULL;
Color *segments_colour = NULL;
Color *triangles_colour = NULL;

bool is_points_draw = false;
bool is_segments_draw = false;
bool is_triangles_draw = false;


Vector2 vector2(const struct Vector p)
{
    return (Vector2) { p.x, p.y };
}


struct Vector util_random_point(void)
{
    return (struct Vector) {
        GetRandomValue(0, win_width), GetRandomValue(0, win_height)
    };
}


void points_randomise(void)
{
    for (size_t i = 0; i < n_points; i++)
        vv[i] = util_random_point();
}


void segments_randomise(void)
{
    for (size_t i = 0; i < n_segments; i++)
        ss[i] = (struct Segment) { util_random_point(), util_random_point() };
}


void triangles_randomise(void)
{
    for (size_t i = 0; i < n_triangles; i++) {
        tt[i] = (struct Triangle) {
            util_random_point(), util_random_point(), util_random_point()
        };

        if (vector_cross(
            vector_diff(tt[i].v1, tt[i].v0),
            vector_diff(tt[i].v2, tt[i].v0)
        ) < 0) {
            tt[i].v2 = vector_sum(tt[i].v1, tt[i].v2);
            tt[i].v1 = vector_diff(tt[i].v1, tt[i].v2);
            tt[i].v2 = vector_sum(tt[i].v1, tt[i].v2);
            tt[i].v1 = vector_scale(tt[i].v1, -1);
        }
    }
}


void points_draw(void)
{
    for (size_t i = 0; i < n_points; i++)
        DrawCircle(vv[i].x, vv[i].y, 3, points_colour[i]);
}


void segments_draw(void)
{
    for (size_t i = 0; i < n_segments; i++)
        DrawLine(ss[i].v0.x, ss[i].v0.y, ss[i].v1.x, ss[i].v1.y, segments_colour[i]);
}


void triangles_draw(void)
{
    for (size_t i = 0; i < n_triangles; i++)
        DrawTriangleLines(
            vector2(tt[i].v0), vector2(tt[i].v1), vector2(tt[i].v2), triangles_colour[i]
        );
}


void draw(void)
{
    BeginDrawing();
    {
        ClearBackground(BLACK);
        if (is_points_draw) points_draw();
        if (is_segments_draw) segments_draw();
        if (is_triangles_draw) triangles_draw();
    }
    EndDrawing();
}


void points_collision(void)
{
    if (!is_points_draw) return;

    if (is_segments_draw) {
        for (size_t i = 0; i < n_points; i++) {
            for (size_t j = 0; j < n_segments; j++) {
                if (is_point_on_segment(vv[i], ss[j], 10)) {
                    points_colour[i] = RED;
                    segments_colour[j] = RED;
                }
            }
        }
    }

    if (is_triangles_draw) {
        for (size_t i = 0; i < n_points; i++) {
            for (size_t j = 0; j < n_triangles; j++) {
                if (is_point_on_triangle(vv[i], tt[j], 1)) {
                    points_colour[i] = RED;
                    triangles_colour[j] = RED;
                }
            }
        }
    }
}


void segments_collision(void)
{
    if (!is_segments_draw) return;

    for (size_t i = 0; i < n_segments; i++) {
        for (size_t j = i+1; j < n_segments; j++) {
            if (is_segment_on_segment(ss[i], ss[j], EPSILON)) {
                segments_colour[i] = RED;
                segments_colour[j] = RED;
            }
        }
    }

    if (is_triangles_draw) {
        for (size_t i = 0; i < n_segments; i++) {
            for (size_t j = 0; j < n_triangles; j++) {
                if (is_segment_on_triangle(ss[i], tt[j], EPSILON)) {
                    segments_colour[i] = RED;
                    triangles_colour[j] = RED;
                }
            }
        }
    }
}


void update_collision(void)
{
    for (size_t i = 0; i < n_points; i++) points_colour[i] = GREEN;
    for (size_t i = 0; i < n_segments; i++) segments_colour[i] = GREEN;
    for (size_t i = 0; i < n_triangles; i++) triangles_colour[i] = GREEN;

    points_collision();
    segments_collision();
}


void update(void)
{
    switch (GetKeyPressed()) {

        case KEY_R:
            points_randomise();
            segments_randomise();
            triangles_randomise();
            break;

        case KEY_P:
            is_points_draw = !is_points_draw;
            break;

        case KEY_S:
            is_segments_draw = !is_segments_draw;
            break;

        case KEY_T:
            is_triangles_draw = !is_triangles_draw;
            break;
    }

    update_collision();
}


void initialise(void)
{
    InitWindow(win_width, win_height, win_title);
    SetRandomSeed(0);

    vv = malloc(n_points * sizeof(struct Vector));
    ss = malloc(n_segments * sizeof(struct Segment));
    tt = malloc(n_triangles * sizeof(struct Triangle));

    points_colour = malloc(n_points * sizeof(Color));
    segments_colour = malloc(n_segments * sizeof(Color));
    triangles_colour = malloc(n_triangles * sizeof(Color));

    points_randomise();
    segments_randomise();
    triangles_randomise();
}


void deinitialise(void)
{
    CloseWindow();
}


int main(void)
{
    initialise();

    while (!WindowShouldClose()) {
        draw();
        update();
    }

    deinitialise();

    return 0;
}
