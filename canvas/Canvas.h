#ifndef CANVAS_H
#define CANVAS_H

#include <SDL.h>
#include <memory>
#include <queue>
#include <unordered_map>
#include <optional>
#include "../mixbox/mixbox.h"
#include <functional>

class Canvas {
public:
    Canvas(SDL_Renderer* renderer, int width, int height, int displayCanvasWidth, int displayCanvasHeight, int windowWidth, int windowHeight);

    void setTextureOffset(int x, int y);
    void setTextureZoom(float zoom, int mouseX, int mouseY);
    void setPixel(int x1, int y1, int x2, int y2, uint32_t color, int blend, int brushSize);
    void rebuildHighResPixels(SDL_Renderer* renderer);
    void resetCanvas(SDL_Renderer* renderer);
    [[nodiscard]] uint32_t getPixel(int x, int y) const;
    [[nodiscard]] std::pair<uint32_t, int> getMostCommonColorInRadius(int centerX, int centerY, int maxRadius, uint32_t excludeColor) const;
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture;
    SDL_Rect srcRect;
    std::optional<uint32_t> firstBlendColor, secondBlendColor;

private:
    struct PixelInfo {
        int x, y;
        uint32_t color;
        int radius;
        PixelInfo(int x, int y, uint32_t color, int radius) : x(x), y(y), color(color), radius(radius) {}
    };

    int virtualCanvasWidth, virtualCanvasHeight;
    int displayCanvasWidth, displayCanvasHeight;
    int windowWidth, windowHeight;
    std::vector<uint32_t> pixels;
    int originalWidth, originalHeight;
    std::unique_ptr<SDL_PixelFormat, decltype(&SDL_FreeFormat)> format;
    std::queue<PixelInfo> drawOrder;
    uint32_t blendColors(int radius, int frequency);
};

#endif // CANVAS_H
