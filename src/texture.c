#include "../inc/maze.h"

static const char *const TEXTURE_FILES[TEX_COUNT] = {
    "textures/wall.bmp",
    "textures/floor.bmp",
    "textures/ceiling.bmp",
    "textures/enemy.bmp",
    "textures/weapon.bmp"
};

static Uint32 rgb(int r, int g, int b)
{
    r = r < 0 ? 0 : (r > 255 ? 255 : r);
    g = g < 0 ? 0 : (g > 255 ? 255 : g);
    b = b < 0 ? 0 : (b > 255 ? 255 : b);
    return 0xFF000000u | ((Uint32)r << 16) | ((Uint32)g << 8) | (Uint32)b;
}

/* Cheap deterministic noise in [-n, n] so generated textures look worn */
static int noise(int x, int y, int n)
{
    unsigned int h = (unsigned int)x * 374761393u + (unsigned int)y * 668265263u;

    h = (h ^ (h >> 13)) * 1274126177u;
    return (int)((h ^ (h >> 16)) % (unsigned int)(2 * n + 1)) - n;
}

static void make_wall(Texture *t)
{
    int x, y, row, offset, mortar, n;

    for (y = 0; y < TEX_SIZE; y++)
        for (x = 0; x < TEX_SIZE; x++) {
            row = y / 16;
            offset = (row % 2) * 16;
            mortar = y % 16 == 0 || (x + offset) % 32 == 0;
            n = noise(x, y, 14) + noise(x / 4, y / 4, 10);
            t->pixels[y * TEX_SIZE + x] = mortar ? rgb(70 + n, 70 + n, 66 + n)
                                                 : rgb(150 + n, 64 + n / 2, 48 + n / 2);
        }
}

static void make_floor(Texture *t)
{
    int x, y, grout, light, n;

    for (y = 0; y < TEX_SIZE; y++)
        for (x = 0; x < TEX_SIZE; x++) {
            grout = x % 32 == 0 || y % 32 == 0;
            light = ((x / 32) + (y / 32)) % 2 ? 10 : -6;
            n = noise(x, y, 9);
            t->pixels[y * TEX_SIZE + x] = grout ? rgb(40, 40, 42)
                                                : rgb(105 + light + n, 103 + light + n, 98 + light + n);
        }
}

static void make_ceiling(Texture *t)
{
    int x, y, seam, n;

    for (y = 0; y < TEX_SIZE; y++)
        for (x = 0; x < TEX_SIZE; x++) {
            seam = x % 16 == 0;
            n = noise(x, y / 6, 8);
            t->pixels[y * TEX_SIZE + x] = seam ? rgb(35, 25, 18) : rgb(92 + n, 66 + n, 44 + n);
        }
}

/* A floating ghost: round head, wavy hem, two dark eyes. Background is clear */
static void make_enemy(Texture *t)
{
    int x, y, inside;
    double dx, dy, hem;

    for (y = 0; y < TEX_SIZE; y++)
        for (x = 0; x < TEX_SIZE; x++) {
            dx = x - 31.5;
            dy = y - 26.0;
            hem = 56 + 4 * sin(x * 0.6);
            inside = (dx * dx + dy * dy < 22.0 * 22.0) || (y >= 26 && fabs(dx) < 22 && y < hem);
            if (!inside) {
                t->pixels[y * TEX_SIZE + x] = CLEAR_PIXEL;
                continue;
            }
            t->pixels[y * TEX_SIZE + x] = rgb(225 - y, 235 - y, 255 - y / 2);
            if ((pow(x - 23, 2) + pow(y - 24, 2) < 16) || (pow(x - 40, 2) + pow(y - 24, 2) < 16))
                t->pixels[y * TEX_SIZE + x] = rgb(20, 20, 60);
        }
}

/* A hand holding a pistol, seen from behind: sight, slide, then the fist */
static void make_weapon(Texture *t)
{
    int x, y, light;
    double dx;
    Uint32 c;

    for (y = 0; y < TEX_SIZE; y++)
        for (x = 0; x < TEX_SIZE; x++) {
            c = CLEAR_PIXEL;
            /* Slide: rounded top, lit from the left */
            dx = fabs(x - 31.5);
            if (y >= 16 && y < 44 && dx < 7 - (y < 19 ? 19 - y : 0)) {
                light = (int)(40 - dx * 6) + (x < 30 ? 25 : 0);
                c = rgb(52 + light, 56 + light, 64 + light);
                if (y == 30 || y == 31)
                    c = rgb(30, 32, 36); /* slide seam */
            }
            if (x >= 30 && x < 34 && y >= 12 && y < 16)
                c = rgb(220, 70, 40); /* front sight */
            /* Fist wrapped round the grip, knuckles as darker creases */
            dx = fabs(x - 31.5);
            if (y >= 40 && dx < 12 + (y - 40) / 3.0) {
                light = (int)(30 - dx * 2) - (y - 40);
                c = rgb(196 + light, 142 + light, 104 + light);
                if ((y == 46 || y == 52) && dx < 10)
                    c = rgb(150, 100, 72);
            }
            t->pixels[y * TEX_SIZE + x] = c;
        }
}

/* Loads a BMP into a 64x64 texture. Magenta (#FF00FF) means transparent */
static int load_bmp(Texture *t, const char *path)
{
    SDL_Surface *raw = SDL_LoadBMP(path);
    SDL_Surface *img;
    Uint32 *src;
    Uint32 c;
    int x, y;

    if (!raw)
        return -1;
    img = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(raw);
    if (!img)
        return -1;
    SDL_LockSurface(img);
    for (y = 0; y < TEX_SIZE; y++)
        for (x = 0; x < TEX_SIZE; x++) {
            src = (Uint32 *)((Uint8 *)img->pixels + (y * img->h / TEX_SIZE) * img->pitch);
            c = src[x * img->w / TEX_SIZE] | 0xFF000000u;
            t->pixels[y * TEX_SIZE + x] = (c & 0xFFFFFF) == 0xFF00FF ? CLEAR_PIXEL : c;
        }
    SDL_UnlockSurface(img);
    SDL_FreeSurface(img);
    return 0;
}

/*
 * Every texture can be replaced by dropping a BMP into textures/. Without one
 * the game generates its own, so it runs straight from a fresh clone.
 */
void load_textures(Game *game)
{
    void (*generate[TEX_COUNT])(Texture *) = {
        make_wall, make_floor, make_ceiling, make_enemy, make_weapon
    };
    int i;

    for (i = 0; i < TEX_COUNT; i++)
        if (load_bmp(&game->textures[i], TEXTURE_FILES[i]) != 0)
            generate[i](&game->textures[i]);
}
