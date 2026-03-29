#ifndef PHYSICS_GEOMETRY_H
#define PHYSICS_GEOMETRY_H

struct Vector;
struct Segment;
struct Triangle;
struct Polygon;

float vector_len_sqr(const struct Vector v);
float vector_len(const struct Vector v);
float vector_dot(const struct Vector v1, const struct Vector v2);
float vector_cross(const struct Vector v1, const struct Vector v2);
struct Vector vector_perp(const struct Vector v);
struct Vector vector_scale(const struct Vector v, const float s);
struct Vector vector_sum(const struct Vector v1, const struct Vector v2);
struct Vector vector_diff(const struct Vector v1, const struct Vector v2);

void polygon_destroy(struct Polygon *p);
struct Polygon *polygon_create(size_t n);
struct Polygon *polygon_duplicate(const struct Polygon *p);
float polygon_area_moment_0(const struct Polygon p);
struct Vector polygon_area_moment_1(const struct Polygon p);
float polygon_area_moment_2(const struct Polygon p);

bool is_segment_on_segment(const struct Segment s1, const struct Segment s2);
bool is_segment_on_triangle( const struct Segment s, const struct Triangle t);
bool is_point_on_triangle(const struct Vector v, const struct Triangle t);
bool is_point_on_segment(
    const struct Vector v, const struct Segment s, const float eps
);

#endif
