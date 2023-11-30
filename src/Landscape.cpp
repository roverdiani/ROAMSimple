//  Landscape.cpp
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
#include <cmath>
#include <iostream>

#include "Landscape.h"

// Definition of the static member variables
int Landscape::m_NextTriNode;
TriTreeNode Landscape::m_TriPool[POOL_SIZE];

// Allocate a TriTreeNode from the pool.
TriTreeNode *Landscape::AllocateTri()
{
    TriTreeNode *pTri;

    // IF we've run out of TriTreeNodes, just return NULL (this is handled gracefully)
    if (m_NextTriNode >= POOL_SIZE)
        return nullptr;

    pTri = &(m_TriPool[m_NextTriNode++]);
    pTri->LeftChild = pTri->RightChild = nullptr;

    return pTri;
}

// Load the terrain height map and initialize all terrain patches
bool Landscape::LoadTerrain(int size)
{
    // Optimization:  Add an extra row above and below the height map.
    //   - The extra top row contains a copy of the last row in the height map.
    //   - The extra bottom row contains a copy of the first row in the height map.
    // This simplifies the wrapping of height values to a trivial case.
    m_HeightMaster = (unsigned char *) malloc(size * size * sizeof(unsigned char) + size * 2);

    // Give the rest of the application a pointer to the actual start of the height map.
    heightMap = m_HeightMaster + size;

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
        memset(m_HeightMaster, 0, size * size + size * 2);
        return false;
    }
    fread(m_HeightMaster + size, 1, (size * size), fp);
    fclose(fp);

    // Copy the last row of the height map into the extra first row.
    memcpy(m_HeightMaster, m_HeightMaster + size * size, size);

    // Copy the first row of the height map into the extra last row.
    memcpy(m_HeightMaster + size * size + size, m_HeightMaster + size, size);

    InitTerrainPatches();

    return true;
}

// Free the Height Field array
void Landscape::FreeTerrain()
{
    if (m_HeightMaster)
        free(m_HeightMaster);
}

// Initialize all terrain patches
void Landscape::InitTerrainPatches()
{
    // Initialize all terrain patches
    for (int y = 0; y < NUM_PATCHES_PER_SIDE; y++)
    {
        for (int x = 0; x < NUM_PATCHES_PER_SIDE; x++)
        {
            Patch *patch = &(m_Patches[y][x]);
            patch->Init(x * PATCH_SIZE, y * PATCH_SIZE, x * PATCH_SIZE, y * PATCH_SIZE, heightMap);
            patch->ComputeVariance();
        }
    }
}

// Reset all patches, recompute variance if needed
void Landscape::Reset(const GLfloat *viewPosition, GLfloat clipAngle, float fovX)
{
    //  Perform simple visibility culling on entire patches.
    //  - Define a triangle set back from the camera by one patch size, following the angle of the frustum.
    //  - A patch is visible if it's center point is included in the angle: Left,Eye,Right
    //  - This visibility test is only accurate if the camera cannot look up or down significantly.

    const float PI_DIV_180 = M_PI / 180.0f;
    const float FOV_DIV_2 = fovX / 2;

    int eyeX = (int) (viewPosition[0] - PATCH_SIZE * sinf(clipAngle * PI_DIV_180));
    int eyeY = (int) (viewPosition[2] + PATCH_SIZE * cosf(clipAngle * PI_DIV_180));

    int leftX = (int) (eyeX + 100.0f * sinf((clipAngle - FOV_DIV_2) * PI_DIV_180));
    int leftY = (int) (eyeY - 100.0f * cosf((clipAngle - FOV_DIV_2) * PI_DIV_180));

    int rightX = (int) (eyeX + 100.0f * sinf((clipAngle + FOV_DIV_2) * PI_DIV_180));
    int rightY = (int) (eyeY - 100.0f * cosf((clipAngle + FOV_DIV_2) * PI_DIV_180));

    // Set the next free triangle pointer back to the beginning
    SetNextTriNode(0);

    // Go through the patches performing resets, compute variances, and linking.
    for (int y = 0; y < NUM_PATCHES_PER_SIDE; y++)
    {
        for (int x = 0; x < NUM_PATCHES_PER_SIDE; x++)
        {
            Patch *patch = &(m_Patches[y][x]);

            // Reset the patch
            patch->Reset();

            // Check to see if this patch has been deformed since last frame.
            // If so, recompute the variance tree for it.
            if (patch->isDirty())
                patch->ComputeVariance();

            // Link all the patches together.
            if (x > 0)
                patch->GetBaseLeft()->LeftNeighbor = m_Patches[y][x - 1].GetBaseRight();
            else
                patch->GetBaseLeft()->LeftNeighbor = nullptr; // Link to bordering Landscape here..

            if (x < (NUM_PATCHES_PER_SIDE - 1))
                patch->GetBaseRight()->LeftNeighbor = m_Patches[y][x + 1].GetBaseLeft();
            else
                patch->GetBaseRight()->LeftNeighbor = nullptr;    // Link to bordering Landscape here..

            if (y > 0)
                patch->GetBaseLeft()->RightNeighbor = m_Patches[y - 1][x].GetBaseRight();
            else
                patch->GetBaseLeft()->RightNeighbor = nullptr;    // Link to bordering Landscape here..

            if (y < (NUM_PATCHES_PER_SIDE - 1))
                patch->GetBaseRight()->RightNeighbor = m_Patches[y + 1][x].GetBaseLeft();
            else
                patch->GetBaseRight()->RightNeighbor = nullptr; // Link to bordering Landscape here..
        }
    }
}

// Create an approximate mesh of the landscape.
void Landscape::Tessellate(GLfloat* viewPosition, float frameVariance)
{
    // Perform Tessellation
    Patch *patch = &(m_Patches[0][0]);
    for (int count = 0; count < NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE; count++, patch++)
        patch->Tessellate(viewPosition, frameVariance);
}

// Render each patch of the landscape & adjust the frame variance.
void Landscape::Render(int desiredTris, float& frameVariance, int drawMode, int& numTrisRendered)
{
    Patch *patch = &(m_Patches[0][0]);

    // Scale the terrain by the terrain scale specified at compile time.
    glScalef(1.0f, MULT_SCALE, 1.0f);

    for (int count = 0; count < NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE; count++, patch++)
        patch->Render(drawMode, numTrisRendered);

    // Check to see if we got close to the desired number of triangles.
    // Adjust the frame variance to a better value.
    if (GetNextTriNode() != desiredTris)
        frameVariance += ((float) GetNextTriNode() - (float) desiredTris) / (float) desiredTris;

    // Bounds checking.
    if (frameVariance < 0)
        frameVariance = 0;
}
