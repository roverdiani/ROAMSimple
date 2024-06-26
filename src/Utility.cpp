//  Utility.cpp
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

#include <SDL.h>
#include <SDL_opengl.h>
#if defined (__APPLE__) && defined(__MACH__)
#   include <OpenGL/glu.h>
#else
#   include <GL/glu.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "Utility.h"
#include "Landscape.h"

// Observer and Follower modes
enum Modes
{
    FOLLOW_MODE = 0,
    OBSERVE_MODE,
    DRIVE_MODE,
    FLY_MODE
};

// Perspective & Window defines
#define FOV_ANGLE 90.0f
#define NEAR_CLIP 1.0f
#define FAR_CLIP 2500.0f

// --------------------------------------
// GLOBALS
// --------------------------------------
Landscape gLand;

// Texture
GLuint gTextureID = 1;

// Camera Stuff
GLfloat gViewPosition[] = {0.f, 5.f, 0.f};
GLfloat gCameraPosition[] = {0.f, 0.f, -555.f};
GLfloat gCameraRotation[] = {42.f, -181.f, 0.f};
GLfloat gAnimateAngle = 0.f;
GLfloat gClipAngle;

// Misc. Globals
int gAnimating = 0;
int gRotating = 0;
int gDrawFrustum = 1;
int gCameraMode = OBSERVE_MODE;
int gDrawMode = DRAW_USE_TEXTURE;
int gStartX = -1, gStartY;
int gNumTrisRendered;
std::chrono::time_point<std::chrono::high_resolution_clock> gStartTime, gEndTime;
unsigned char *gHeightMap;
unsigned char *gHeightMaster;
int gNumFrames;
float gFovX = 90.0f;

// Beginning frame variance (should be high, it will adjust automatically)
float gFrameVariance = 50;

// Desired number of Binary Triangle tessellations per frame.
// This is not the desired number of triangles rendered!
// There are usually twice as many Binary Triangle structures as there are rendered triangles.
int gDesiredTris = 10000;

// Load the Height Field from a data file
void loadTerrain(int size, unsigned char **dest)
{
    // Optimization:  Add an extra row above and below the height map.
    //   - The extra top row contains a copy of the last row in the height map.
    //   - The extra bottom row contains a copy of the first row in the height map.
    // This simplifies the wrapping of height values to a trivial case.
    gHeightMaster = (unsigned char *) malloc(size * size * sizeof(unsigned char) + size * 2);

    // Give the rest of the application a pointer to the actual start of the height map.
    *dest = gHeightMaster + size;

    char fileName[30];
    sprintf(fileName, "Height%d.raw", size);
    FILE *fp = fopen(fileName, "rb");

    // TESTING: READ A TREAD MARKS MAP...
    if (!fp)
    {
        sprintf(fileName, "Map.ved");
        fp = fopen(fileName, "rb");
        if (fp)
        {
            fseek(fp, 40, SEEK_SET);    // Skip to the goods...
            std::cout << "Tread Marks Map file found: " << fileName << std::endl;
        }
    } else
        std::cout << "Raw Map file found: " << fileName << std::endl;

    if (!fp)
    {
        // Oops!  Couldn't find the file.
        std::cout << "No Map file found." << std::endl;

        // Clear the board.
        memset(gHeightMaster, 0, size * size + size * 2);
        return;
    }
    fread(gHeightMaster + size, 1, (size * size), fp);
    fclose(fp);

    // Copy the last row of the height map into the extra first row.
    memcpy(gHeightMaster, gHeightMaster + size * size, size);

    // Copy the first row of the height map into the extra last row.
    memcpy(gHeightMaster + size * size + size, gHeightMaster + size, size);
}

// Free the Height Field array
void freeTerrain()
{
    if (gHeightMaster)
        free(gHeightMaster);
}

// Switch GL Contexts when moving between draw modes to improve performance.
void SetDrawModeContext()
{
    switch (gDrawMode)
    {
        case DRAW_USE_TEXTURE:
            glDisable(GL_LIGHTING);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glPolygonMode(GL_FRONT, GL_FILL);
            break;

        case DRAW_USE_LIGHTING:
            glEnable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glPolygonMode(GL_FRONT, GL_FILL);
            break;

        case DRAW_USE_FILL_ONLY:
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glPolygonMode(GL_FRONT, GL_FILL);
            break;

        default:
        case DRAW_USE_WIREFRAME:
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glPolygonMode(GL_FRONT, GL_LINE);
            break;
    }
}

// Initialize the ROAM implementation
bool roamInit(unsigned char *map)
{
    // Perform some bounds checking on the #define statements
    if (gDesiredTris > POOL_SIZE)
        return false;

    if (POOL_SIZE < 100)
        return false;

    // TEXTURE INITIALIZATION
    glBindTexture(GL_TEXTURE_2D, gTextureID);

    unsigned char *pTexture = (unsigned char *) malloc(TEXTURE_SIZE * TEXTURE_SIZE * 3);
    unsigned char *pTexWalk = pTexture;

    if (!pTexture)
        return false;

    // Create a random stipple pattern for the texture.  Only use the Green channel.
    // This could easily be modified to load in a bitmap or other texture source.
    for (int x = 0; x < TEXTURE_SIZE; x++)
    {
        for (int y = 0; y < TEXTURE_SIZE; y++)
        {
            int color = (int) (128.0 + (40.0 * rand()) / RAND_MAX);
            if (color > 255)
                color = 255;
            if (color < 0)
                color = 0;

            *(pTexWalk++) = 0;
            *(pTexWalk++) = color;    // Only use the Green chanel.
            *(pTexWalk++) = 0;
        }
    }

    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TEXTURE_SIZE, TEXTURE_SIZE, GL_RGB, GL_UNSIGNED_BYTE, pTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    free(pTexture);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Landscape Initialization
    gLand.Init(map);

    return true;
}

// Call all functions needed to draw a frame of the landscape
void roamDrawFrame()
{
    // Perform all the functions needed to render one frame.
    gLand.Reset();
    gLand.Tessellate();
    gLand.Render();
}

// Draw a simplistic frustum for debug purposes.
void drawFrustum()
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
    glVertex3f(gViewPosition[0], gViewPosition[1], gViewPosition[2]);

    glVertex3f(gViewPosition[0] + 50.0f * sinf(gClipAngle * M_PI / 180.0f), gViewPosition[1],
               gViewPosition[2] - 50.0f * cosf(gClipAngle * M_PI / 180.0f));

    // Draw the view frustum (blue)
    glColor3f(0, 0, 1);
    glVertex3f(gViewPosition[0], gViewPosition[1], gViewPosition[2]);
    glVertex3f(gViewPosition[0] + 1000.0f * sinf((gClipAngle - 45.0f) * M_PI / 180.0f), gViewPosition[1],
               gViewPosition[2] - 1000.0f * cosf((gClipAngle - 45.0f) * M_PI / 180.0f));
    glVertex3f(gViewPosition[0], gViewPosition[1], gViewPosition[2]);
    glVertex3f(gViewPosition[0] + 1000.0f * sinf((gClipAngle + 45.0f) * M_PI / 180.0f), gViewPosition[1],
               gViewPosition[2] - 1000.0f * cosf((gClipAngle + 45.0f) * M_PI / 180.0f));

    // Draw the clipping planes behind the eye (yellow)
    const float PI_DIV_180 = M_PI / 180.0f;
    const float FOV_DIV_2 = gFovX / 2;

    int ptEyeX = (int) (gViewPosition[0] - PATCH_SIZE * sinf(gClipAngle * PI_DIV_180));
    int ptEyeY = (int) (gViewPosition[2] + PATCH_SIZE * cosf(gClipAngle * PI_DIV_180));

    int ptLeftX = (int) (ptEyeX + 100.0f * sinf((gClipAngle - FOV_DIV_2) * PI_DIV_180));
    int ptLeftY = (int) (ptEyeY - 100.0f * cosf((gClipAngle - FOV_DIV_2) * PI_DIV_180));

    int ptRightX = (int) (ptEyeX + 100.0f * sinf((gClipAngle + FOV_DIV_2) * PI_DIV_180));
    int ptRightY = (int) (ptEyeY - 100.0f * cosf((gClipAngle + FOV_DIV_2) * PI_DIV_180));

    glColor3f(1, 1, 0);
    glVertex3f((float) ptEyeX, gViewPosition[1], (float) ptEyeY);
    glVertex3f((float) ptLeftX, gViewPosition[1], (float) ptLeftY);
    glVertex3f((float) ptEyeX, gViewPosition[1], (float) ptEyeY);
    glVertex3f((float) ptRightX, gViewPosition[1], (float) ptRightY);

    glEnd();

    glLineWidth(1.f);
    glColor3f(1, 1, 1);

    SetDrawModeContext();
}

// Key Binding Functions
void KeyObserveToggle()
{
    gCameraMode++;
    if (gCameraMode > FLY_MODE)
        gCameraMode = FOLLOW_MODE;

    // Turn animation back on...
    if (gCameraMode == FOLLOW_MODE)
        gAnimating = 1;
}

void KeyDrawModeSurf()
{
    gDrawMode++;
    if (gDrawMode > DRAW_USE_WIREFRAME)
        gDrawMode = DRAW_USE_TEXTURE;

    SetDrawModeContext();
}

void KeyForward()
{
    switch (gCameraMode)
    {
        default:
        case FOLLOW_MODE:
            break;

        case OBSERVE_MODE:
            gCameraPosition[2] += 5.0f;
            break;

        case DRIVE_MODE:
            gViewPosition[0] += 5.0f * sinf(gCameraRotation[ROTATE_YAW] * M_PI / 180.0f);
            gViewPosition[2] -= 5.0f * cosf(gCameraRotation[ROTATE_YAW] * M_PI / 180.0f);

            if (gViewPosition[0] > MAP_SIZE)
                gViewPosition[0] = MAP_SIZE;
            if (gViewPosition[0] < 0)
                gViewPosition[0] = 0;

            if (gViewPosition[2] > MAP_SIZE)
                gViewPosition[2] = MAP_SIZE;
            if (gViewPosition[2] < 0)
                gViewPosition[2] = 0;

            gViewPosition[1] = (MULT_SCALE * gHeightMap[(int) gViewPosition[0] + ((int) gViewPosition[2] * MAP_SIZE)]) + 4.0f;
            break;

        case FLY_MODE:
            gViewPosition[0] +=
                    5.0f * sinf(gCameraRotation[ROTATE_YAW] * M_PI / 180.0f) * cosf(gCameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            gViewPosition[2] -=
                    5.0f * cosf(gCameraRotation[ROTATE_YAW] * M_PI / 180.0f) * cosf(gCameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            gViewPosition[1] -= 5.0f * sinf(gCameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            break;
    }
}

void KeyLeft()
{
    if (gCameraMode == OBSERVE_MODE)
        gCameraRotation[1] -= 5.0f;
}

void KeyBackward()
{
    switch (gCameraMode)
    {
        default:
        case FOLLOW_MODE:
            break;

        case OBSERVE_MODE:
            gCameraPosition[2] -= 5.0f;
            break;

        case DRIVE_MODE:
            gViewPosition[0] -= 5.0f * sinf(gCameraRotation[ROTATE_YAW] * M_PI / 180.0f);
            gViewPosition[2] += 5.0f * cosf(gCameraRotation[ROTATE_YAW] * M_PI / 180.0f);

            if (gViewPosition[0] > MAP_SIZE)
                gViewPosition[0] = MAP_SIZE;
            if (gViewPosition[0] < 0)
                gViewPosition[0] = 0;

            if (gViewPosition[2] > MAP_SIZE)
                gViewPosition[2] = MAP_SIZE;
            if (gViewPosition[2] < 0)
                gViewPosition[2] = 0;

            gViewPosition[1] = (MULT_SCALE * gHeightMap[(int) gViewPosition[0] + ((int) gViewPosition[2] * MAP_SIZE)]) + 4.0f;
            break;

        case FLY_MODE:
            gViewPosition[0] -=
                    5.0f * sinf(gCameraRotation[ROTATE_YAW] * M_PI / 180.0f) * cosf(gCameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            gViewPosition[2] +=
                    5.0f * cosf(gCameraRotation[ROTATE_YAW] * M_PI / 180.0f) * cosf(gCameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            gViewPosition[1] += 5.0f * sinf(gCameraRotation[ROTATE_PITCH] * M_PI / 180.0f);
            break;
    }
}

void KeyRight()
{
    if (gCameraMode == OBSERVE_MODE)
        gCameraRotation[1] += 5.0f;
}

void KeyAnimateToggle()
{
    gAnimating = !gAnimating;
}

void KeyDrawFrustumToggle()
{
    gDrawFrustum = !gDrawFrustum;
}

void KeyUp()
{
    if (gCameraMode == OBSERVE_MODE)
        gCameraPosition[1] -= 5.0f;
}

void KeyDown()
{
    if (gCameraMode == OBSERVE_MODE)
        gCameraPosition[1] += 5.0f;
}

void KeyMoreDetail()
{
    gDesiredTris += 500;
    if (gDesiredTris > 20000)
        gDesiredTris = 20000;
}

void KeyLessDetail()
{
    gDesiredTris -= 500;
    if (gDesiredTris < 500)
        gDesiredTris = 500;
}

// Called when the window has changed size
void ChangeSize(GLsizei w, GLsizei h)
{
    GLfloat fAspect;
    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if (h == 0)
        h = 1;

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    fAspect = (GLfloat) w / (GLfloat) h;

    // Reset coordinate system
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    {
        // Produce the perspective projection
        double left;
        double right;
        double bottom;
        double top;

        right = NEAR_CLIP * tan(gFovX / 2.0 * M_PI / 180.0f);
        top = right / fAspect;
        bottom = -top;
        left = -right;
        glFrustum(left, right, bottom, top, NEAR_CLIP, FAR_CLIP);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void KeyFOVDown()
{
    gFovX -= 1.0f;
    ChangeSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void KeyFOVUp()
{
    gFovX += 1.0f;
    ChangeSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}


// Called to update the window
void RenderScene()
{
    // Clear the GL buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    switch (gCameraMode)
    {
        default:
        case FOLLOW_MODE:
            // Face the center of the map, while walking in a circle around the midpoint (animated).
            glRotatef(-gAnimateAngle, 0.f, 1.f, 0.f);
            glTranslatef(-gViewPosition[0], -gViewPosition[1], -gViewPosition[2]);

            gClipAngle = -gAnimateAngle;
            break;

        case OBSERVE_MODE:

            // Face the center of the map from a certain distance & allow user to orbit around the midpoint.
            //
            // Move to the Camera Position
            glTranslatef(0.f, gCameraPosition[1], gCameraPosition[2]);

            // Rotate to the Camera Rotation angle
            glRotatef(gCameraRotation[ROTATE_PITCH], 1.f, 0.f, 0.f);
            glRotatef(gCameraRotation[ROTATE_YAW], 0.f, 1.f, 0.f);

            // Adjust the origin to be the center of the map...
            glTranslatef(-((GLfloat) MAP_SIZE * 0.5f), 0.f, -((GLfloat) MAP_SIZE * 0.5f));

            gClipAngle = -gAnimateAngle;
            break;

        case DRIVE_MODE:
        case FLY_MODE:
            gAnimating = 0;

            // Rotate to the Camera Rotation angle
            glRotatef(gCameraRotation[ROTATE_PITCH], 1.f, 0.f, 0.f);
            glRotatef(gCameraRotation[ROTATE_YAW], 0.f, 1.f, 0.f);

            // Move to the Camera Position
            glTranslatef(-gViewPosition[0], -gViewPosition[1], -gViewPosition[2]);

            gClipAngle = gCameraRotation[ROTATE_YAW];
            break;
    }

    // Perform the actual rendering of the mesh.
    roamDrawFrame();

    if (gDrawFrustum)
        drawFrustum();

    glPopMatrix();

    // Increment the frame counter.
    gNumFrames++;
}

// Called when user moves the mouse
void MouseMove(int mouseX, int mouseY)
{
    // If we're rotating, perform the updates needed
    if (gRotating && (gCameraMode != FOLLOW_MODE))
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

        if (gCameraMode == OBSERVE_MODE)
            dy = mouseY - gStartY;
        else
            dy = gStartY - mouseY; // Invert mouse in Drive/Fly mode

        // Update the camera rotations
        gCameraRotation[0] += (GLfloat) dy * 0.5f;
        gCameraRotation[1] += (GLfloat) dx * 0.5f;

        // Reset the deltas for the next idle call
        gStartX = mouseX;
        gStartY = mouseY;
    }
}

// Called when application is idle
void IdleFunction()
{
    // If animating, move the view position
    if (gAnimating)
    {
        gAnimateAngle += 0.4f;

        gViewPosition[0] = ((GLfloat) MAP_SIZE / 4.f) + ((sinf(gAnimateAngle * M_PI / 180.f) + 1.f) * ((GLfloat) MAP_SIZE / 4.f));
        gViewPosition[2] = ((GLfloat) MAP_SIZE / 4.f) + ((cosf(gAnimateAngle * M_PI / 180.f) + 1.f) * ((GLfloat) MAP_SIZE / 4.f));

        gViewPosition[1] = (MULT_SCALE * gHeightMap[(int) gViewPosition[0] + ((int) gViewPosition[2] * MAP_SIZE)]) + 4.0f;
        gAnimating = 0;
    }
}

// This function does any needed initialization on the rendering
// context.  Here it sets up and initializes the lighting for
// the scene.
void SetupRC()
{
    glEnable(GL_DEPTH_TEST); // Hidden surface removal
    glFrontFace(GL_CCW); // Counter clock-wise polygons face out
    glEnable(GL_CULL_FACE); // Cull back-facing triangles

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Lighting Setup
    // Light values and coordinates
    GLfloat whiteLight[] = {0.45f, 0.45f, 0.45f, 1.0f};
    GLfloat ambientLight[] = {0.25f, 0.25f, 0.25f, 1.0f};
    GLfloat diffuseLight[] = {0.50f, 0.50f, 0.50f, 1.0f};
    GLfloat lightPos[] = {0.00f, 300.00f, 0.00f, 0.0f};

    // Setup and enable light 0
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHT0);

    // Enable color tracking
    glEnable(GL_COLOR_MATERIAL);

    // Set Material properties to follow glColor values
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Set the color for the landscape
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, whiteLight);

    // Texture Setup
    // Use Generated Texture Coordinates
    static GLfloat s_vector[4] = {1.0 / (GLfloat) TEXTURE_SIZE, 0, 0, 0};
    static GLfloat t_vector[4] = {0, 0, 1.0 / (GLfloat) TEXTURE_SIZE, 0};

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_S, GL_OBJECT_PLANE, s_vector);

    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, t_vector);
}

// Reduces a normal vector specified as a set of three coordinates,
// to a unit normal vector of length one.
void Utility::ReduceToUnit(float *vector)
{
    // Calculate the length of the vector
    float length = sqrtf((vector[0] * vector[0]) + (vector[1] * vector[1]) + (vector[2] * vector[2]));

    // Keep the program from blowing up by providing an exceptable
    // value for vectors that may calculated too close to zero.
    if (length == 0.0f)
        length = 1.0f;

    // Dividing each element by the length will result in a
    // unit normal vector.
    vector[0] /= length;
    vector[1] /= length;
    vector[2] /= length;
}

// Points p1, p2, & p3 specified in counter clock-wise order
void Utility::CalcNormal(float (*v)[3], float *out)
{
    float v1[3], v2[3];
    static const int x = 0;
    static const int y = 1;
    static const int z = 2;

    // Calculate two vectors from the three points
    v1[x] = v[0][x] - v[1][x];
    v1[y] = v[0][y] - v[1][y];
    v1[z] = v[0][z] - v[1][z];

    v2[x] = v[1][x] - v[2][x];
    v2[y] = v[1][y] - v[2][y];
    v2[z] = v[1][z] - v[2][z];

    // Take the cross product of the two vectors to get
    // the normal vector which will be stored in out
    out[x] = v1[y] * v2[z] - v1[z] * v2[y];
    out[y] = v1[z] * v2[x] - v1[x] * v2[z];
    out[z] = v1[x] * v2[y] - v1[y] * v2[x];

    // Normalize the vector (shorten length to one)
    ReduceToUnit(out);
}
