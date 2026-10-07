#include "../inc/maze.h"

int init_game(Game *game)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    game->window = SDL_CreateWindow("Maze Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!game->window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    game->renderer = SDL_CreateRenderer(game->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!game->renderer)
        game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_SOFTWARE);
    if (!game->renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);

    game->screen = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    game->frame = malloc(sizeof(Uint32) * SCREEN_WIDTH * SCREEN_HEIGHT);
    if (!game->screen || !game->frame) {
        printf("Could not allocate the frame buffer\n");
        return -1;
    }

    game->show_map = 0;
    game->rain_active = 0;
    game->game_running = 1;
    game->player_health = PLAYER_HEALTH;
    game->state = STATE_PLAYING;
    return 0;
}

/* Back to the start of the same map: fresh enemies, full health */
int restart_game(Game *game)
{
    free_map(game);
    game->enemy_count = 0;
    game->enemies_left = 0;
    game->player_health = PLAYER_HEALTH;
    game->hurt_timer = 0;
    game->fire_cooldown = 0;
    game->flash_timer = 0;
    game->walk_time = 0;
    game->state = STATE_PLAYING;
    if (load_map(game, game->map_file) != 0)
        return -1;
    update_title(game);
    return 0;
}

static void try_move(Game *game, double dx, double dy)
{
    Player *p = &game->player;

    /* Each axis on its own, so walking into a wall at an angle slides along it */
    if (!check_collision(game, p->x + dx, p->y))
        p->x += dx;
    if (!check_collision(game, p->x, p->y + dy))
        p->y += dy;
}

static void rotate(Player *p, double angle)
{
    double c = cos(angle);
    double s = sin(angle);
    double old_dir_x = p->dir_x;
    double old_plane_x = p->plane_x;

    p->dir_x = p->dir_x * c - p->dir_y * s;
    p->dir_y = old_dir_x * s + p->dir_y * c;
    p->plane_x = p->plane_x * c - p->plane_y * s;
    p->plane_y = old_plane_x * s + p->plane_y * c;
}

void handle_input(Game *game, double dt)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    Player *p = &game->player;
    double step = MOVE_SPEED * dt;
    double dx = 0;
    double dy = 0;

    if (game->state != STATE_PLAYING)
        return;

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
        dx += p->dir_x * step;
        dy += p->dir_y * step;
    }
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
        dx -= p->dir_x * step;
        dy -= p->dir_y * step;
    }
    /* The player's right is (-dir_y, dir_x): map rows grow downwards */
    if (keys[SDL_SCANCODE_A]) {
        dx += p->dir_y * step;
        dy -= p->dir_x * step;
    }
    if (keys[SDL_SCANCODE_D]) {
        dx -= p->dir_y * step;
        dy += p->dir_x * step;
    }
    if (dx != 0 || dy != 0) {
        try_move(game, dx, dy);
        game->walk_time += dt;
    }

    if (keys[SDL_SCANCODE_LEFT])
        rotate(p, -ROTATION_SPEED * dt);
    if (keys[SDL_SCANCODE_RIGHT])
        rotate(p, ROTATION_SPEED * dt);
}

void update_game(Game *game, double dt)
{
    if (game->rain_active)
        update_rain(game, dt);
    if (game->state != STATE_PLAYING)
        return; /* the scene stays frozen behind the end screen */

    update_enemies(game, dt);
    update_weapon(game, dt);
    if (game->hurt_timer > 0)
        game->hurt_timer -= dt;

    if (game->player_health <= 0) {
        game->state = STATE_LOST;
        update_title(game);
    } else if (game->enemy_count == 0) {
        game->state = STATE_WON; /* the last ghost has finished fading out */
        update_title(game);
    }
}

/* Draws the whole frame into the renderer without presenting it */
static void compose_frame(Game *game)
{
    draw_floor_ceiling(game);
    cast_rays(game);
    draw_enemies(game);
    draw_weapon(game);

    SDL_UpdateTexture(game->screen, NULL, game->frame, SCREEN_WIDTH * sizeof(Uint32));
    SDL_RenderClear(game->renderer);
    SDL_RenderCopy(game->renderer, game->screen, NULL, NULL);

    /* Overlays are drawn by the renderer on top, with alpha blending */
    if (game->rain_active)
        draw_rain(game);
    if (game->show_map)
        draw_minimap(game);
    if (game->state == STATE_PLAYING) {
        draw_crosshair(game);
        draw_hud(game);
    } else
        draw_end_screen(game);
}

void render_game(Game *game)
{
    compose_frame(game);
    SDL_RenderPresent(game->renderer);
}

int save_screenshot(Game *game, const char *filename)
{
    SDL_Surface *shot;
    int status;

    shot = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
        SDL_PIXELFORMAT_ARGB8888);
    if (!shot)
        return -1;
    compose_frame(game);
    status = SDL_RenderReadPixels(game->renderer, NULL, SDL_PIXELFORMAT_ARGB8888,
        shot->pixels, shot->pitch);
    if (status == 0)
        status = SDL_SaveBMP(shot, filename);
    if (status != 0)
        printf("Could not save %s: %s\n", filename, SDL_GetError());
    SDL_FreeSurface(shot);
    return status;
}

void cleanup_game(Game *game)
{
    if (game->screen)
        SDL_DestroyTexture(game->screen);
    if (game->renderer)
        SDL_DestroyRenderer(game->renderer);
    if (game->window)
        SDL_DestroyWindow(game->window);
    SDL_Quit();
    free(game->frame);
    free_map(game);
}
