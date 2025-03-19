#include "../inc/maze.h"

void load_textures(Game *game)
{
    SDL_Surface *surface;

    /* Load wall texture */
    surface = SDL_LoadBMP("textures/wall.bmp");
    if (!surface) {
        printf("Error loading wall texture: %s\n", SDL_GetError());
        exit(1);
    }
    game->wall_texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    /* Load floor texture */
    surface = SDL_LoadBMP("textures/floor.bmp");
    if (!surface) {
        printf("Error loading floor texture: %s\n", SDL_GetError());
        exit(1);
    }
    game->floor_texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    /* Load ceiling texture */
    surface = SDL_LoadBMP("textures/ceiling.bmp");
    if (!surface) {
        printf("Error loading ceiling texture: %s\n", SDL_GetError());
        exit(1);
    }
    game->ceiling_texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    /* Load weapon texture */
    surface = SDL_LoadBMP("textures/weapon.bmp");
    if (!surface) {
        printf("Error loading weapon texture: %s\n", SDL_GetError());
        exit(1);
    }
    game->weapon_texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    /* Load enemy texture */
    surface = SDL_LoadBMP("textures/enemy.bmp");
    if (!surface) {
        printf("Error loading enemy texture: %s\n", SDL_GetError());
        exit(1);
    }
    game->enemy_texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);
}

void apply_texture(Game *game, int ray_index, double distance, int wall_orientation)
{
    int wall_height;
    int wall_top;
    int wall_bottom;
    SDL_Rect src_rect;
    SDL_Rect dst_rect;
    int texture_x;
    int texture_width = 64;
    int texture_height = 64;

    if (distance < 1) distance = 1;
    wall_height = (int)(SCREEN_HEIGHT / distance);
    wall_top = (SCREEN_HEIGHT - wall_height) / 2;
    wall_bottom = wall_top + wall_height;
    int dummy_wall = wall_bottom;
    (void)dummy_wall;

    /* Calculate texture coordinates */
    if (wall_orientation == 0 || wall_orientation == 1) {
        texture_x = (int)((game->player.y + distance * sin(game->player.angle)) * texture_width / TILE_SIZE) % texture_width;
    } else {
        texture_x = (int)((game->player.x + distance * cos(game->player.angle)) * texture_width / TILE_SIZE) % texture_width;
    }

    src_rect.x = texture_x;
    src_rect.y = 0;
    src_rect.w = 1;
    src_rect.h = texture_height;

    dst_rect.x = ray_index;
    dst_rect.y = wall_top;
    dst_rect.w = 1;
    dst_rect.h = wall_height;

    /* Draw textured wall */
    SDL_RenderCopy(game->renderer, game->wall_texture, &src_rect, &dst_rect);
} 
