#pragma once

#include <cmath>
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <SDL_ttf.h>

#define MAIN_GRADIENT_SIZE  (200)
#define HUE_GRADIENT_WIDTH  (200)
#define HUE_GRADIENT_HEIGHT (30)
#define SAMPLE_BOX_SIZE     (30)
#define HUE_SLIDER_WIDTH    (3)

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} RGBColor;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} HSVColor;

HSVColor rgb_to_hsv(RGBColor in);
RGBColor hsv_to_rgb(HSVColor in);

int clamp(int lower, int higher, int num);
bool point_in_rect(SDL_Rect rect, int x, int y);
SDL_Color from_RGBColor(RGBColor rgb_color);

void draw_hue_slider(SDL_Renderer* renderer, double hue);
void draw_sample_box(SDL_Renderer* renderer, SDL_Color color);

typedef struct {
    uint32_t buttons;
    int x;
    int y;
} MouseState;

enum UIState {
    UI_NONE,
    UI_SLIDER_CHANGE,
    UI_GRADIENT_CHANGE,
};

enum UIState get_click_state(MouseState m);
void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y);
void set_pixel(SDL_Surface* surface, SDL_Color color, int x, int y);
void draw_gradient(SDL_Renderer* renderer, double hue);
void draw_hue_gradient(SDL_Renderer* renderer);
void draw_info_text(SDL_Renderer* renderer, HSVColor hsv_color, TTF_Font* font);
