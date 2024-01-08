#include "ColorPicker.h"

ColorPicker::ColorPicker(SDL_Window *parentWindow, TTF_Font *font)
{
    this->parentWindow = parentWindow;
    this->font = font;
    this->subWindow = nullptr;
    this->subRenderer = nullptr;
    this->subWindowOpen = false;
}

ColorPicker::~ColorPicker()
{
    if (subRenderer) {
        SDL_DestroyRenderer(subRenderer);
    }
    if (subWindow) {
        SDL_DestroyWindow(subWindow);
    }
}

HSVColor ColorPicker::ShowPicker(const HSVColor &initialColor)
{
    int x = 0, y = 0;
    SDL_GetWindowPosition(parentWindow, &x, &y);
    this->subWindow = SDL_CreateWindow("Color Picker", x, y, 250, 300, SDL_WINDOW_SHOWN);
    this->subRenderer = SDL_CreateRenderer(this->subWindow, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetWindowAlwaysOnTop(this->subWindow, SDL_TRUE);

    this->currentColor = initialColor;
    this->subWindowOpen = true;

    while (subWindowOpen) {
        processEvents();
        render();
    }

    SDL_DestroyRenderer(subRenderer);
    SDL_DestroyWindow(subWindow);
    subRenderer = nullptr;
    subWindow = nullptr;

    return currentColor;
}

bool ColorPicker::IsOpen() const
{
    return subWindowOpen;
}

bool isLeftMouseButtonPressed = false;

SDL_Event subEvent;

MouseState mouseState;

UIState uiState = UI_NONE;

void ColorPicker::processEvents()
{

    while (SDL_PollEvent(&subEvent) != 0) {
        switch (subEvent.type) {
            case SDL_WINDOWEVENT:
                if (subEvent.window.event == SDL_WINDOWEVENT_CLOSE)
                    subWindowOpen = false;
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEMOTION:
                mouseState.buttons = SDL_GetMouseState(&mouseState.x, &mouseState.y);
                uiState = (subEvent.type == SDL_MOUSEBUTTONDOWN) ? get_click_state(mouseState) : uiState;

                if (uiState == UI_GRADIENT_CHANGE) {
                    currentColor.s =
                        static_cast<double>(clamp(0, MAIN_GRADIENT_SIZE, mouseState.x)) / MAIN_GRADIENT_SIZE;
                    currentColor.v =
                        1.0 - static_cast<double>(clamp(0, MAIN_GRADIENT_SIZE, mouseState.y)) / MAIN_GRADIENT_SIZE;
                }
                else if (uiState == UI_SLIDER_CHANGE) {
                    currentColor.h =
                        static_cast<double>(clamp(0, HUE_GRADIENT_WIDTH, mouseState.x)) / HUE_GRADIENT_WIDTH * 360.0;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                uiState = UI_NONE;
                break;
        }
    }
}

void ColorPicker::render()
{
    SDL_SetRenderDrawColor(subRenderer, 0, 0, 0, 255);
    SDL_RenderClear(subRenderer);

    draw_gradient(subRenderer, currentColor.h);
    draw_hue_gradient(subRenderer);
    draw_hue_slider(subRenderer, currentColor.h);
    draw_sample_box(subRenderer, from_RGBColor(hsv_to_rgb(currentColor)));
    draw_info_text(subRenderer, currentColor, font);

    SDL_RenderPresent(subRenderer);
}
