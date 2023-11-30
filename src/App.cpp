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
#include "App.h"
#include "Utility.h"
#include "Roam.h"

Roam roam;

void App::Init()
{
    InitSDL();

    // Setup OpenGL
    Utility::SetupRC();
    Utility::SetDrawModeContext(m_drawMode);
    Utility::ChangeSize(WINDOW_WIDTH, WINDOW_HEIGHT, m_fovX);

    if (!roam.Init())
        return;

    // Load landscape data file
    roam.LoadTerrain();

    std::cout << "ROAM initialized." << std::endl;

    // Get the start time in milliseconds
    m_startTime = std::chrono::high_resolution_clock::now();

    m_animateAngle += 0.4f;
    m_viewPosition[0] = ((GLfloat) MAP_SIZE / 4.f) + ((sinf(m_animateAngle * M_PI / 180.f) + 1.f) * ((GLfloat) MAP_SIZE / 4.f));
    m_viewPosition[2] = ((GLfloat) MAP_SIZE / 4.f) + ((cosf(m_animateAngle * M_PI / 180.f) + 1.f) * ((GLfloat) MAP_SIZE / 4.f));
}

bool App::InitSDL()
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

    m_Window = SDL_CreateWindow("ROAM Terrain View", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                               SDL_WINDOW_OPENGL);
    if (!m_Window)
    {
        std::cout << "Could not create SDL window: " << SDL_GetError() << std::endl;
        return false;
    }

    std::cout << "SDL window created!" << std::endl;
    std::cout << "Creating SDL GL Context..." << std::endl;

    m_GlContext = SDL_GL_CreateContext(m_Window);
    if (!m_GlContext)
    {
        std::cout << "Could not create a GL context: " << SDL_GetError() << std::endl;
        return false;
    }

    std::cout << "SDL GL Context created!" << std::endl;

    // Disable V-Sync
    SDL_GL_SetSwapInterval(0);

    return true;
}

void App::Shutdown()
{
    // Calculate the average number of frames per second.
    m_endTime = std::chrono::high_resolution_clock::now();
    m_AvgFrames = (int) ((m_numFrames * 1000) / std::chrono::duration_cast<std::chrono::milliseconds>(m_endTime - m_startTime).count());

    roam.FreeTerrain();

    std::cout << "Quitting SDL." << std::endl;
    SDL_Quit();

    std::cout << "Quitting." << std::endl;
    std::cout << "Average FPS: " << m_AvgFrames << std::endl;
}

void App::Loop()
{
    while (m_IsRunning)
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
                    m_IsRunning = false;
                    break;
            }
        }

        Update();
        RenderScene();

        // Copy image to window
        SDL_GL_SwapWindow(m_Window);
    }
}

void App::SDLMouseMove(SDL_MouseMotionEvent *event)
{
    if (!(event->state & SDL_BUTTON(1)))
        return;

    // If we aren't rotating, skip the updates needed
    if (!m_isRotating)
        return;

    int dx, dy;
    int mouseX = event->x;
    int mouseY = event->y;

    // Check for start flag
    if (gStartX == -1)
    {
        gStartX = mouseX;
        gStartY = mouseY;
    }

    // Find the delta of the mouse
    dx = mouseX - gStartX;
    dy = mouseY - gStartY;

    // Update the camera rotations
    m_cameraRotation[0] += (GLfloat) dy * 0.5f;
    m_cameraRotation[1] += (GLfloat) dx * 0.5f;

    // Reset the deltas for the next idle call
    gStartX = mouseX;
    gStartY = mouseY;
}

void App::SDLMouseClick(SDL_MouseButtonEvent *event)
{
    if (event->button == SDL_BUTTON_LEFT)
    {
        if (event->state == SDL_PRESSED)
        {
            m_isRotating = true;
            gStartX = -1;
        } else
            m_isRotating = false;
    }
}

void App::SDLKeyDown(SDL_Keysym *keysym)
{
    switch (keysym->sym)
    {
        case SDLK_w:
            m_cameraPosition[2] += 5.0f;
            break;
        case SDLK_a:
            m_cameraRotation[1] -= 5.0f;
            break;
        case SDLK_s:
            m_cameraPosition[2] -= 5.0f;
            break;
        case SDLK_d:
            m_cameraRotation[1] += 5.0f;
            break;

        case SDLK_q:
            KeyDrawModeSurface();
            break;

        case SDLK_0:
            roam.IncreaseDetail();
            break;
        case SDLK_9:
            roam.DecreaseDetail();
            break;

        case SDLK_UP:
            m_cameraPosition[1] -= 5.0f;
            break;
        case SDLK_DOWN:
            m_cameraPosition[1] += 5.0f;
            break;

        case SDLK_ESCAPE:
            m_IsRunning = false;
            break;

        default:
            break;
    }
}

void App::Update()
{
}

// Called to update the window
void App::RenderScene()
{
    // Clear the GL buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Face the center of the map from a certain distance & allow user to orbit around the midpoint.
    // Move to the Camera Position
    glTranslatef(0.f, m_cameraPosition[1], m_cameraPosition[2]);

    // Rotate to the Camera Rotation angle
    glRotatef(m_cameraRotation[ROTATE_PITCH], 1.f, 0.f, 0.f);
    glRotatef(m_cameraRotation[ROTATE_YAW], 0.f, 1.f, 0.f);

    // Adjust the origin to be the center of the map...
    glTranslatef(-((GLfloat) MAP_SIZE * 0.5f), 0.f, -((GLfloat) MAP_SIZE * 0.5f));

    m_clipAngle = -m_animateAngle;

    // Perform the actual rendering of the mesh.
    // Reset rendered triangle count.
    m_numTrisRendered = 0;
    roam.Draw(m_viewPosition, m_clipAngle, m_fovX, m_drawMode, m_numTrisRendered);

    glPopMatrix();

    // Increment the frame counter.
    m_numFrames++;
}

void App::KeyDrawModeSurface()
{
    m_drawMode++;
    if (m_drawMode > DRAW_USE_WIREFRAME)
        m_drawMode = DRAW_USE_TEXTURE;

    Utility::SetDrawModeContext(m_drawMode);
}
