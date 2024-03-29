
// Defines...
#define WINDOW_WIDTH  (640)
#define WINDOW_HEIGHT (480)

// Globals...
extern long gStartTime, gEndTime;
extern int gNumFrames;
extern unsigned char *gHeightMap;
extern int gAnimating;
extern int gRotating;
extern int gStartX, gStartY;

// Functions...
extern void ReduceToUnit(float vector[3]);
extern void calcNormal(float v[3][3], float out[3]);
extern void loadTerrain(int size, unsigned char **dest);
extern void freeTerrain();
extern void SetDrawModeContext();
extern int roamInit(unsigned char *map);
extern void roamDrawFrame();
extern void drawFrustum();

extern void KeyObserveToggle(void);
extern void KeyDrawModeSurf(void);
extern void KeyForward(void);
extern void KeyLeft(void);
extern void KeyBackward(void);
extern void KeyRight(void);
extern void KeyAnimateToggle(void);
extern void KeyDrawFrustumToggle(void);
extern void KeyUp(void);
extern void KeyDown(void);
extern void KeyMoreDetail(void);
extern void KeyLessDetail(void);
extern void ChangeSize(GLsizei w, GLsizei h);
extern void KeyFOVDown(void);
extern void KeyFOVUp(void);

extern void RenderScene(void);
extern void IdleFunction(void);
extern void MouseMove(int mouseX, int mouseY);
extern void SetupRC();
