#include "../inc/maze.h"

/*
 * A 5x7 bitmap font with just the letters the HUD needs: SDL2 alone has no
 * text rendering, and pulling in SDL_ttf for a dozen words is not worth it.
 * Each row is five bits, most significant bit on the left.
 */
typedef struct {
    char c;
    unsigned char rows[7];
} Glyph;

static const Glyph FONT[] = {
    {'A', {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'C', {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}},
    {'D', {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}},
    {'E', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}},
    {'G', {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}},
    {'H', {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'I', {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {'L', {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}},
    {'M', {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}},
    {'N', {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}},
    {'O', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'R', {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
    {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
    {'T', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
    {'U', {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'V', {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}},
    {'W', {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}},
    {'Y', {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}},
    {'!', {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}}
};

static const unsigned char *glyph(char c)
{
    size_t i;

    for (i = 0; i < sizeof(FONT) / sizeof(FONT[0]); i++)
        if (FONT[i].c == c)
            return FONT[i].rows;
    return NULL; /* space, or a letter the HUD never uses */
}

static int text_width(const char *text, int scale)
{
    return (int)strlen(text) * 6 * scale - scale;
}

/* Draws text centred on cx, top edge at y, in blocks of scale pixels */
static void draw_text(Game *game, const char *text, int cx, int y, int scale)
{
    int x = cx - text_width(text, scale) / 2;
    const unsigned char *rows;
    int row, col;

    for (; *text; text++, x += 6 * scale) {
        rows = glyph(*text);
        if (!rows)
            continue;
        for (row = 0; row < 7; row++)
            for (col = 0; col < 5; col++)
                if (rows[row] & (0x10 >> col))
                    SDL_RenderFillRect(game->renderer, &(SDL_Rect){
                        x + col * scale, y + row * scale, scale, scale});
    }
}

/* ------------------------------------------------------------- in play */

static const unsigned char HEART[6] = {0x36, 0x7F, 0x7F, 0x3E, 0x1C, 0x08};

static void draw_heart(Game *game, int x, int y, int scale, int full)
{
    int row, col;

    if (full)
        SDL_SetRenderDrawColor(game->renderer, 230, 40, 50, 255);
    else
        SDL_SetRenderDrawColor(game->renderer, 70, 70, 75, 200);
    for (row = 0; row < 6; row++)
        for (col = 0; col < 7; col++)
            if (HEART[row] & (0x40 >> col))
                SDL_RenderFillRect(game->renderer, &(SDL_Rect){
                    x + col * scale, y + row * scale, scale, scale});
}

/* Lives in the top-left corner, and a red wash for a moment after a hit */
void draw_hud(Game *game)
{
    int i;

    if (game->hurt_timer > 0) {
        SDL_SetRenderDrawColor(game->renderer, 200, 0, 0,
            (Uint8)(140 * game->hurt_timer / HURT_TIME));
        SDL_RenderFillRect(game->renderer, NULL);
    }
    for (i = 0; i < PLAYER_HEALTH; i++)
        draw_heart(game, 14 + i * 34, 14, 4, i < game->player_health);
}

/* ------------------------------------------------------------ end screen */

static SDL_Rect button_rect(int which)
{
    SDL_Rect r = {0, SCREEN_HEIGHT / 2 + 40, 170, 54};

    r.x = which == END_RESTART ? SCREEN_WIDTH / 2 - 190 : SCREEN_WIDTH / 2 + 20;
    return r;
}

int end_button_at(int x, int y)
{
    SDL_Point p = {x, y};
    SDL_Rect restart = button_rect(END_RESTART);
    SDL_Rect close = button_rect(END_CLOSE);

    if (SDL_PointInRect(&p, &restart))
        return END_RESTART;
    if (SDL_PointInRect(&p, &close))
        return END_CLOSE;
    return 0;
}

static void draw_button(Game *game, int which, const char *label, int hover)
{
    SDL_Rect r = button_rect(which);

    SDL_SetRenderDrawColor(game->renderer, hover ? 90 : 45, hover ? 90 : 45, hover ? 100 : 52, 240);
    SDL_RenderFillRect(game->renderer, &r);
    SDL_SetRenderDrawColor(game->renderer, 220, 220, 230, 255);
    SDL_RenderDrawRect(game->renderer, &r);
    draw_text(game, label, r.x + r.w / 2, r.y + (r.h - 21) / 2, 3);
}

void draw_end_screen(Game *game)
{
    int won = game->state == STATE_WON;
    int mx, my, hover;

    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 170);
    SDL_RenderFillRect(game->renderer, NULL);

    if (won)
        SDL_SetRenderDrawColor(game->renderer, 90, 230, 110, 255);
    else
        SDL_SetRenderDrawColor(game->renderer, 235, 60, 60, 255);
    draw_text(game, won ? "YOU WIN!" : "GAME OVER", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 110, 9);

    SDL_SetRenderDrawColor(game->renderer, 210, 210, 220, 255);
    draw_text(game, won ? "ALL GHOSTS CLEARED" : "THE GHOSTS GOT YOU",
        SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20, 3);

    SDL_GetMouseState(&mx, &my);
    hover = end_button_at(mx, my);
    draw_button(game, END_RESTART, "RESTART", hover == END_RESTART);
    draw_button(game, END_CLOSE, "CLOSE", hover == END_CLOSE);
}
