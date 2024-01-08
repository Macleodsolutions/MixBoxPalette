#include "Toolbar.h"
Toolbar::Toolbar(const std::vector<Tool *> &tools,
                 Alignment alignment,
                 SDL_Rect rect,
                 bool drawBackground,
                 bool drawHighlights)
    : tools(tools), alignment(alignment), rect(rect), currentTool(ToolType::Paint), drawBackground(drawBackground),
      drawHighlights(drawHighlights)
{}

Toolbar::~Toolbar() = default;

void Toolbar::draw(SDL_Renderer *renderer) const
{
    if (drawBackground) {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Grey background
        SDL_RenderFillRect(renderer, &rect);
    }
    // Calculate starting X position based on alignment
    int startX;
    if (alignment == Alignment::Left) {
        startX = rect.x;
    }
    else { // Alignment::Right
        startX = rect.x + rect.w - static_cast<int>(tools.size()) * buttonWidth;
    }
    if (drawHighlights) {

        SDL_Rect
            highlightRect = {startX + static_cast<int>(currentTool) * buttonWidth, rect.y, buttonWidth, buttonHeight};

        if (alignment == Alignment::Right) {
            highlightRect.x = startX + (static_cast<int>(currentTool) * buttonWidth)
                - static_cast<int>(tools.size() + 1) * buttonWidth;
        }

        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); // Light blue for highlight
        SDL_RenderFillRect(renderer, &highlightRect);
    }

    // Draw each tool icon with alignment
    for (size_t i = 0; i < tools.size(); ++i) {
        if (tools[i]->icon != nullptr) {
            SDL_Rect iconRect = {startX + static_cast<int>(i) * buttonWidth, rect.y, buttonWidth, buttonHeight};
            SDL_RenderCopy(renderer, tools[i]->icon.value(), nullptr, &iconRect);
        }
    }
}

bool Toolbar::hitTest(int mouseX, int mouseY)
{
    if (mouseX >= rect.x && mouseX <= rect.x + rect.w &&
        mouseY >= rect.y && mouseY <= rect.y + rect.h) {

        int startX;
        int totalButtonsWidth = static_cast<int>(tools.size()) * buttonWidth;
        if (alignment == Alignment::Left) { startX = rect.x; }
        else { startX = rect.x + rect.w - totalButtonsWidth; }
        if (mouseX < startX) { return false; }
        int buttonIndex = (mouseX - startX) / buttonWidth;
        if (buttonIndex >= 0 && buttonIndex < static_cast<int>(tools.size())) {
            currentTool = tools[buttonIndex]->id;
            return true;
        }
        return true;
    }
    return false;
}