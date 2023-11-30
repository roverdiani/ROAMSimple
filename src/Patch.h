//  Patch.h
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

#ifndef PATCH_H
#define PATCH_H

// Depth of variance tree: should be near SQRT(PATCH_SIZE) + 1
#define VARIANCE_DEPTH 9

// Predefines...
class Landscape;

// TriTreeNode Struct
// Store the triangle tree data, but no coordinates!
struct TriTreeNode
{
    TriTreeNode *LeftChild;
    TriTreeNode *RightChild;
    TriTreeNode *BaseNeighbor;
    TriTreeNode *LeftNeighbor;
    TriTreeNode *RightNeighbor;
};

// Patch Class
// Store information needed at the Patch level
class Patch
{
protected:
    unsigned char *m_HeightMap;                                    // Pointer to height map to use
    int m_WorldX, m_WorldY;                                        // World coordinate offset of this patch.

    unsigned char m_VarianceLeft[1 << (VARIANCE_DEPTH)];        // Left variance tree
    unsigned char m_VarianceRight[1 << (VARIANCE_DEPTH)];        // Right variance tree

    unsigned char *m_CurrentVariance;                            // Which varience we are currently using. [Only valid during the Tessellate and ComputeVariance passes]
    bool m_VarianceDirty;                                        // Does the Varience Tree need to be recalculated for this Patch?
    bool m_isVisible;                                            // Is this patch visible in the current frame?

    TriTreeNode m_BaseLeft;                                        // Left base triangle tree node
    TriTreeNode m_BaseRight;                                    // Right base triangle tree node

public:
    // Some encapsulation functions & extras
    TriTreeNode *GetBaseLeft()
    {
        return &m_BaseLeft;
    }

    TriTreeNode *GetBaseRight()
    {
        return &m_BaseRight;
    }

    bool isDirty() const
    {
        return m_VarianceDirty;
    }

    // The static half of the Patch Class
    void Init(int heightX, int heightY, int worldX, int worldY, unsigned char *hMap);
    void Reset();

    void Tessellate(GLfloat* viewPosition, float frameVariance);
    void Render(int drawMode, int& numTrisRendered);
    void ComputeVariance();

    // The recursive half of the Patch Class
    void Split(TriTreeNode *tri);

    void RecursTessellate(TriTreeNode *tri, int leftX, int leftY, int rightX, int rightY, int apexX, int apexY, int node, const GLfloat* viewPosition, float frameVariance);

    void RecursRender(TriTreeNode *tri, int leftX, int leftY, int rightX, int rightY, int apexX, int apexY, int drawMode, int &numTrisRendered);

    unsigned char RecursComputeVariance(int leftX, int leftY, unsigned char leftZ, int rightX, int rightY, unsigned char rightZ,
                                                int apexX, int apexY, unsigned char apexZ, int node);
};

#endif
