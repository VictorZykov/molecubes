/*

FullCube.h

FullCube holds information regarding the two halves of a cube as a whole.  In particular, which faces other
cubes are attached to.  This will prevent multiple cubes from being creating at the same location.

*/

#ifndef FULLCUBEDATA_H
#define FULLCUBEDATA_H

#include "nxOgre.h"
#include "Ogre.h"

using namespace nxOgre;
using namespace Ogre;

class FullCubeData {

public: 
	body *attachedCubes[5];					// The attached half-cubes
	Vector3 jointOrientation;

	// cubeOrientation
	//    0: TOP; 1: BOTTOM; 2: LEFT; 3: RIGHT; 4: FORWARD; 5: BACK 6: NOT_SET
	//    The int needs to be *casted* to CubeOrientation when used in CubeInterface
	//	     e.g. (CubeOrientation)(((ActorUserData *)(halfCubes[0]))->fullCubeData->cubeOrientation)
	int cubeOrientation;

	FullCubeData() : cubeOrientation(0) {
		for(int i = 0; i < 6; i++) {
			attachedCubes[i] = NULL;
		}
	}

	std::string *outputData();
	~FullCubeData() {}

};
#endif