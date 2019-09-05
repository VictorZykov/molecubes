/*

#########################################################################################

Cube Interface ver 0.1

Computational Synthesis Lab
Cornell University
Aug 09, 2007

Written by Andrew Chan

Based on code segments by Robin Southern (author of NxOGRE) and  Steve Streeting (project lead of OGRE)

OGRE is released under the GNU Lesser General Public License (LGPL)
NxOGRE is released under the GNU Lesser General Public License (LGPL)
Some images and media files are under the Creative Commons License (CCL)

Half-cube mesh by Viktor Zykov

Libraries used:
OGRE ver 1.4.3 (Eihort)	(newest version as of Aug 09, 2007)
NxOGRE ver 0.4 RC3		(older version)
AGEIA PhysX ver 2.6.4	(older version)

Older versions of NxOGRE and PhysX are used because NxOGRE ver 0.4 RC3 has the most functionality implemented,
but it only supports PhysX ver 2.6.4.  Thus, in order to take advantage of as many features that PhysX offers
that has been implemented in NxOGRE, an older version of PhysX is used.

The newest version of PhysX as of Aug 9, 2007 is PhysX ver 2.7.2.
The newest version of NxOGRE as of Aug 9, 2007 is NxOGRE ver 0.9.

#########################################################################################

For more information and newer releases, see the following:

[1] http://www.ogre3d.org/wiki/
[2] http://www.nxogre.org
[3] http://devsupport.ageia.com 

Keys:

- Escape		Quit
- F1			Save a screenshot

*/

/////////////////////////////////////////////////////////////////////////////////////////

// These numbers need to be tweaked to achieve the desired behavior
#define GRAVITY -30						// Magnitude of gravity; make sure it's a NEGATIVE value 
#define DENSITY 5						// Density of half cubes
#define LINEAR_DAMPING 1				// Natural linear damping of the half cubes
#define ANGULAR_DAMPING 1				// Natural angular damping of the half cubes
#define CHEIGHT .6						// Height of half cubes
#define TWIST_FORCE_LIMIT 200			// Maximum force that the motor applies to rotate
#define HALT_FORCE_LIMIT 300			// Maximum force that the motor applies to halt rotation
#define TARGET_VELOCITY 3				// Velocity that the motor attempts to achieve

#include "nxOgre.h"
#include "Ogre.h"
#include "Eihort.h"
#include "ActorUserData.h"

using namespace nxOgre;
using namespace Ogre;

/* 

CubeInterface is the main class for the application.  It extends SimpleTutorial, which provides many of the
basic functionality pertaining to rendering and user input.

Functions:
start() is the initialization function.
newFrame() is executed each time a frame is processed.

createCube() generates a cube at the specified position and with the given orientation

*/

class CubeInterface : public Eihort {
	
public:
	// globals
	world *mWorld;
	scene* mScene;
	
	// actors/bodies
	body *halfCube[255];

	// every cube is assigned an ID number
	int idCounter;
	
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Construtor initilizes idCounter to 0
	CubeInterface():idCounter(0) {}

	// Initialization function
	void start() {
		/////////////////////////////////////////////////////////////////////////////////
		// Set up the world and Scene
		mWorld = new world();
		
		/////////////////////////////////////////////////////////////
		// Another aspect that can be tweaked to produce the desire results

		// (1) Use this to set a fixed timestep for the simulation
		nxOgre::blueprint<nxOgre::scene> blueprint;
		blueprint.toDefault();

		blueprint.mTimeStepMethod = NX_TIMESTEP_FIXED;

		// Different levels of accuracy
		//blueprint.mMaxTimestep = 1.0f/60.0f;			// Joints separate too much
		//blueprint.mMaxTimestep = 1.0f/120.0f;			// Good and moderately fast
		blueprint.mMaxTimestep = 1.0f/180.0f;			// Better but a little slow
		//blueprint.mMaxTimestep = 1.0f/240.0f;			// Accurate but kind of slow
		mScene = blueprint.create("Main",mSceneMgr);

		//***************************************************
		// (2) Use this to let the time steps be handled automatically (variable time steps)
		//
		//mScene = mWorld->createScene("Main",mSceneMgr);	// Joints separate too much
		//
		/////////////////////////////////////////////////////////////

		mScene->hasGravityAndFloor(Ogre::Vector3(0,GRAVITY,0));
		//mScene->hasFloor();
		//mScene->hasGravity();
		
		// NX_SKIN_WIDTH is the allowable interpenetration depth between two objects
		//    If this is too low, the simulation becomes jittery
		mScene->mPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.05);

		/////////////////////////////////////////////////////////////////////////////////

		// Create initial bodies
		halfCube[0] = createCube(pose(Vector3(0,CHEIGHT,0), Quaternion(Radian(Math::PI/2), Vector3::UNIT_Y )),0,false);
		
		//halfCube[0] = createCube(pose(Vector3(0,CHEIGHT,0)), 0, true);
		//halfCube[1] = addCubePosY(halfCube[0],1);
	}

	body* addCubePosY(body *selectedCube, int id) {
		// Declarations for cube and joint positions
		Vector3 position, addLocation, jointOrientation;

		// orientationY is the local Y axis of the body relative to the global axes
		Vector3 orientationY;
		Quaternion quatern, jointQuatern;
		body *anchorCube, *addedCube;
		fixedJoint *tempFixedJoint;

		if(((ActorUserData*)(selectedCube->ActorUserData))->lowerHalf) {
			anchorCube = ((ActorUserData*)(selectedCube->ActorUserData))->associatedBody;
		}
		else {
			anchorCube = selectedCube;
		}

		position = anchorCube->getGlobalPosition();
		quatern = anchorCube->getGlobalOrientation();

		// Extract the local y-axis of the body

		orientationY = Vector3(quatern.yAxis().x, quatern.yAxis().y, quatern.yAxis().z);
		orientationY.normalise();	

		// Add the cube based on the anchor cubes position and orientation
		addLocation = position + 2*CHEIGHT*orientationY;				// Determine where to add the cube
		addedCube = createCube(pose(addLocation, quatern),id,false);

		// Create the fixed joint that fixes the new cube to the anchor cube
		// The joint connects the anchor cube's top half to the added cube's lower half
		tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientationY);

		// Record the fixed joint to the corresponding half cubes
		((ActorUserData*)(anchorCube->ActorUserData))->fixJoint = tempFixedJoint;
		((ActorUserData*)(addedCube->ActorUserData))->fixJoint = tempFixedJoint;

		return addedCube;
	}

	body* createCube(pose cPose, int id, bool kinematic = false) {
		body *tempBody1, *tempBody2;
		SDFJoint *tempD6Joint;
		
		// Create lower half of cube
		tempBody1 = mScene->createBody(
			"halfCube1." + StringConverter::toString(id),	// Half-cube name
			"MSCube19.mesh",								// Half-cube image
			//"Pyramid13.mesh",								
			new convexShape("Pyramid13.mesh"),				// Half-cube collision mesh
			DENSITY,										// Half-cube density
			cPose											// Half-cube pose
			);
		tempBody1->setKinematic(kinematic);
		tempBody1->setAngularDamping(ANGULAR_DAMPING);
		tempBody1->setLinearDamping(LINEAR_DAMPING);

		// Create upper half of cube
		tempBody2 = mScene->createBody(
			"halfCube2." + StringConverter::toString(id),	// Half-cube name
			"MSCube20.mesh",								// Half-cube image
			//"Pyramid14.mesh",
			new convexShape("Pyramid14.mesh"),				// Half-cube collision mesh
			DENSITY,										// Half-cube density
			cPose											// Half-cube pose
		);
		tempBody2->setAngularDamping(ANGULAR_DAMPING);
		tempBody2->setLinearDamping(LINEAR_DAMPING);

		// Declarations for determining joint position and orientation
		Quaternion quatern;
		Vector3 orientationX, orientationY, orientationZ;
		Vector3 jointOrientation;

		// Detemine joint position and orientation
		quatern = NxTools::convert(cPose.q);

		orientationX = Vector3(quatern.xAxis().x, quatern.xAxis().y, quatern.xAxis().z);
		orientationX.normalise();
		orientationY = Vector3(quatern.yAxis().x, quatern.yAxis().y, quatern.yAxis().z);
		orientationY.normalise();	
		orientationZ = Vector3(quatern.zAxis().x, quatern.zAxis().y, quatern.zAxis().z);
		orientationZ.normalise();

		jointOrientation = Quaternion(Radian(Math::PI/4), orientationY)
							*Quaternion(Radian(-.955316618), orientationZ)*orientationY;

		//printf("jointOrientation.x: %f, jointOrientation.y: %f, jointOrientation.z: %f\n",
		//	jointOrientation.x, jointOrientation.y, jointOrientation.z);

		tempD6Joint = new SDFJoint(tempBody1, tempBody2, cPose.getVector3(), jointOrientation);
		
		// Create ActorUserData for each of the halves
		//    ActorUserData(int id, D6Joint joint, body assocBody, bool lowHalf)
		tempBody1->ActorUserData = new ActorUserData(id, tempD6Joint, tempBody2, true);
		tempBody2->ActorUserData = new ActorUserData(id, tempD6Joint, tempBody1, false);

		// Increment the ID number; every cube is assigned an ID number
		idCounter++;

		// Returns the anchor actor; the "bottom" cube
		return tempBody1;
	}

	void rotateClockwise(SDFJoint *joint) {
		NxD6JointDesc d6Desc;
		joint->mJoint->saveToDesc(d6Desc);

		d6Desc.twistMotion = NX_D6JOINT_MOTION_FREE;
		d6Desc.twistDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
		d6Desc.twistDrive.forceLimit = TWIST_FORCE_LIMIT;
		d6Desc.driveAngularVelocity = NxVec3(TARGET_VELOCITY,0,0);
		
		d6Desc.projectionMode = NX_JPM_NONE;
		
		/* Projection settings
		   Projection creates a bounding region around the joint
		   If the two actors attached to the joint separate beyond the bound, PhysX will 
		      force the actors back within the bound
		   It is designed to fix severe joint errors
		   At the moment, projection is disabled
		*/

		//d6Desc.projectionMode = NX_JPM_POINT_MINDIST;
		//d6Desc.projectionDistance = 0.1f;
		//d6Desc.projectionAngle = 0.0872f;

		joint->mJoint->loadFromDesc(d6Desc);
	}

	void rotateCounterClockwise(SDFJoint *joint) {
		NxD6JointDesc d6Desc;
		joint->mJoint->saveToDesc(d6Desc);

		d6Desc.twistMotion = NX_D6JOINT_MOTION_FREE;
		d6Desc.twistDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
		d6Desc.twistDrive.forceLimit = TWIST_FORCE_LIMIT;
		d6Desc.driveAngularVelocity = NxVec3(-TARGET_VELOCITY,0,0);
		d6Desc.projectionMode = NX_JPM_NONE;

		// Projection settings (currently commented out; see note in rotateClockwise())

		//d6Desc.projectionMode = NX_JPM_POINT_MINDIST;
		//d6Desc.projectionDistance = 0.1f;
		//d6Desc.projectionAngle = 0.0872f;

		joint->mJoint->loadFromDesc(d6Desc);
	}

	void haltRotation(SDFJoint *joint) {
		NxD6JointDesc d6Desc;
		joint->mJoint->saveToDesc(d6Desc);

		d6Desc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
		d6Desc.twistDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
		d6Desc.twistDrive.forceLimit = HALT_FORCE_LIMIT;
		d6Desc.driveAngularVelocity = NxVec3(0,0,0);
		d6Desc.projectionMode = NX_JPM_NONE;

		// Projection settings (currently commented out; see note in rotateClockwise())

		//d6Desc.projectionMode = NX_JPM_POINT_MINDIST;
		//d6Desc.projectionDistance = 0.1f;
		//d6Desc.projectionAngle = 0.0872f;		// About 5 degrees

		// Set the Global Anchor and Axis so that it doesn't move back its original position
		// This is where the "joint separation" bug probably occurs
		//    It probably comes from floating point precision errors
		
		d6Desc.setGlobalAnchor(joint->mJoint->getGlobalAnchor());
		d6Desc.setGlobalAxis(joint->mJoint->getGlobalAxis());
		
		joint->mJoint->loadFromDesc(d6Desc);
	}

	// Clean up and shut down the program
	void stop() {
		// Go through all the half cubes and delete their associated ActorUserData objects
		for(int i = 0; i < idCounter; i++) {
			delete (ActorUserData*)(halfCube[i]->ActorUserData);
		}
		delete mWorld;
	}

	void newFrame(float _time) {
		// Handle user input
		if (targetBody == 0) {
			// If no object is selected, hide all bounding boxes
			for(int i = 0; i < idCounter; i++) {
				halfCube[i]->mNode->showBoundingBox(false);
				((ActorUserData*)(halfCube[i]->ActorUserData))->associatedBody->mNode->showBoundingBox(false);
			}
			return;
		}
		
		// Display the bounding box for the selected object
		targetBody->mNode->showBoundingBox(true);

		// Keys are defined in "Eihort.h"
		if (isKeyDown(U)) {
			rotateClockwise(((ActorUserData*)(targetBody->ActorUserData))->D6joint);
		}

		if (isKeyDown(I)) {
			rotateCounterClockwise(((ActorUserData*)(targetBody->ActorUserData))->D6joint);
		}
		
		if (isKeyDown(O)) {
			haltRotation(((ActorUserData*)(targetBody->ActorUserData))->D6joint);
		}

		if (isKeyDown(J)) {
			halfCube[idCounter-1] = addCubePosY(targetBody,idCounter);
		}
	}

	void getTutorialSettings() {
		mTutorialName = "Cube Interface";
		mTutorialDescription = "Cube Interface for PhysX simulation";
	}
	
	void prestart() {}
	void prestop() {}
};

int main(int argc, char *argv[]) {
	try {
		CubeInterface* cubeInterface = new CubeInterface();
		cubeInterface->Run();

		// Clean up and shut down
		void prestop();
		cubeInterface->Shutdown();
		delete cubeInterface;
	} catch( Ogre::Exception& e ) {
		std::cout << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
	}
	std::cout << "--Done." << std::endl;
	return 0;
}