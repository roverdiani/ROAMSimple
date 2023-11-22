//
// Created by Rodrigo Verdiani on 22/11/2023.
//

#ifndef ROAMSDL_ROAM_H
#define ROAMSDL_ROAM_H

#include "Landscape.h"

class Roam
{
public:
    bool Init() const;
    void LoadTerrain();
    void FreeTerrain();

    void Draw(GLfloat *viewPosition, GLfloat clipAngle, float fovX, int drawMode, int& numTrisRendered);

    void IncreaseDetail();
    void DecreaseDetail();

    unsigned char* GetHeightMap()
    {
        return m_landscape.heightMap;
    }

private:
    // Desired number of Binary Triangle tessellations per frame. This is not the desired number of triangles rendered!
    // There are usually twice as many Binary Triangle structures as there are rendered triangles.
    int m_desiredTris = 10000;

    // Beginning frame variance (should be high, it will adjust automatically)
    float m_frameVariance = 50;

    // Texture
    GLuint m_textureId = 1;

    Landscape m_landscape;
};

#endif //ROAMSDL_ROAM_H
