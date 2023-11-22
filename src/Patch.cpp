//  Patch.cpp
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

#include <SDL_opengl.h>
#include <cmath>
#include <algorithm>

#include "Landscape.h"
#include "Patch.h"
#include "Utility.h"

// Initialize a patch.
void Patch::Init(int heightX, int heightY, int worldX, int worldY, unsigned char *hMap)
{
    // Clear all the relationships
    m_BaseLeft.RightNeighbor = m_BaseLeft.LeftNeighbor = m_BaseRight.RightNeighbor = m_BaseRight.LeftNeighbor =
    m_BaseLeft.LeftChild = m_BaseLeft.RightChild = m_BaseRight.LeftChild = m_BaseLeft.LeftChild = nullptr;

    // Attach the two m_Base triangles together
    m_BaseLeft.BaseNeighbor = &m_BaseRight;
    m_BaseRight.BaseNeighbor = &m_BaseLeft;

    // Store Patch offsets for the world and heightmap.
    m_WorldX = worldX;
    m_WorldY = worldY;

    // Store pointer to first byte of the height data for this patch.
    m_HeightMap = &hMap[heightY * MAP_SIZE + heightX];

    // Initialize flags
    m_VarianceDirty = true;
    m_isVisible = false;
}

// Reset the patch.
void Patch::Reset()
{
    // Assume patch is not visible.
    m_isVisible = false;

    // Reset the important relationships
    m_BaseLeft.LeftChild = m_BaseLeft.RightChild = m_BaseRight.LeftChild = m_BaseLeft.LeftChild = nullptr;

    // Attach the two m_Base triangles together
    m_BaseLeft.BaseNeighbor = &m_BaseRight;
    m_BaseRight.BaseNeighbor = &m_BaseLeft;

    // Clear the other relationships.
    m_BaseLeft.RightNeighbor = m_BaseLeft.LeftNeighbor = m_BaseRight.RightNeighbor = m_BaseRight.LeftNeighbor = nullptr;
}

// Will correctly force-split diamonds.
void Patch::Split(TriTreeNode *tri)
{
    // We are already split, no need to do it again.
    if (tri->LeftChild)
        return;

    // If this triangle is not in a proper diamond, force split our base neighbor
    if (tri->BaseNeighbor && (tri->BaseNeighbor->BaseNeighbor != tri))
        Split(tri->BaseNeighbor);

    // Create children and link into mesh
    tri->LeftChild = Landscape::AllocateTri();
    tri->RightChild = Landscape::AllocateTri();

    // If creation failed, just exit.
    if (!tri->LeftChild || !tri->RightChild)
        return;

    // Fill in the information we can get from the parent (neighbor pointers)
    tri->LeftChild->BaseNeighbor = tri->LeftNeighbor;
    tri->LeftChild->LeftNeighbor = tri->RightChild;

    tri->RightChild->BaseNeighbor = tri->RightNeighbor;
    tri->RightChild->RightNeighbor = tri->LeftChild;

    // Link our Left Neighbor to the new children
    if (tri->LeftNeighbor)
    {
        if (tri->LeftNeighbor->BaseNeighbor == tri)
            tri->LeftNeighbor->BaseNeighbor = tri->LeftChild;
        else if (tri->LeftNeighbor->LeftNeighbor == tri)
            tri->LeftNeighbor->LeftNeighbor = tri->LeftChild;
        else if (tri->LeftNeighbor->RightNeighbor == tri)
            tri->LeftNeighbor->RightNeighbor = tri->LeftChild;
    }

    // Link our Right Neighbor to the new children
    if (tri->RightNeighbor)
    {
        if (tri->RightNeighbor->BaseNeighbor == tri)
            tri->RightNeighbor->BaseNeighbor = tri->RightChild;
        else if (tri->RightNeighbor->RightNeighbor == tri)
            tri->RightNeighbor->RightNeighbor = tri->RightChild;
        else if (tri->RightNeighbor->LeftNeighbor == tri)
            tri->RightNeighbor->LeftNeighbor = tri->RightChild;
    }

    // Link our Base Neighbor to the new children
    if (tri->BaseNeighbor)
    {
        if (tri->BaseNeighbor->LeftChild)
        {
            tri->BaseNeighbor->LeftChild->RightNeighbor = tri->RightChild;
            tri->BaseNeighbor->RightChild->LeftNeighbor = tri->LeftChild;
            tri->LeftChild->RightNeighbor = tri->BaseNeighbor->RightChild;
            tri->RightChild->LeftNeighbor = tri->BaseNeighbor->LeftChild;
        } else
            Split(tri->BaseNeighbor); // Base Neighbor (in a diamond with us) was not split yet, so do that now.
    } else
    {
        // An edge triangle, trivial case.
        tri->LeftChild->RightNeighbor = nullptr;
        tri->RightChild->LeftNeighbor = nullptr;
    }
}

// Tessellate a Patch.
// Will continue to split until the variance metric is met.
void Patch::RecursTessellate(TriTreeNode *tri, int leftX, int leftY, int rightX, int rightY, int apexX, int apexY, int node)
{
    float TriVariance;

    // Compute X and Y coordinates of center of Hypotenuse
    int centerX = (leftX + rightX) >> 1;
    int centerY = (leftY + rightY) >> 1;

    if (node < (1 << VARIANCE_DEPTH))
    {
        // Extremely slow distance metric (sqrt is used).
        // Replace this with a faster one!
        float distance = 1.0f + sqrtf(((float) centerX - gViewPosition[0]) * ((float) centerX - gViewPosition[0]) +
                ((float) centerY - gViewPosition[2]) * ((float) centerY - gViewPosition[2]));

        // Egads!  A division too?  What's this world coming to!
        // This should also be replaced with a faster operation.
        // Take both distance and variance into consideration
        TriVariance = ((float) m_CurrentVariance[node] * MAP_SIZE * 2) / distance;
    }

    // IF we do not have variance info for this node, then we must have gotten here by splitting, so continue down to the lowest level.
    // OR if we are not below the variance tree, test for variance.
    if ((node >= (1 << VARIANCE_DEPTH)) || (TriVariance > gFrameVariance))
    {
        // Split this triangle.
        Split(tri);

        // If this triangle was split, try to split its children as well.
        // Tessellate all the way down to one vertex per height field entry
        if (tri->LeftChild && ((abs(leftX - rightX) >= 3) || (abs(leftY - rightY) >= 3)))
        {
            RecursTessellate(tri->LeftChild, apexX, apexY, leftX, leftY, centerX, centerY, node << 1);
            RecursTessellate(tri->RightChild, rightX, rightY, apexX, apexY, centerX, centerY, 1 + (node << 1));
        }
    }
}

// Render the tree.  Simple no-fan method.
void Patch::RecursRender(TriTreeNode *tri, int leftX, int leftY, int rightX, int rightY, int apexX, int apexY)
{
    // All non-leaf nodes have both children, so just check for one
    if (tri->LeftChild)
    {
        // Compute X and Y coordinates of center of Hypotenuse
        int centerX = (leftX + rightX) >> 1;
        int centerY = (leftY + rightY) >> 1;

        RecursRender(tri->LeftChild, apexX, apexY, leftX, leftY, centerX, centerY);
        RecursRender(tri->RightChild, rightX, rightY, apexX, apexY, centerX, centerY);
    } else
    {
        // A leaf node!  Output a triangle to be rendered.
        // Actual number of rendered triangles...
        gNumTrisRendered++;

        GLfloat leftZ = m_HeightMap[(leftY * MAP_SIZE) + leftX];
        GLfloat rightZ = m_HeightMap[(rightY * MAP_SIZE) + rightX];
        GLfloat apexZ = m_HeightMap[(apexY * MAP_SIZE) + apexX];

        // Perform lighting calculations if requested.
        if (gDrawMode == DRAW_USE_LIGHTING)
        {
            float v[3][3];
            float out[3];

            // Create a vertex normal for this triangle.
            // NOTE: This is an extremely slow operation for illustration purposes only.
            //       You should use a texture map with the lighting pre-applied to the texture.
            v[0][0] = (GLfloat) leftX;
            v[0][1] = (GLfloat) leftZ;
            v[0][2] = (GLfloat) leftY;

            v[1][0] = (GLfloat) rightX;
            v[1][1] = (GLfloat) rightZ;
            v[1][2] = (GLfloat) rightY;

            v[2][0] = (GLfloat) apexX;
            v[2][1] = (GLfloat) apexZ;
            v[2][2] = (GLfloat) apexY;

            Utility::CalcNormal(v, out);
            glNormal3fv(out);
        }

        // Perform polygon coloring based on a height sample
        float fColor = (60.0f + leftZ) / 256.0f;
        if (fColor > 1.0f)
            fColor = 1.0f;
        glColor3f(fColor, fColor, fColor);

        // Output the LEFT VERTEX for the triangle
        glVertex3f((GLfloat) leftX, (GLfloat) leftZ, (GLfloat) leftY);

        // Gouraud shading based on height samples instead of light normal
        if (gDrawMode == DRAW_USE_TEXTURE || gDrawMode == DRAW_USE_FILL_ONLY)
        {
            fColor = (60.0f + rightZ) / 256.0f;
            if (fColor > 1.0f)
                fColor = 1.0f;

            glColor3f(fColor, fColor, fColor);
        }

        // Output the RIGHT VERTEX for the triangle
        glVertex3f((GLfloat) rightX, (GLfloat) rightZ, (GLfloat) rightY);

        // Gouraud shading based on height samples instead of light normal
        if (gDrawMode == DRAW_USE_TEXTURE || gDrawMode == DRAW_USE_FILL_ONLY)
        {
            fColor = (60.0f + apexZ) / 256.0f;
            if (fColor > 1.0f)
                fColor = 1.0f;

            glColor3f(fColor, fColor, fColor);
        }

        // Output the APEX VERTEX for the triangle
        glVertex3f((GLfloat) apexX, (GLfloat) apexZ, (GLfloat) apexY);
    }
}

// Computes Variance over the entire tree.  Does not examine node relationships.
unsigned char Patch::RecursComputeVariance(int leftX, int leftY, unsigned char leftZ, int rightX, int rightY, unsigned char rightZ,
                                           int apexX, int apexY, unsigned char apexZ, int node)
{
    //        /|\
	//      /  |  \
	//    /    |    \
	//  /      |      \
	//  ~~~~~~~*~~~~~~~  <-- Compute the X and Y coordinates of '*'
    //
    // Compute X and Y coordinates of center of Hypotenuse
    int centerX = (leftX + rightX) >> 1;
    int centerY = (leftY + rightY) >> 1;

    // Get the height value at the middle of the Hypotenuse
    unsigned char centerZ = m_HeightMap[(centerY * MAP_SIZE) + centerX];

    // Variance of this triangle is the actual height at its hypotenuse midpoint minus the interpolated height.
    // Use values passed on the stack instead of re-accessing the Height Field.
    unsigned char myVariance = abs((int) centerZ - (((int) leftZ + (int) rightZ) >> 1));

    // Since we're after speed and not perfect representations, only calculate variance down to a 8x8 block
    if ((abs(leftX - rightX) >= 8) || (abs(leftY - rightY) >= 8))
    {
        // Final Variance for this node is the max of its own variance and that of its children.
        myVariance = std::max(myVariance, RecursComputeVariance(apexX, apexY, apexZ, leftX,
                                                           leftY, leftZ, centerX, centerY,
                                                           centerZ, node << 1));
        myVariance = std::max(myVariance, RecursComputeVariance(rightX, rightY, rightZ, apexX,
                                                           apexY, apexZ, centerX, centerY,
                                                           centerZ, 1 + (node << 1)));
    }

    // Store the final variance for this node.  Note Variance is never zero.
    if (node < (1 << VARIANCE_DEPTH))
        m_CurrentVariance[node] = 1 + myVariance;

    return myVariance;
}

// Compute the variance tree for each of the Binary Triangles in this patch.
void Patch::ComputeVariance()
{
    // Compute variance on each of the base triangles...

    m_CurrentVariance = m_VarianceLeft;
    RecursComputeVariance(0, PATCH_SIZE, m_HeightMap[PATCH_SIZE * MAP_SIZE], PATCH_SIZE,
                          0, m_HeightMap[PATCH_SIZE], 0, 0, m_HeightMap[0], 1);

    m_CurrentVariance = m_VarianceRight;
    RecursComputeVariance(PATCH_SIZE, 0, m_HeightMap[PATCH_SIZE], 0, PATCH_SIZE,
                          m_HeightMap[PATCH_SIZE * MAP_SIZE], PATCH_SIZE, PATCH_SIZE,
                          m_HeightMap[(PATCH_SIZE * MAP_SIZE) + PATCH_SIZE], 1);

    // Clear the dirty flag for this patch
    m_VarianceDirty = false;
}

// Set patch's visibility flag.
void Patch::SetVisibility(int eyeX, int eyeY, int leftX, int leftY, int rightX, int rightY)
{
    // Get patch's center point
    int patchCenterX = m_WorldX + PATCH_SIZE / 2;
    int patchCenterY = m_WorldY + PATCH_SIZE / 2;

    // Set visibility flag (orientation of both triangles must be counter-clockwise)
    m_isVisible = (Utility::Orientation(eyeX, eyeY, rightX, rightY, patchCenterX, patchCenterY) < 0) &&
                  (Utility::Orientation(leftX, leftY, eyeX, eyeY, patchCenterX, patchCenterY) < 0);
}

// Create an approximate mesh.
void Patch::Tessellate()
{
    // Split each of the base triangles
    m_CurrentVariance = m_VarianceLeft;
    RecursTessellate(&m_BaseLeft, m_WorldX, m_WorldY + PATCH_SIZE, m_WorldX + PATCH_SIZE,
                     m_WorldY, m_WorldX, m_WorldY, 1);

    m_CurrentVariance = m_VarianceRight;
    RecursTessellate(&m_BaseRight, m_WorldX + PATCH_SIZE, m_WorldY, m_WorldX,
                     m_WorldY + PATCH_SIZE, m_WorldX + PATCH_SIZE, m_WorldY + PATCH_SIZE, 1);
}

// Render the mesh.
void Patch::Render()
{
    // Store old matrix
    glPushMatrix();

    // Translate the patch to the proper world coordinates
    glTranslatef((GLfloat) m_WorldX, 0, (GLfloat) m_WorldY);

    glBegin(GL_TRIANGLES);

    RecursRender(&m_BaseLeft, 0, PATCH_SIZE, PATCH_SIZE, 0, 0, 0);
    RecursRender(&m_BaseRight, PATCH_SIZE, 0, 0, PATCH_SIZE,
                 PATCH_SIZE, PATCH_SIZE);

    glEnd();

    // Restore the matrix
    glPopMatrix();
}
