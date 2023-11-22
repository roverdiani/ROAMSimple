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

    // Start the animation loop running.
    m_isAnimating = true;

    // Get the start time in milliseconds
    gStartTime = std::chrono::high_resolution_clock::now();
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
    //SDL_GL_SetSwapInterval(0);

    return true;
}

void App::Shutdown()
{
    // Calculate the average number of frames per second.
    gEndTime = std::chrono::high_resolution_clock::now();
    m_AvgFrames = (int) ((m_numFrames * 1000) / std::chrono::duration_cast<std::chrono::milliseconds>(gEndTime - gStartTime).count());

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
    if (event->state & SDL_BUTTON(1))
        MouseMove(event->x, event->y);
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
            KeyDrawModeSurface();
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
            m_IsRunning = false;
            break;

        default:
            break;
    }
}

void App::Update()
{
// If animating, move the view position
    if (m_isAnimating)
    {
        m_animateAngle += 0.4f;

        m_viewPosition[0] = ((GLfloat) MAP_SIZE / 4.f) + ((sinf(m_animateAngle * M_PI / 180.f) + 1.f) * ((GLfloat) MAP_SIZE / 4.f));
        m_viewPosition[2] = ((GLfloat) MAP_SIZE / 4.f) + ((cosf(m_animateAngle * M_PI / 180.f) + 1.f) * ((GLfloat) MAP_SIZE / 4.f));

        //gViewPosition[1] = (MULT_SCALE * gLand.heightMap[(int) gViewPosition[0] + ((int) gViewPosition[2] * MAP_SIZE)]) + 4.0f;
        m_isAnimating = false;
    }
}

// Called to update the window
void App::RenderScene()
{
    // Clear the GL buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    switch (m_cameraMode)
    {
        default:
        case FOLLOW_MODE:
            // Face the center of the map, while walking in a circle around the midpoint (animated).
            glRotatef(-m_animateAngle, 0.f, 1.f, 0.f);
            glTranslatef(-m_viewPosition[0], -m_viewPosition[1], -m_viewPosition[2]);

            m_clipAngle = -m_animateAngle;
            break;

        case OBSERVE_MODE:

            // Face the center of the map from a certain distance & allow user to orbit around the midpoint.
            //
            // Move to the Camera Position
            glTranslatef(0.f, m_cameraPosition[1], m_cameraPosition[2]);

            // Rotate to the Camera Rotation angle
            glRotatef(m_cameraRotation[ROTATE_PITCH], 1.f, 0.f, 0.f);
            glRotatef(m_cameraRotation[ROTATE_YAW], 0.f, 1.f, 0.f);

            // Adjust the origin to be the center of the map...
            glTranslatef(-((GLfloat) MAP_SIZE * 0.5f), 0.f, -((GLfloat) MAP_SIZE * 0.5f));

            m_clipAngle = -m_animateAngle;
            break;

        case DRIVE_MODE:
        case FLY_MODE:
            m_isAnimating = false;

            // Rotate to the Camera Rotation angle
            glRotatef(m_cameraRotation[ROTATE_PITCH], 1.f, 0.f, 0.f);
            glRotatef(m_cameraRotation[ROTATE_YAW], 0.f, 1.f, 0.f);

            // Move to the Camera Position
            glTranslatef(-m_viewPosition[0], -m_viewPosition[1], -m_viewPosition[2]);

            m_clipAngle = m_cameraRotation[ROTATE_YAW];
            break;
    }

    // Perform the actual rendering of the mesh.
    // Reset rendered triangle count.
    m_numTrisRendered = 0;
    roam.Draw(m_viewPosition, m_clipAngle, m_fovX, m_drawMode, m_numTrisRendered);

    if (m_drawFrustum)
        DrawFrustum();

    glPopMatrix();

    // Increment the frame counter.
    m_numFrames++;
}

void App::KeyObserveToggle()
{
    m_cameraMode++;
    if (m_cameraMode > FLY_MODE)
        m_cameraMode = FOLLOW_MODE;

    // Turn animation back on...
    if (m_cameraMode == FOLLOW_MODE)
        m_isAnimating = true;
}

void App::KeyForward()
{
    switch (m_cameraMode)
    {
        default:
        case FOLLOW_MODE:
            break;

        case OBSERVE_MODE:
            m_cameraPosition[2] += 5.0f;
            break;

        case DRIVE_MODE:
            m_viewPosition[0] += 5.0f * sinf(m_cameraRotation[ROTATE_YAW] * M_PI / 180.0f);
            m_viewPosition[2] -= 5.0f * cosf(m_cameraRotation[ROTATE_YAW] * M_PI / 180.0f);

            if (m_viewPosition[0] > MAP_SIZE)
                m_viewPosition[0] = MAP_SIZE;
            if (m_viewPosition[0] < 0)
                m_viewPosition[0] = 0;

            if (m_viewPosition[2] > MAP_SIZE)
                m_viewPosition[2] = MAP_SIZE;
            if (m_viewPosition[2] < 0)
                m_viewPosition[2] = 0;

            m_viewPosition[1] = (MULT_SCALE * roam.GetHeightMap()[(int) m_viewPosition[0] + ((int) m_viewPosition[2] * MAP_SIZE)]) + 4.0f;
            break;

        case FLY_MODE:
            m_viewPosition[0] +=
                    5.0f * sinf(m_cameraRotation[ROTATE_YAW] * M_PI / 180.0f) * cosf(m_cameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            m_viewPosition[2] -=
                    5.0f * cosf(m_cameraRotation[ROTATE_YAW] * M_PI / 180.0f) * cosf(m_cameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            m_viewPosition[1] -= 5.0f * sinf(m_cameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            break;
    }
}

void App::KeyLeft()
{
    if (m_cameraMode == OBSERVE_MODE)
        m_cameraRotation[1] -= 5.0f;
}

void App::KeyBackward()
{
    switch (m_cameraMode)
    {
        default:
        case FOLLOW_MODE:
            break;

        case OBSERVE_MODE:
            m_cameraPosition[2] -= 5.0f;
            break;

        case DRIVE_MODE:
            m_viewPosition[0] -= 5.0f * sinf(m_cameraRotation[ROTATE_YAW] * M_PI / 180.0f);
            m_viewPosition[2] += 5.0f * cosf(m_cameraRotation[ROTATE_YAW] * M_PI / 180.0f);

            if (m_viewPosition[0] > MAP_SIZE)
                m_viewPosition[0] = MAP_SIZE;
            if (m_viewPosition[0] < 0)
                m_viewPosition[0] = 0;

            if (m_viewPosition[2] > MAP_SIZE)
                m_viewPosition[2] = MAP_SIZE;
            if (m_viewPosition[2] < 0)
                m_viewPosition[2] = 0;

            m_viewPosition[1] = (MULT_SCALE * roam.GetHeightMap()[(int) m_viewPosition[0] + ((int) m_viewPosition[2] * MAP_SIZE)]) + 4.0f;
            break;

        case FLY_MODE:
            m_viewPosition[0] -=
                    5.0f * sinf(m_cameraRotation[ROTATE_YAW] * M_PI / 180.0f) * cosf(m_cameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            m_viewPosition[2] +=
                    5.0f * cosf(m_cameraRotation[ROTATE_YAW] * M_PI / 180.0f) * cosf(m_cameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            m_viewPosition[1] += 5.0f * sinf(m_cameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            break;
    }
}

void App::KeyRight()
{
    if (m_cameraMode == OBSERVE_MODE)
        m_cameraRotation[1] += 5.0f;
}

void App::KeyAnimateToggle()
{
    m_isAnimating = !m_isAnimating;
}

void App::KeyDrawFrustumToggle()
{
    m_drawFrustum = !m_drawFrustum;
}

void App::KeyUp()
{
    if (m_cameraMode == OBSERVE_MODE)
        m_cameraPosition[1] -= 5.0f;
}

void App::KeyDown()
{
    if (m_cameraMode == OBSERVE_MODE)
        m_cameraPosition[1] += 5.0f;
}

void App::KeyMoreDetail()
{
    roam.IncreaseDetail();
}

void App::KeyLessDetail()
{
    roam.DecreaseDetail();
}

void App::KeyFOVDown()
{
    m_fovX -= 1.0f;
    Utility::ChangeSize(WINDOW_WIDTH, WINDOW_HEIGHT, m_fovX);
}

void App::KeyFOVUp()
{
    m_fovX += 1.0f;
    Utility::ChangeSize(WINDOW_WIDTH, WINDOW_HEIGHT, m_fovX);
}

// Called when user moves the mouse
void App::MouseMove(int mouseX, int mouseY)
{
    // If we're rotating, perform the updates needed
    if (m_isRotating && (m_cameraMode != FOLLOW_MODE))
    {
        int dx, dy;

        // Check for start flag
        if (gStartX == -1)
        {
            gStartX = mouseX;
            gStartY = mouseY;
        }

        // Find the delta of the mouse
        dx = mouseX - gStartX;

        if (m_cameraMode == OBSERVE_MODE)
            dy = mouseY - gStartY;
        else
            dy = gStartY - mouseY; // Invert mouse in Drive/Fly mode

        // Update the camera rotations
        m_cameraRotation[0] += (GLfloat) dy * 0.5f;
        m_cameraRotation[1] += (GLfloat) dx * 0.5f;

        // Reset the deltas for the next idle call
        gStartX = mouseX;
        gStartY = mouseY;
    }
}

void App::KeyDrawModeSurface()
{
    m_drawMode++;
    if (m_drawMode > DRAW_USE_WIREFRAME)
        m_drawMode = DRAW_USE_TEXTURE;

    Utility::SetDrawModeContext(m_drawMode);
}

// Draw a simplistic frustum for debug purposes.
void App::DrawFrustum()
{
    //
    // Draw the camera eye & frustum
    //
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glPointSize(5.f);
    glLineWidth(3.f);

    glBegin(GL_LINES);

    // Draw the View Vector starting at the eye (red)
    glColor3f(1, 0, 0);
    glVertex3f(m_viewPosition[0], m_viewPosition[1], m_viewPosition[2]);

    glVertex3f(m_viewPosition[0] + 50.0f * sinf(m_clipAngle * M_PI / 180.0f), m_viewPosition[1],
               m_viewPosition[2] - 50.0f * cosf(m_clipAngle * M_PI / 180.0f));

    // Draw the view frustum (blue)
    glColor3f(0, 0, 1);
    glVertex3f(m_viewPosition[0], m_viewPosition[1], m_viewPosition[2]);
    glVertex3f(m_viewPosition[0] + 1000.0f * sinf((m_clipAngle - 45.0f) * M_PI / 180.0f), m_viewPosition[1],
               m_viewPosition[2] - 1000.0f * cosf((m_clipAngle - 45.0f) * M_PI / 180.0f));
    glVertex3f(m_viewPosition[0], m_viewPosition[1], m_viewPosition[2]);
    glVertex3f(m_viewPosition[0] + 1000.0f * sinf((m_clipAngle + 45.0f) * M_PI / 180.0f), m_viewPosition[1],
               m_viewPosition[2] - 1000.0f * cosf((m_clipAngle + 45.0f) * M_PI / 180.0f));

    // Draw the clipping planes behind the eye (yellow)
    const float PI_DIV_180 = M_PI / 180.0f;
    const float FOV_DIV_2 = m_fovX / 2;

    int ptEyeX = (int) (m_viewPosition[0] - PATCH_SIZE * sinf(m_clipAngle * PI_DIV_180));
    int ptEyeY = (int) (m_viewPosition[2] + PATCH_SIZE * cosf(m_clipAngle * PI_DIV_180));

    int ptLeftX = (int) (ptEyeX + 100.0f * sinf((m_clipAngle - FOV_DIV_2) * PI_DIV_180));
    int ptLeftY = (int) (ptEyeY - 100.0f * cosf((m_clipAngle - FOV_DIV_2) * PI_DIV_180));

    int ptRightX = (int) (ptEyeX + 100.0f * sinf((m_clipAngle + FOV_DIV_2) * PI_DIV_180));
    int ptRightY = (int) (ptEyeY - 100.0f * cosf((m_clipAngle + FOV_DIV_2) * PI_DIV_180));

    glColor3f(1, 1, 0);
    glVertex3f((float) ptEyeX, m_viewPosition[1], (float) ptEyeY);
    glVertex3f((float) ptLeftX, m_viewPosition[1], (float) ptLeftY);
    glVertex3f((float) ptEyeX, m_viewPosition[1], (float) ptEyeY);
    glVertex3f((float) ptRightX, m_viewPosition[1], (float) ptRightY);

    glEnd();

    glLineWidth(1.f);
    glColor3f(1, 1, 1);

    Utility::SetDrawModeContext(m_drawMode);
}
