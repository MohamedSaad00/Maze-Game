#include "../inc/maze.h"

#define MAX_LINE 1024

/* Counts rows and the widest row, ignoring trailing newlines */
static int measure_map(FILE *file, int *width, int *height)
{
    char line[MAX_LINE];
    int len;

    *width = 0;
    *height = 0;
    while (fgets(line, sizeof(line), file)) {
        len = (int)strcspn(line, "\r\n");
        if (len == 0)
            continue;
        if (len > *width)
            *width = len;
        (*height)++;
    }
    rewind(file);
    return (*width > 0 && *height > 0) ? 0 : -1;
}

static int alloc_map(Game *game, int width, int height)
{
    int y;

    game->map = calloc(height, sizeof(int *));
    if (!game->map)
        return -1;
    game->map_width = width;
    game->map_height = height;
    for (y = 0; y < height; y++) {
        game->map[y] = calloc(width, sizeof(int));
        if (!game->map[y])
            return -1;
    }
    return 0;
}

static void place_player(Game *game, int x, int y)
{
    game->player.x = x + 0.5;
    game->player.y = y + 0.5;
    game->player.dir_x = 1.0;
    game->player.dir_y = 0.0;
    game->player.plane_x = 0.0;
    game->player.plane_y = CAMERA_PLANE;
}

/*
 * '1' wall, '0' or ' ' floor, '2' enemy, 'P' player start (facing east).
 * Short rows are padded with walls and the border is always solid, so a
 * ray or a player can never leave the map.
 */
static int parse_cell(Game *game, char c, int x, int y, int *has_player)
{
    int border = x == 0 || y == 0 || x == game->map_width - 1 || y == game->map_height - 1;

    if (c == '1' || border) {
        game->map[y][x] = 1;
        return 0;
    }
    if (c == '2')
        add_enemy(game, x + 0.5, y + 0.5);
    else if (c == 'P' || c == 'p') {
        place_player(game, x, y);
        *has_player = 1;
    } else if (c != '0' && c != ' ') {
        printf("Map error: unknown character '%c' at row %d, column %d\n", c, y + 1, x + 1);
        return -1;
    }
    return 0;
}

static int find_start(Game *game)
{
    int x;
    int y;

    for (y = 0; y < game->map_height; y++)
        for (x = 0; x < game->map_width; x++)
            if (!game->map[y][x]) {
                place_player(game, x, y);
                return 0;
            }
    return -1;
}

int load_map(Game *game, const char *filename)
{
    FILE *file = fopen(filename, "r");
    char line[MAX_LINE];
    int width, height, x, y = 0, len, has_player = 0, status = 0;

    if (!file) {
        printf("Error opening map file: %s\n", filename);
        return -1;
    }
    if (measure_map(file, &width, &height) != 0 || width < 3 || height < 3) {
        printf("Map error: %s is empty or smaller than 3x3\n", filename);
        fclose(file);
        return -1;
    }
    if (alloc_map(game, width, height) != 0) {
        printf("Out of memory loading %s\n", filename);
        fclose(file);
        return -1;
    }
    while (status == 0 && fgets(line, sizeof(line), file)) {
        len = (int)strcspn(line, "\r\n");
        if (len == 0)
            continue;
        for (x = 0; x < width && status == 0; x++)
            status = parse_cell(game, x < len ? line[x] : '1', x, y, &has_player);
        y++;
    }
    fclose(file);
    if (status == 0 && !has_player && find_start(game) != 0) {
        printf("Map error: no open cell to start in\n");
        status = -1;
    }
    if (status != 0)
        free_map(game);
    return status;
}

void free_map(Game *game)
{
    int y;

    if (!game->map)
        return;
    for (y = 0; y < game->map_height; y++)
        free(game->map[y]);
    free(game->map);
    game->map = NULL;
}

int is_wall(Game *game, int x, int y)
{
    if (x < 0 || y < 0 || x >= game->map_width || y >= game->map_height)
        return 1;
    return game->map[y][x] == 1;
}

/* A position collides if any corner of the player's square is inside a wall */
int check_collision(Game *game, double x, double y)
{
    double r = PLAYER_RADIUS;

    return is_wall(game, (int)floor(x - r), (int)floor(y - r)) ||
           is_wall(game, (int)floor(x + r), (int)floor(y - r)) ||
           is_wall(game, (int)floor(x - r), (int)floor(y + r)) ||
           is_wall(game, (int)floor(x + r), (int)floor(y + r));
}

void draw_minimap(Game *game)
{
    int side = game->map_width > game->map_height ? game->map_width : game->map_height;
    int tile = MINIMAP_SIZE / side > 1 ? MINIMAP_SIZE / side : 2;
    int left = SCREEN_WIDTH - game->map_width * tile - 10;
    int top = 10;
    double px = left + game->player.x * tile;
    double py = top + game->player.y * tile;
    int x, y, i;

    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){
        left - 4, top - 4, game->map_width * tile + 8, game->map_height * tile + 8});

    SDL_SetRenderDrawColor(game->renderer, 230, 230, 230, 200);
    for (y = 0; y < game->map_height; y++)
        for (x = 0; x < game->map_width; x++)
            if (game->map[y][x] == 1)
                SDL_RenderFillRect(game->renderer,
                    &(SDL_Rect){left + x * tile, top + y * tile, tile, tile});

    SDL_SetRenderDrawColor(game->renderer, 255, 200, 0, 255);
    for (i = 0; i < game->enemy_count; i++)
        if (game->enemies[i].health > 0)
            SDL_RenderFillRect(game->renderer, &(SDL_Rect){
            left + (int)(game->enemies[i].x * tile) - 2,
            top + (int)(game->enemies[i].y * tile) - 2, 4, 4});

    SDL_SetRenderDrawColor(game->renderer, 255, 40, 40, 255);
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){(int)px - 2, (int)py - 2, 5, 5});
    SDL_SetRenderDrawColor(game->renderer, 60, 255, 60, 255);
    SDL_RenderDrawLine(game->renderer, (int)px, (int)py,
        (int)(px + game->player.dir_x * tile * 1.5),
        (int)(py + game->player.dir_y * tile * 1.5));
}
