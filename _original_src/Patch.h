
#ifndef PATCH_H
#define PATCH_H

// Depth of variance tree: should be near SQRT(PATCH_SIZE) + 1
#define VARIANCE_DEPTH (9)

// Predefines...
class Landscape;

//
// TriTreeNode Struct
// Store the triangle tree data, but no coordinates!
//
struct TriTreeNode
{
	TriTreeNode *LeftChild;
	TriTreeNode *RightChild;
	TriTreeNode *BaseNeighbor;
	TriTreeNode *LeftNeighbor;
	TriTreeNode *RightNeighbor;
};

//
// Patch Class
// Store information needed at the Patch level
//
class Patch
{
protected:
	unsigned char *m_HeightMap;									// Pointer to height map to use
	int m_WorldX, m_WorldY;										// World coordinate offset of this patch.

	unsigned char m_VarianceLeft[ 1<<(VARIANCE_DEPTH)];			// Left variance tree
	unsigned char m_VarianceRight[1<<(VARIANCE_DEPTH)];			// Right variance tree

	unsigned char *m_CurrentVariance;							// Which varience we are currently using. [Only valid during the Tessellate and ComputeVariance passes]
	unsigned char m_VarianceDirty;								// Does the Varience Tree need to be recalculated for this Patch?
	unsigned char m_isVisible;									// Is this patch visible in the current frame?

	TriTreeNode m_BaseLeft;										// Left base triangle tree node
	TriTreeNode m_BaseRight;									// Right base triangle tree node

public:
	// Some encapsulation functions & extras
	TriTreeNode *GetBaseLeft()  { return &m_BaseLeft; }
	TriTreeNode *GetBaseRight() { return &m_BaseRight; }
	char isDirty()     { return m_VarianceDirty; }
	int  isVisibile( ) { return m_isVisible; }
	void SetVisibility( int eyeX, int eyeY, int leftX, int leftY, int rightX, int rightY );

	// The static half of the Patch Class
	virtual void Init( int heightX, int heightY, int worldX, int worldY, unsigned char *hMap );
	virtual void Reset();
	virtual void Tessellate();
	virtual void Render();
	virtual void ComputeVariance();

	// The recursive half of the Patch Class
	virtual void			Split( TriTreeNode *tri);
	virtual void			RecursTessellate( TriTreeNode *tri, int leftX, int leftY, int rightX, int rightY, int apexX, int apexY, int node );
	virtual void			RecursRender( TriTreeNode *tri, int leftX, int leftY, int rightX, int rightY, int apexX, int apexY );
	virtual unsigned char	RecursComputeVariance(	int leftX,  int leftY,  unsigned char leftZ,
													int rightX, int rightY, unsigned char rightZ,
													int apexX,  int apexY,  unsigned char apexZ,
													int node);
};


#endif
