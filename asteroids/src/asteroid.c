
static const size_t ASTEROID_NUM_CORNERS[NUM_ASTEROID_LEVELS] = {
    3, 4, 6
};

static const Color ASTEROID_COLOUR[NUM_ASTEROID_LEVELS] = {
    { 255, 255, 255, 255 },
    { 255, 255, 255, 255 },
    { 255, 255, 255, 255 }
};

static const float ASTEROID_RADIUS[NUM_ASTEROID_LEVELS] = {
    12.0f, 18.0f, 24.0f
};


void asteroid_clear(struct Asteroid *ast)
{
    *ast = (struct Asteroid){ 0 };
}


size_t asteroid_num_corners(struct Asteroid *ast)
{
    return ASTEROID_NUM_CORNERS[ast->level];
}


float asteroid_radius(struct Asteroid *ast)
{
    return ASTEROID_RADIUS[ast->level];
}


Vector2 asteroid_vertex(struct Asteroid *ast, size_t i)
{
    return Vector2Add(
        ast->position,
        Vector2Rotate(ast->corners[i % asteroid_num_corners(ast)], ast->rotation)
    );
}


Color asteroid_colour(struct Asteroid *ast)
{
    return ASTEROID_COLOUR[ast->level];
}


bool asteroid_is_alive(struct Asteroid *ast)
{
    return (0 < ast->hitpoints);
}


void asteroid_randomise(struct Asteroid *ast)
{
    if (!ast) return;

    asteroid_clear(ast);
    ast->level = random() % NUM_ASTEROID_LEVELS;

    size_t num_corners = ASTEROID_NUM_CORNERS[ast->level];
    Vector2 corners[num_corners];
    Vector2 centroid = { 0 };

    float radius = ASTEROID_RADIUS[ast->level];
    float angle_step = (2 * PI) / num_corners;
    float angle_delta = 0;
    for (size_t i = 0; i < num_corners; i++) {
        angle_delta = (random()%(2*num_corners)) * angle_step / (2.0f*num_corners);
        corners[i] = (Vector2) {
            radius * cos(i*angle_step + angle_delta),
            radius * sin(i*angle_step + angle_delta)
        };
        centroid = Vector2Add(centroid, corners[i]);
    }

    centroid = Vector2Scale(centroid, 1.0f/(1.0f * (float)num_corners));
    for (size_t i = 0; i < num_corners; i++) {
        ast->corners[i] = Vector2Subtract(corners[i], centroid);
    }

    ast->hitpoints = (ast->level + (random() % ast->level)) * (ast->level + 1);

    ast->position = (Vector2) { random() % WINDOW_WIDTH, random() % WINDOW_HEIGHT };
    ast->velocity = (Vector2) {
        3*((random()%12) - (random()%6)),
        3*((random()%12) - (random()%6))
    };

    ast->rotation = (random() % 360) * (2 * PI) / 360;
    ast->spin = 1.0f * (random() % 360) * (2 * PI) / (ast->hitpoints);
}


void asteroid_draw(struct Asteroid *ast)
{
    if (!ast || !asteroid_alive(ast)) return;

    size_t num_corners = asteroid_num_corners(ast);
    Vector2 vertex1 = { 0, 0 }, vertex2 = asteroid_vertex(ast, 0);

    for (size_t i = 0; i < num_corners; i++) {
        vertex1 = vertex2;
        vertex2 = asteroid_vertex(ast, (i + 1) % num_corners);
        DrawLineV(vertex1, vertex2, asteroid_colour(ast));
    }
}


void asteroid_do_kinematics(struct Asteroid *ast, float dt)
{
    if (!ast) return;

    size_t num_corners = asteroid_num_corners(ast);

    float delta_angle = ast->spin*dt;
    ast->rotation += delta_angle;
    for (size_t i = 0; i < num_corners; i++) {
        ast->corners[i] = Vector2Rotate(ast->corners[i], delta_angle);
    }

    ast->position = vector2_wrap(
        Vector2Add(ast->position, Vector2Scale(ast->velocity, dt)),
        BOUNDS_ASTEROID
    );
}


static size_t num_asteroids = 0;
static struct Asteroid asteroids[NUM_ASTEROIDS_MAX] = { 0 };

void asteroids_update(float dt)
{
    for (size_t i = 0; i < num_asteroids; i++) {
        while (num_asteroids && !asteroid_is_alive(asteroids + i)) {
            asteroids[i] = asteroids[--num_asteroids];
        }
        asteroid_do_kinematics(asteroids + i, dt);
    }
}
