#include <stdlib.h>
#include <sys/types.h>

#include "geometry.c"
#include "body.c"


struct RigidBodyArena
{
    /* management data */
    size_t n_max;
    size_t n;
    size_t *ix;
    size_t *id;

    /* body data */
    float *rot;
    float *v_ang;
    float *I_inv;
    float *m_inv;
    float *R;
    struct Vector *pos;
    struct Vector *v_lin;
    struct Polygon **pts;
};


/*
 * UTILITY
 */


/* keep reserved memory but zero it out
 * dealloc any points data
 */
void rigidbodyarena_clear(struct RigidBodyArena *rba)
{
    if (!rba) return;

    for (size_t i = 0; i < rba->n_max; i++) {
        rba->id[i] = 0;
        rba->ix[i] = 0;
        rba->rot[i] = 0;
        rba->v_ang[i] = 0;
        rba->I_inv[i] = 0;
        rba->m_inv[i] = 0;
        rba->R[i] = 0;
        rba->pos[i] = (struct Vector) { 0 };
        rba->v_lin[i] = (struct Vector) { 0 };
        polygon_destroy(rba->pts[i]);
        rba->pts[i] = NULL;
    }
    rba->n = 0;
}


void rigidbodyarena_free(struct RigidBodyArena *rba)
{
    if (!rba) return;

    rigidbodyarena_clear(rba);

    free(rba->id);
    free(rba->ix);
    free(rba->rot);
    free(rba->v_ang);
    free(rba->I_inv);
    free(rba->m_inv);
    free(rba->R);
    free(rba->pos);
    free(rba->v_lin);
    free(rba->pts);

    *rba = (struct RigidBodyArena) { 0 };
}


/* release reserved memory
 */
void rigidbodyarena_destroy(struct RigidBodyArena *rba)
{
    if (!rba) return;

    rigidbodyarena_free(rba);
    free(rba);
}


int rigidbodyarena_malloc(struct RigidBodyArena *rba, size_t n)
{
    if (!rba) return 1;

    rba->ix = malloc(n * sizeof(size_t));
    rba->id = malloc(n * sizeof(size_t));
    rba->rot = malloc(n * sizeof(float));
    rba->v_ang = malloc(n * sizeof(float));
    rba->I_inv = malloc(n * sizeof(float));
    rba->m_inv = malloc(n * sizeof(float));
    rba->R = malloc(n * sizeof(float));
    rba->pos = malloc(n * sizeof(struct Vector));
    rba->v_lin = malloc(n * sizeof(struct Vector));
    rba->pts = malloc(n * sizeof(struct Polygon *));

    if (
        !rba->ix || !rba->id || !rba->rot || !rba->v_ang || !rba->I_inv || !rba->m_inv
        || !rba->R || !rba->pos || !rba->v_lin || !rba->pts
    ) {
        rigidbodyarena_free(rba);
        return 2;
    }

    rba->n_max = n;
    return 0;
}


struct RigidBodyArena *rigidbodyarena_create(size_t n)
{
    struct RigidBodyArena *rba =  malloc(sizeof(struct RigidBodyArena));
    int err = rigidbodyarena_malloc(rba, n);
    if (err) {
        rigidbodyarena_destroy(rba);
        return NULL;
    }

    rigidbodyarena_clear(rba);

    return rba;
}


/*
 *  ARRAY MANAGEMENT
 */


void rigidbodyarena_reset(struct RigidBodyArena *rba)
{
    if (!rba) return;

    for (size_t i = 0; i < rba->n_max; i++) {
        rba->id[i] = i;
        rba->ix[i] = i;
    }
    rba->n = 0;
}


ssize_t rigidbodyarena_register(struct RigidBodyArena *rba, const struct Body *b)
{

    if (rba->n == rba->n_max) return -1;

    /* copy body data into arena at index rba->n */
    rba->rot[rba->n] = b->rot;
    rba->v_ang[rba->n] = b->v_ang;
    rba->I_inv[rba->n] = b->I_inv;
    rba->m_inv[rba->n] = b->m_inv;
    rba->R[rba->n] = b->R;
    rba->pos[rba->n] = b->pos;
    rba->v_lin[rba->n] = b->v_lin;

    polygon_destroy(rba->pts[rba->n]);
    rba->pts[rba->n] = polygon_duplicate(&(b->pts));

    size_t id = rba->id[rba->n];
    rba->ix[id] = rba->n;
    rba->n++;

    return (ssize_t) id; // return the id # allocated to the body
}


void rigidbodyarena_deregister(struct RigidBodyArena *rba, const size_t id)
{
    if (!rba) return;
    if (!rba->n || id >= rba->n_max) return;

    size_t p_i = rba->ix[id];       /* the index of the element with this id */
    size_t p_n = rba->n - 1;        /* the index of the last element */
    if (p_i > p_n) return;          /* the id is already unregistered */
    size_t i_n = rba->id[p_n];      /* the id of the element at index p_n */

    /* swapback last element */
    rba->rot[p_i] = rba->rot[p_n];
    rba->v_ang[p_i] = rba->v_ang[p_n];
    rba->I_inv[p_i] = rba->I_inv[p_n];
    rba->m_inv[p_i] = rba->m_inv[p_n];
    rba->R[p_i] = rba->R[p_n];
    rba->pos[p_i] = rba->pos[p_n];
    rba->v_lin[p_i] = rba->v_lin[p_n];

    polygon_destroy(rba->pts[p_i]);
    rba->pts[p_i] = rba->pts[p_n];
    rba->pts[p_n] = NULL;

    /* update the id list */
    rba->id[p_n] = id;          /* the element at index n - 1 now has this id */
    rba->id[p_i] = i_n;         /* the element at index p_i now has id i_n*/

    /* update the index list */
    rba->ix[id] = p_n;          /* the element with this id is now last */
    rba->ix[i_n] = p_i;         /* the element with id i_n is now at index p_i */

    rba->n--;
}


/*
 *  PHYSICS
 */

void rigidbodyarena_kinematics(struct RigidBodyArena *rba, const float dt)
{
    if (!rba) return;

    for (size_t i = 0; i < rba->n; i++) {
        rba->pos[i] = vector_sum(rba->pos[i], vector_scale(rba->v_lin[i], dt));
        rba->rot[i] += rba->v_ang[i]*dt;
    }
}
