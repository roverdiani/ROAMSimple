//
// Created by Rodrigo Verdiani on 22/11/2023.
//

#include <cstdlib>
#include "Roam.h"
#include "Landscape.h"

#define MAX_DESIRED_TRIS 20000
#define MIN_DESIRED_TRIS 500

// Initialize the ROAM implementation
bool Roam::Init() const
{
    // Perform some bounds checking on the #define statements
    if (m_desiredTris > POOL_SIZE)
        return false;

    if (POOL_SIZE < 100)
        return false;

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
void Roam::Draw(GLfloat *viewPosition, int& numTrisRendered)
{
    // Perform all the functions needed to render one frame.
    m_landscape.Reset();
    m_landscape.Tessellate(viewPosition, m_frameVariance);
    m_landscape.Render(m_desiredTris, m_frameVariance, numTrisRendered);
}

void Roam::IncreaseDetail()
{
    m_desiredTris += 500;
    if (m_desiredTris > MAX_DESIRED_TRIS)
        m_desiredTris = MAX_DESIRED_TRIS;
}

void Roam::DecreaseDetail()
{
    m_desiredTris -= 500;
    if (m_desiredTris < MIN_DESIRED_TRIS)
        m_desiredTris = MIN_DESIRED_TRIS;
}
