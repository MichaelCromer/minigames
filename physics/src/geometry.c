#include <stddef.h>
#include <math.h>


struct Vector { float x; float y; };
struct Segment { struct Vector v0; struct Vector v1; };
struct Triangle { struct Vector v0; struct Vector v1; struct Vector v2; };
struct Polygon { size_t n; struct Vector *v; };


/*
 *  POINT/VECTOR
 */


float vector_len_sqr(const struct Vector v)
{
    return (v.x*v.x) + (v.y*v.y);
}


float vector_len(const struct Vector v)
{
    return sqrt(vector_len_sqr(v));
}


float vector_dot(const struct Vector v1, const struct Vector v2)
{
    return (v1.x*v2.x) + (v1.y*v2.y);
}


float vector_cross(const struct Vector v1, const struct Vector v2)
{
    return (v1.x*v2.y) - (v1.y*v2.x);
}


struct Vector vector_perp(const struct Vector v)
{
    return (struct Vector) { -v.y, v.x };
}


struct Vector vector_scale(const struct Vector v, const float s)
{
    return (struct Vector) { s*v.x, s*v.y };
}


struct Vector vector_sum(const struct Vector v1, const struct Vector v2)
{
    return (struct Vector) { v1.x + v2.x, v1.y + v2.y };
}


struct Vector vector_diff(const struct Vector v1, const struct Vector v2)
{
    return (struct Vector) { v1.x - v2.x, v1.y - v2.y };
}



/*
 * POLYGON
 */


void polygon_destroy(struct Polygon *p)
{
    if (!p) return;
    free(p->v);
    p->v = NULL;
    p->n = 0;
    free(p);
}


struct Polygon *polygon_create(size_t n)
{
    struct Polygon *p = malloc(sizeof(struct Polygon));
    if (!p) return NULL;
    p->v = malloc(n * sizeof(struct Vector));

    if (!p->v) {
        polygon_destroy(p);
        return NULL;
    }

    p->n = n;
    p->v = NULL;

    return p;
}


struct Polygon *polygon_duplicate(const struct Polygon *p)
{
    struct Polygon *q = polygon_create(p->n);
    if (!q) return NULL;
    for (size_t i = 0; i < q->n; i++) {
        q->v[i] = p->v[i];
    }

    return q;
}


float polygon_area_moment_0(const struct Polygon p)
{
    float m0 = 0;
    for (size_t i = 0; i < p.n; i++) { m0 += vector_cross(p.v[i], p.v[(i+1) % p.n]); }
    return 0.5f * fabsf(m0);
}


struct Vector polygon_area_moment_1(const struct Polygon p)
{
    float factor = 0, denom = 0;
    struct Vector m1 = { 0 };
    struct Vector curr = { 0 }, next = p.v[0];

    for (size_t i = 0; i < p.n; i++) {
        curr = next, next = p.v[(i+1) % p.n];
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
    struct Vector curr = { 0 }, next = p.v[0];

    for (size_t i = 0; i < p.n; i++) {
        curr = next, next = p.v[(i+1) % p.n];
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


bool is_point_on_triangle(const struct Vector v, const struct Triangle t)
{
    const struct Vector dt1 = vector_diff(t.v1, t.v0);
    const struct Vector dt2 = vector_diff(t.v2, t.v0);
    const struct Vector dv = vector_diff(v, t.v0);

    const float det = vector_cross(dt1, dt2);
    const float x = vector_cross(dv, dt2);
    const float y = vector_cross(dt1, dv);

    return (
        ((det > 0) && ((x > 0) && (y > 0) && ((x + y) < det)))
            ||
        ((det < 0) && ((x < 0) && (y < 0) && ((x + y) > det)))
    );
}


bool is_point_on_segment
(
    const struct Vector v, const struct Segment s, const float eps
)
{
    const struct Vector ds = vector_diff(s.v1, s.v0);
    const struct Vector dv = vector_diff(v, s.v0);

    const float det = vector_len_sqr(ds);
    const float x = vector_dot(dv, ds);
    const float y = fabsf(vector_cross(dv, ds));

    return (
        (x >= 0) && (x <= det) && (y*y <= eps*eps*det)
    );
}


bool is_segment_on_segment
(
    const struct Segment s1, const struct Segment s2
)
{
    const struct Vector ds1 = vector_diff(s1.v1, s1.v0);
    const struct Vector ds2 = vector_diff(s2.v1, s2.v0);
    const struct Vector d12 = vector_diff(
        vector_sum(s2.v0, s2.v1), vector_sum(s1.v0, s1.v1)
    );

    const float u = fabsf(vector_cross(d12, ds2));
    const float v = fabsf(vector_cross(d12, ds1));
    const float w = fabsf(vector_cross(ds2, ds1));

    return ((u <= w) && (v <= w));
}


bool is_segment_on_triangle
(
    const struct Segment s, const struct Triangle t
)
{
    /* TODO this is wrong because s may be *inside* t */
    return (
        is_segment_on_segment(s, (struct Segment) { t.v0, t.v1 })
            ||
        is_segment_on_segment(s, (struct Segment) { t.v1, t.v2 })
            ||
        is_segment_on_segment(s, (struct Segment) { t.v2, t.v0 })
   );
}
