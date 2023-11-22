//  Landscape.h
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

#ifndef LANDSCAPE_H
#define LANDSCAPE_H

#include <SDL_opengl.h>
#include "Patch.h"

// Various Pre-Defined map sizes & their #define counterparts:

#define MAP_1024

#ifdef MAP_2048
// ------- 2048x2048 MAP -------
#define MAP_SIZE (2048)
#define NUM_PATCHES_PER_SIDE (32)
#else
#ifdef MAP_1024
// ------- 1024x1024 MAP -------
#define MAP_SIZE (1024)
#define NUM_PATCHES_PER_SIDE (16)
#else
// ------- 512x512 MAP -------
#define MAP_SIZE (512)
#define NUM_PATCHES_PER_SIDE (8)
#endif
#endif

// Scale of the terrain ie: 1 unit of the height map == how many world units (meters)?
// 1.0f == 1 meter resolution
// 0.5f == 1/2 meter resolution
// 0.25f == 1/4 meter resolution
// etc..
#define MULT_SCALE 0.5f

// How many TriTreeNodes should be allocated?
#define POOL_SIZE 25000

// Some more definitions
#define PATCH_SIZE (MAP_SIZE / NUM_PATCHES_PER_SIDE)
#define TEXTURE_SIZE 128

#define DEG2RAD(a) (((a) * M_PI) / 180.0f)

// Landscape Class
// Holds all the information to render an entire landscape.
class Landscape
{
public:
    static TriTreeNode *AllocateTri();

    bool LoadTerrain(int size);
    void FreeTerrain();

    void Reset(const GLfloat *viewPosition, GLfloat clipAngle, float fovX);
    void Tessellate(GLfloat* viewPosition, float frameVariance);
    void Render(int desiredTris, float& frameVariance, int drawMode, int& numTrisRendered);

    unsigned char *heightMap = nullptr;

protected:
    void InitTerrainPatches();

    unsigned char *m_HeightMaster = nullptr;                        // HeightMap of the Landscape
    Patch m_Patches[NUM_PATCHES_PER_SIDE][NUM_PATCHES_PER_SIDE];    // Array of patches

    static int m_NextTriNode;                                       // Index to next free TriTreeNode
    static TriTreeNode m_TriPool[POOL_SIZE];                        // Pool of TriTree nodes for splitting

    static int GetNextTriNode()
    {
        return m_NextTriNode;
    }

    static void SetNextTriNode(int nNextNode)
    {
        m_NextTriNode = nNextNode;
    }
};

#endif