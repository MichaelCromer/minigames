#include <stdlib.h>
#include <string.h>

#include "geometry.c"


struct RigidBodyArena
{
    /* management data */
    size_t n_max;

    /* body data */
    float *rot;
    float *v_ang;
    float *I_inv;
    float *m_inv;
    float *rad
    struct Vector *pos;
    struct Vector *v_lin;
    struct Polygon *pts;
};


void rigidbodyarena_clear(struct RigidBodyArena *rba)
{
    if (!rba) return;

    if (rba->rot) memset(rba->rot, 0, rba->n_max * sizeof(float));
    if (rba->v_ang) memset(rba->v_ang, 0, rba->n_max * sizeof(float));
    if (rba->I_inv) memset(rba->I_inv, 0, rba->n_max * sizeof(float));
    if (rba->m_inv) memset(rba->m_inv, 0, rba->n_max * sizeof(float));
    if (rba->rad) memset(rba->rad, 0, rba->n_max * sizeof(float));
    if (rba->pos) memset(rba->pos, 0, rba->n_max * sizeof(struct Vector));
    if (rba->v_lin) memset(rba->v_lin, 0, rba->n_max * sizeof(struct Vector));
    if (rba->pts) memset(rba->pts, 0, rba->n_max * sizeof(struct Polygon));
}

void rigidbodyarena_free(struct RigidBodyArena *rba)
{
    if (!rba) return;

    if (rba->rot) free(rba->rot);
    if (rba->v_ang) free(rba->v_ang);
    if (rba->I_inv) free(rba->I_inv);
    if (rba->m_inv) free(rba->m_inv);
    if (rba->rad) free(rba->rad);
    if (rba->pos) free(rba->pos);
    if (rba->v_lin) free(rba->v_lin);
    if (rba->pts) free(rba->pts);

    rba->n_max = 0;
    rba->rot = NULL;
    rba->v_ang = NULL;
    rba->I_inv = NULL;
    rba->m_inv = NULL;
    rba->rad = NULL;
    rba->pos = NULL;
    rba->v_lin = NULL;
    rba->pts = NULL;
}


void rigidbodyarena_destroy(struct RigidBodyArena *rba)
{
    rigidbodyarena_clear(rba);
    rigidbodyarena_free(rba);
    
    free(rba);
}


void rigidbodyarena_malloc(struct RigidBodyArena *rba, size_t n)
{
    if (!rba || !n) return;

    if (
        rba->n_max || rba->rot || rba->v_ang || rba->I_inv || rba->m_inv || rba->rad 
        || rba->pos || rba->v_lin || rba->pts 
    ) return;

    rba->n_max = n;
    rba->rot = malloc(n * sizeof(float));
    rba->v_ang = malloc(n * sizeof(float));
    rba->I_inv = malloc(n * sizeof(float));
    rba->m_inv = malloc(n * sizeof(float));
    rba->rad = malloc(n * sizeof(float));
    rba->pos = malloc(n * sizeof(struct Vector));
    rba->v_lin = malloc(n * sizeof(struct Vector));
    rba->pts = malloc(n * sizeof(struct Polygon));

    if (
        !rba->rot || !rba->v_ang || !rba->I_inv || !rba->m_inv || !rba->rad ||
        !rba->pos || !rba->v_lin || !rba->pts
    ) rigidbodyarena_free(rba);
}


struct RigidBodyArena *rigidbodyarena_create(void)
{
    struct RigidBodyArena *rba =  malloc(sizeof(struct RigidBodyArena));
    rba->n_max = 0;
    rba->rot = NULL;
    rba->v_ang = NULL;
    rba->I_inv = NULL;
    rba->m_inv = NULL;
    rba->rad = NULL;
    rba->pos = NULL;
    rba->v_lin = NULL;
    rba->pts = NULL;

    return rba;
}
