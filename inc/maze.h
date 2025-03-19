#ifndef MAZE_H
#define MAZE_H

#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FOV 60
#define RAY_COUNT 800
#define MAX_DEPTH 16
#define TILE_SIZE 64
#define PLAYER_SPEED 5
#define ROTATION_SPEED 3
#define PI 3.14159265359

typedef struct {
    double x;
    double y;
    double angle;
} Player;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *wall_texture;
    SDL_Texture *floor_texture;
    SDL_Texture *ceiling_texture;
    SDL_Texture *weapon_texture;
    SDL_Texture *enemy_texture;
    int **map;
    int map_width;
    int map_height;
    Player player;
    int show_map;
    int rain_active;
    int game_running;
} Game;

/* Core game functions */
void init_game(Game *game);
void handle_input(Game *game);
void update_game(Game *game);
void render_game(Game *game);
void cleanup_game(Game *game);

/* Raycasting functions */
void cast_rays(Game *game);
void draw_wall(Game *game, int ray_index, double distance, int wall_orientation);

/* Map functions */
void load_map(Game *game, const char *filename);
void draw_minimap(Game *game);

/* Texture functions */
void load_textures(Game *game);
void apply_texture(Game *game, int ray_index, double distance, int wall_orientation);

/* Collision functions */
int check_collision(Game *game, double new_x, double new_y);

/* Enemy functions */
void update_enemies(Game *game);
void draw_enemies(Game *game);

/* Weather functions */
void update_rain(Game *game);
void draw_rain(Game *game);

#endif /* MAZE_H */ 
