#include "../inc/maze.h"

void init_game(Game *game)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    game->window = SDL_CreateWindow("Maze Game",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (!game->window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
    if (!game->renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    game->player.x = 1.5 * TILE_SIZE;
    game->player.y = 1.5 * TILE_SIZE;
    game->player.angle = 0;
    game->show_map = 0;
    game->rain_active = 0;
    game->game_running = 1;
}

void handle_input(Game *game)
{
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    double move_speed = PLAYER_SPEED;
    double rot_speed = ROTATION_SPEED;
    double new_x = game->player.x;
    double new_y = game->player.y;

    if (keyboard_state[SDL_SCANCODE_W]) {
        new_x += cos(game->player.angle) * move_speed;
        new_y += sin(game->player.angle) * move_speed;
    }
    if (keyboard_state[SDL_SCANCODE_S]) {
        new_x -= cos(game->player.angle) * move_speed;
        new_y -= sin(game->player.angle) * move_speed;
    }
    if (keyboard_state[SDL_SCANCODE_A]) {
        new_x += cos(game->player.angle - PI/2) * move_speed;
        new_y += sin(game->player.angle - PI/2) * move_speed;
    }
    if (keyboard_state[SDL_SCANCODE_D]) {
        new_x += cos(game->player.angle + PI/2) * move_speed;
        new_y += sin(game->player.angle + PI/2) * move_speed;
    }
    if (keyboard_state[SDL_SCANCODE_LEFT]) {
        game->player.angle -= rot_speed;
    }
    if (keyboard_state[SDL_SCANCODE_RIGHT]) {
        game->player.angle += rot_speed;
    }

    if (!check_collision(game, new_x, new_y)) {
        game->player.x = new_x;
        game->player.y = new_y;
    }
}

void update_game(Game *game)
{
    update_enemies(game);
    if (game->rain_active) {
        update_rain(game);
    }
}

void render_game(Game *game)
{
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);

    /* Draw ceiling */
    SDL_SetRenderDrawColor(game->renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT/2});

    /* Draw floor */
    SDL_SetRenderDrawColor(game->renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){0, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT/2});

    cast_rays(game);
    draw_enemies(game);
    
    if (game->rain_active) {
        draw_rain(game);
    }

    if (game->show_map) {
        draw_minimap(game);
    }

    SDL_RenderPresent(game->renderer);
}

void cleanup_game(Game *game)
{
    SDL_DestroyTexture(game->wall_texture);
    SDL_DestroyTexture(game->floor_texture);
    SDL_DestroyTexture(game->ceiling_texture);
    SDL_DestroyTexture(game->weapon_texture);
    SDL_DestroyTexture(game->enemy_texture);
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDL_Quit();

    for (int i = 0; i < game->map_height; i++) {
        free(game->map[i]);
    }
    free(game->map);
} 
