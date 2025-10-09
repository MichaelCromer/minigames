struct Player
{
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float mass;
    float engine;
    float drag;
    float reload;
};


bool player_can_fire(struct Player *player)
{
    return (0 >= player->reload);
}


void player_destroy(struct Player *player)
{
    free(player);
}


struct Player *player_create(void)
{
    struct Player *player = malloc(sizeof(struct Player));
    return player;
}


void player_update_rotation(struct Player *p, float dt)
{
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) p->rotation -= dt * 6;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) p->rotation += dt * 6;
}


void player_update_position(struct Player *player, float dt)
{
    Vector2 force = { 0, 0 };

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        Vector2 df = { cos(player->rotation), sin(player->rotation) };
        force = Vector2Add(force, df);
    }

    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) { 
        Vector2 df = { -0.3 * cos(player->rotation), -0.3 * sin(player->rotation) };
        force = Vector2Add(force, df);
    }

    force = Vector2Add(force, Vector2Scale(player->velocity, -1 * player->drag));

    player->velocity = Vector2Add(
        player->velocity, Vector2Scale(force, dt * player->engine / player->mass)
    );

    player->position = vector2_wrap(
        Vector2Add(player->position, Vector2Scale(player->velocity, dt)),
        (const Vector2) { 0, 0 },
        (const Vector2) { WINDOW_WIDTH, WINDOW_HEIGHT }
    );
}


void player_update(struct Player *player, float dt)
{
    player_update_position(player, dt);
    player_update_rotation(player, dt);

    if (0 < player->reload) {
        player->reload -= dt;
        if (0 > player->reload) player->reload = 0;
    }
}


void player_draw(struct Player *p)
{
    Vector2 offset1 = Vector2Rotate((Vector2){ 12, 0 }, p->rotation);
    Vector2 offset2 = Vector2Rotate((Vector2){ -6, -6 }, p->rotation);
    Vector2 offset3 = Vector2Rotate((Vector2){ -3, 0 }, p->rotation);
    Vector2 offset4 = Vector2Rotate((Vector2){ -6, 6 }, p->rotation);

    Vector2 ver1 = Vector2Add(p->position, offset1);
    Vector2 ver2 = Vector2Add(p->position, offset2);
    Vector2 ver3 = Vector2Add(p->position, offset3);
    Vector2 ver4 = Vector2Add(p->position, offset4);

    DrawTriangle(ver1, ver2, ver3, WHITE);
    DrawTriangle(ver4, ver1, ver3, WHITE);
}


Vector2 player_barrel(struct Player *player)
{
    return (Vector2) { 
        player->position.x + 12*cos(player->rotation),
        player->position.y + 12*sin(player->rotation)
    };
}
