#include "Tool.h"

const int BUTTON_WIDTH = 50;

const int BUTTON_HEIGHT = 50;

// Constructor with iconPath
Tool::Tool(ToolType type, const std::string &iconPath, SDL_Renderer *renderer)
    : id(type), iconPath(iconPath)
{
    // Load the icon texture
    if (!iconPath.empty()) {
        icon = IMG_LoadTexture(renderer, iconPath.c_str());
        if (!icon) {
            std::cerr << "Failed to load texture: " << iconPath << " - SDL_Error: " << SDL_GetError() << std::endl;
        }
    }
}

// Constructor with color
Tool::Tool(ToolType type, SDL_Color color, SDL_Renderer *renderer)
    : id(type), iconColor(color)
{
    // Create a texture from the icon color
    if (iconColor.a != 0) {
        SDL_Surface *surface = SDL_CreateRGBSurface(0, BUTTON_WIDTH, BUTTON_HEIGHT, 32, 0, 0, 0, 0);
        SDL_FillRect(surface,
                     nullptr,
                     SDL_MapRGBA(surface->format, iconColor.r, iconColor.g, iconColor.b, iconColor.a));
        icon = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!icon) {
            std::cerr << "Failed to create texture from surface - SDL_Error: " << SDL_GetError() << std::endl;
        }
    }
}

Tool::~Tool()
{
    if (icon) {
        SDL_DestroyTexture(*icon);
        icon.reset();
    }
}

void Tool::setColor(SDL_Color color, SDL_Renderer *renderer)
{
    iconColor = color;
    if (iconColor.a != 0) {
        SDL_Surface *surface = SDL_CreateRGBSurface(0, BUTTON_WIDTH, BUTTON_HEIGHT, 32, 0, 0, 0, 0);
        SDL_FillRect(surface,
                     nullptr,
                     SDL_MapRGBA(surface->format, iconColor.r, iconColor.g, iconColor.b, iconColor.a));
        icon = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!icon) {
            std::cerr << "Failed to create texture from surface - SDL_Error: " << SDL_GetError() << std::endl;
        }
    }
}
