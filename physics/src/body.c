#include "geometry.c"


struct Body
{
    float rot;
    float v_ang;
    float I_inv;
    float m_inv;
    float R;
    struct Vector pos;
    struct Vector v_lin;
    struct Polygon pts;
}


struct Vector body_pos(const struct Body *b) { return b->pos; }

void body_set_pos(const struct Body *b, const Vector v) { b->pos = v; }
void body_set_mass(const struct Body *b, const float m) { m ? b->m_inv = 1/m : return; }

void body_immobilise(const struct Body *b) { b->m_inv = 0; b->I_inv = 0; }


void body_impulse(const struct Body *b, const Vector dp)
{
    b->v_lin = vector_sum(b->v_lin, dp);
}


void body_force(const struct Body *b, const Vector F, const float dt)
{
    b->v_lin = vector_sum(b->v_lin, vector_scale(F, b->m_inv*dt));
}


void body_accelerate(const struct Body *b, const Vector a, const float dt)
{
    b->v_lin = vector_sum(b->v_lin, vector_scale(a, dt));
}


void body_update(const struct Body *b, const float dt)
{
    b->pos = vector_sum(b->pos, vector_scale(b->v_lin, dt));
    b->rot += v_ang*dt;
}

/*
void asteroid_collide(struct Asteroid *ast1, struct Asteroid *ast2)
{
     early exit if too far apart
    float dr = Vector2Length(Vector2Subtract(ast1->centre, ast2->centre));
    if (dr > (asteroid_radius(ast1) + asteroid_radius(ast2))*(1 + EPSILON)) return;

     calculate collision axis and point
    Vector2 n, P;
    asteroid_collision_data(ast1, ast2, &n, &P);

     early exit if no collision axis
    if (Vector2Equals(n, Vector2Zero())) return;

     distances from centres to collision point
    Vector2 r1_P = Vector2Subtract(P, ast1->centre);
    Vector2 r2_P = Vector2Subtract(P, ast2->centre);

     tangential vector at collision point
    Vector2 t1_P = vector2_perp(r1_P);
    Vector2 t2_P = vector2_perp(r2_P);

     rotational velocity
    Vector2 w1_P = Vector2Scale(t1_P, ast1->spin);
    Vector2 w2_P = Vector2Scale(t2_P, ast2->spin);

     net velocities at collision point
    Vector2 v1_P = Vector2Add(w1_P, ast1->velocity);
    Vector2 v2_P = Vector2Add(w2_P, ast2->velocity);

     relative velocity at collision points
    Vector2 v_12 = Vector2Subtract(v2_P, v1_P);

     early exit if velocity on collision axis is negative
    if (vector2_dot(n, v_12) <= 0) return;

    ast1->collision = true, ast2->collision = true;
    ast1->colour = GREEN, ast2->colour = RED;

     j is the magic scalar
    float j_numer = -2 * vector2_dot(n, v_12);
    float j_denom = (
        vector2_dot(n, n) * (ast1->inv_mass + ast2->inv_mass) +
        (ast1->inv_moi) * vector2_dot(t1_P, n) * vector2_dot(t1_P, n) +
        (ast2->inv_moi) * vector2_dot(t2_P, n) * vector2_dot(t2_P, n)
    );
    float j = (iszero(j_denom)) ? 0 : j_numer / j_denom;

    ast1->velocity = Vector2Add(ast1->velocity, Vector2Scale(n, j * ast1->inv_mass));
    ast2->velocity = Vector2Add(ast2->velocity, Vector2Scale(n, -j * ast2->inv_mass));


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
}

*/
