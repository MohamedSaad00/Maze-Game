#include "../inc/maze.h"

/* Darkens a colour with distance, so far corridors fade into shadow */
Uint32 shade(Uint32 color, double distance)
{
    double f = 1.0 / (1.0 + distance * distance * 0.04);
    Uint32 r = (Uint32)(((color >> 16) & 0xFF) * f);
    Uint32 g = (Uint32)(((color >> 8) & 0xFF) * f);
    Uint32 b = (Uint32)((color & 0xFF) * f);

    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

/*
 * Floor casting: for each row below the horizon, walk across the floor from
 * the leftmost to the rightmost ray and sample the texture. The ceiling is the
 * same row mirrored.
 */
void draw_floor_ceiling(Game *game)
{
    Player *p = &game->player;
    Texture *floor_tex = &game->textures[TEX_FLOOR];
    Texture *ceil_tex = &game->textures[TEX_CEILING];
    double ray_x0 = p->dir_x - p->plane_x, ray_y0 = p->dir_y - p->plane_y;
    double ray_x1 = p->dir_x + p->plane_x, ray_y1 = p->dir_y + p->plane_y;
    double row_distance, step_x, step_y, fx, fy;
    int x, y, tx, ty, cell_x, cell_y;

    /* Sampling each row at its centre (+0.5) keeps the horizon row finite */
    for (y = SCREEN_HEIGHT / 2; y < SCREEN_HEIGHT; y++) {
        row_distance = (0.5 * SCREEN_HEIGHT) / (y - SCREEN_HEIGHT / 2.0 + 0.5);
        step_x = row_distance * (ray_x1 - ray_x0) / SCREEN_WIDTH;
        step_y = row_distance * (ray_y1 - ray_y0) / SCREEN_WIDTH;
        fx = p->x + row_distance * ray_x0;
        fy = p->y + row_distance * ray_y0;
        for (x = 0; x < SCREEN_WIDTH; x++) {
            cell_x = (int)floor(fx);
            cell_y = (int)floor(fy);
            tx = (int)(TEX_SIZE * (fx - cell_x)) & (TEX_SIZE - 1);
            ty = (int)(TEX_SIZE * (fy - cell_y)) & (TEX_SIZE - 1);
            fx += step_x;
            fy += step_y;
            game->frame[y * SCREEN_WIDTH + x] =
                shade(floor_tex->pixels[ty * TEX_SIZE + tx], row_distance);
            game->frame[(SCREEN_HEIGHT - y - 1) * SCREEN_WIDTH + x] =
                shade(ceil_tex->pixels[ty * TEX_SIZE + tx], row_distance);
        }
    }
}

/* DDA: step from grid line to grid line until a wall is hit. Returns the side */
static int march(Game *game, double ray_x, double ray_y, int *map_x, int *map_y,
                 double *distance)
{
    Player *p = &game->player;
    double delta_x = ray_x == 0 ? 1e30 : fabs(1 / ray_x);
    double delta_y = ray_y == 0 ? 1e30 : fabs(1 / ray_y);
    int step_x = ray_x < 0 ? -1 : 1;
    int step_y = ray_y < 0 ? -1 : 1;
    double side_x = ray_x < 0 ? (p->x - *map_x) * delta_x : (*map_x + 1.0 - p->x) * delta_x;
    double side_y = ray_y < 0 ? (p->y - *map_y) * delta_y : (*map_y + 1.0 - p->y) * delta_y;
    int side = 0;

    while (!is_wall(game, *map_x, *map_y)) {
        if (side_x < side_y) {
            side_x += delta_x;
            *map_x += step_x;
            side = 0;
        } else {
            side_y += delta_y;
            *map_y += step_y;
            side = 1;
        }
    }
    /* Distance to the camera plane, not to the player: no fish-eye */
    *distance = side == 0 ? side_x - delta_x : side_y - delta_y;
    return side;
}

static void draw_column(Game *game, int x, double distance, int tex_x, int side)
{
    Texture *wall = &game->textures[TEX_WALL];
    int height = (int)(SCREEN_HEIGHT / distance) > 0 ? (int)(SCREEN_HEIGHT / distance) : 1;
    int start = -height / 2 + SCREEN_HEIGHT / 2;
    int end = height / 2 + SCREEN_HEIGHT / 2;
    double step = (double)TEX_SIZE / height;
    double tex_pos;
    Uint32 color;
    int y;

    if (start < 0)
        start = 0;
    if (end >= SCREEN_HEIGHT)
        end = SCREEN_HEIGHT - 1;
    tex_pos = (start - SCREEN_HEIGHT / 2 + height / 2) * step;
    for (y = start; y <= end; y++) {
        color = wall->pixels[((int)tex_pos & (TEX_SIZE - 1)) * TEX_SIZE + tex_x];
        tex_pos += step;
        /* North/south faces a shade darker, so corners read clearly */
        if (side == 1)
            color = (color >> 1) & 0xFF7F7F7Fu;
        game->frame[y * SCREEN_WIDTH + x] = shade(color, distance);
    }
}

void cast_rays(Game *game)
{
    Player *p = &game->player;
    double camera_x, ray_x, ray_y, distance, wall_x;
    int x, map_x, map_y, side, tex_x;

    for (x = 0; x < SCREEN_WIDTH; x++) {
        camera_x = 2.0 * x / SCREEN_WIDTH - 1;
        ray_x = p->dir_x + p->plane_x * camera_x;
        ray_y = p->dir_y + p->plane_y * camera_x;
        map_x = (int)p->x;
        map_y = (int)p->y;

        side = march(game, ray_x, ray_y, &map_x, &map_y, &distance);
        if (distance < 0.05)
            distance = 0.05;
        game->z_buffer[x] = distance;

        /* Where along the wall the ray landed picks the texture column */
        wall_x = side == 0 ? p->y + distance * ray_y : p->x + distance * ray_x;
        wall_x -= floor(wall_x);
        tex_x = (int)(wall_x * TEX_SIZE);
        if ((side == 0 && ray_x > 0) || (side == 1 && ray_y < 0))
            tex_x = TEX_SIZE - tex_x - 1;

        draw_column(game, x, distance, tex_x, side);
    }
}
