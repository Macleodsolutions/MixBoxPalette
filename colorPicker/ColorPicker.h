// ColorPicker.h
#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "utils.h"

class Window;

class ColorPicker {
public:
    ColorPicker(SDL_Window* parentWindow, TTF_Font* font);
    ~ColorPicker();
    HSVColor ShowPicker(const HSVColor& initialColor = { 1, 1, 1 });
    [[nodiscard]] bool IsOpen() const;
    HSVColor currentColor = { 1, 1, 1 };

private:
    SDL_Window* parentWindow;
    SDL_Window* subWindow;
    SDL_Renderer* subRenderer;
    TTF_Font* font;
    bool subWindowOpen;

    void processEvents();
    void render();

};

#endif // COLORPICKER_H
