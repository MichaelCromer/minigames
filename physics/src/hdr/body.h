#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

struct Body;

void body_destroy(struct Body *b);
struct Body *body_create(size_t n);
struct Vector body_pos(const struct Body *b);
void body_set_pos(struct Body *b, const struct Vector v);
void body_set_mass(struct Body *b, const float m);
void body_immobilise(struct Body *b);
void body_impulse(struct Body *b, const struct Vector dp);
void body_force(struct Body *b, const struct Vector F, const float dt);
void body_accelerate(struct Body *b, const struct Vector a, const float dt);
void body_kinematics(struct Body *b, const float dt);

#endif
