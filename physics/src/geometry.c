#include <raylib.h>
#include <raymath.h>
#include <stddef.h>

struct Point { float x, float y };
struct Segment { Vector2 v0; Vector2 v1; };
struct Triangle { Vector2 v0; Vector2 v1; Vector2 v2; };
struct Polygon { size_t n, Vector2 *v };

/*
 *  VECTOR
 */


float vector2_dot(const Vector2 v1, const Vector2 v2)
{
    return (v1.x*v2.x) + (v1.y*v2.y);
}


float vector2_cross(const Vector2 v1, const Vector2 v2)
{
    return (v1.x*v2.y) - (v1.y*v2.x);
}


Vector2 vector2_perp(const Vector2 v)
{
    return (Vector2) { -v.y, v.x };
}


/*
 *  TRIANGLE
 */


float triangle_area(const struct Triangle t)
{
    return fabsf(
        vector2_cross(Vector2Subtract(t.v1, t.v0), Vector2Subtract(t.v2, t.v0))
    );
}


struct Triangle triangle_translate(const struct Triangle t, const Vector2 v)
{
    return (struct Triangle) {
        Vector2Add(t.v0, v), Vector2Add(t.v1, v), Vector2Add(t.v2, v)
    };
}


/*
 * POLYGON
 */

float polygon_area(const struct Polygon p)
{
    float area = 0;
    for (size_t i = 0; i < p.n; i++) { area += vector2_cross(p.v[i], p.v[(i+1) % p.n]) }
    return 0.5 * fabsf(area);
}


Vector2 polygon_centroid(const struct Polygon p)
{
    Vector2 centroid = { 0 };

    float denom = 0;
    Vector2 curr = { 0 }, p.v[0];
    for (size_t i = 0; i < p.n; i++) {
        curr = next, next = p.v[(i+1) % p.n];
        factor = vector2_cross(curr, next);
        centroid = Vector2Add(centroid, (Vector2Scale(Vector2Add(curr, next), factor)));
        denom += factor;
    }

    return Vector2Scale(centroid, 1/(3.0f*denom));
}


float polygon_area_moment_2(Vector2 *vertices, size_t n)
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


/*
 * INTERSECTIONS
 */


bool is_point_on_triangle(const Vector2 p, const struct Triangle t, const float eps)
{
    const Vector2 dt1 = Vector2Subtract(t.v1, t.v0);
    const Vector2 dt2 = Vector2Subtract(t.v2, t.v0);
    const Vector2 dp = Vector2Subtract(p, t.v0);

    const float det = fabsf(vector2_cross(dt1, dt2));
    const float x = fabsf(vector2_cross(dp, dt2));
    const float y = fabsf(vector2_cross(dt1, dp));

    return (
        (det > EPSILON) && (x > -eps*det) && (y > -eps*det) && ((x+y) < (1+eps)*det)
    );
}


bool is_point_on_segment(const Vector2 p, const struct Segment s, const float eps)
{
    const Vector2 ds = Vector2Subtract(s.v1, s.v0);
    const Vector2 dp = Vector2Subtract(p, s.v0);

    const float det = Vector2LengthSqr(ds);
    const float x = vector2_dot(dp, ds);
    const float y = fabsf(vector2_cross(dp, ds));

    return (
        (det > EPSILON) && (x > -eps*det) && (x < (1 + eps)*det) && (y < eps*det)
    );
}


bool is_segment_on_segment(const struct Segment s1, const struct Segment s2)
{
    /* solve the system (K : p0 + a*dp) == (L : q0 + b*dq)
     * check that the solution has a, b in [0, 1]
     */

    const Vector2

    Vector2 dp = Vector2Subtract(p1, p0);
    Vector2 dq = Vector2Subtract(q1, q0);
    Vector2 r = Vector2Subtract(q0, p0);

    float s = vector2_cross(r, dq);
    float t = vector2_cross(r, dp);
    float u = vector2_cross(dp, dq);
    
    if (u < 0) { s = -s, t = -t, u = -u; }
    return ((s >= 0) && (t >= 0) && (s <= u) && (t <= u));
}


bool is_polygon_is_axis_separate
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


bool is_polygon_on_polygon
(
    Vector2 *vertices1, size_t n1, Vector2 *vertices2, size_t n2
)
{
    return (
        polygon_is_axis_separate(vertices1, n1, vertices2, n2) ||
        polygon_is_axis_separate(vertices2, n2, vertices1, n1)
    );
}
