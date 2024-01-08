#include "MixBoxPalette.h"

int windowWidth = 512;

int windowHeight = 512;

std::optional<int> previousX;

std::optional<int> previousY;

const int virtualCanvasWidth = 256;

const int virtualCanvasHeight = 256;

const int displayCanvasWidth = 1024;

const int displayCanvasHeight = 1024;

float zoom = 1;

bool isMouseButtonDown = false;

bool isPanning = false;

std::vector<Toolbar *> toolbars;

void sdlInit();

std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> sdlSetupWindow();

std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> sdlSetupRenderer(SDL_Window *window);

void drawUI(SDL_Renderer *renderer);

void pickColor(int x, int y, Canvas *canvas, SDL_Renderer *renderer, ColorPicker *colorPicker, Tool *colorPickerTool);

void handleMouseWheelEvent(SDL_Event &e, float &currentZoom, Canvas &canvas);

void handleMouseButtonDown(const std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> &renderer,
                           Tool *colorPickerTool,
                           const Toolbar &brushToolbar,
                           const SDL_Event &e,
                           ColorPicker &colorPicker,
                           Canvas &canvas);

void handleMouseMotion(const std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> &renderer,
                       Tool *colorPickerTool,
                       const Toolbar &brushToolbar,
                       const Toolbar &brushSizeToolbar,
                       SDL_Event &e,
                       ColorPicker &colorPicker,
                       Canvas &canvas);
int main(int argc, char *argv[])
{
    sdlInit();
    auto window = sdlSetupWindow();
    auto renderer = sdlSetupRenderer(window.get());
    std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)>
        font(TTF_OpenFont("assets/Roboto-Regular.ttf", 28), TTF_CloseFont);

    ColorPicker colorPicker(window.get(), font.get());
    Canvas canvas(renderer.get(),
                  virtualCanvasWidth,
                  virtualCanvasHeight,
                  displayCanvasWidth,
                  displayCanvasHeight,
                  windowWidth,
                  windowHeight);

    auto paintTool = new Tool(ToolType::Paint, "assets/paintIcon.png", renderer.get());
    auto blendTool = new Tool(ToolType::Blend, "assets/blendIcon.png", renderer.get());
    auto eyeDropperTool = new Tool(ToolType::EyeDropper, "assets/eyeDropper.png", renderer.get());
    auto colorPickerTool =
        new Tool(ToolType::ColorPicker, from_RGBColor(hsv_to_rgb(colorPicker.currentColor)), renderer.get());
    auto smallBrushTool = new Tool(ToolType::SmallBrush, "assets/smallBrushIcon.png", renderer.get());
    auto mediumBrushTool = new Tool(ToolType::MediumBrush, "assets/mediumBrushIcon.png", renderer.get());
    auto largeBrushTool = new Tool(ToolType::LargeBrush, "assets/largeBrushIcon.png", renderer.get());
    auto resetCanvasTool = new Tool(ToolType::ResetCanvas, "assets/clearIcon.png", renderer.get());

    Toolbar brushToolbar({paintTool, blendTool, eyeDropperTool}, Toolbar::Alignment::Left,
                         {0, windowHeight - 50, windowWidth, 50}, true, true);
    Toolbar colorPickerToolbar
        ({resetCanvasTool, colorPickerTool}, Toolbar::Alignment::Right, {0, windowHeight - 50, windowWidth, 50},
         false, false);
    Toolbar brushSizeToolbar({smallBrushTool, mediumBrushTool, largeBrushTool}, Toolbar::Alignment::Right,
                             {0, -25, windowWidth, 50}, false, true);
    brushSizeToolbar.currentTool = ToolType::MediumBrush;

    toolbars.push_back(&brushToolbar);
    toolbars.push_back(&colorPickerToolbar);
    toolbars.push_back(&brushSizeToolbar);

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;

                case SDL_MOUSEWHEEL:
                    handleMouseWheelEvent(e, zoom, canvas);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    isMouseButtonDown = true;
                    handleMouseButtonDown(renderer, colorPickerTool, brushToolbar, e, colorPicker, canvas);
                    break;

                case SDL_MOUSEBUTTONUP:
                    isMouseButtonDown = false;
                    isPanning = false;
                    previousX.reset();
                    previousY.reset();
                    canvas.firstBlendColor.reset();
                    canvas.secondBlendColor.reset();
                    break;

                case SDL_MOUSEMOTION:
                    handleMouseMotion(renderer,
                                      colorPickerTool,
                                      brushToolbar,
                                      brushSizeToolbar,
                                      e,
                                      colorPicker,
                                      canvas);
                    break;
            }
        }

        SDL_RenderClear(renderer.get());
        SDL_RenderCopy(renderer.get(), canvas.texture.get(), &canvas.srcRect, nullptr);
        drawUI(renderer.get());
        SDL_RenderPresent(renderer.get());
    }

    return 0;
}

void handleMouseWheelEvent(SDL_Event &e, float &currentZoom, Canvas &canvas)
{
    currentZoom *= (e.wheel.y < 0) ? 1.1 : 0.9;
    if (currentZoom < 0.1) currentZoom = 0.1;
    if (currentZoom > 1) currentZoom = 1;

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    canvas.setTextureZoom(currentZoom, mouseX, mouseY);
}

void handleMouseMotion(const std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> &renderer,
                       Tool *colorPickerTool,
                       const Toolbar &brushToolbar,
                       const Toolbar &brushSizeToolbar,
                       SDL_Event &e,
                       ColorPicker &colorPicker,
                       Canvas &canvas)
{
    SDL_SetCursor(SDL_CreateSystemCursor(
        brushToolbar.currentTool == ToolType::EyeDropper && e.button.y < windowHeight - 50 ? SDL_SYSTEM_CURSOR_CROSSHAIR
                                                                                           : SDL_SYSTEM_CURSOR_ARROW));
    if (isPanning) {
        if (previousX.has_value() && previousY.has_value()) {
            int deltaX = e.motion.x - previousX.value();
            int deltaY = e.motion.y - previousY.value();
            canvas.setTextureOffset(-deltaX, -deltaY);
        }
        previousX = e.motion.x;
        previousY = e.motion.y;
    }
    else if (isMouseButtonDown && brushToolbar.currentTool == ToolType::EyeDropper) {
        pickColor(e.motion.x, e.motion.y, &canvas, renderer.get(), &colorPicker, colorPickerTool);
    }
    else if (isMouseButtonDown) {
        RGBColor rgbColor = hsv_to_rgb(colorPicker.currentColor);
        uint32_t color = (static_cast<uint32_t>(rgbColor.r * 255) << 24) |
            (static_cast<uint32_t>(rgbColor.g * 255) << 16) |
            (static_cast<uint32_t>(rgbColor.b * 255) << 8) | 255;
        int currentX = e.motion.x / (windowWidth / virtualCanvasWidth);
        int currentY = e.motion.y / (windowHeight / virtualCanvasHeight);
        if (previousX.has_value() && previousY.has_value()) {
            canvas.setPixel(currentX,
                            currentY,
                            previousX.value(),
                            previousY.value(),
                            color,
                            static_cast<int>(brushToolbar.currentTool),
                            static_cast<int>(brushSizeToolbar.currentTool));
            canvas.rebuildHighResPixels(renderer.get());
        }
        previousX = currentX;
        previousY = currentY;
    }
}
void handleMouseButtonDown(const std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> &renderer,
                           Tool *colorPickerTool,
                           const Toolbar &brushToolbar,
                           const SDL_Event &e,
                           ColorPicker &colorPicker,
                           Canvas &canvas)
{
    if (e.button.button == SDL_BUTTON_LEFT) {
        for (auto t: toolbars) {
            if (t->hitTest(e.button.x, e.button.y)) {
                isMouseButtonDown = false;
                isPanning = false;
                if (t->currentTool == ToolType::ColorPicker) {
                    colorPicker.ShowPicker(colorPicker.currentColor);
                    dynamic_cast<Tool *>(colorPickerTool)
                        ->setColor(from_RGBColor(hsv_to_rgb(colorPicker.currentColor)), renderer.get());
                }
                if (t->currentTool == ToolType::ResetCanvas) {
                    canvas.resetCanvas(renderer.get());
                }
            }
        }
        if (isMouseButtonDown && brushToolbar.currentTool == ToolType::EyeDropper) {
            pickColor(e.motion.x, e.motion.y, &canvas, renderer.get(), &colorPicker, colorPickerTool);
        }
    }
    else if (e.button.button == SDL_BUTTON_RIGHT) {
        isPanning = true;
        previousX = e.button.x;
        previousY = e.button.y;
    }
}

void sdlInit()
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
}

std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> sdlSetupWindow()
{
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight,
                         SDL_WINDOW_BORDERLESS),
        SDL_DestroyWindow
    );

    SDL_SetWindowResizable(window.get(), SDL_TRUE);
    SDL_SetWindowAlwaysOnTop(window.get(), SDL_TRUE);

    SDL_SetWindowHitTest(window.get(), [](SDL_Window *win, const SDL_Point *area, void *data) -> SDL_HitTestResult
    {
        const int resize_border_size = 10;
        int w, h;
        SDL_GetWindowSize(win, &w, &h);
        if (area->x >= 0 && area->x <= 25 && area->y >= 0 && area->y <= 25 - area->x) { return SDL_HITTEST_DRAGGABLE; }
        return SDL_HITTEST_NORMAL;
    }, nullptr);

    return window;
}

std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> sdlSetupRenderer(SDL_Window *window)
{
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer(
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        SDL_DestroyRenderer
    );
    return renderer;
}

void drawUI(SDL_Renderer *renderer)
{
    for (auto toolbar: toolbars) { toolbar->draw(renderer); }

    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
    for (int y = 25; y >= 0; --y) { SDL_RenderDrawLine(renderer, 0, y, 25 - y, y); }
}

void pickColor(int x, int y, Canvas *canvas, SDL_Renderer *renderer, ColorPicker *colorPicker, Tool *colorPickerTool)
{
    uint32_t color = canvas->getPixel(x, y);
    if (color != 0) {
        RGBColor rgbColor{
            (float) ((color >> 24) & 0xFF) / 255.0f, // Red
            (float) ((color >> 16) & 0xFF) / 255.0f, // Green
            (float) ((color >> 8) & 0xFF) / 255.0f   // Blue
        };

        colorPicker->currentColor = rgb_to_hsv(rgbColor);
        colorPickerTool->setColor(from_RGBColor(rgbColor), renderer);
    }
}