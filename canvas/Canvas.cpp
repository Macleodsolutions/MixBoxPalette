#include "Canvas.h"

Canvas::Canvas(SDL_Renderer *renderer,
               int width,
               int height,
               int displayCanvasWidth,
               int displayCanvasHeight,
               int windowWidth,
               int windowHeight)
    : virtualCanvasWidth(width), virtualCanvasHeight(height),
      displayCanvasWidth(displayCanvasWidth), displayCanvasHeight(displayCanvasHeight),
      windowWidth(windowWidth), windowHeight(windowHeight),
      pixels(),
      texture(SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_TARGET,
                                displayCanvasWidth,
                                displayCanvasHeight), SDL_DestroyTexture),
      srcRect{0, 0, displayCanvasWidth, displayCanvasHeight},
      originalWidth(displayCanvasWidth), originalHeight(displayCanvasHeight),
      format(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), SDL_FreeFormat)
{
    resetCanvas(renderer);
}

void Canvas::resetCanvas(SDL_Renderer *renderer)
{
    SDL_SetRenderTarget(renderer, texture.get()), SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);
    pixels.assign(displayCanvasWidth * displayCanvasHeight, 0xFFFFFF00);
}

void Canvas::setTextureZoom(float zoom, int mouseX, int mouseY)
{
    float relativeZoomX = static_cast<float>(mouseX + srcRect.x) / originalWidth;
    float relativeZoomY = static_cast<float>(mouseY + srcRect.y) / originalHeight;

    srcRect.w = static_cast<int>(originalWidth * zoom);
    srcRect.h = static_cast<int>(originalHeight * zoom);

    srcRect.x = std::clamp(static_cast<int>(relativeZoomX * originalWidth - srcRect.w * relativeZoomX),
                           0,
                           originalWidth - srcRect.w);
    srcRect.y = std::clamp(static_cast<int>(relativeZoomY * originalHeight - srcRect.h * relativeZoomY),
                           0,
                           originalHeight - srcRect.h);
}

void Canvas::setTextureOffset(int deltaX, int deltaY)
{
    float scale = originalWidth / static_cast<float>(srcRect.w);

    srcRect.x = std::clamp(srcRect.x + static_cast<int>(deltaX * scale), 0, originalWidth - srcRect.w);
    srcRect.y = std::clamp(srcRect.y + static_cast<int>(deltaY * scale), 0, originalHeight - srcRect.h);
}

void Canvas::setPixel(int x1, int y1, int x2, int y2, uint32_t color, int blend, int brushSize)
{
    float zoomFactorX = static_cast<float>(displayCanvasWidth) / srcRect.w;
    float zoomFactorY = static_cast<float>(displayCanvasHeight) / srcRect.h;
    float scaleX = static_cast<float>(displayCanvasWidth) / virtualCanvasWidth;
    float scaleY = static_cast<float>(displayCanvasHeight) / virtualCanvasHeight;

    auto scaleCoord = [zoomFactorX, zoomFactorY, scaleX, scaleY, &rect = srcRect](int x, int y)
    {
        return std::make_pair(
            static_cast<int>((x / zoomFactorX) + (rect.x / scaleX)),
            static_cast<int>((y / zoomFactorY) + (rect.y / scaleY))
        );
    };

    std::tie(x1, y1) = scaleCoord(x1, y1);
    std::tie(x2, y2) = scaleCoord(x2, y2);

    int radius = (brushSize - 3) * 15;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = std::max(abs(dx), abs(dy));
    float xInc = dx / static_cast<float>(steps);
    float yInc = dy / static_cast<float>(steps);
    float x = x1;
    float y = y1;

    auto drawPixels = [this, &x, &y, steps, xInc, yInc, radius](uint32_t drawColor)
    {
        for (int i = 0; i <= steps; i++) {
            int roundX = round(x);
            int roundY = round(y);
            if (roundX >= 0 && roundX < displayCanvasWidth && roundY >= 0 && roundY < displayCanvasHeight) {
                drawOrder.emplace(roundX, roundY, drawColor, radius);
            }
            x += xInc;
            y += yInc;
        }
    };

    if (!blend) {
        drawPixels(color);
        return;
    }

    if (!firstBlendColor) {
        auto [mostCommonColor, frequency] = getMostCommonColorInRadius(x, y, radius, 0x00000000);
        if (frequency == 0) return;
        firstBlendColor = mostCommonColor;
    }

    auto [mostCommonColor, frequency] = getMostCommonColorInRadius(x, y, radius, firstBlendColor.value());
    if (frequency != 0) { secondBlendColor = mostCommonColor; }
    if (secondBlendColor) { firstBlendColor = blendColors(radius, frequency); }

    drawPixels(firstBlendColor.value_or(color));
}

uint32_t Canvas::blendColors(int radius, int frequency)
{
    Uint8 r1 = (firstBlendColor.value() >> 24) & 0xFF;
    Uint8 g1 = (firstBlendColor.value() >> 16) & 0xFF;
    Uint8 b1 = (firstBlendColor.value() >> 8) & 0xFF;

    Uint8 r2 = (secondBlendColor.value() >> 24) & 0xFF;
    Uint8 g2 = (secondBlendColor.value() >> 16) & 0xFF;
    Uint8 b2 = (secondBlendColor.value() >> 8) & 0xFF;

    float mixingRatio = std::clamp(static_cast<float>(frequency) / (radius * radius * M_PI), 0.0, 1.0);
    mixbox_lerp(r1, g1, b1, r2, g2, b2, mixingRatio, &r2, &g2, &b2);
    return (r2 << 24) | (g2 << 16) | (b2 << 8) | 255;
}

void Canvas::rebuildHighResPixels(SDL_Renderer *renderer)
{
    int scaleFactorX = displayCanvasWidth / virtualCanvasWidth;
    int scaleFactorY = displayCanvasHeight / virtualCanvasHeight;
    SDL_SetRenderTarget(renderer, texture.get());
    while (!drawOrder.empty()) {
        PixelInfo pixelInfo = drawOrder.front();
        drawOrder.pop();
        Uint8 r1, g1, b1, a1;
        SDL_GetRGBA(pixelInfo.color, format.get(), &r1, &g1, &b1, &a1);
        SDL_SetRenderDrawColor(renderer, r1, g1, b1, a1);
        int centerX = pixelInfo.x * scaleFactorX;
        int centerY = pixelInfo.y * scaleFactorY;
        int radius = pixelInfo.radius;

        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (dx * dx + dy * dy <= radius * radius) {
                    int highResX = centerX + dx;
                    int highResY = centerY + dy;
                    if (highResX >= 0 && highResX < displayCanvasWidth && highResY >= 0
                        && highResY < displayCanvasHeight) {
                        int highResIndex = highResY * displayCanvasWidth + highResX;
                        SDL_RenderDrawPoint(renderer, highResX, highResY);
                        pixels[highResIndex] = pixelInfo.color;
                    }
                }
            }
        }
    }
    SDL_SetRenderTarget(renderer, nullptr);
}

uint32_t Canvas::getPixel(int x, int y) const
{
    int zoomedX = (x * displayCanvasWidth / windowWidth) * srcRect.w / displayCanvasWidth + srcRect.x;
    int zoomedY = (y * displayCanvasHeight / windowHeight) * srcRect.h / displayCanvasHeight + srcRect.y;

    if (zoomedX < 0 || zoomedX >= displayCanvasWidth || zoomedY < 0 || zoomedY >= displayCanvasHeight) {
        return 0x00000000;
    }

    return pixels[zoomedY * displayCanvasWidth + zoomedX];
}

std::pair<uint32_t, int>
Canvas::getMostCommonColorInRadius(int centerX, int centerY, int maxRadius, uint32_t excludeColor) const
{
    std::unordered_map<Uint32, int> colorFrequency;
    int displayCanvasCenterX = centerX * displayCanvasWidth / virtualCanvasWidth;
    int displayCanvasCenterY = centerY * displayCanvasHeight / virtualCanvasHeight;

    for (int y = -maxRadius; y <= maxRadius; ++y) {
        for (int x = -maxRadius; x <= maxRadius; ++x) {
            if (x * x + y * y <= maxRadius * maxRadius) {
                int displayCanvasX = displayCanvasCenterX + x, displayCanvasY = displayCanvasCenterY + y;

                if (displayCanvasX >= 0 && displayCanvasX < displayCanvasWidth && displayCanvasY >= 0
                    && displayCanvasY < displayCanvasHeight) {
                    uint32_t color = pixels[displayCanvasY * displayCanvasWidth + displayCanvasX];
                    if (color != excludeColor && color != 0x00FFFFFF && color != 0x00000000 && color != 0xFFFFFF00) {
                        colorFrequency[color]++;
                    }
                }
            }
        }
    }

    auto mostCommonColor = std::max_element(colorFrequency.begin(), colorFrequency.end(),
                                            [](const auto &a, const auto &b)
                                            { return a.second < b.second; });

    if (mostCommonColor != colorFrequency.end()) { return *mostCommonColor; }
    else { return std::make_pair(0, 0); }
}