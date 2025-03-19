#include "../inc/maze.h"

void cast_rays(Game *game)
{
    double ray_angle;
    double distance;
    int wall_orientation;
    int map_x, map_y;
    double step_x, step_y;
    double side_dist_x, side_dist_y;
    double delta_dist_x, delta_dist_y;

    for (int i = 0; i < RAY_COUNT; i++) {
        ray_angle = game->player.angle - (FOV / 2.0) * (PI / 180.0) +
                   (FOV * (double)i / RAY_COUNT) * (PI / 180.0);

        map_x = (int)(game->player.x / TILE_SIZE);
        map_y = (int)(game->player.y / TILE_SIZE);

        delta_dist_x = fabs(1 / cos(ray_angle));
        delta_dist_y = fabs(1 / sin(ray_angle));

        if (cos(ray_angle) < 0) {
            step_x = -1;
            side_dist_x = (game->player.x - map_x * TILE_SIZE) * delta_dist_x;
        } else {
            step_x = 1;
            side_dist_x = ((map_x + 1) * TILE_SIZE - game->player.x) * delta_dist_x;
        }

        if (sin(ray_angle) < 0) {
            step_y = -1;
            side_dist_y = (game->player.y - map_y * TILE_SIZE) * delta_dist_y;
        } else {
            step_y = 1;
            side_dist_y = ((map_y + 1) * TILE_SIZE - game->player.y) * delta_dist_y;
        }

        while (1) {
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                map_x += step_x;
                wall_orientation = (step_x < 0) ? 0 : 1; /* WEST or EAST */
            } else {
                side_dist_y += delta_dist_y;
                map_y += step_y;
                wall_orientation = (step_y < 0) ? 2 : 3; /* NORTH or SOUTH */
            }

            if (map_x < 0 || map_x >= game->map_width ||
                map_y < 0 || map_y >= game->map_height ||
                game->map[map_y][map_x] == 1) {
                break;
            }
        }

        if (wall_orientation == 0 || wall_orientation == 1) {
            distance = (map_x - game->player.x / TILE_SIZE +
                       (1 - step_x) / 2) / cos(ray_angle);
        } else {
            distance = (map_y - game->player.y / TILE_SIZE +
                       (1 - step_y) / 2) / sin(ray_angle);
        }

        distance = fabs(distance * TILE_SIZE);
        draw_wall(game, i, distance, wall_orientation);
    }
}

void draw_wall(Game *game, int ray_index, double distance, int wall_orientation)
{
    int wall_height;
    int wall_top;
    int wall_bottom;
    SDL_Rect wall_rect;

    if (distance < 1) distance = 1;
    wall_height = (int)(SCREEN_HEIGHT / distance);
    wall_top = (SCREEN_HEIGHT - wall_height) / 2;
    wall_bottom = wall_top + wall_height;

    wall_rect.x = ray_index;
    wall_rect.y = wall_top;
    wall_rect.w = 1;
    wall_rect.h = wall_height;

int dummy = wall_bottom; // Prevents unused variable warning
(void)dummy;  // Explicitly tell the compiler this variable is used


    switch (wall_orientation) {
        case 0: /* WEST */
            SDL_SetRenderDrawColor(game->renderer, 200, 0, 0, 255);
            break;
        case 1: /* EAST */
            SDL_SetRenderDrawColor(game->renderer, 0, 200, 0, 255);
            break;
        case 2: /* NORTH */
            SDL_SetRenderDrawColor(game->renderer, 0, 0, 200, 255);
            break;
        case 3: /* SOUTH */
            SDL_SetRenderDrawColor(game->renderer, 200, 200, 0, 255);
            break;
    }

    SDL_RenderFillRect(game->renderer, &wall_rect);
} 
