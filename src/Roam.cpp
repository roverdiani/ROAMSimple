//
// Created by Rodrigo Verdiani on 22/11/2023.
//

#include <cstdlib>
#include "Roam.h"
#include "Landscape.h"
#include <GL/glu.h>

// Initialize the ROAM implementation
bool Roam::Init() const
{
    // Perform some bounds checking on the #define statements
    if (m_desiredTris > POOL_SIZE)
        return false;

    if (POOL_SIZE < 100)
        return false;

    // TEXTURE INITIALIZATION
    glBindTexture(GL_TEXTURE_2D, m_textureId);

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

    return true;
}

void Roam::LoadTerrain()
{
    m_landscape.LoadTerrain(MAP_SIZE);
}

void Roam::FreeTerrain()
{
    m_landscape.FreeTerrain();
}

// Call all functions needed to draw a frame of the landscape
void Roam::Draw(GLfloat *viewPosition, GLfloat clipAngle, float fovX, int drawMode, int& numTrisRendered)
{
    // Perform all the functions needed to render one frame.
    m_landscape.Reset(viewPosition, clipAngle, fovX);
    m_landscape.Tessellate(viewPosition, m_frameVariance);
    m_landscape.Render(m_desiredTris, m_frameVariance, drawMode, numTrisRendered);
}

void Roam::IncreaseDetail()
{
    m_desiredTris += 500;
    if (m_desiredTris > 20000)
        m_desiredTris = 20000;
}

void Roam::DecreaseDetail()
{
    m_desiredTris -= 500;
    if (m_desiredTris < 500)
        m_desiredTris = 500;
}
