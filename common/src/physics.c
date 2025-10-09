#include <raylib.h>
#include <raymath.h>

/* Geometry */

struct Polygon
{
    Vector2 *vertices;
    size_t len;
};


void polygon_clear(struct Polygon *polygon)
{
    if (!polygon) return;
    free(polygon->vertices);
    polygon->vertices = NULL;
    polygon->len = 0;
}


bool polygon_is_null(struct Polygon *polygon)
{
    return (!polygon || !polygon->vertices || !polygon->len);
}


float vector2_dot(Vector2 v1, Vector2 v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y);
}


float vector2_cross(Vector2 v1, Vector2 v2)
{
    return (v1.x * v2.y) - (v1.y * v2.x);
}


float polygon_area_moment_0(struct Polygon *polyon)
{
    if (polygon_is_null()) return 0;

    float area = 0;
    Vector2 curr = { 0 }, next = polygon->vertices[0];
    for (size_t i = 0; i < polygon->len; i++) {
        curr = next, next = polygon->vertices[(i+1) % polygon->len];
        area += vector2_cross(curr, next);
    }

    return area;
}


Vector2 polygon_area_moment_1(struct Polygon *polygon)
{
    if (polygon_is_null()) return (Vector2) { 0, 0 };

    Vector2 moment_1 = { 0 };
    float factor = 0, denominator = 0;
    Vector2 curr = { 0 }, next = polygon->vertices[0];

    for (size_t i = 0; i < polygon->len; i++) {
        curr = next, next = polygon->vertices[(i+1) % polygon->len];
        factor = vector2_cross(curr, next);
        moment_1.x += (curr.x + next.x) * factor;
        moment_1.y += (curr.y + next.y) * factor;
        denominator += factor;
    }

    return Vector2Scale(moment_1, 1/(3*denominator));
}


float polygon_area_moment_2(struct Polygon *polygon)
{
    if (polygon_is_null()) return 0;

    float moment_2 = 0;
    float factor = 0, denominator = 0;
    Vector2 curr = { 0 }, next = polygon->vertices[0];

    for (size_t i = 0; i < polygon->len; i++) {
        curr = next, next = polygon->vertices[(i+1) % polygon->len];
        factor = vector2_cross(curr, next);
        moment_2 += factor * (
            Vector2LengthSqr(curr) + vector2_dot(curr, next) + Vector2LengthSqr(next)
        );
        denominator += factor;
    }
    
    return moment_2 / (6*denominator);
}


struct PhysicsBody
{
    struct Polygon *hull;
    Vector2 position;
    Vector2 velocity;
    float   mass;
    float   rotation;
    float   spin;
    float   moi;
};
