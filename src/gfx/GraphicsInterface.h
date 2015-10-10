/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <SDL2/SDL.h>

#include "GFXLibraryDefine.h"

namespace gfx
{
    class GFX_API IGraphics
    {
    public:
        virtual void InitializeWindow(int width, int height,int msaa , bool fullscreen, bool vsync) = 0;
        virtual void Swap() = 0;
        virtual SDL_Window* GetWindow() = 0;
        virtual void Draw() = 0;
    };
}
