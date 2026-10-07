#include "../inc/maze.h"

static void usage(const char *name)
{
    printf("Usage: %s <map_file> [--screenshot <file.bmp>]\n", name);
}

static void restart_or_quit(Game *game)
{
    if (restart_game(game) != 0)
        game->game_running = 0;
}

/* Win or lose: R / Enter restarts, Esc / Q closes, or click a button */
static void handle_end_event(Game *game, SDL_Event *event)
{
    SDL_Keycode key;

    if (event->type == SDL_KEYDOWN && !event->key.repeat) {
        key = event->key.keysym.sym;
        if (key == SDLK_r || key == SDLK_RETURN || key == SDLK_KP_ENTER)
            restart_or_quit(game);
        else if (key == SDLK_ESCAPE || key == SDLK_q)
            game->game_running = 0;
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (end_button_at(event->button.x, event->button.y) == END_RESTART)
            restart_or_quit(game);
        else if (end_button_at(event->button.x, event->button.y) == END_CLOSE)
            game->game_running = 0;
    }
}

static void handle_events(Game *game)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            game->game_running = 0;
        else if (game->state != STATE_PLAYING)
            handle_end_event(game, &event);
        else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
            if (event.key.keysym.sym == SDLK_ESCAPE)
                game->game_running = 0;
            else if (event.key.keysym.sym == SDLK_m)
                game->show_map = !game->show_map;
            else if (event.key.keysym.sym == SDLK_r)
                game->rain_active = !game->rain_active;
            else if (event.key.keysym.sym == SDLK_SPACE)
                fire_weapon(game);
        } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            fire_weapon(game);
    }
}

int main(int argc, char *argv[])
{
    Game game;
    const char *screenshot = NULL;
    Uint64 last;
    Uint64 now;
    double dt;
    int status = 0;

    if (argc == 4 && strcmp(argv[2], "--screenshot") == 0)
        screenshot = argv[3];
    else if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    memset(&game, 0, sizeof(game));
    game.map_file = argv[1];
    if (load_map(&game, argv[1]) != 0)
        return 1;
    if (init_game(&game) != 0) {
        cleanup_game(&game);
        return 1;
    }
    load_textures(&game);
    init_rain(&game);
    update_title(&game);

    /* One frame to a file and out: for the README and for checking a build */
    if (screenshot) {
        game.show_map = 1;
        game.rain_active = 1;
        update_game(&game, 0.5);
        status = save_screenshot(&game, screenshot) == 0 ? 0 : 1;
        cleanup_game(&game);
        return status;
    }

    last = SDL_GetPerformanceCounter();
    while (game.game_running) {
        now = SDL_GetPerformanceCounter();
        dt = (double)(now - last) / (double)SDL_GetPerformanceFrequency();
        last = now;
        if (dt > 0.1)
            dt = 0.1; /* a stalled frame must not teleport anyone through a wall */

        handle_events(&game);
        handle_input(&game, dt);
        update_game(&game, dt);
        render_game(&game);
    }

    cleanup_game(&game);
    return 0;
}
