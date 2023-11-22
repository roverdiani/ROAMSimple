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

class App
{
public:
    void Init();
    void Shutdown();

    void Loop();

private:
    bool InitSDL();

    static void SDLMouseMove(SDL_MouseMotionEvent *event);
    static void SDLMouseClick(SDL_MouseButtonEvent *event);
    void SDLKeyDown(SDL_Keysym *keysym);

    SDL_Window *m_Window = nullptr;
    SDL_GLContext m_GlContext = nullptr;

    bool m_IsRunning = true;

    int m_AvgFrames = -1;
};

#endif //ROAMSDL_APP_H
