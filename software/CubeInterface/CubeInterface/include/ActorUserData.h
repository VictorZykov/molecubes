/*
ActorUserData.h

ActorUserData is attached to each half cube.  The class holds data pertaining to the half cube's
associated body (a pointer to the upper half if the class is attached to a lower half, and vice versa),
its D6joint, its fixed joint and its ID number.

To access the ActorUserData, you first need to cast the ActorUserData pointer to the appropriate type.
For example, to access a half cubes D6Joint use the following:
		((ActorUserData *)(halfCube->ActorUserData))->joint = replacementJoint;
	or to change the ID number:		
		((ActorUserData *)(halfCube->ActorUserData))->id = 16;
*/

#ifndef ACTORUSERDATA_H
#define ACTORUSERDATA_H

#include "FullCubeData.h"

using namespace nxOgre;
using namespace Ogre;

class ActorUserData {

public: 
	SDFJoint *D6joint;
	fixedJoint *fixJoint;
	FullCubeData *fullCubeData;
	Vector3 jointOrientation;

	body *associatedBody;

	int id;
	bool lowerHalf;

	ActorUserData() : id(0) {
		D6joint = NULL;
		fixJoint = NULL;
		fullCubeData = NULL;
		associatedBody = NULL;
		lowerHalf = false;
	}

	ActorUserData(int idNumber) : id(idNumber){
		D6joint = NULL;
		fixJoint = NULL;
		fullCubeData = NULL;
		associatedBody = NULL;

		lowerHalf = false;
	}

	ActorUserData(int idNumber, SDFJoint *joint, body *assocBody, bool lowHalf) : id(idNumber){
		D6joint = joint;
		fixJoint = NULL;
		associatedBody = assocBody;
		fullCubeData = NULL;
		lowerHalf = lowHalf;
	}

	~ActorUserData() {
		delete D6joint;
		delete fixJoint;
		delete fullCubeData;
	}
};
#endif