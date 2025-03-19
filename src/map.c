#include "../inc/maze.h"

void load_map(Game *game, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening map file: %s\n", filename);
        exit(1);
    }

    char line[1024];
    int max_width = 0;
    int height = 0;
    int **temp_map = NULL;
    int temp_capacity = 10;
    (void)temp_map;
    (void)temp_capacity;

    /* First pass: determine dimensions */
    while (fgets(line, sizeof(line), file)) {
        int width = strlen(line);
        if (line[width - 1] == '\n') width--;
        if (width > max_width) max_width = width;
        height++;
    }

    /* Allocate map array */
    game->map = malloc(height * sizeof(int *));
    for (int i = 0; i < height; i++) {
        game->map[i] = calloc(max_width, sizeof(int));
    }

    /* Second pass: load map data */
    rewind(file);
    int y = 0;
    while (fgets(line, sizeof(line), file)) {
        int x = 0;
        while (line[x] && line[x] != '\n') {
            game->map[y][x] = (line[x] == '1') ? 1 : 0;
            x++;
        }
        y++;
    }

    game->map_width = max_width;
    game->map_height = height;
    fclose(file);
}

int check_collision(Game *game, double new_x, double new_y)
{
    int map_x = (int)(new_x / TILE_SIZE);
    int map_y = (int)(new_y / TILE_SIZE);

    /* Check if new position is within map bounds */
    if (map_x < 0 || map_x >= game->map_width ||
        map_y < 0 || map_y >= game->map_height) {
        return 1;
    }

    /* Check if new position is a wall */
    if (game->map[map_y][map_x] == 1) {
        return 1;
    }

    /* Check corners for smoother collision */
    double corner_x = new_x - map_x * TILE_SIZE;
    double corner_y = new_y - map_y * TILE_SIZE;
    double radius = 10; /* Player radius */

    if (corner_x < radius && game->map[map_y][map_x - 1] == 1) return 1;
    if (corner_x > TILE_SIZE - radius && game->map[map_y][map_x + 1] == 1) return 1;
    if (corner_y < radius && game->map[map_y - 1][map_x] == 1) return 1;
    if (corner_y > TILE_SIZE - radius && game->map[map_y + 1][map_x] == 1) return 1;

    return 0;
}

void draw_minimap(Game *game)
{
    int minimap_size = 200;
    int minimap_x = SCREEN_WIDTH - minimap_size - 10;
    int minimap_y = 10;
    int tile_size = minimap_size / game->map_width;

    /* Draw map background */
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 128);
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){
        minimap_x, minimap_y, minimap_size, minimap_size
    });

    /* Draw walls */
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    for (int y = 0; y < game->map_height; y++) {
        for (int x = 0; x < game->map_width; x++) {
            if (game->map[y][x] == 1) {
                SDL_RenderFillRect(game->renderer, &(SDL_Rect){
                    minimap_x + x * tile_size,
                    minimap_y + y * tile_size,
                    tile_size, tile_size
                });
            }
        }
    }

    /* Draw player */
    SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){
        minimap_x + (int)(game->player.x / TILE_SIZE) * tile_size - 2,
        minimap_y + (int)(game->player.y / TILE_SIZE) * tile_size - 2,
        4, 4
    });

    /* Draw player direction */
    SDL_SetRenderDrawColor(game->renderer, 0, 255, 0, 255);
    SDL_RenderDrawLine(game->renderer,
        minimap_x + (int)(game->player.x / TILE_SIZE) * tile_size,
        minimap_y + (int)(game->player.y / TILE_SIZE) * tile_size,
        minimap_x + (int)(game->player.x / TILE_SIZE) * tile_size +
            cos(game->player.angle) * 20,
        minimap_y + (int)(game->player.y / TILE_SIZE) * tile_size +
            sin(game->player.angle) * 20
    );
} 
