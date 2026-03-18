#include "geometry.c"


struct Body
{
    float rot;
    float v_ang;
    float I;
    float I_inv;
    float m;
    float m_inv;
    float R;
    struct Vector pos;
    struct Vector v_lin;
    struct Polygon pts;
}


void body_set_pos(const struct Body *b, const Vector v)
{
    b->pos = v;
}


void body_kinematics(const struct Body *b, const float dt)
{
    b->pos = vector_sum(b->pos, vector_scale(b->v_lin, dt));
    b->rot += v_ang*dt;
}


void body_bounce(const struct Body *b, const Vector r)
{
    b->v_lin = vector_reflect_across(b->v_lin, r);
}
