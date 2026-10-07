#ifndef MAZE_H
#define MAZE_H

#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TEX_SIZE 64
#define MOVE_SPEED 3.0       /* tiles per second */
#define ROTATION_SPEED 2.5   /* radians per second */
#define PLAYER_RADIUS 0.2    /* tiles */
#define CAMERA_PLANE 0.66    /* half the screen width in camera space: ~66 deg FOV */
#define MAX_ENEMIES 32
#define ENEMY_SPEED 1.2      /* tiles per second */
#define ENEMY_SIGHT 8.0      /* enemies chase the player within this many tiles */
#define ENEMY_REACH 0.7      /* and stop this close */
#define ENEMY_HEALTH 3       /* hits to kill */
#define HIT_TIME 0.15        /* seconds an enemy flashes red when hit */
#define DEATH_TIME 0.45      /* seconds an enemy takes to vanish */
#define FIRE_COOLDOWN 0.3    /* seconds between shots */
#define FLASH_TIME 0.08      /* seconds the muzzle flash shows */
#define PLAYER_HEALTH 3      /* touches from a ghost before game over */
#define HURT_TIME 1.0        /* seconds of red flash, and of safety, after a touch */
#define ENEMY_ATTACK_COOLDOWN 1.5 /* seconds before the same ghost can touch again */
#define MAX_RAIN_DROPS 600
#define MINIMAP_SIZE 180
#define CLEAR_PIXEL 0x00000000u

/* Textures are plain ARGB pixel arrays: the frame is drawn pixel by pixel */
enum {
    TEX_WALL,
    TEX_FLOOR,
    TEX_CEILING,
    TEX_ENEMY,
    TEX_WEAPON,
    TEX_COUNT
};

enum {
    STATE_PLAYING,
    STATE_WON,
    STATE_LOST
};

/* The two buttons on the end screen */
enum {
    END_RESTART = 1,
    END_CLOSE
};

typedef struct {
    Uint32 pixels[TEX_SIZE * TEX_SIZE];
} Texture;

/* Position in tiles, a unit direction vector and the camera plane */
typedef struct {
    double x;
    double y;
    double dir_x;
    double dir_y;
    double plane_x;
    double plane_y;
} Player;

typedef struct {
    double x;
    double y;
    double distance;  /* to the player, refreshed each frame for sorting */
    int health;       /* 0 once killed: it then fades out over DEATH_TIME */
    double hit_timer;
    double dying;
    double attack_timer;
} Enemy;

typedef struct {
    float x;
    float y;
    float speed;
    float length;
} RainDrop;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *screen;              /* streaming texture the frame lands in */
    Uint32 *frame;                    /* SCREEN_WIDTH * SCREEN_HEIGHT pixels */
    double z_buffer[SCREEN_WIDTH];    /* wall distance per column, for sprites */
    Texture textures[TEX_COUNT];
    int **map;
    int map_width;
    int map_height;
    Player player;
    Enemy enemies[MAX_ENEMIES];
    int enemy_count;
    RainDrop rain[MAX_RAIN_DROPS];
    double walk_time;                 /* drives the weapon bob */
    double fire_cooldown;
    double flash_timer;               /* muzzle flash and recoil */
    int enemies_left;                 /* shown in the window title */
    int player_health;
    double hurt_timer;
    int state;                        /* STATE_PLAYING, STATE_WON or STATE_LOST */
    const char *map_file;             /* reloaded on restart */
    int show_map;
    int rain_active;
    int game_running;
} Game;

/* Core game functions */
int init_game(Game *game);
void handle_input(Game *game, double dt);
void update_game(Game *game, double dt);
void render_game(Game *game);
int save_screenshot(Game *game, const char *filename);
int restart_game(Game *game);
void cleanup_game(Game *game);

/* Raycasting functions */
void draw_floor_ceiling(Game *game);
void cast_rays(Game *game);
Uint32 shade(Uint32 color, double distance);

/* Map functions */
int load_map(Game *game, const char *filename);
void free_map(Game *game);
int is_wall(Game *game, int x, int y);
void draw_minimap(Game *game);

/* Texture functions */
void load_textures(Game *game);

/* Collision functions */
int check_collision(Game *game, double x, double y);

/* Enemy functions */
void add_enemy(Game *game, double x, double y);
void update_enemies(Game *game, double dt);
void draw_enemies(Game *game);

/* Weapon and weather functions */
void fire_weapon(Game *game);
void update_weapon(Game *game, double dt);
void draw_weapon(Game *game);
void draw_crosshair(Game *game);
void update_title(Game *game);

/* HUD and end screen */
void draw_hud(Game *game);
void draw_end_screen(Game *game);
int end_button_at(int x, int y);
void init_rain(Game *game);
void update_rain(Game *game, double dt);
void draw_rain(Game *game);

#endif /* MAZE_H */
