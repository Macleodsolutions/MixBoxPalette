#include "ColorPicker.h"

HSVColor rgb_to_hsv(RGBColor in)
{
    HSVColor out;
    double min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min < in.b ? min : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max > in.b ? max : in.b;

    out.v = max; // v
    delta = max - min;
    if (delta < 0.00001) {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max); // s
    }
    else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN; // its now undefined
        return out;
    }
    if (in.r >= max) // > is bogus, just keeps compiler happy
        out.h = (in.g - in.b) / delta; // between yellow & magenta
    else if (in.g >= max)
        out.h = 2.0 + (in.b - in.r) / delta; // between cyan & yellow
    else
        out.h = 4.0 + (in.r - in.g) / delta; // between magenta & cyan

    out.h *= 60.0; // degrees

    if (out.h < 0.0)
        out.h += 360.0;

    return out;
}

RGBColor hsv_to_rgb(HSVColor in)
{
    double hh, p, q, t, ff;
    long i;
    RGBColor out;

    if (in.s <= 0.0) { // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long) hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch (i) {
        case 0:
            out.r = in.v;
            out.g = t;
            out.b = p;
            break;
        case 1:
            out.r = q;
            out.g = in.v;
            out.b = p;
            break;
        case 2:
            out.r = p;
            out.g = in.v;
            out.b = t;
            break;
        case 3:
            out.r = p;
            out.g = q;
            out.b = in.v;
            break;
        case 4:
            out.r = t;
            out.g = p;
            out.b = in.v;
            break;
        case 5:
        default:
            out.r = in.v;
            out.g = p;
            out.b = q;
            break;
    }
    return out;
}

int clamp(int lower, int higher, int num)
{
    if (num < lower) {
        return lower;
    }
    else if (num > higher) {
        return higher;
    }
    else {
        return num;
    }
}

bool point_in_rect(SDL_Rect rect, int x, int y)
{
    return x > rect.x && x < rect.x + rect.w && y > rect.y && y < rect.y + rect.h;
}

SDL_Color from_RGBColor(RGBColor rgb_color)
{
    SDL_Color color = {
        (Uint8) (rgb_color.r * 255),
        (Uint8) (rgb_color.g * 255),
        (Uint8) (rgb_color.b * 255),
        0xFF
    };
    return color;
}

void draw_hue_slider(SDL_Renderer *renderer, double hue)
{
    SDL_Rect draw_rect = {
        (int) ((hue / 360.0) * HUE_GRADIENT_WIDTH) - (HUE_SLIDER_WIDTH / 2),
        MAIN_GRADIENT_SIZE,
        HUE_SLIDER_WIDTH,
        HUE_GRADIENT_HEIGHT
    };
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(renderer, &draw_rect);
}

void draw_sample_box(SDL_Renderer *renderer, SDL_Color color)
{
    SDL_Rect draw_rect = {
        HUE_GRADIENT_WIDTH,
        MAIN_GRADIENT_SIZE,
        SAMPLE_BOX_SIZE,
        SAMPLE_BOX_SIZE
    };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xff);
    SDL_RenderFillRect(renderer, &draw_rect);
}

enum UIState get_click_state(MouseState m)
{
    SDL_Rect main_gradient = {0, 0, MAIN_GRADIENT_SIZE, MAIN_GRADIENT_SIZE};
    SDL_Rect slider_gradient = {0, MAIN_GRADIENT_SIZE, HUE_GRADIENT_WIDTH, HUE_GRADIENT_HEIGHT};
    if (point_in_rect(main_gradient, m.x, m.y)) {
        return UI_GRADIENT_CHANGE;
    }
    else if (point_in_rect(slider_gradient, m.x, m.y)) {
        return UI_SLIDER_CHANGE;
    }
    else {
        return UI_NONE;
    }
}

void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y)
{
    SDL_Color textColor = {255, 255, 255, 255}; // White color for text
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture != nullptr) {
        SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
        SDL_DestroyTexture(textTexture);
    }
    SDL_FreeSurface(textSurface);
}

void set_pixel(SDL_Surface *surface, SDL_Color color, int x, int y)
{
    auto *pixels = (Uint32 *) surface->pixels;
    Uint32 pixel_color = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);

    pixels[(y * surface->w) + x] = pixel_color;
}

void draw_gradient(SDL_Renderer *renderer, double hue)
{
    SDL_Surface *surface = SDL_CreateRGBSurface(0,
                                                MAIN_GRADIENT_SIZE,
                                                MAIN_GRADIENT_SIZE,
                                                32,
                                                0x00FF0000,
                                                0x0000FF00,
                                                0x000000FF,
                                                0xFF000000);
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            HSVColor hsv_color = {hue, (double) x / surface->w, 1.0 - (double) y / surface->h};
            set_pixel(surface, from_RGBColor(hsv_to_rgb(hsv_color)), x, y);
        }
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect render_rect = {0, 0, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &render_rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void draw_hue_gradient(SDL_Renderer *renderer)
{
    SDL_Surface *surface = SDL_CreateRGBSurface(0,
                                                HUE_GRADIENT_WIDTH,
                                                HUE_GRADIENT_HEIGHT,
                                                32,
                                                0x00FF0000,
                                                0x0000FF00,
                                                0x000000FF,
                                                0xFF000000);
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            HSVColor hsv_color = {(double) x / surface->w * 360, 1.0, 1.0};
            set_pixel(surface, from_RGBColor(hsv_to_rgb(hsv_color)), x, y);
        }
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect render_rect = {0, MAIN_GRADIENT_SIZE, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &render_rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void draw_info_text(SDL_Renderer *renderer, HSVColor hsv_color, TTF_Font *font)
{
    RGBColor rgb_color = hsv_to_rgb(hsv_color);
    char rgb_buffer[128];
    sprintf(rgb_buffer, "R: %d G: %d B: %d",
            (int) (rgb_color.r * 255),
            (int) (rgb_color.g * 255),
            (int) (rgb_color.b * 255));

    draw_text(renderer, font, rgb_buffer, 10, MAIN_GRADIENT_SIZE + HUE_GRADIENT_HEIGHT + 10);

    char hsv_buffer[128];
    sprintf(hsv_buffer, "H: %d S: %d V: %d",
            (int) hsv_color.h,
            (int) (hsv_color.s * 100),
            (int) (hsv_color.v * 100));

    draw_text(renderer, font, hsv_buffer, 10, MAIN_GRADIENT_SIZE + HUE_GRADIENT_HEIGHT + 30);
}