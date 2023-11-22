//  Utility.h
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

#include <chrono>

// Defines
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Globals
extern std::chrono::time_point<std::chrono::high_resolution_clock> gStartTime, gEndTime;
extern int gNumFrames;
extern unsigned char* gHeightMap;
extern int gAnimating;
extern int gRotating;
extern int gStartX, gStartY;

// Functions
extern void loadTerrain(int size, unsigned char **dest);
extern void freeTerrain();
extern void SetDrawModeContext();
extern bool roamInit(unsigned char* map);
extern void roamDrawFrame();
extern void drawFrustum();

extern void KeyObserveToggle();
extern void KeyDrawModeSurf();
extern void KeyForward();
extern void KeyLeft();
extern void KeyBackward();
extern void KeyRight();
extern void KeyAnimateToggle();
extern void KeyDrawFrustumToggle();
extern void KeyUp();
extern void KeyDown();
extern void KeyMoreDetail();
extern void KeyLessDetail();
extern void ChangeSize(GLsizei w, GLsizei h);
extern void KeyFOVDown();
extern void KeyFOVUp();

extern void RenderScene();
extern void IdleFunction();
extern void MouseMove(int mouseX, int mouseY);
extern void SetupRC();

class Utility
{
public:
    static void ReduceToUnit(float vector[3]);
    static void CalcNormal(float v[3][3], float out[3]);

    // Discover the orientation of a triangle's points:
    // Taken from "Programming Principles in Computer Graphics", L. Ammeraal (Wiley)
    static inline int Orientation(int pX, int pY, int qX, int qY, int rX, int rY)
    {
        int aX = qX - pX;
        int aY = qY - pY;

        int bX = rX - pX;
        int bY = rY - pY;

        float d = (float) aX * (float) bY - (float) aY * (float) bX;
        return (d < 0) ? (-1) : (d > 0);
    }
};
