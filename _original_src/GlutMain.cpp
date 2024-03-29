//
// RoamMain.cpp (GLUT Version)
// Bryan Turner
//
// Parts of the code in this file were borrowed from numerous public sources &
//   literature.  I reserve NO rights to this code and give a hearty thank-you to all the
//   excellent sources used in this project.  These include, but are not limited to:
//
//   Longbow Digital Arts Programming Forum (www.LongbowDigitalArts.com)
//   Gamasutra Features (www.Gamasutra.com)
//   GameDev References (www.GameDev.net)
//   C. Cookson's ROAM implementation (C.J.Cookson@dcs.warwick.ac.uk OR cjcookson@hotmail.com)
//   OpenGL Super Bible (Waite Group Press)
//   And many more...
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gl/gl.h>		// OpenGL
#include <gl/glut.h>	// GLUT

#include "utility.h"
#include "landscape.h"


void GLUTRenderScene(void)
{
	RenderScene();

	// Copy image to window
	glutSwapBuffers();
}

void GLUTIdleFunction(void)
{
	IdleFunction();
	GLUTRenderScene();
}

void GLUTKeyDown( unsigned char key, int x, int y )
{
	// Key Bindings
	switch( key )
	{
	case 'w': KeyForward();				break;
	case 'a': KeyLeft();				break;
	case 's': KeyBackward();			break;
	case 'd': KeyRight();				break;

	case 'f': KeyAnimateToggle();		break;
	case 'o': KeyObserveToggle();		break;
	case 'q': KeyDrawModeSurf();		break;
	case 'r': KeyDrawFrustumToggle();	break;

	case '0': KeyMoreDetail();			break;
	case '9': KeyLessDetail();			break;

	case '1': KeyFOVDown();				break;
	case '2': KeyFOVUp();				break;
	}
}

void GLUTKeySpecialDown( int key, int x, int y )
{
	// More key bindings
	switch( key )
	{
	case GLUT_KEY_UP:	KeyUp();	break;
	case GLUT_KEY_DOWN:	KeyDown();	break;
	}
}

void GLUTMouseClick(int button, int state, int x, int y)
{
	// Handle LEFT mouse button UP/DOWN events...
	if ( button == GLUT_LEFT_BUTTON )
	{
		if ( state == GLUT_DOWN )
		{
			gRotating = 1;
			gStartX = -1;
		}
		else
			gRotating = 0;
	}
}

// ---------------------------------------------------------------------
// Main body of program
//
int	main(int argc, char *argv[])
{
	// GLUT window setup and initialization
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	if ( glutCreateWindow("ROAM Terrain View") < 0)
	{
		printf("ERROR: Create Window Failed!\n");
		exit(0);
	}

	glutReshapeFunc(ChangeSize);		// Set function to call when window is resized
	glutIdleFunc(GLUTIdleFunction);		// Set function to call when program is idle
	glutKeyboardFunc(GLUTKeyDown);
	glutSpecialFunc(GLUTKeySpecialDown);
	glutMouseFunc(GLUTMouseClick);
	glutMotionFunc(MouseMove);
	glutDisplayFunc(GLUTRenderScene);

	// Setup OpenGL
	SetupRC();
	SetDrawModeContext();

	// Load landscape data file
	loadTerrain(MAP_SIZE, &gHeightMap);

	int nAvgFrames = -1;
	if ( roamInit(gHeightMap) == 0 )
	{
		// Start the animation loop running.
		gAnimating = 1;
		
		// Get the start time in milliseconds
		gStartTime = timeGetTime();
		
		// Start main loop
		glutMainLoop();

		// Calculate the average number of frames per second.
		gEndTime = timeGetTime();
		nAvgFrames = (gNumFrames * 1000) / (gEndTime - gStartTime);
	}

	freeTerrain();

	return nAvgFrames;
}

// -------------------------------------------------
// Stub function to maintain Windows compatability
//
int APIENTRY WinMain(	HINSTANCE 	hInstance,
	HINSTANCE 	hPrevInstance,
	LPSTR 		lpCmdLine,
	int			nCmdShow)
{
	return main( nCmdShow, &lpCmdLine );
}



