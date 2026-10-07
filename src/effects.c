#include "../inc/maze.h"

/* ---------------------------------------------------------------- enemies */

void add_enemy(Game *game, double x, double y)
{
    Enemy *e;

    if (game->enemy_count >= MAX_ENEMIES)
        return;
    e = &game->enemies[game->enemy_count++];
    memset(e, 0, sizeof(*e));
    e->x = x;
    e->y = y;
    e->health = ENEMY_HEALTH;
    game->enemies_left++;
}

/*
 * Enemies drift towards a nearby player and slide along walls, as the player
 * does. Once close enough they touch: one life lost, then a moment of safety.
 */
static void chase(Game *game, Enemy *e, double dt)
{
    double dx = game->player.x - e->x;
    double dy = game->player.y - e->y;
    double distance = sqrt(dx * dx + dy * dy);
    double step;

    if (distance > ENEMY_SIGHT)
        return;
    if (distance < ENEMY_REACH + 0.1 && e->attack_timer <= 0 && game->hurt_timer <= 0) {
        game->player_health--;
        game->hurt_timer = HURT_TIME;
        e->attack_timer = ENEMY_ATTACK_COOLDOWN;
    }
    if (distance < ENEMY_REACH)
        return;
    step = ENEMY_SPEED * dt / distance;
    if (!check_collision(game, e->x + dx * step, e->y))
        e->x += dx * step;
    if (!check_collision(game, e->x, e->y + dy * step))
        e->y += dy * step;
}

void update_enemies(Game *game, double dt)
{
    Enemy *e;
    int i = 0;

    while (i < game->enemy_count) {
        e = &game->enemies[i];
        if (e->hit_timer > 0)
            e->hit_timer -= dt;
        if (e->attack_timer > 0)
            e->attack_timer -= dt;
        if (e->health > 0) {
            chase(game, e, dt);
        } else if ((e->dying -= dt) <= 0) {
            /* Gone: the last enemy takes its slot */
            game->enemies[i] = game->enemies[--game->enemy_count];
            continue;
        }
        i++;
    }
}

/*
 * Camera-space position of an enemy: its screen column and on-screen size.
 * Returns 0 when it is behind the camera.
 */
static int project(Game *game, Enemy *e, double *depth, int *screen_x, int *size)
{
    Player *p = &game->player;
    double rel_x = e->x - p->x, rel_y = e->y - p->y;
    double inv = 1.0 / (p->plane_x * p->dir_y - p->dir_x * p->plane_y);
    double cam_x = inv * (p->dir_y * rel_x - p->dir_x * rel_y);

    *depth = inv * (-p->plane_y * rel_x + p->plane_x * rel_y);
    if (*depth < 0.1)
        return 0;
    *screen_x = (int)((SCREEN_WIDTH / 2) * (1 + cam_x / *depth));
    *size = abs((int)(SCREEN_HEIGHT / *depth));
    if (e->health <= 0)
        *size = (int)(*size * (e->dying / DEATH_TIME)); /* shrinks away when killed */
    return 1;
}

/* Blends a colour towards red: the flash of a hit */
static Uint32 redden(Uint32 c)
{
    Uint32 r = ((c >> 16) & 0xFF) / 2 + 128;
    Uint32 g = ((c >> 8) & 0xFF) / 3;
    Uint32 b = (c & 0xFF) / 3;

    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

static int farther_first(const void *a, const void *b)
{
    double da = ((const Enemy *)a)->distance;
    double db = ((const Enemy *)b)->distance;

    return (da < db) - (da > db);
}

/* Draws one sprite, only in the columns not hidden behind a wall */
static void draw_sprite(Game *game, Enemy *e)
{
    Texture *tex = &game->textures[TEX_ENEMY];
    int red = e->hit_timer > 0 || e->health <= 0;
    double depth;
    int screen_x, size, left, top, x, y, tx, ty;
    Uint32 c;

    if (!project(game, e, &depth, &screen_x, &size) || size <= 0)
        return;
    left = screen_x - size / 2;
    top = SCREEN_HEIGHT / 2 - size / 2;
    for (x = left < 0 ? 0 : left; x < left + size && x < SCREEN_WIDTH; x++) {
        if (depth >= game->z_buffer[x])
            continue;
        tx = (x - left) * TEX_SIZE / size;
        for (y = top < 0 ? 0 : top; y < top + size && y < SCREEN_HEIGHT; y++) {
            ty = (y - top) * TEX_SIZE / size;
            c = tex->pixels[ty * TEX_SIZE + tx];
            if (c != CLEAR_PIXEL)
                game->frame[y * SCREEN_WIDTH + x] = shade(red ? redden(c) : c, depth);
        }
    }
}

void draw_enemies(Game *game)
{
    Enemy sorted[MAX_ENEMIES];
    double dx, dy;
    int i;

    for (i = 0; i < game->enemy_count; i++) {
        sorted[i] = game->enemies[i];
        dx = sorted[i].x - game->player.x;
        dy = sorted[i].y - game->player.y;
        sorted[i].distance = dx * dx + dy * dy;
    }
    /* Far to near, so nearer enemies overlap farther ones */
    qsort(sorted, game->enemy_count, sizeof(Enemy), farther_first);
    for (i = 0; i < game->enemy_count; i++)
        draw_sprite(game, &sorted[i]);
}

/* ----------------------------------------------------------------- weapon */

/*
 * Hitscan down the centre column: the nearest living enemy whose body covers
 * the crosshair and stands in front of the wall there takes the shot.
 */
void fire_weapon(Game *game)
{
    Enemy *target = NULL;
    double depth, nearest = game->z_buffer[SCREEN_WIDTH / 2];
    int screen_x, size, i;

    if (game->fire_cooldown > 0 || game->state != STATE_PLAYING)
        return;
    game->fire_cooldown = FIRE_COOLDOWN;
    game->flash_timer = FLASH_TIME;

    for (i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].health <= 0)
            continue;
        if (!project(game, &game->enemies[i], &depth, &screen_x, &size))
            continue;
        /* The ghost's body is about two thirds of its sprite's width */
        if (abs(screen_x - SCREEN_WIDTH / 2) < size / 3 && depth < nearest) {
            nearest = depth;
            target = &game->enemies[i];
        }
    }
    if (!target)
        return;
    target->hit_timer = HIT_TIME;
    if (--target->health == 0) {
        target->dying = DEATH_TIME;
        game->enemies_left--;
        update_title(game);
    }
}

void update_weapon(Game *game, double dt)
{
    if (game->fire_cooldown > 0)
        game->fire_cooldown -= dt;
    if (game->flash_timer > 0)
        game->flash_timer -= dt;
}

/* A round burst of fire, hot white at the centre fading to orange */
static void draw_flash(Game *game, int cx, int cy, int radius)
{
    int x, y;
    double d;

    for (y = cy - radius; y <= cy + radius; y++)
        for (x = cx - radius; x <= cx + radius; x++) {
            if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
                continue;
            d = sqrt((double)(x - cx) * (x - cx) + (double)(y - cy) * (y - cy)) / radius;
            if (d > 1.0)
                continue;
            game->frame[y * SCREEN_WIDTH + x] = 0xFF000000u | (255u << 16) |
                ((Uint32)(255 - d * 120) << 8) | (Uint32)(220 * (1 - d) * (1 - d));
        }
}

void draw_weapon(Game *game)
{
    Texture *tex = &game->textures[TEX_WEAPON];
    int scale = 4;
    int firing = game->flash_timer > 0;
    int bob_x = (int)(sin(game->walk_time * 7.0) * 10);
    int bob_y = (int)(fabs(cos(game->walk_time * 7.0)) * 8) + (firing ? 14 : 0); /* recoil */
    int left = SCREEN_WIDTH / 2 - TEX_SIZE * scale / 2 + bob_x;
    int top = SCREEN_HEIGHT - TEX_SIZE * scale + 12 + bob_y;
    int x, y, sx, sy;
    Uint32 c;

    if (firing)
        draw_flash(game, left + TEX_SIZE * scale / 2, top + 10 * scale, 30);
    for (y = 0; y < TEX_SIZE; y++)
        for (x = 0; x < TEX_SIZE; x++) {
            c = tex->pixels[y * TEX_SIZE + x];
            if (c == CLEAR_PIXEL)
                continue;
            for (sy = top + y * scale; sy < top + (y + 1) * scale; sy++)
                for (sx = left + x * scale; sx < left + (x + 1) * scale; sx++)
                    if (sx >= 0 && sx < SCREEN_WIDTH && sy >= 0 && sy < SCREEN_HEIGHT)
                        game->frame[sy * SCREEN_WIDTH + sx] = c;
        }
}

void draw_crosshair(Game *game)
{
    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;

    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 190);
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){cx - 9, cy - 1, 6, 2});
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){cx + 3, cy - 1, 6, 2});
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){cx - 1, cy - 9, 2, 6});
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){cx - 1, cy + 3, 2, 6});
}

void update_title(Game *game)
{
    char title[64];

    if (!game->window)
        return;
    if (game->state == STATE_LOST)
        snprintf(title, sizeof(title), "Maze Game - game over");
    else if (game->enemies_left > 0)
        snprintf(title, sizeof(title), "Maze Game - %d %s left", game->enemies_left,
                 game->enemies_left == 1 ? "ghost" : "ghosts");
    else
        snprintf(title, sizeof(title), "Maze Game - all ghosts cleared!");
    SDL_SetWindowTitle(game->window, title);
}

/* ------------------------------------------------------------------- rain */

static void reset_drop(RainDrop *d, int anywhere)
{
    d->x = (float)(rand() % SCREEN_WIDTH);
    d->y = anywhere ? (float)(rand() % SCREEN_HEIGHT) : -(float)(rand() % 40);
    d->speed = 500.0f + (float)(rand() % 400);  /* pixels per second */
    d->length = 6.0f + (float)(rand() % 10);
}

void init_rain(Game *game)
{
    int i;

    for (i = 0; i < MAX_RAIN_DROPS; i++)
        reset_drop(&game->rain[i], 1);
}

void update_rain(Game *game, double dt)
{
    RainDrop *d;
    int i;

    for (i = 0; i < MAX_RAIN_DROPS; i++) {
        d = &game->rain[i];
        d->y += d->speed * (float)dt;
        d->x -= d->speed * 0.08f * (float)dt; /* a little wind */
        if (d->y > SCREEN_HEIGHT || d->x < 0)
            reset_drop(d, 0);
    }
}

void draw_rain(Game *game)
{
    RainDrop *d;
    int i;

    /* A cold tint over the scene, then the streaks */
    SDL_SetRenderDrawColor(game->renderer, 20, 30, 60, 60);
    SDL_RenderFillRect(game->renderer, NULL);
    SDL_SetRenderDrawColor(game->renderer, 190, 200, 255, 140);
    for (i = 0; i < MAX_RAIN_DROPS; i++) {
        d = &game->rain[i];
        SDL_RenderDrawLine(game->renderer, (int)d->x, (int)d->y,
            (int)(d->x - d->length * 0.08f), (int)(d->y + d->length));
    }
}
