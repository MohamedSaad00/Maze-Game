#include "../inc/maze.h"

#define MAX_ENEMIES 10
#define MAX_RAIN_DROPS 1000

typedef struct {
    double x;
    double y;
    double angle;
    int active;
} Enemy;

typedef struct {
    double x;
    double y;
    double speed;
} RainDrop;

static Enemy enemies[MAX_ENEMIES];
static RainDrop rain_drops[MAX_RAIN_DROPS];
static int enemy_count = 0;
static int rain_drop_count = 0;

void init_enemies(Game *game)
{
    /* Place enemies in the map */
    for (int y = 0; y < game->map_height; y++) {
        for (int x = 0; x < game->map_width; x++) {
            if (game->map[y][x] == 2 && enemy_count < MAX_ENEMIES) {
                enemies[enemy_count].x = x * TILE_SIZE + TILE_SIZE / 2;
                enemies[enemy_count].y = y * TILE_SIZE + TILE_SIZE / 2;
                enemies[enemy_count].angle = 0;
                enemies[enemy_count].active = 1;
                enemy_count++;
            }
        }
    }
}

void update_enemies(Game *game)
{
    for (int i = 0; i < enemy_count; i++) {
        if (!enemies[i].active) continue;

        /* Calculate angle to player */
        double dx = game->player.x - enemies[i].x;
        double dy = game->player.y - enemies[i].y;
        double angle_to_player = atan2(dy, dx);

        /* Move towards player */
        double move_speed = 2;
        enemies[i].x += cos(angle_to_player) * move_speed;
        enemies[i].y += sin(angle_to_player) * move_speed;

        /* Update enemy angle */
        enemies[i].angle = angle_to_player;
    }
}

void draw_enemies(Game *game)
{
    for (int i = 0; i < enemy_count; i++) {
        if (!enemies[i].active) continue;

        /* Calculate enemy position on screen */
        double dx = enemies[i].x - game->player.x;
        double dy = enemies[i].y - game->player.y;
        double distance = sqrt(dx * dx + dy * dy);
        double angle = atan2(dy, dx) - game->player.angle;

        /* Normalize angle */
        while (angle < -PI) angle += 2 * PI;
        while (angle > PI) angle -= 2 * PI;

        /* Check if enemy is in view */
        if (angle > -FOV/2 * PI/180 && angle < FOV/2 * PI/180) {
            int screen_x = (angle + FOV/2 * PI/180) * SCREEN_WIDTH / (FOV * PI/180);
            int height = SCREEN_HEIGHT / distance;
            int y = (SCREEN_HEIGHT - height) / 2;

            SDL_Rect src_rect = {0, 0, 64, 64};
            SDL_Rect dst_rect = {screen_x - height/2, y, height, height};
            SDL_RenderCopy(game->renderer, game->enemy_texture, &src_rect, &dst_rect);
        }
    }
}

void init_rain(Game *game)
{
	(void)game;
    for (int i = 0; i < MAX_RAIN_DROPS; i++) {
        rain_drops[i].x = rand() % SCREEN_WIDTH;
        rain_drops[i].y = rand() % SCREEN_HEIGHT;
        rain_drops[i].speed = 5 + rand() % 5;
    }
    rain_drop_count = MAX_RAIN_DROPS;
}

void update_rain(Game *game)
{
	(void)game;
    for (int i = 0; i < rain_drop_count; i++) {
        rain_drops[i].y += rain_drops[i].speed;
        if (rain_drops[i].y > SCREEN_HEIGHT) {
            rain_drops[i].y = 0;
            rain_drops[i].x = rand() % SCREEN_WIDTH;
        }
    }
}

void draw_rain(Game *game)
{
    SDL_SetRenderDrawColor(game->renderer, 200, 200, 255, 100);
    for (int i = 0; i < rain_drop_count; i++) {
        SDL_RenderDrawLine(game->renderer,
            rain_drops[i].x, rain_drops[i].y,
            rain_drops[i].x, rain_drops[i].y + 5);
    }
} 
