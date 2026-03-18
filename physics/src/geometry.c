#include <raylib.h>
#include <raymath.h>
#include <stddef.h>


struct Point { float x; float y; };
struct Segment { struct Point p0; struct Point p1; };
struct Triangle { struct Point p0; struct Point p1; struct Point p2; };
struct Polygon { size_t n; struct Point *p; };


/*
 *  POINT/VECTOR
 */


float vector_len_sqr(const struct Point p)
{
    return (p.x*p.x) + (p.y*p.y);
}


float vector_dot(const struct Point p1, const struct Point p2)
{
    return (p1.x*p2.x) + (p1.y*p2.y);
}


float vector_cross(const struct Point p1, const struct Point p2)
{
    return (p1.x*p2.y) - (p1.y*p2.x);
}


struct Point vector_perp(const struct Point v)
{
    return (struct Point) { -v.y, v.x };
}


struct Point vector_scale(const struct Point p, const float s)
{
    return (struct Point) { s*p.x, s*p.y };
}


struct Point vector_sum(const struct Point p1, const struct Point p2)
{
    return (struct Point) { p1.x + p2.x, p1.y + p2.y };
}


struct Point vector_diff(const struct Point p1, const struct Point p2)
{
    return (struct Point) { p1.x - p2.x, p1.y - p2.y };
}


/*
 *  TRIANGLE
 */


float triangle_area(const struct Triangle t)
{
    return fabsf(
        vector_cross(vector_diff(t.p1, t.p0), vector_diff(t.p2, t.p0))
    );
}


struct Triangle triangle_translate(const struct Triangle t, const struct Point v)
{
    return (struct Triangle) {
        vector_sum(t.p0, v), vector_sum(t.p1, v), vector_sum(t.p2, v)
    };
}


/*
 * POLYGON
 */


float polygon_area_moment_0(const struct Polygon p)
{
    float m0 = 0;
    for (size_t i = 0; i < p.n; i++) { m0 += vector_cross(p.p[i], p.p[(i+1) % p.n]); }
    return 0.5f * fabsf(m0);
}


struct Point polygon_area_moment_1(const struct Polygon p)
{
    float factor = 0, denom = 0;
    struct Point m1 = { 0 };
    struct Point curr = { 0 }, next = p.p[0];

    for (size_t i = 0; i < p.n; i++) {
        curr = next, next = p.p[(i+1) % p.n];
        factor = vector_cross(curr, next);
        m1 = vector_sum(m1, (vector_scale(vector_sum(curr, next), factor)));
        denom += factor;
    }

    return vector_scale(m1, 1/(3.0f*denom));
}


float polygon_area_moment_2(const struct Polygon p)
{
    float m2 = 0;
    float factor = 0, denom = 0;
    struct Point curr = { 0 }, next = p.p[0];

    for (size_t i = 0; i < p.n; i++) {
        curr = next, next = p.p[(i+1) % p.n];
        factor = vector_cross(curr, next);
        m2 += factor * (
            vector_len_sqr(curr) + vector_dot(curr, next) + vector_len_sqr(next)
        );
        denom += factor;
    }
    
    return m2 / (6.0f*denom);
}


/*
 * INTERSECTIONS
 */


bool is_point_on_triangle(const struct Point p, const struct Triangle t, const float eps)
{
    /*
     * need to check if coordinates given by a(t1 - t0) + b(t2 - t0) = p - t0
     * are within bounds (i.e. have a > 0 and b > 0 and a + b < 1)
     *
     *  dt1 = (t1 - t0),    dt2 = (t2 - t0),    dp = (p - t0)
     *
     * get
     *  a(dt1 x dt2) = dp x dt2,    b(dt1 x dt2) = dt1 x dp
     *
     *  check requires matching sign! i.e. dt1 x dt2 may be negative
     */

    const struct Point dt1 = vector_diff(t.p1, t.p0);
    const struct Point dt2 = vector_diff(t.p2, t.p0);
    const struct Point dp = vector_diff(p, t.p0);

    const float det = vector_cross(dt1, dt2);
    const float x = vector_cross(dp, dt2);
    const float y = vector_cross(dt1, dp);

    return (
        ((det > 0) && ((x > -2*eps) && (y > -2*eps) && ((x + y) < (1+2*eps)*det)))
            ||
        ((det < 0) && ((x < 2*eps) && (y < 2*eps) && ((x + y) > (1+2*eps)*det)))
    );
}


bool is_point_on_segment(const struct Point p, const struct Segment s, const float eps)
{
    const struct Point ds = vector_diff(s.p1, s.p0);
    const struct Point dp = vector_diff(p, s.p0);

    const float det = vector_len_sqr(ds);
    const float x = vector_dot(dp, ds);
    const float y = fabsf(vector_cross(dp, ds));

    return (
        (det > EPSILON) && (x > -eps*det) && (x < (1 + eps)*det) && (y < eps*det)
    );
}


bool is_segment_on_segment
(
    const struct Segment s1, const struct Segment s2, const float eps
)
{
    const struct Point m1 = vector_sum(s1.p0, s1.p1);
    const struct Point m2 = vector_sum(s2.p0, s2.p1);

    const struct Point ds1 = vector_diff(s1.p1, s1.p0);
    const struct Point ds2 = vector_diff(s2.p1, s2.p0);
    const struct Point d12 = vector_diff(m2, m1);

    const float u = fabsf(vector_cross(d12, ds2));
    const float v = fabsf(vector_cross(d12, ds1));
    const float w = fabsf(vector_cross(ds2, ds1));

    return ((u <= (1 + eps)*w) && (v <= (1 + eps)*w));
}


bool is_segment_on_triangle
(
    const struct Segment s, const struct Triangle t, const float eps
)
{
    return (
        is_segment_on_segment(s, (struct Segment) { t.p0, t.p1 }, eps)
            ||
        is_segment_on_segment(s, (struct Segment) { t.p1, t.p2 }, eps)
            ||
        is_segment_on_segment(s, (struct Segment) { t.p2, t.p0 }, eps)
   );
}
