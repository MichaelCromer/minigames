struct Bullet
{
    Vector2 position;
    Vector2 velocity;
    float lifetime;
};


struct BulletQueue
{
    struct Bullet *bullets;
    size_t len;
    size_t max;
};



bool bullet_alive(struct Bullet *b)
{
    return (0 < b->lifetime);
}


void bullet_clear(struct Bullet *b)
{
    b->position = (Vector2) { 0, 0 };
    b->velocity = (Vector2) { 0, 0 };
    b->lifetime = 0;
}


void bullet_draw(struct Bullet *b)
{
    if (!b || !bullet_alive(b)) return;
    DrawCircle(b->position.x, b->position.y, 2, WHITE);
}


void bullet_update(struct Bullet *b, float dt)
{
    if (!b || !bullet_alive(b)) return;
    b->position = vector2_wrap(
        Vector2Add(b->position, Vector2Scale(b->velocity, dt)),
        (Vector2) { 0, 0 },
        (Vector2) { WINDOW_WIDTH, WINDOW_HEIGHT }
    );
    b->lifetime -= dt;
}


void bulletqueue_destroy(struct BulletQueue *bq)
{
    if (!bq) return;
    if (bq->bullets) free(bq->bullets);
    free(bq);
}


struct BulletQueue *bulletqueue_create(size_t max)
{
    struct BulletQueue *bq = malloc(sizeof(struct BulletQueue));
    if (!bq) return NULL;

    bq->bullets = malloc(max * sizeof(struct Bullet));
    if (!bq->bullets) {
        bulletqueue_destroy(bq);
        return NULL;
    }

    for (size_t i = 0; i < max; i++) bullet_clear(bq->bullets + i);

    bq->len = 0;
    bq->max = max;

    return bq;
}


void bulletqueue_apply
(
    struct BulletQueue *bq,
    void (*func)(struct Bullet *)
)
{
    if (!bq || !func) return;
    for (size_t i = 0; i < bq->len; i++) func(bq->bullets + i);
}


void bulletqueue_insert(struct BulletQueue *bq, struct Bullet b)
{
    if (!bq || (bq->len >= bq->max)) return;
    bq->bullets[bq->len++] = b;
}


void bulletqueue_remove(struct BulletQueue *bq, size_t i)
{
    if (!bq || i >= bq->len) return;
    bq->bullets[i] = bq->bullets[--bq->len];
}


void bulletqueue_draw(struct BulletQueue *bq)
{
    bulletqueue_apply(bq, bullet_draw);
}


void bulletqueue_update(struct BulletQueue *bq, float dt)
{
    size_t i = 0;
    while ((i < bq->len) && bullet_alive(bq->bullets + i)) {
        bullet_update(bq->bullets + i, dt);

        if (!bullet_alive(bq->bullets + i)) bulletqueue_remove(bq, i);
        else i++;
    }
}
