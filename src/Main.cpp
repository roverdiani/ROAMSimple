//  Main.cpp (SDL Version)
//  Bryan Turner (original version), Rodrigo Verdiani (SDL version)
//
//  Parts of the code in this file were borrowed from numerous public sources &
//  literature.  I reserve NO rights to this code and give a hearty thank-you to all the
//  excellent sources used in this project.  These include, but are not limited to:
//
//  Longbow Digital Arts Programming Forum (www.LongbowDigitalArts.com)
//  Gamasutra Features (www.Gamasutra.com)
//  GameDev References (www.GameDev.net)
//  C. Cookson's ROAM implementation (C.J.Cookson@dcs.warwick.ac.uk OR cjcookson@hotmail.com)
//  OpenGL Super Bible (Waite Group Press)
//  And many more...

#include <iostream>
#include <SDL.h>
#include <SDL_opengl.h>
#include <timeapi.h>

#include "Landscape.h"
#include "Utility.h"

SDL_Window *gWindow;
SDL_GLContext gGlContext;

bool isRunning = true;

void SDLKeyDown(SDL_Keysym *keysym)
{
    switch (keysym->sym)
    {
        case SDLK_w:
            KeyForward();
            break;
        case SDLK_a:
            KeyLeft();
            break;
        case SDLK_s:
            KeyBackward();
            break;
        case SDLK_d:
            KeyRight();
            break;

        case SDLK_f:
            KeyAnimateToggle();
            break;
        case SDLK_o:
            KeyObserveToggle();
            break;
        case SDLK_q:
            KeyDrawModeSurf();
            break;
        case SDLK_r:
            KeyDrawFrustumToggle();
            break;

        case SDLK_0:
            KeyMoreDetail();
            break;
        case SDLK_9:
            KeyLessDetail();
            break;

        case SDLK_1:
            KeyFOVDown();
            break;
        case SDLK_2:
            KeyFOVUp();
            break;

        case SDLK_UP:
            KeyUp();
            break;
        case SDLK_DOWN:
            KeyDown();
            break;

        case SDLK_ESCAPE:
            isRunning = false;
            break;

        default:
            break;
    }
}

void SDLMouseClick(SDL_MouseButtonEvent *event)
{
    if (event->button == SDL_BUTTON_LEFT)
    {
        if (event->state == SDL_PRESSED)
        {
            gRotating = 1;
            gStartX = -1;
        } else
            gRotating = 0;
    }
}

void SDLMouseMove(SDL_MouseMotionEvent *event)
{
    if (event->state & SDL_BUTTON(1))
        MouseMove(event->x, event->y);
}

bool InitSDL()
{
    std::cout << "Initializing SDL..." << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Could not initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    std::cout << "SDL initialized!" << std::endl;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    std::cout << "Creating SDL window..." << std::endl;

    gWindow = SDL_CreateWindow("ROAM Terrain View", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                               SDL_WINDOW_OPENGL);
    if (!gWindow)
    {
        std::cout << "Could not create SDL window: " << SDL_GetError() << std::endl;
        return false;
    }

    std::cout << "SDL window created!" << std::endl;
    std::cout << "Creating SDL GL Context..." << std::endl;

    gGlContext = SDL_GL_CreateContext(gWindow);
    if (!gGlContext)
    {
        std::cout << "Could not create a GL context: " << SDL_GetError() << std::endl;
        return false;
    }

    std::cout << "SDL GL Context created!" << std::endl;

    // Disable V-Sync
    //SDL_GL_SetSwapInterval(0);

    return true;
}

int main(int argc, char *argv[])
{
    InitSDL();

    // Setup OpenGL
    SetupRC();
    SetDrawModeContext();
    ChangeSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Load landscape data file
    loadTerrain(MAP_SIZE, &gHeightMap);

    int avgFrames = -1;
    if (roamInit(gHeightMap))
    {
        std::cout << "ROAM initialized." << std::endl;

        // Start the animation loop running.
        gAnimating = 1;

        // Get the start time in milliseconds
        gStartTime = timeGetTime();

        while (isRunning)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_KEYDOWN:
                        SDLKeyDown(&event.key.keysym);
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                        SDLMouseClick(&event.button);
                        break;
                    case SDL_MOUSEMOTION:
                        SDLMouseMove(&event.motion);
                        break;
                    case SDL_QUIT:
                        isRunning = false;
                        break;
                }
            }

            IdleFunction();
            RenderScene();

            // Copy image to window
            SDL_GL_SwapWindow(gWindow);
        }

        // Calculate the average number of frames per second.
        gEndTime = timeGetTime();
        avgFrames = (gNumFrames * 1000) / (gEndTime - gStartTime);
    }

    freeTerrain();

    std::cout << "Quitting SDL." << std::endl;
    SDL_Quit();

    std::cout << "Quitting." << std::endl;
    std::cout << "Average FPS: " << avgFrames << std::endl;

    return avgFrames;
}
