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

#ifndef ROAMSDL_APP_H
#define ROAMSDL_APP_H

#include <SDL.h>
#include <SDL_opengl.h>
#include <chrono>
#include "Utility.h"

class App
{
public:
    void Init();
    void Shutdown();

    void Loop();

private:
    bool InitSDL();

    void SDLMouseMove(SDL_MouseMotionEvent *event);
    void SDLMouseClick(SDL_MouseButtonEvent *event);
    void SDLKeyDown(SDL_Keysym *keysym);

    void KeyObserveToggle();
    void KeyForward();
    void KeyLeft();
    void KeyBackward();
    void KeyRight();
    void KeyAnimateToggle();
    void KeyDrawFrustumToggle();
    void KeyUp();
    void KeyDown();
    void KeyFOVDown();
    void KeyFOVUp();
    static void KeyMoreDetail();
    static void KeyLessDetail();
    void KeyDrawModeSurface();

    void MouseMove(int mouseX, int mouseY);

    void DrawFrustum();

    void Update();
    void RenderScene();

    SDL_Window *m_Window = nullptr;
    SDL_GLContext m_GlContext = nullptr;

    bool m_IsRunning = true;

    int m_AvgFrames = -1;
    int m_numFrames = 0;

    int m_numTrisRendered = 0;

    bool m_isAnimating = false;
    bool m_isRotating = false;
    bool m_drawFrustum = true;
    int m_cameraMode = OBSERVE_MODE;
    int m_drawMode = DRAW_USE_TEXTURE;

    // Misc
    int gStartX = -1, gStartY = 0;

    // Camera Stuff
    GLfloat m_viewPosition[3] = {0.f, 5.f, 0.f};
    GLfloat m_cameraPosition[3] = {0.f, 0.f, -555.f};
    GLfloat m_cameraRotation[3] = {42.f, -181.f, 0.f};
    GLfloat m_animateAngle = 0.f;
    GLfloat m_clipAngle = 0.f;
    float m_fovX = 90.0f;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime, m_endTime;
};

#endif //ROAMSDL_APP_H
