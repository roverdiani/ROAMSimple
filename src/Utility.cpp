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
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "Utility.h"
#include "Landscape.h"
#include "Roam.h"

// Perspective & Window defines
#define FOV_ANGLE 90.0f
#define NEAR_CLIP 1.0f
#define FAR_CLIP 2500.0f

// --------------------------------------
// GLOBALS
// --------------------------------------



std::chrono::time_point<std::chrono::high_resolution_clock> gStartTime, gEndTime;

// Switch GL Contexts when moving between draw modes to improve performance.
void Utility::SetDrawModeContext(int drawMode)
{
    switch (drawMode)
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

// This function does any needed initialization on the rendering context.  Here it sets up and initializes the lighting
// for the scene.
void Utility::SetupRC()
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

// Called when the window has changed size
void Utility::ChangeSize(GLsizei w, GLsizei h, float fovX)
{
    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if (h == 0)
        h = 1;

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    GLfloat fAspect = (GLfloat) w / (GLfloat) h;

    // Reset coordinate system
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Produce the perspective projection
    double left;
    double right;
    double bottom;
    double top;

    right = NEAR_CLIP * tan(fovX / 2.0 * M_PI / 180.0f);
    top = right / fAspect;
    bottom = -top;
    left = -right;
    glFrustum(left, right, bottom, top, NEAR_CLIP, FAR_CLIP);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
