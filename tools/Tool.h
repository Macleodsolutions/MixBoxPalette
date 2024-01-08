#ifndef TOOL_H
#define TOOL_H

#include <SDL.h>
#include <optional>
#include <string>
#include <SDL_image.h>
#include <iostream>

enum class ToolType {
    Paint,
    Blend,
    EyeDropper,
    ColorPicker,
    SmallBrush,
    MediumBrush,
    LargeBrush,
    ResetCanvas
};

class Tool {
public:
    Tool(ToolType type, const std::string& iconPath, SDL_Renderer*);
    Tool(ToolType type, SDL_Color color, SDL_Renderer*);
    ~Tool();
    void setColor(SDL_Color color, SDL_Renderer*);

public:
    ToolType id;
    std::string iconPath;
    SDL_Color iconColor = {0, 0, 0 ,0};
    std::optional<SDL_Texture*> icon;
};

#endif // TOOL_H
