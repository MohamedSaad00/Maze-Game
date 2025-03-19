#include "../inc/maze.h"

int main(int argc, char *argv[])
{
    Game game;
    SDL_Event event;

    if (argc != 2) {
        printf("Usage: %s <map_file>\n", argv[0]);
        return 1;
    }

    init_game(&game);
    load_map(&game, argv[1]);
    load_textures(&game);

    while (game.game_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                game.game_running = 0;
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    game.game_running = 0;
                else if (event.key.keysym.sym == SDLK_m)
                    game.show_map = !game.show_map;
                else if (event.key.keysym.sym == SDLK_r)
                    game.rain_active = !game.rain_active;
            }
        }

        handle_input(&game);
        update_game(&game);
        render_game(&game);
        SDL_Delay(16); /* Cap at ~60 FPS */
    }

    cleanup_game(&game);
    return 0;
} 
