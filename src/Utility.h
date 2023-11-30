//  Utility.h
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

#ifndef UTILITY_H
#define UTILITY_H

// Defines
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Perspective & Window defines
#define FOV_ANGLE 90.0f
#define NEAR_CLIP 1.0f
#define FAR_CLIP 2500.0f

// Rotation Indexes
enum ROTATION_INDEXES
{
    ROTATE_PITCH = 0,
    ROTATE_YAW,
    ROTATE_ROLL
};

#endif
