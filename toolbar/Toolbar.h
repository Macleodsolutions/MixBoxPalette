#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "../tools/Tool.h"
#include <vector>
#include <SDL.h>

class Toolbar {
public:
    enum class Alignment { Left, Right };

    Toolbar(const std::vector<Tool*>& tools, Alignment alignment, SDL_Rect rect, bool drawBackground, bool drawHighlights);
    ~Toolbar();

    void draw(SDL_Renderer* renderer) const;
    bool hitTest(int mouseX, int mouseY);
    ToolType currentTool;
    std::vector<Tool*> tools;

private:
    Alignment alignment;
    SDL_Rect rect;
    static const int buttonWidth = 50;
    static const int buttonHeight = 50;
    bool drawBackground;
    bool drawHighlights;
};

#endif // TOOLBAR_H
