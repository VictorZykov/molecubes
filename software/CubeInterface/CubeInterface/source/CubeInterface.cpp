/*
#########################################################################################

Cube Interface ver 0.1

Computational Synthesis Lab
Cornell University
Mar 26, 2008

Written by Andrew Chan and Nicolas Lassabe and Hang Li

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

// These numbers need to be *tweaked* to achieve the desired behavior
#define GRAVITY -9.80665				// Magnitude of gravity; make sure it's a NEGATIVE value 
#define LINEAR_DAMPING 1				// Natural linear damping of the half cubes
#define ANGULAR_DAMPING 1				// Natural angular damping of the half cubes
#define CHEIGHT 0.6						// Height of half cubes (0.6m) 10 time big
#define TWIST_FORCE_LIMIT 48.5	 		// Maximum force that the motor applies to rotate  (Torque ax12 servo 16.5kg.cm 1.618Nm) molecube 49.5kg.cm 4.854Nm 10time because the object is time bigger
#define HALT_FORCE_LIMIT  48.5			// Maximum force that the motor applies to halt rotation
#define TARGET_VELOCITY 1.78			// Velocity that the motor attempts to achieve
#define SPEED_FACTOR 3495				// Conversion factor from the revolution by second to servo motor unit
#define MAX_MOTOR_SPEED 1023			// Maximum speed of AX12 servo
#define TRANSPARENT_VAL 0.50f

// Time step intervals
//#define TIME_STEP_INTERVAL (1.0f/60.0f)		// Joints separate too much
//#define TIME_STEP_INTERVAL (1.0f/120.0f)		// Good and moderately fast
//#define TIME_STEP_INTERVAL (1.0f/180.0f)		// Better but a little slow
//#define TIME_STEP_INTERVAL (1.0f/240.0f)		// Accurate but a little slower
//#define TIME_STEP_INTERVAL (1.0f/300.0f)		// Accurate but slower
//#define TIME_STEP_INTERVAL (1.0f/360.0f)		// Helps alleviate joint separation and collison problems
#define TIME_STEP_INTERVAL   (1.0f/500.0f)

#include "nxOgre.h"
#include "Ogre.h"
#include "EntityMaterialInstance.h"
#include "DynamicLines.h"
#include "Eihort.h"

#include "ActorUserData.h"
#include "Evolvement.h"
#include "math.h"
#include "molecubeMorphology.h"
#include "network.h"
#include "parameter.h"
#include <sstream>
#include <cstdlib>
#include <vld.h>
#include "Linear.h"
#include "stdafx.h"
#include "SequenceWriter.h"
#include "commands.h"


using namespace nxOgre;
using namespace Ogre;

void simulation(std::vector<body*>& halfCubes);

ofstream geGene;
ofstream bestMove;
ofstream evoResult;
ofstream data;
ifstream rBestMove;
ifstream rEvoResult;

std::wstring output0;
std::wstring output1;
std::wstring output2;
std::wstring output3;

double iPosition[2];
double angle[maxCubes][tRec];
static int rCounter = 0;

SYSTEMTIME now;

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
	
	Entity *ent ;

	// Vector to hold half cubes (only holds the lower half cubes)
	std::vector<body*> halfCubes;	

	// Stores the sequence of "add cube" commands for saving/loading
	std::string designString;
	MolecubeMorphology molecubeMorphology;

	geneType parent[Population];
	geneType spring[Population];
	
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Constructor
	CubeInterface() {}

	// Initialization function
	void start() {
		/////////////////////////////////////////////////////////////////////////////////
		// Set up the world and Scene
		mWorld = new world();

		////////////////////////////////////////////////////////////////////////////
		// Another aspect that can be tweaked to produce the desire results
		//
		// (1) Use this to set a fixed timestep for the simulation
		nxOgre::blueprint<nxOgre::scene> blueprint;
		blueprint.toDefault();

		blueprint.mTimeStepMethod = NX_TIMESTEP_FIXED;

		// Different levels of accuracy for PhysX
		blueprint.mMaxTimestep = TIME_STEP_INTERVAL;
		mScene = blueprint.create("Main",mSceneMgr);

		//***************************************************
		//
		// (2) Use this to let the time steps be handled automatically (variable time steps)
		//
		//mScene = mWorld->createScene("Main",mSceneMgr);	// Joints separate too much
		//
		////////////////////////////////////////////////////////////////////////////

		mScene->hasGravityAndFloor(Ogre::Vector3(0,GRAVITY,0));
		
		
		// NX_SKIN_WIDTH is the allowable interpenetration depth between two objects
		//    If this is too low, the simulation becomes jittery
		mScene->mPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.02);

		/////////////////////////////////////////////////////////////////////////////////
		std::cout <<"density battery: "<< DENSITY_BATTERY <<endl;
		std::cout <<"density controller: "<< DENSITY_CONTROLLER <<endl;
		std::cout <<"density gripper: "<< DENSITY_GRIPPER<<endl;
		std::cout <<"density actuator: "<< DENSITY_ACTUATOR <<endl;

		// Create initial bodies
		
		ent=createCubeTranparence(pose(Vector3(0,CHEIGHT,0), Quaternion(Radian(0), Vector3::UNIT_Y )),Vector3::UNIT_Y,cube_orientation,0,CLASS_BATTERY,faceOnTop,cube_angle,1,false);
		//  createCube(pose(Vector3(0,CHEIGHT,0), Quaternion(Radian(0), Vector3::UNIT_Y )),Vector3::UNIT_Y,cube_orientation,0,CLASS_BATTERY,faceOnTop,cube_angle,1,false);							
	}
	
	// Returns the lower cube if successful; returns NULL if unsuccessful

		

    unsigned int getID(body *selectedCube){
		for(unsigned int n=0;n<halfCubes.size();n++)
			if(selectedCube==halfCubes[n]||((ActorUserData*)(halfCubes[n]->ActorUserData))->associatedBody==selectedCube)
			  return n;
		cout <<"ERRROR Virtual ID**************************** "<<endl;
		cout <<"Size List Blocs: "<<(int)halfCubes.size()<<" Size Molecube: "<<(int)molecubeMorphology.getCubeCount()<<endl;
		return 0;
	}

void drawNetwork(Network network, vector<body*> halfCubes, float size){	
	float maxY=0;
	for(int n=0;n<(int)halfCubes.size();n++){
		body *cube=halfCubes[n];
		Vector3 pos=cube->getGlobalPosition();
		if(pos.y>maxY)
			maxY=pos.y;
	}
	for(int n=0;n<(int)network.modules.size();n++){
		body *cube=halfCubes[n];	
		Vector3 pos=cube->getGlobalPosition();			
		drawSquareLine(n,pos.x,pos.y+maxY+CHEIGHT,pos.z,size,FILENAME_BATTERY);
		network.modules[n]->setX(pos.x);
		network.modules[n]->setY(pos.y+maxY+CHEIGHT);
		network.modules[n]->setZ(pos.z);
	
	}
	
	for(int n=0;n<(int)network.modules.size();n++){
		for(int i=0;i<(int)network.modules[n]->inputs.size();i++){	
			network.modules[n]->inputs[i]->setX(size*(i+1)/(network.modules[n]->inputs.size()+1)-size/2);									
			network.modules[n]->inputs[i]->setY(0);
			network.modules[n]->inputs[i]->setZ(size*3/4-size/2);

			
		}
		for(int i=0;i<(int)network.modules[n]->hidens.size();i++){
			network.modules[n]->hidens[i]->setX(size*(1+i)/(network.modules[n]->hidens.size()+1)-size/2);				
			network.modules[n]->hidens[i]->setY(0);
			network.modules[n]->hidens[i]->setZ(size*2/4-size/2);
		}
		for(int i=0;i<(int)network.modules[n]->outputs.size();i++){
			network.modules[n]->outputs[i]->setX(size*(i+1)/(network.modules[n]->outputs.size()+1)-size/2);					
			network.modules[n]->outputs[i]->setY(0);
			network.modules[n]->outputs[i]->setZ(size*1/4-size/2);
		}
	}

	DynamicLines *lines = new DynamicLines(RenderOperation::OT_LINE_LIST,"gripper");
	for(int n=0;n<(int)network.modules.size();n++){
		for(int i=0;i<(int)network.modules[n]->inputs.size();i++){					
			for(int j=0;j<(int)network.modules[n]->inputs[i]->synapses.size();j++){
				Neural *input=network.modules[n]->inputs[i]->synapses[j]->getInput();
				Neural *output=network.modules[n]->inputs[i]->synapses[j]->getOutput();				
				Module *attachedModule=input->getAttachedModule();
				lines->addPoint(attachedModule->getX()+input->getX(),
								attachedModule->getY()+input->getY(),
								attachedModule->getZ()+input->getZ());
				attachedModule=output->getAttachedModule();
				lines->addPoint(attachedModule->getX()+output->getX(),
								attachedModule->getY()+output->getY(),
								attachedModule->getZ()+output->getZ());
			}
		}
		for(int i=0;i<(int)network.modules[n]->hidens.size();i++){					
			for(int j=0;j<(int)network.modules[n]->hidens[i]->synapses.size();j++){
				Neural *input=network.modules[n]->hidens[i]->synapses[j]->getInput();
				Neural *output=network.modules[n]->hidens[i]->synapses[j]->getOutput();	
				Module *attachedModule=input->getAttachedModule();
				lines->addPoint(attachedModule->getX()+input->getX(),
								attachedModule->getY()+input->getY(),
								attachedModule->getZ()+input->getZ());
				attachedModule=output->getAttachedModule();
				lines->addPoint(attachedModule->getX()+output->getX(),
								attachedModule->getY()+output->getY(),
								attachedModule->getZ()+output->getZ());
			}
		}
		for(int i=0;i<(int)network.modules[n]->outputs.size();i++){					
			for(int j=0;j<(int)network.modules[n]->outputs[i]->synapses.size();j++){
				Neural *input=network.modules[n]->outputs[i]->synapses[j]->getInput();
				Neural *output=network.modules[n]->outputs[i]->synapses[j]->getOutput();				
				Module *attachedModule=input->getAttachedModule();
				lines->addPoint(attachedModule->getX()+input->getX(),
								attachedModule->getY()+input->getY(),
								attachedModule->getZ()+input->getZ());
				attachedModule=output->getAttachedModule();
				lines->addPoint(attachedModule->getX()+output->getX(),
								attachedModule->getY()+output->getY(),
								attachedModule->getZ()+output->getZ());
			}
		}
	}
	
		
	lines->update();
	SceneNode *linesNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("network");
	linesNode->attachObject(lines);	
}

void drawSquareLine(int id,float x, float y, float z, float size, const char *material="BaseWhiteNoLighting"){	
		DynamicLines *lines = new DynamicLines(RenderOperation::OT_LINE_STRIP,material);
        lines->addPoint(x-size/2,y,z-size/2);
		lines->addPoint(x+size/2,y,z-size/2);
		lines->addPoint(x+size/2,y,z+size/2);
		lines->addPoint(x-size/2,y,z+size/2);	
		lines->addPoint(x-size/2,y,z-size/2);		
		lines->update();
		SceneNode *linesNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("line"+StringConverter::toString(id));
		linesNode->attachObject(lines);
/*
		MovableText* msg = new MovableText("TXT_001"+StringConverter::toString(id), "this is the caption");
		msg->setTextAlignment(MovableText::H_CENTER, MovableText::V_ABOVE); 
		msg->setAdditionalHeight( 2.0f );
		linesNode->attachObject(msg); 
*/
}

body* addCube(body *selectedCube,unsigned char molecubeID,unsigned char cube_typeCubeID, unsigned facing, char faceOnTop,unsigned char cubeOrientation, unsigned short angle) {
	// Declarations for cube and joint positions
		Vector3 position, addLocation, jointOrientation;

		// orientation is one of the local axes of the body relative to the global axes
		Vector3 orientation;
		Quaternion quatern, jointQuatern;
		body *anchorCube, *addedCube;
		fixedJoint *tempFixedJoint;

		// Check to ensure that a cube hasn't already been added to the selected side

		switch(facing) {
			case 0:		
				if(((ActorUserData*)(selectedCube->ActorUserData))->fullCubeData->attachedCubes[0]) {
					std::cout <<"ERROR: face0 Check to ensure that a cube hasn't already been added to the selected side!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
					return NULL;
				}
				break;
			case 1:		
				if(((ActorUserData*)(selectedCube->ActorUserData))->fullCubeData->attachedCubes[1]) {
				std::cout <<"ERROR: face1 Check to ensure that a cube hasn't already been added to the selected side!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;		
				return NULL;
				}
				break;
			case 2:	
				if(((ActorUserData*)(selectedCube->ActorUserData))->fullCubeData->attachedCubes[2]) {
				std::cout <<"ERROR: face2 Check to ensure that a cube hasn't already been added to the selected side!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
				return NULL;
				}
				break;
			case 3:						
				if(((ActorUserData*)(selectedCube->ActorUserData))->fullCubeData->attachedCubes[3]) {
				std::cout <<"ERROR: face3 Check to ensure that a cube hasn't already been added to the selected side!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
				return NULL;
				}
				break;
			case 4:	
				if(((ActorUserData*)(selectedCube->ActorUserData))->fullCubeData->attachedCubes[4]) {
				std::cout <<"ERROR: face4 Check to ensure that a cube hasn't already been added to the selected side!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
				return NULL;
				}
				break;
			case 5:		
				
				if(((ActorUserData*)(selectedCube->ActorUserData))->fullCubeData->attachedCubes[5]) {
				std::cout <<"ERROR: face5 Check to ensure that a cube hasn't already been added to the selected side!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
				return NULL;
				}
				break;
			default:	// Invalid facing.  Default is face0.
				if(((ActorUserData*)(selectedCube->ActorUserData))->fullCubeData->attachedCubes[0]) {
					std::cout <<"ERROR: DEFAULF Check to ensure that a cube hasn't already been added to the selected side!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
					return NULL;
				}
				break;
		}
	
		// Force selection to upper half cube if lower half cube is selected
		if(((ActorUserData*)(selectedCube->ActorUserData))->lowerHalf) {
			anchorCube = ((ActorUserData*)(selectedCube->ActorUserData))->associatedBody;
		}
		else {
			anchorCube = selectedCube;
		}

		// Choose the appropriate half cube to attach the new cube to
		switch(facing) {
			case 3:		
				anchorCube = anchorCube;
				break;

			case 0:	
				anchorCube = ((ActorUserData*)(anchorCube->ActorUserData))->associatedBody;
				break;

			case 1:
				anchorCube = ((ActorUserData*)(anchorCube->ActorUserData))->associatedBody;
				break;

			case 5:
				anchorCube = anchorCube;
				break;

			case 4:	
				anchorCube = anchorCube;
				break;

			case 2:		
				anchorCube = ((ActorUserData*)(anchorCube->ActorUserData))->associatedBody;
				break;

			default:
			std::cout <<"ERROR: Choose the appropriate half cube to attach the new cube to !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
				anchorCube = anchorCube;
				break;	// Default is upper cube selected
		}
		
		position = anchorCube->getGlobalPosition();
		quatern = anchorCube->getGlobalOrientation();

		// Extract the specified local axis of the body
		switch(facing) {
			case 3:		
				orientation = Vector3(quatern.yAxis().x, quatern.yAxis().y, quatern.yAxis().z);
				break;
			case 0:		
				orientation = Vector3(-quatern.yAxis().x, -quatern.yAxis().y, -quatern.yAxis().z);
				break;
			case 1:		
				orientation = Vector3(quatern.zAxis().x, quatern.zAxis().y, quatern.zAxis().z);
				break;
			case 5:		
				orientation = Vector3(-quatern.zAxis().x, -quatern.zAxis().y, -quatern.zAxis().z);
				break;
			case 4:	
				orientation = Vector3(quatern.xAxis().x, quatern.xAxis().y, quatern.xAxis().z);
				break;
			case 2:		
				orientation = Vector3(-quatern.xAxis().x, -quatern.xAxis().y, -quatern.xAxis().z);
				break;
			default:
				 std::cout <<"ERROR: Extract the specified local axis of the body !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
				// Default
				orientation = Vector3(quatern.yAxis().x, quatern.yAxis().y, quatern.yAxis().z);
		}
		orientation.normalise();	

		// Add the cube based on the anchor cubes position and orientation
		addLocation = position + 2*CHEIGHT*orientation;
		
		
		// Create the fixed joint that fixes the new cube to the anchor cube
		// The joint connects the anchor cube's top half to the added cube's lower half
		
		switch(facing) {
			case 3:	
				if(faceOnTop<3){		
				  addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
				  addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;	
				}
				else
				  addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);

				tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			case 0:		// BOTTOM
				if(faceOnTop>2){				 				  
				 addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
				 addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;
				}
				else
				 addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);
				 tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			case 1:		
				if(faceOnTop>2){
				 addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
				 addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;
				}
				else
				 addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);	
				tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			case 5:
				if(faceOnTop<3){				 
				  addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
				  addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;	
				}
				else
				addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);
				tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			case 4:	
				if(faceOnTop<3){
				  addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
				  addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;	
				}
				else
				  addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);
				tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			case 2:	
				if(faceOnTop>2){
					addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
					addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;
				}
				else
				   addedCube = createCube(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);
				tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			default:
				cout << "ERROR: Joint creation error\n";
				break;// Default is no joint created
		}


		//tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);

		// Record the fixed joint to the corresponding half cubes
		((ActorUserData*)(anchorCube->ActorUserData))->fixJoint = tempFixedJoint;
		((ActorUserData*)(addedCube->ActorUserData))->fixJoint = tempFixedJoint;

		// Record which facings are being used
		//    Record for both cubes
		
		//std::cout <<"attached ID "<<(int)molecubeID<<" face"<<(int)getAddedFace(facing,faceOnTop)<<endl;

		((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[getAddedFace(facing,faceOnTop)] = anchorCube;
		((ActorUserData*)(selectedCube->ActorUserData))->fullCubeData->attachedCubes[facing]=addedCube;

		/*
	   ((ActorUserData*)(selectedCube->ActorUserData))->fullCubeData->attachedCubes[fromMMFselfFaceToVirtualFace(getAddedFace(facing,faceOnTop))] = addedCube;	

		switch(facing) {
			case 0:		// TOP								
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[1] = anchorCube;
				break;
			case 1:		// BOTTOM				
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[0] = anchorCube;
				break;
			case 2:		// LEFT				
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[3] = anchorCube;
				break;
			case 3:		// RIGHT			   
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[2] = anchorCube;
				break;
			case 4:		// FORWARD			
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[5] = anchorCube;
				break;
			case 5:		// BACK
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[4] = anchorCube;
				break;
			default:
				// Default is TOP
				std::cout<<"ERROR: attached error!!"<<endl;				
				break;
	    }
		*/
		//std::cout <<"molecubeID: "<<(int)molecubeID<<endl;
	  
	  //displayHalfList();
	  if(halfCubes.size()>0){
		unsigned char  cube1=molecubeMorphology.searchVirtualID(getID(selectedCube));
		unsigned char  selfFace=facing;
		ModuleDescriptor *cube2=molecubeMorphology.cubes[molecubeMorphology.searchVirtualID(halfCubes.size())];
		unsigned char neighborFace=getAddedFace(facing,faceOnTop);
		
		
		if(molecubeMorphology.cubes[cube1]->getMolecubeClass()==CLASS_BATTERY)
		  selfFace=batteryConvertToMMF(selfFace);

		if(cube2->getMolecubeClass()==CLASS_BATTERY)
		  neighborFace=batteryConvertToMMF(neighborFace);

		molecubeMorphology.cubes[cube1]->addLink(selfFace,cube2,neighborFace,cubeOrientation);

		/*
		std::cout <<"***************************"<<endl;
		std::cout <<"nbcube:"<<(int)molecubeMorphology.getCubeCount()<<endl;
		for(int n=0;n<molecubeMorphology.getCubeCount();n++)			
			std::cout <<"n:"<<n<<" ID:"<<(int)molecubeMorphology.cubes[n]->getID()<<" class:"<<(int)molecubeMorphology.cubes[n]->getMolecubeClass()<<" nblink:"<<(int)molecubeMorphology.cubes[n]->getNumLinks()<<endl;		 
		std::cout <<"Links:"<<(int)molecubeMorphology.searchCubeID(getID(selectedCube))<<"->"<<(int)molecubeMorphology.searchCubeID(halfCubes.size())<<endl;
		std::cout <<"face:"<<(int)facing<<" face:"<<(int)getAddedFace(facing,faceOnTop)<<" faceOnTop"<<(int)faceOnTop<<endl;
		*/
	  }
		return addedCube;
	}



Entity* addCubeTranparence(body *selectedCube,unsigned char molecubeID,unsigned char cube_typeCubeID, unsigned facing, char faceOnTop,unsigned char cubeOrientation, unsigned short angle) {
	// Declarations for cube and joint positions
		Vector3 position, addLocation, jointOrientation;

		// orientation is one of the local axes of the body relative to the global axes
		Vector3 orientation;
		Quaternion quatern, jointQuatern;
		body *anchorCube;
		Entity *addedCube;
		
		// Force selection to upper half cube if lower half cube is selected

		if(((ActorUserData*)(selectedCube->ActorUserData))->lowerHalf) {
			anchorCube = ((ActorUserData*)(selectedCube->ActorUserData))->associatedBody;
		}
		else {
			anchorCube = selectedCube;
		}
		
		// Choose the appropriate half cube to attach the new cube to
	
		switch(facing) {
			case 3:	
				anchorCube = anchorCube;
				break;

			case 0:
				anchorCube = ((ActorUserData*)(anchorCube->ActorUserData))->associatedBody;
				break;

			case 1:	
				anchorCube = ((ActorUserData*)(anchorCube->ActorUserData))->associatedBody;
				break;

			case 5:	
				anchorCube = anchorCube;
				break;

			case 4:		
				anchorCube = anchorCube;
				break;

			case 2:	
				anchorCube = ((ActorUserData*)(anchorCube->ActorUserData))->associatedBody;
				break;

			default:
			std::cout <<"ERROR: Choose the appropriate half cube to attach the new cube to !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
				anchorCube = anchorCube;
				break;	// Default is upper cube selected
		}
		
		position = anchorCube->getGlobalPosition();
		quatern = anchorCube->getGlobalOrientation();

		// Extract the specified local axis of the body
		switch(facing) {
			case 3:
				orientation = Vector3(quatern.yAxis().x, quatern.yAxis().y, quatern.yAxis().z);
				break;
			case 0:
				orientation = Vector3(-quatern.yAxis().x, -quatern.yAxis().y, -quatern.yAxis().z);
				break;
			case 1:	
				orientation = Vector3(quatern.zAxis().x, quatern.zAxis().y, quatern.zAxis().z);
				break;
			case 5:
				orientation = Vector3(-quatern.zAxis().x, -quatern.zAxis().y, -quatern.zAxis().z);
				break;
			case 4:	
				orientation = Vector3(quatern.xAxis().x, quatern.xAxis().y, quatern.xAxis().z);
				break;
			case 2:	
				orientation = Vector3(-quatern.xAxis().x, -quatern.xAxis().y, -quatern.xAxis().z);
				break;
			default:
				 std::cout <<"ERROR: Extract the specified local axis of the body !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
				// Default is TOP
				orientation = Vector3(quatern.yAxis().x, quatern.yAxis().y, quatern.yAxis().z);
		}
		orientation.normalise();	

		// Add the cube based on the anchor cubes position and orientation
		addLocation = position + 2*CHEIGHT*orientation;
		
		
		// Create the fixed joint that fixes the new cube to the anchor cube
		// The joint connects the anchor cube's top half to the added cube's lower half

		switch(facing) {
			case 3:
				if(faceOnTop<3){		
				  addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
				  //addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;	
				}
				else
				  addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);

				  //tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			case 0:
				if(faceOnTop>2){				 				  
				 addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
				// addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;
				}
				else
				 addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);
				 //tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			case 1:		
				if(faceOnTop>2){
				 addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
				 //addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;
				}
				else
				 addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);	
				//tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			case 5:
				if(faceOnTop<3){				 
				  addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
				  //addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;	
				}
				else
				addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);
				//tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			case 4:	
				if(faceOnTop<3){
				  addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
				  //addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;	
				}
				else
				  addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);
				  //tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			case 2:	
				if(faceOnTop>2){
					addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,0,false);
					//addedCube = ((ActorUserData*)(addedCube->ActorUserData))->associatedBody;
				}
				else
				   addedCube = createCubeTranparence(pose(addLocation, quatern),orientation,cubeOrientation,molecubeID,cube_typeCubeID,faceOnTop,angle,1,false);
				   //tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);
				break;

			default:
				cout << "ERROR: Joint creation error\n";
				break;// Default is no joint created
		}




		//tempFixedJoint = new fixedJoint(anchorCube, addedCube, position+CHEIGHT*orientation);

		// Record the fixed joint to the corresponding half cubes
		//((ActorUserData*)(anchorCube->ActorUserData))->fixJoint = tempFixedJoint;
		//((ActorUserData*)(addedCube->ActorUserData))->fixJoint = tempFixedJoint;

		// Record which facings are being used
		//    Record for both cubes
		/*	
		std::cout <<"ttached ID "<<(int)molecubeID<<" face"<<(int)fromMMFselfFaceToVirtualFace(getAddedFace(facing,faceOnTop))<<"-> face"<<(int)getAddedFace(facing,faceOnTop)<<endl;
		((ActorUserData*)(selectedCube->ActorUserData))->fullCubeData->attachedCubes[fromMMFselfFaceToVirtualFace(getAddedFace(facing,faceOnTop))] = addedCube;	
		switch(facing) {
			case 0:		// TOP								
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[1] = anchorCube;
				break;
			case 1:		// BOTTOM				
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[0] = anchorCube;
				break;
			case 2:		// LEFT				
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[3] = anchorCube;
				break;
			case 3:		// RIGHT			   
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[2] = anchorCube;
				break;
			case 4:		// FORWARD			
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[5] = anchorCube;
				break;
			case 5:		// BACK
				((ActorUserData*)(addedCube->ActorUserData))->fullCubeData->attachedCubes[4] = anchorCube;
				break;
			default:
				// Default is TOP
				std::cout<<"attached error!!"<<endl;				
				break;
	    }
		std::cout <<"molecubeID: "<<(int)molecubeID<<endl;


		unsigned char  cube1=molecubeMorphology.searchCubeID(getID(selectedCube));
		unsigned char  selfFace=fromVirtualFaceToMMFselfFace(facing);
		ModuleDescriptor *cube2=molecubeMorphology.cubes[molecubeMorphology.searchCubeID(halfCubes.size())];
		unsigned char neighborFace=getAddedFace(facing,faceOnTop);
		unsigned char orient=fromCubeOrientationToMMFneighborFaceOrientation(cubeOrientation);
		
		if(molecubeMorphology.cubes[cube1]->getMolecubeClass()==CLASS_BATTERY)
		  selfFace=batteryConvertToMMF(selfFace);

		if(cube2->getMolecubeClass()==CLASS_BATTERY)
		  neighborFace=batteryConvertToMMF(neighborFace);

		molecubeMorphology.cubes[cube1]->addLink(selfFace,cube2,neighborFace,orient);
		*/
		/* to remove 
		molecubeMorphology.cubes[molecubeMorphology.searchCubeID(getID(selectedCube))]->addLink(
			fromVirtualFaceToMMFselfFace(facing),molecubeMorphology.cubes[molecubeMorphology.searchCubeID(halfCubes.size())],
			getAddedFace(facing,faceOnTop),
			fromCubeOrientationToMMFneighborFaceOrientation(cubeOrientation));
		*/
		/*	
		std::cout <<"***************************"<<endl;
		std::cout <<"nbcube:"<<(int)molecubeMorphology.getCubeCount()<<endl;
		for(int n=0;n<molecubeMorphology.getCubeCount();n++)			
			std::cout <<"n:"<<n<<" ID:"<<(int)molecubeMorphology.cubes[n]->getID()<<" class:"<<(int)molecubeMorphology.cubes[n]->getMolecubeClass()<<" nblink:"<<(int)molecubeMorphology.cubes[n]->getNumLinks()<<endl;		 
		std::cout <<"Links:"<<(int)molecubeMorphology.searchCubeID(getID(selectedCube))<<"->"<<(int)molecubeMorphology.searchCubeID(halfCubes.size())<<endl;
		std::cout <<"face:"<<(int)fromVirtualFaceToMMFselfFace(facing)<<" face:"<<(int)getAddedFace(facing,faceOnTop)<<" facing"<<(int)facing<<" faceOnTop"<<(int)faceOnTop<<endl;
		*/
		return addedCube;
	}

	char batteryConvertFace(char face){
		char table[6];
		table[0]=0;
		table[1]=1;
		table[2]=2;
		table[3]=5;		
		table[4]=3;
		table[5]=4;
		if(face==5){
			std::cout <<"ERROR: Battery wrong face"<<endl;
		//exit(0);
		}
		return table[face];
	}

	char batteryConvertToMMF(char face){
		char table[6];
		table[0]=0;
		table[1]=1;
		table[2]=2;
		table[3]=4;		
		table[4]=5;
		table[5]=3;
		if(table[face]==5){
			std::cout <<"ERROR: Battery wrong face"<<endl;
		//exit(0);
		}
		return table[face];
	}

	char getTopFace(char selfSide,char neighborSide){
		char table[6][6];
		table[0][0]=0;
		table[0][1]=1;
		table[0][2]=2;
		table[0][3]=3;
		table[0][4]=4;
		table[0][5]=5;
		
		table[1][0]=1;
		table[1][1]=2;
		table[1][2]=0;
		table[1][3]=4;
		table[1][4]=5;
		table[1][5]=3;

		table[2][0]=2;
		table[2][1]=0;
		table[2][2]=1;
		table[2][3]=5;
		table[2][4]=3;
		table[2][5]=4;
		
		table[3][0]=3;
		table[3][1]=5;
		table[3][2]=4;
		table[3][3]=0;
		table[3][4]=2;
		table[3][5]=1;
		
		table[4][0]=5;
		table[4][1]=4;
		table[4][2]=3;
		table[4][3]=2;
		table[4][4]=1;
		table[4][5]=0;

		table[5][0]=4;
		table[5][1]=3;
		table[5][2]=5;
		table[5][3]=1;
		table[5][4]=0;
		table[5][5]=2;
		return table[selfSide][neighborSide];
	}

char getAddedFace(unsigned facing,char faceOnTop){
		char table[6][6];		
		table[0][0]=0;
		table[0][1]=1;
		table[0][2]=2;
		table[0][3]=3;
		table[0][4]=4;
		table[0][5]=5;
		
		table[1][0]=1;
		table[1][1]=2;
		table[1][2]=0;
		table[1][3]=4;
		table[1][4]=5;
		table[1][5]=3;

		table[2][0]=2;
		table[2][1]=0;
		table[2][2]=1;
		table[2][3]=5;
		table[2][4]=3;
		table[2][5]=4;
		
		table[3][0]=3;
		table[3][1]=5;
		table[3][2]=4;
		table[3][3]=0;
		table[3][4]=2;
		table[3][5]=1;
		
		table[4][0]=5;
		table[4][1]=4;
		table[4][2]=3;
		table[4][3]=2;
		table[4][4]=1;
		table[4][5]=0;

		table[5][0]=4;
		table[5][1]=3;
		table[5][2]=5;
		table[5][3]=1;
		table[5][4]=0;
		table[5][5]=2;
		
		//std::cout<<"facing: "<<(int)fromVirtualFaceToMMFselfFace(facing)<<" faceOnTop"<<(int)faceOnTop<<endl;
		for(int n=0;n<6;n++){
			//std::cout<<"table["<<(int)fromVirtualFaceToMMFselfFace(facing)<<"]["<<n<<"]="<<(int)table[((int)fromVirtualFaceToMMFselfFace(facing))][n]<<" "<<(int)(Facing)faceOnTop<<endl;
			if(table[facing][n]==faceOnTop){
				//std::cout <<"return "<<n<<endl;
			 return n;
			}
		}
  std::cout<<"error getAddedFace Function"<<endl;
  return 0;
}

	void deleteCube(body* cube){
			body *bodyLinked;
			int id=getID(cube);
			std::cout <<"Cube virtual id"<<id<<" deleted."<<endl;
			displayHalfList();
			for (int i =id;i<(int)(halfCubes.size()-1);i++)
				halfCubes[i]=halfCubes[i+1];			
			halfCubes.pop_back();											
			displayHalfList();

			for(int n=0;n<6;n++){
			    if(((ActorUserData*)(cube->ActorUserData))->fullCubeData->attachedCubes[n]!=NULL){
					bodyLinked=((ActorUserData*)(cube->ActorUserData))->fullCubeData->attachedCubes[n];
					for(int i=0;i<6;i++)
						if(((ActorUserData*)(bodyLinked->ActorUserData))->fullCubeData->attachedCubes[i]==cube||((ActorUserData*)(bodyLinked->ActorUserData))->fullCubeData->attachedCubes[i]==((ActorUserData*)(cube->ActorUserData))->associatedBody)
						  ((ActorUserData*)(bodyLinked->ActorUserData))->fullCubeData->attachedCubes[i]=NULL;
				}						
			}
			
			std::cout <<"Cube "<<cube->getName()<<" deleted."<<endl;

			if(((ActorUserData *)(cube->ActorUserData))->associatedBody!=NULL&&((ActorUserData *)(cube->ActorUserData))->associatedBody!=cube)
				mScene->destroyBody(((ActorUserData *)(cube->ActorUserData))->associatedBody->getName());
			mScene->destroyBody(cube->getName());
	}


	// Every cube is assigned an ID number
	body* createCube(pose cPose, Vector3 orientation, unsigned char cubeOrientation,unsigned char molecubeID,unsigned char typeOfCube,char faceOnTop, unsigned short int angle, bool rotU, bool kinematic) {
		body *tempBody1, *tempBody2;
		SDFJoint *tempD6Joint;
		int id = halfCubes.size();
		Quaternion quatern;
		pose cPoseL, cPoseU;
		
		Vector3 vx,vy,vz;
		quatern = NxTools::convert(cPose.q);
		vx = Vector3(quatern.xAxis().x, quatern.xAxis().y, quatern.xAxis().z);
		vx.normalise();
		vy = Vector3(quatern.yAxis().x, quatern.yAxis().y, quatern.yAxis().z);
		vy.normalise();
		vz = Vector3(quatern.zAxis().x, quatern.zAxis().y, quatern.zAxis().z);
		vz.normalise();

		//std::cout <<"Face on top: "<< (int)faceOnTop <<endl;
		switch(faceOnTop) {
			case 0: 
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI),vz)*cPose.getQuaternion());
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI/2),vy)*cPose.getQuaternion());
				break;
			case 1:  
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI*3/2),vx)*cPose.getQuaternion());
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI),vy)*cPose.getQuaternion());
				break;
			case 2: 
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI*3/2),vz)*cPose.getQuaternion());
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI),vy)*cPose.getQuaternion());
				break;
			case 3: 				
				break;
			case 4:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI/2),vz)*cPose.getQuaternion());
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI*3/2),vy)*cPose.getQuaternion());
				break;
			case 5:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI/2),vx)*cPose.getQuaternion());
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI/2),vy)*cPose.getQuaternion());
				break;
			default:
				std::cout << "ERROR: For face on TOP "<<faceOnTop<<"\n";			
				break;
		}
		
		switch(cubeOrientation) {
			case 2:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(0),orientation)*cPose.getQuaternion());
				break;
			case 3:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI/2),orientation)*cPose.getQuaternion());
				break;
			case 0:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI),orientation)*cPose.getQuaternion());
				break;
			case 1:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI*3/2),orientation)*cPose.getQuaternion());
				break;
			default:
				std::cout << "ERROR: INVALID CUBE_ORIENTATION VALUE\nDefault is NONE\n";
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(0),orientation)*cPose.getQuaternion());
				break;
		}
	
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

		//rotation of the upper half of cube
		if(rotU){			
			cPoseU = pose(Vector3(cPose.getVector3()), Quaternion(Degree((angle)/10),jointOrientation)*cPose.getQuaternion());	
			cPoseL = cPose;
		}
		else{
			cPoseU = cPose;
			cPoseL = pose(Vector3(cPose.getVector3()), Quaternion(Degree((3600-angle)/10),jointOrientation)*cPose.getQuaternion());	
		}

		switch(typeOfCube){
		case CLASS_ACTUATOR:
		// Create lower half of cube
		tempBody1 = mScene->createBody(
			"halfCube1." + StringConverter::toString(id),	// Half-cube name
			"cubeB.mesh",   							// Half-cube image
			new convexShape("collisionPinCubeB.mesh"), // Half-cube collision mesh		
			DENSITY_ACTUATOR,										// Half-cube density
			cPoseL											// Half-cube pose
			);
		tempBody1->setKinematic(kinematic);
		tempBody1->setAngularDamping(ANGULAR_DAMPING);
		tempBody1->setLinearDamping(LINEAR_DAMPING);
		

		// Create upper half of cube
		tempBody2 = mScene->createBody(
			"halfCube2." + StringConverter::toString(id),	// Half-cube name
			"cubeT.mesh",// Half-cube image			
		     new convexShape("collisionPinCubeT.mesh"),// Half-cube collision mesh
			DENSITY_ACTUATOR,										// Half-cube density
			cPoseU											// Half-cube pose
		);
	

		tempBody2->setAngularDamping(ANGULAR_DAMPING);
		tempBody2->setLinearDamping(LINEAR_DAMPING);

		tempD6Joint = new SDFJoint(tempBody1, tempBody2, cPose.getVector3(), jointOrientation);
		break;

		
		case CLASS_BATTERY:
			tempBody1 = mScene->createBody(
			"battery." + StringConverter::toString(id),	// Half-cube name
			"Battery.mesh",										// cube image								
			
			new convexShape("collisionController.mesh"),		
			DENSITY_BATTERY,											// Half-cube density
			cPose												// Half-cube pose
			);		
			tempBody2=tempBody1;
			tempD6Joint=NULL;
		break;

		 case CLASS_GRIPPER:
			tempBody1 = mScene->createBody(
			"gripper." + StringConverter::toString(id),	// Half-cube name
			"Gripper.mesh",										// cube image										
			new convexShape("collisionGripper.mesh"),		
			DENSITY_GRIPPER,											// Half-cube density
			cPose												// Half-cube pose
			);		
			tempBody2=tempBody1;
			tempD6Joint=NULL;
		break;

		case CLASS_BASE:			
			tempBody1 = mScene->createBody(
			"base." + StringConverter::toString(id),
			"base.mesh",																			
			new convexShape("collisionBase.mesh"),		
			DENSITY_BASE,								
			cPose									
			);
			tempBody2=tempBody1;
			tempD6Joint=NULL;
			
			break;

		case CLASS_CONTROLLER:
			
			tempBody1 = mScene->createBody(
			"controller." + StringConverter::toString(id),	// Half-cube name
			"controller.mesh",										// cube image										
			new convexShape("collisionController.mesh"),		
			DENSITY_CONTROLLER,											// Half-cube density
			cPose												// Half-cube pose
			);		
			//tempBody1->getEntity();
			tempBody2=tempBody1;
			tempD6Joint=NULL;

			
			break;
		
		default:
			std::cout << "error typeOfcube !! "<< typeOfCube<<endl;
		break;
		}
		// Create ActorUserData for each of the halves
		//    ActorUserData(int id, D6Joint joint, body assocBody, bool lowHalf)
		tempBody1->ActorUserData = new ActorUserData(id, tempD6Joint, tempBody2, true);
		tempBody2->ActorUserData = new ActorUserData(id, tempD6Joint, tempBody1, false);
		

		FullCubeData *fullCubeData = new FullCubeData();
		
		((ActorUserData *)(tempBody1->ActorUserData))->fullCubeData = fullCubeData;
		((ActorUserData *)(tempBody2->ActorUserData))->fullCubeData = fullCubeData;

		((ActorUserData *)(tempBody1->ActorUserData))->jointOrientation = jointOrientation;
		((ActorUserData *)(tempBody2->ActorUserData))->jointOrientation = jointOrientation;

		// Record the cube's orientation in FullCubeData
		((ActorUserData *)(tempBody1->ActorUserData))->fullCubeData->cubeOrientation = cubeOrientation;


		// Returns the anchor actor; the "bottom" cube
		
		molecubeMorphology.addMolecube(new ModuleDescriptor(molecubeID,typeOfCube,angle,id));
		return tempBody1;
	}


// Every cube is assigned an ID number
 Entity* createCubeTranparence(pose cPose, Vector3 orientation, unsigned cubeOrientation,unsigned char molecubeID,unsigned char typeOfCube,char faceOnTop, unsigned short int angle, bool rotU, bool kinematic) {
		Entity *ent;
		SceneNode *node;
		EntityMaterialInstance *emi;
			
		//body *tempBody1, *tempBody2;
		//SDFJoint *tempD6Joint;
		int id = halfCubes.size();
		Quaternion quatern;
		pose cPoseL, cPoseU;
		
		Vector3 vx,vy,vz;
		quatern = NxTools::convert(cPose.q);
		vx = Vector3(quatern.xAxis().x, quatern.xAxis().y, quatern.xAxis().z);
		vx.normalise();
		vy = Vector3(quatern.yAxis().x, quatern.yAxis().y, quatern.yAxis().z);
		vy.normalise();
		vz = Vector3(quatern.zAxis().x, quatern.zAxis().y, quatern.zAxis().z);
		vz.normalise();

		std::cout <<"Face on top: "<< (int)faceOnTop <<endl;
		switch(faceOnTop) {
			case 0: 
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI),vz)*cPose.getQuaternion());
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI/2),vy)*cPose.getQuaternion());
				break;
			case 1:  
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI*3/2),vx)*cPose.getQuaternion());
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI),vy)*cPose.getQuaternion());
				break;
			case 2: 
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI*3/2),vz)*cPose.getQuaternion());
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI),vy)*cPose.getQuaternion());
				break;
			case 3: 				
				break;
			case 4:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI/2),vz)*cPose.getQuaternion());
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI*3/2),vy)*cPose.getQuaternion());
				break;
			case 5:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI/2),vx)*cPose.getQuaternion());
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI/2),vy)*cPose.getQuaternion());
				break;
			default:
				std::cout << "ERROR: For face on TOP "<<faceOnTop<<"\n";			
				break;
		}
		
		switch(cubeOrientation) {
			case 2:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(0),orientation)*cPose.getQuaternion());
				break;
			case 3:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI/2),orientation)*cPose.getQuaternion());
				break;
			case 0:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI),orientation)*cPose.getQuaternion());
				break;
			case 1:
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(Math::PI*3/2),orientation)*cPose.getQuaternion());
				break;
			default:
				std::cout << "ERROR: INVALID CUBE_ORIENTATION VALUE\nDefault is NONE\n";
				cPose = pose(Vector3(cPose.getVector3()), Quaternion(Radian(0),orientation)*cPose.getQuaternion());
				break;
		}
	
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

		//rotation of the upper half of cube
		if(rotU){			
			cPoseU = pose(Vector3(cPose.getVector3()), Quaternion(Degree((angle)/10),jointOrientation)*cPose.getQuaternion());	
			cPoseL = cPose;
		}
		else{
			cPoseU = cPose;
			cPoseL = pose(Vector3(cPose.getVector3()), Quaternion(Degree((3600-angle)/10),jointOrientation)*cPose.getQuaternion());	
		}

		switch(typeOfCube){
		case CLASS_ACTUATOR:
			ent = mSceneMgr->createEntity ("cubeB", "cubeB.mesh");			
			node = mSceneMgr->getRootSceneNode()->createChildSceneNode ("cubeB");
			node->attachObject (ent);
			node->setPosition(Vector3(cPoseL.v.x,cPoseL.v.y,cPoseL.v.z));
			node->setOrientation(Quaternion((float)cPoseL.q.w,(float)cPoseL.q.x,(float)cPoseL.q.y,(float)cPoseL.q.z));	
			emi = new EntityMaterialInstance (ent);
			emi->setTransparency (TRANSPARENT_VAL);
			emi->setMaterialName ("cube");					
			emi->setSceneBlending (SBT_TRANSPARENT_ALPHA);		
			emi->setSceneBlending (SBT_ADD);

			ent = mSceneMgr->createEntity ("cubeT", "cubeT.mesh");			
			node = mSceneMgr->getRootSceneNode()->createChildSceneNode ("cubeT");
			node->attachObject (ent);
			node->setPosition(Vector3(cPoseU.v.x,cPoseU.v.y,cPoseU.v.z));
			node->setOrientation(Quaternion(cPoseU.q.w,cPoseU.q.x,cPoseU.q.y,cPoseU.q.z));			
			emi = new EntityMaterialInstance (ent);
			emi->setTransparency (TRANSPARENT_VAL);
			emi->setMaterialName ("cube");			
			emi->setSceneBlending (SBT_TRANSPARENT_ALPHA);		
			emi->setSceneBlending (SBT_ADD);
		break;		
		
		case CLASS_BATTERY:
			ent = mSceneMgr->createEntity ("battery", "battery.mesh");			
			node = mSceneMgr->getRootSceneNode()->createChildSceneNode ("battery");
			node->attachObject (ent);
			node->setPosition(Vector3(cPose.v.x,cPose.v.y,cPose.v.z));
			node->setOrientation(Quaternion(cPose.q.w,cPose.q.x,cPose.q.y,cPose.q.z));			
			emi = new EntityMaterialInstance (ent);
			emi->setTransparency (TRANSPARENT_VAL);
			emi->setMaterialName ("battery");					
			emi->setSceneBlending (SBT_TRANSPARENT_ALPHA);		
			emi->setSceneBlending (SBT_ADD);
		break;

		 case CLASS_GRIPPER:
			ent = mSceneMgr->createEntity ("gripper", "gripper.mesh");			
			node = mSceneMgr->getRootSceneNode()->createChildSceneNode ("gripper");
			node->attachObject (ent);
			node->setPosition(Vector3(cPose.v.x,cPose.v.y,cPose.v.z));
			node->setOrientation(Quaternion(cPose.q.w,cPose.q.x,cPose.q.y,cPose.q.z));			
			emi = new EntityMaterialInstance (ent);
			emi->setTransparency (TRANSPARENT_VAL);
			emi->setMaterialName ("gripper");			
			emi->setSceneBlending (SBT_TRANSPARENT_ALPHA);							
			emi->setSceneBlending (SBT_ADD);
		break;

		case CLASS_BASE:			
			ent = mSceneMgr->createEntity (FILENAME_BASE, "base.mesh");			
			node = mSceneMgr->getRootSceneNode()->createChildSceneNode (FILENAME_BASE);
			node->attachObject (ent);
			node->setPosition(Vector3(cPose.v.x,cPose.v.y,cPose.v.z));
			node->setOrientation(Quaternion(cPose.q.w,cPose.q.x,cPose.q.y,cPose.q.z));			
			emi = new EntityMaterialInstance (ent);
			emi->setTransparency (TRANSPARENT_VAL);
			emi->setMaterialName (FILENAME_BASE);			
			emi->setSceneBlending (SBT_TRANSPARENT_ALPHA);
			emi->setSceneBlending (SBT_ADD);
			break;
		
		case CLASS_CONTROLLER:			
			ent = mSceneMgr->createEntity ("controller", "controller.mesh");			
			node = mSceneMgr->getRootSceneNode()->createChildSceneNode ("controller");
			node->attachObject (ent);
			node->setPosition(Vector3(cPose.v.x,cPose.v.y,cPose.v.z));
			node->setOrientation(Quaternion(cPose.q.w,cPose.q.x,cPose.q.y,cPose.q.z));			
			emi = new EntityMaterialInstance (ent);
			emi->setTransparency (TRANSPARENT_VAL);
			emi->setMaterialName ("controller");
			emi->setSceneBlending (SBT_TRANSPARENT_ALPHA);
			emi->setSceneBlending (SBT_ADD);
			break;
		
		default:
			std::cout << "error typeOfcube !! "<< typeOfCube<<endl;
		break;
		}
		
		return ent;
	}

	void setAngle(body *selectedCube,int angle){
		/*
		pose cPose;
		Vector3 position;
		Quaternion quatern;
		Vector3 jointOrientation;
		jointOrientation=((ActorUserData*)(selectedCube->ActorUserData))->jointOrientation;			
		if(((ActorUserData*)(selectedCube->ActorUserData))->lowerHalf)
		  selectedCube = ((ActorUserData*)(selectedCube->ActorUserData))->associatedBody;
		
		position = selectedCube->getGlobalPosition();
		quatern =  selectedCube->getGlobalOrientation();
		
		cPose=pose(position, Quaternion(Degree(angle),jointOrientation)*quatern);
		
		selectedCube->setGlobalOrientation(cPose.q.w,cPose.q.x,cPose.q.y,cPose.q.z);
		//selectedCube->setGlobalPosition(cPose.v.x,cPose.v.y,cPose.v.z);	
		selectedCube->setGlobalPosition(0,100,0);	
		*/
	}


	void rotateClockwise(SDFJoint *joint) {
	  if(joint!=NULL){
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
	}

	void rotateCounterClockwise(SDFJoint *joint){	
      if(joint!=NULL){
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
	}

	void haltRotation(SDFJoint *joint) {
      if(joint!=NULL){
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
	}
	
	void saveDesign(){
		updateAngle();
		molecubeMorphology.display();
		molecubeMorphology.saveMMFfile(mGUI_fileName->getValue().c_str());
	}

	void saveDesign2() {
		ofstream design(mGUI_fileName->getValue().c_str(), ios::out);
		//for(std::vector<body*>::iterator iter = halfCubes.begin(); iter != halfCubes.end(); iter++) {
		//	std::cout << i << endl;
		//	design << *(((ActorUserData*)(*iter)->ActorUserData)->fullCubeData->outputData());
		//}
		design << designString;
		design.close();
	}

void loadParameter(){
	Parameter p;
	string s=mGUI_fileName->getValue().c_str();
	string name = s.substr(0,(s.length() - 4));
	s = name + ".par";
	if(p.load(&s[0])){
		faceOnTop=p.getFaceOnTop();
		cube_faceOnTop=true;
		height_star=p.getHeight_star();			
		char str[6];
		gcvt(height_star,2,str);		
		mGUI_CubeHight->setValue(str);
	}
}

void saveParameter(){
	Parameter p;
	string s=mGUI_fileName->getValue().c_str();
	string name = s.substr(0,(s.length() - 4));
	s = name + ".par";
    p.setFaceOnTop(faceOnTop);
	p.setHeight_star(height_star);
	p.save(&s[0]);		
}

void loadDesign(){	
		restart(halfCubes);	
		displayHalfList();		
		molecubeMorphology.clear();

		MolecubeMorphology molecube;
		unsigned char facing;
		unsigned char cubeOrientation;

		body *selectedCube;					
	
		if(molecube.loadMMFfile(mGUI_fileName->getValue().c_str())){		
			
			//Give a virtualID to the physical Cube
			for(int n=0;n<molecube.getCubeCount();n++)
				molecube.cubes[n]->setVirtualID(n);								

			molecube.display();
			
			if(molecube.getCubeCount()>0){
				halfCubes.push_back(createCube(pose(Vector3(0,CHEIGHT*height_star,0), Quaternion(Radian(0), Vector3::UNIT_Y )),Vector3::UNIT_Y,cube_orientation,molecube.cubes[0]->getID(),molecube.cubes[0]->getMolecubeClass(),faceOnTop,molecube.cubes[0]->getAngle(),1,false));					
				selectedCube=halfCubes[molecube.cubes[0]->getVirtualID()];			
			

			for(int n=0;n<molecube.getCubeCount();n++){
				//std::cout<<n<<" ID: "<< (int)molecube.cubes[n]->getID()<<" nbLinks: "<<(int)molecube.cubes[n]->getNumLinks()<<endl;

			 for(int i=0;i<molecube.cubes[n]->getNumLinks();i++){
				//molecubeMorphology.display();
				//std::cout<<"  -link:"<<(int)molecube.cubes[n]->links[i]->getSelfID()<<"->"<<(int)molecube.cubes[n]->links[i]->getNeighborID()<<endl; 

				int cubesPos1=molecubeMorphology.searchCubeID(molecube.cubes[n]->links[i]->getSelfID());
				int cubesPos2=molecube.searchCubeID(molecube.cubes[n]->links[i]->getNeighborID());				
				
				//std::cout <<"  -pos1:"<< (int)cubesPos1<<"  -pos2:"<< (int)cubesPos2<<endl;
				//std::cout <<"  -virtual:"<<(int)molecubeMorphology.cubes[cubesPos1]->getVirtualID()<<"->"<<(int)molecube.cubes[cubesPos2]->getVirtualID()<<" face1:"<< (int)molecube.cubes[n]->links[i]->getSelfSide()<< " face2:"<< (int)molecube.cubes[n]->links[i]->getNeighborSide()<<" orientation:"<< (int)molecube.cubes[n]->links[i]->getSelfOrient()<<" classOfcube:"<<hex<<(int)molecube.cubes[n]->links[i]->getNeighborClass()<<endl;					
				
				unsigned char virtualID=molecubeMorphology.cubes[cubesPos1]->getVirtualID();
				if(virtualID<halfCubes.size()){
				 selectedCube = halfCubes[virtualID];

				// if(molecubeMorphology.cubes[cubesPos1]->getVirtualID()<molecube.cubes[cubesPos2]->getVirtualID()){
				//	if(molecubeMorphology.cubes[cubesPos1]->getVirtualID()<halfCubes.size()){
					//std::cout <<" virtual:"<<(int)molecube.cubes[cubesPos1]->getVirtualID()<<"->"<<(int)molecube.cubes[cubesPos2]->getVirtualID()<<" face1:"<< (int)molecube.cubes[n]->links[i]->getSelfSide()<< " face2:"<< (int)molecube.cubes[n]->links[i]->getNeighborSide()<<" orientation:"<< (int)molecube.cubes[n]->links[i]->getSelfOrient()<<" classOfcube:"<<hex<<(int)molecube.cubes[n]->links[i]->getNeighborClass()<<endl;
				
				 //if the cube doesn't exit, add it.
				 if(molecubeMorphology.searchCubeID(molecube.cubes[n]->links[i]->getNeighborID())==255){				
					char faceOnTop,face1,face2;	

					if(molecube.cubes[n]->links[i]->getNeighborClass()!=CLASS_BATTERY)
						face2=molecube.cubes[n]->links[i]->getNeighborSide();
					else
						face2=batteryConvertFace(molecube.cubes[n]->links[i]->getNeighborSide());

					if(molecube.cubes[n]->links[i]->getSelfClass()!=CLASS_BATTERY)
						face1=molecube.cubes[n]->links[i]->getSelfSide();
					else
						face1=batteryConvertFace(molecube.cubes[n]->links[i]->getSelfSide());
					
					facing=face1;
					cubeOrientation = molecube.cubes[n]->links[i]->getSelfOrient();				
					
					int nID=molecube.cubes[n]->links[i]->getNeighborID();														
					
					faceOnTop=getTopFace(face1,face2);
					unsigned char nebClass=molecube.cubes[n]->links[i]->getNeighborClass();
										
					unsigned int molecubeAngle=molecube.cubes[molecube.searchCubeID(nID)]->getAngle();	
					//molecube.display();
					//cout <<dec<<"angle: "<<(int)molecubeAngle<<" nID: "<<(int)nID<<" idpos:"<<(int)molecube.searchCubeID(nID)<<" class:"<<(int)molecube.cubes[molecube.searchCubeID(nID)]->getMolecubeClass()<<endl;
					halfCubes.push_back(addCube(selectedCube,nID,nebClass,facing,faceOnTop,cubeOrientation,molecubeAngle));					
				}
				/*else
					std::cout <<" no"<<endl;*/
			}
		  }
		}
	  }
	}
		/*
	Network network(halfCubes.size());
	network.addSynapse();
	network.addSynapse();
	network.addSynapse();
	network.addSynapse();
	network.addSynapse();
	network.addSynapse();
	network.addSynapse();
	network.addSynapse();
	drawNetwork(network,halfCubes,CHEIGHT*1.8);
	*/
    molecubeMorphology.display();
	//displayHalfList();
}

void displayHalfList(void){
	std::cout <<endl<<"---------------------------------------------------------------"<<endl;
	std::cout <<"halfCube size "<<(int)halfCubes.size()<<endl;
	for(int n=0;n<(int)halfCubes.size();n++)
		std::cout <<"id: "<<n<<" "<<halfCubes[n]<<endl;
    std::cout <<"---------------------------------------------------------------"<<endl<<endl;
}


	// Clean up and shut down the program
	void stop() {
		restart(halfCubes);			
		/*
		for(std::vector<body*>::iterator iter = halfCubes.begin(); iter != halfCubes.end(); iter++) {
			delete (ActorUserData*)(*iter)->ActorUserData;
		}
		*/
		delete mWorld;
	}
	
	// delete existing cubes and restart
	void restart(std::vector<body*>& halfCubes) {
		targetBody = 0;
		hasTargetBody = false;
		objectHasBeenSelected = false;
		molecubeMorphology.clear();
		
		if(ent!=NULL){				
				mSceneMgr->destroySceneNode(ent->getName());
				mSceneMgr->destroyEntity(ent);
				
				if(mSceneMgr->hasEntity("cubeB")){
				   mSceneMgr->destroySceneNode("cubeB");
				   mSceneMgr->destroyEntity("cubeB");
				  }
				ent=NULL;
				}
	            
		for (int id =0;id< (int)halfCubes.size();id++){
			std::cout <<"ID to delete: "<<(int)id<<" "<<halfCubes[id]<<endl;
			
			if(((ActorUserData *)(halfCubes[id]->ActorUserData))->associatedBody!=NULL)
			  if(((ActorUserData *)(halfCubes[id]->ActorUserData))->associatedBody!=halfCubes[id])
				 mScene->destroyBody(((ActorUserData *)(halfCubes[id]->ActorUserData))->associatedBody->getName());
			if(halfCubes[id]!=NULL)
  			  mScene->destroyBody(halfCubes[id]->getName());				 			
			 
		}
		
		halfCubes.~vector();
	}

	double getDisplacement(std::vector<body*>& halfCubes) {
		double x = halfCubes[0]->getGlobalPosition().x - iPosition[0];
		double z = halfCubes[0]->getGlobalPosition().z - iPosition[1];
		std::cout << halfCubes[0]->getGlobalPosition() << endl;
		return sqrt(pow(x+5.51343e-006,2)+pow(z-1.96993e-005,2));
	}
	
	void saveEvolve (geneType parent[Population]){
		if (generationID == 1) {
			bestMove.open (output1.c_str(), ios::out);
			geGene.open(output0.c_str(), ios::out);
		}
		else {
			bestMove.open (output1.c_str(), ios::app);
			geGene.open(output0.c_str(), ios::app);
		}

		geGene << "Generation" + StringConverter::toString(generationID) + ": \n";
		geGene << "current time: " << curTime << endl;
		geGene << "initial time: " << iniTime << endl;
		geGene << "time accumulater: " << timeAcc << endl;

		
		bool better = generationID == 1 || parent[0].getFitness() < parent[bestMoveID].getFitness();

		if (better) {
			evoResult.open (output2.c_str(), ios::out);

			bestMove << "Generation" + StringConverter::toString(generationID) + ": Group: "
			<< bestMoveID << ": "
			<< parent[bestMoveID].getFitness() << "; \n";

			evoResult << "Generation" + StringConverter::toString(generationID) + ": Group: "
			<< bestMoveID << ": "
			<< parent[bestMoveID].getFitness() << "; \n";

			
		}
	//	std::stringstream ostr;
		for (int a = 0; a < Population; a++){
			geGene << "  Member" + StringConverter::toString(a) + "(Fitness: ";
			geGene << parent[a].getFitness();
			geGene << "): \n";
			for (int b = 0; b < numOfCubes; b++){
				geGene << "    Cube" + StringConverter::toString(b) + ": " + parent[a].getGene(b) + "\n";
				if (a == 0 && better) {
					bestMove << parent[bestMoveID].getGene(b) << "\n";
					evoResult << parent[bestMoveID].getGene(b) << "\n";
				}
			}
		}
		
		if (better)
			evoResult.close();
					
		bestMove.close();
		geGene.close();	
	}

/*	void saveEvolve (geneType parent[Population]){
		if (generationID == 1) {
			bestMove.open (output1.c_str(), ios::out);
			geGene.open(output0.c_str(), ios::out);
		}
		else {
			bestMove.open (output1.c_str(), ios::app);
			geGene.open(output0.c_str(), ios::app);
		}

		evoResult.open (output2.c_str(), ios::out);

		geGene << "Generation" + StringConverter::toString(generationID) + ": \n";
		geGene << "current time: " << curTime << endl;
		geGene << "initial time: " << iniTime << endl;
		geGene << "time accumulater: " << timeAcc << endl;
		
		bool better = generationID == 1 || parent[0].getFitness() < parent[bestMoveID].getFitness();

		if (better) {
			bestMove << "Generation" + StringConverter::toString(generationID) + ": Group: "
			<< bestMoveID << ": "
			<< parent[bestMoveID].getFitness() << "; \n";

			evoResult << "Generation" + StringConverter::toString(generationID) + ": Group: "
			<< bestMoveID << ": "
			<< parent[bestMoveID].getFitness() << "; \n";
		}
	//	std::stringstream ostr;
		for (int a = 0; a < Population; a++){
			geGene << "  Member" + StringConverter::toString(a) + "(Fitness: ";
			geGene << parent[a].getFitness();
			geGene << "): \n";
			for (int b = 0; b < numOfCubes; b++){
				geGene << "    Cube" + StringConverter::toString(b) + ": " + parent[a].getGene(b) + "\n";
				if (a == 0 && better) {
					bestMove << parent[bestMoveID].getGene(b) << "\n";
					evoResult << parent[bestMoveID].getGene(b) << "\n";
				}
			}
		}

		evoResult.close();
		bestMove.close();
		geGene.close();	
	}
*/


typedef struct command{
		unsigned char classCube;
		unsigned char id;
		double time;
		unsigned char bus;
		double pDegree;
		double speed;
	}COMMAND;
	
	int getAngle(body *targetBody){
		Quaternion upper, lower;
		Vector3 ux, uy, lx, ly, uv, lv, dv, cv;
		double cAngle;

		if(((ActorUserData*) targetBody->ActorUserData)->lowerHalf==true){
	      upper = targetBody->getGlobalOrientation();
		  lower = ((ActorUserData*) targetBody->ActorUserData)->associatedBody->getGlobalOrientation();
		}
		else{
		  lower = targetBody->getGlobalOrientation();
		  upper = ((ActorUserData*) targetBody->ActorUserData)->associatedBody->getGlobalOrientation();
		}

		ux = Vector3(upper.xAxis().x, upper.xAxis().y, upper.xAxis().z);
		lx = Vector3(lower.xAxis().x, lower.xAxis().y, lower.xAxis().z);

		uy = Vector3(upper.yAxis().x, upper.yAxis().y, upper.yAxis().z);
		ly = Vector3(lower.yAxis().x, lower.yAxis().y, lower.yAxis().z);

		uv = ux - uy;
		lv = lx - ly;

		cAngle = uv.dotProduct(lv)/(uv.length()*lv.length());

		if (cAngle > 1)
			cAngle = 1;
		else if (cAngle < -1)
			cAngle = -1;

		dv = Vector3(upper.xAxis().x + upper.yAxis().x + upper.zAxis().x, upper.xAxis().y + upper.yAxis().y + upper.zAxis().y,
					 upper.xAxis().z + upper.yAxis().z + upper.zAxis().z);
			
		cv = uv.crossProduct(lv);

		if (dv.dotProduct(cv)/(uv.length()*lv.length()) > 0)
				cAngle =  acos(cAngle);
		else
				cAngle = 2 * Math::PI - acos(cAngle);		

	    return ((cAngle*180/Math::PI)+0.5); //+0.5 to have the nearest value after converion in int and *10 				
	}

	void updateAngle(){
		Quaternion upper, lower;
		Vector3 ux, uy, lx, ly, uv, lv, dv, cv;
		double cAngle;
	
		numOfCubes=halfCubes.size();
		   
		    //std::cout << "total number of cubes: " <<  numOfCubes <<"total number of cubes actuator" <<numOfCubesActuator<<endl;
		for (int a = 0; a < numOfCubes; a++)
		{
			upper = halfCubes[a]->getGlobalOrientation();
			lower = ((ActorUserData*) halfCubes[a]->ActorUserData)->associatedBody->getGlobalOrientation();

			ux = Vector3(upper.xAxis().x, upper.xAxis().y, upper.xAxis().z);
			lx = Vector3(lower.xAxis().x, lower.xAxis().y, lower.xAxis().z);

			uy = Vector3(upper.yAxis().x, upper.yAxis().y, upper.yAxis().z);
			ly = Vector3(lower.yAxis().x, lower.yAxis().y, lower.yAxis().z);

			uv = ux - uy;	
			lv = lx - ly;

			cAngle = uv.dotProduct(lv)/(uv.length()*lv.length());

			if (cAngle > 1)
				cAngle = 1;
			else if (cAngle < -1)
				cAngle = -1;

			dv = Vector3(upper.xAxis().x + upper.yAxis().x + upper.zAxis().x, upper.xAxis().y + upper.yAxis().y + upper.zAxis().y,
						 upper.xAxis().z + upper.yAxis().z + upper.zAxis().z);
			
			cv = uv.crossProduct(lv);

			if (dv.dotProduct(cv)/(uv.length()*lv.length()) > 0)
				cAngle =  acos(cAngle);
			else
				cAngle = 2 * Math::PI - acos(cAngle);		

			std::cout <<dec<<"angle "<<a<<" "<<cAngle<<" "<<cAngle*180/Math::PI<<" true value:"<<(int)molecubeMorphology.cubes[molecubeMorphology.searchVirtualID(a)]->getAngle()/10<<endl;
			if(molecubeMorphology.cubes[molecubeMorphology.searchVirtualID(a)]->getMolecubeClass()==CLASS_ACTUATOR)
			  molecubeMorphology.cubes[molecubeMorphology.searchVirtualID(a)]->setAngle((cAngle*180/Math::PI+0.5)*10); //+0.5 to have the nearest value after converion in int and *10 
		}		
	}

	void saveData (std::vector<body*>& halfCubes){
		Quaternion upper, lower;
		Vector3 ux, uy, lx, ly, uv, lv, dv, cv;
		double cAngle;	
	
		numOfCubes=halfCubes.size();
		   
		    //std::cout << "total number of cubes: " <<  numOfCubes <<"total number of cubes actuator" <<numOfCubesActuator<<endl;
		for (int a = 0; a < numOfCubes; a++)
		{
			upper = halfCubes[a]->getGlobalOrientation();
			lower = ((ActorUserData*) halfCubes[a]->ActorUserData)->associatedBody->getGlobalOrientation();

			ux = Vector3(upper.xAxis().x, upper.xAxis().y, upper.xAxis().z);
			lx = Vector3(lower.xAxis().x, lower.xAxis().y, lower.xAxis().z);

			uy = Vector3(upper.yAxis().x, upper.yAxis().y, upper.yAxis().z);
			ly = Vector3(lower.yAxis().x, lower.yAxis().y, lower.yAxis().z);

			uv = ux - uy;
			lv = lx - ly;

			cAngle = uv.dotProduct(lv)/(uv.length()*lv.length());

			if (cAngle > 1)
				cAngle = 1;
			else if (cAngle < -1)
				cAngle = -1;

			dv = Vector3(upper.xAxis().x + upper.yAxis().x + upper.zAxis().x, upper.xAxis().y + upper.yAxis().y + upper.zAxis().y,
						 upper.xAxis().z + upper.yAxis().z + upper.zAxis().z);
			
			cv = uv.crossProduct(lv);

			if (dv.dotProduct(cv)/(uv.length()*lv.length()) > 0)
				angle[a][rCounter] =  acos(cAngle);
			else
				angle[a][rCounter] = 2 * 3.1415926 - acos(cAngle);
			
			std::cout<<"angle: "<<angle[a][rCounter]<<endl;
		}

	 rCounter = rCounter + 1;	
	}

	void saveDataToMSF(int tRec,int recIni){
		 numOfCubes = halfCubes.size();
		
		

		 /*
		 int numOfCubesActuator=0;
		 for (int a = 0; a < numOfCubes; a++)
		   if(molecubeMorphology.cubes[a]->getMolecubeClass()==CLASS_ACTUATOR)
				numOfCubesActuator++;
		*/
			data.open (output3.c_str(), ios::out);
			data << "TIN";
			for (int a = 0; a < numOfCubes; a++) data << "," << a;
			data << endl;
					
			// new a two dimentional array
			double** outY = new double* [numOfCubes];
			outY[0] = new double [numOfCubes * tRec];
			
			
			COMMAND* recordTable;
			recordTable=(COMMAND*)malloc(tRec *sizeof(struct COMMAND));

			for (int b = 1; b < numOfCubes; b++)
				outY[b] = outY[b-1] + tRec;

			linearlization (tRec,numOfCubes, angle, outY);

			int numCom = 0;

			for (int a = 0; a < numOfCubes; a++) {
				for (int b = 0; b < tRec - 1; b++) {
					if (outY[a][b] != 0) {
						numCom = numCom + 1;
					}
					else {
						numCom = numCom;
					}
				}
			}
			/*
			int numComActuator=0;
			for (int a = 1; a < rCounter; a++) {
				for (int b = 0; b < numOfCubes; b++){
					double tem = outY[b][a];
					if (tem > 0.000001){
						if (a != tRec - 1) {
						   if(molecubeMorphology.cubes[b]->getMolecubeClass()==CLASS_ACTUATOR)
							 numComActuator += 1; // 15 bytes
						}
						else
							cout << "";
					}
				}
			}					
	
			*/

			string st = mGUI_fileName->getValue().c_str();
			st = st.substr(0,(st.length() - 4));
			st = st + ".msf";

			FILE *seqFile;
			seqFile = fopen(&st[0], "wb");
			int sTime[maxCubes];

			SEQHEADER *trgseq;
			//CMDPKT *pkt;
			
			// cubecount, packet count, total packet bytes
			//trgseq = generateSampleSequenceHeader(numOfCubes, numCom, numCom*15);
			

			//if(!(seqFile = fopen(filename, "wb")))
			//	return 1;
						
			

			for (int a = 0; a < numOfCubes; a++)
				sTime[a] = 0;

			int n=0;
			for (int a = 1; a < tRec; a++) {
				
				for (int b = 0; b < numOfCubes; b++){
					//std::cout <<a<<" angle: "<< angle[b][a]<<endl;

					double tem = outY[b][a];				// tem - current angle value for cube 'b' at time 'a'
					//std::cout <<tem<<endl;	

					if (tem > 0.000001){
						data << a << ", ";
						data << tem << endl;					
						if (a != tRec - 1) {
							//double time = a * recIni; // time is in msec

							double time = sTime[b] * recIni;						// time of last recorded point
							double pDegree = outY[b][sTime[b]]* 3600 / (2 * 3.1415926) ;
							double cDegree = tem * 3600/ (2 * 3.1415926) ;
							double TimeDiff = (a - sTime[b])* recIni;				// time interval is in msec

							double speed = (abs(cDegree - pDegree)*SPEED_FACTOR/3600) / (TimeDiff / 1000);

							if ((cDegree < 1800) && (pDegree > 1800))
							{
								if ((cDegree + (3600 - pDegree)) < 1800)
								{
									speed = ((cDegree + (3600 - pDegree))*SPEED_FACTOR/3600) / (TimeDiff / 1000);
								}
							}

							if ((cDegree > 1800) && (pDegree < 1800))
							{
								if ((pDegree + (3600 - cDegree)) < 1800)
								{
									speed = ((pDegree + (3600 - cDegree))*SPEED_FACTOR/3600) / (TimeDiff / 1000);
								}
							}

							if (speed>MAX_MOTOR_SPEED)
								speed=MAX_MOTOR_SPEED;

						   if(molecubeMorphology.cubes[b]->getMolecubeClass()==CLASS_ACTUATOR)
							   std::cout << dec << 
							   a <<" tem: "<< tem <<" time: "<< time << " TimeDiff: " << TimeDiff <<" pDegree: "<<
							   pDegree <<" cDregree: " <<cDegree<<" speed: "<<speed<<endl;

							//std::cout <<"SAVE******************************** MSF class "<<hex<<(int)molecubeMorphology.cubes[molecubeMorphology.searchVirtualID(getID(halfCubes[b]))]->getMolecubeClass()<<endl;							
							
						    if(molecubeMorphology.cubes[b]->getMolecubeClass()==CLASS_ACTUATOR&&speed>1){
							//if(molecubeMorphology.cubes[b]->getMolecubeClass()==CLASS_ACTUATOR){
							 recordTable[n].time=time;
							 recordTable[n].bus=EXTERNAL_BUS;							 
							 recordTable[n].classCube=SOUTH_CLASS;
							 recordTable[n].id=b;
							 recordTable[n].speed=speed;
							 recordTable[n].pDegree=pDegree;
							 n++;
						   }							 						   

						}
						else
							cout << "";

						sTime[b] = a;
					}
					else
						cout << "";
				}
			}

			trgseq = generateSampleSequenceHeader(numOfCubes,n,n*15);
			fwrite(trgseq,sizeof(SEQHEADER),1,seqFile);
			for(int a=0;a<tRec;a++){
				for(int i=0;i<n;i++){
					if(a*recIni==recordTable[i].time){
						 writePacket(setAngleAndSpeed(
							 recordTable[i].classCube,
							 recordTable[i].id,
							 recordTable[i].time,
							 recordTable[i].bus,
							 recordTable[i].pDegree,
							 recordTable[i].speed), seqFile); // 15 bytes
					}			
				}

			}
				  
			
			// deallocation
			delete [] outY[0];
			delete [] outY;

			data.close();
		
			std::cout <<endl<<"+++++++++++++++++++++++++++++++++ Save Data To MSF File ++++++++++++++++++++++++++++++++++++++++++++++++"<<endl;
			std::cout << "rCounter: "<<tRec<<endl;
			std::cout << "total number of cubes: "<<  numOfCubes;
			std::cout << "nb command saved: "<<n<<endl;
			std::cout <<endl<<"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<endl;
		
	}
/*
	void read(string gene[]) {
		
	}
*/
	

	void newFrame(float _time) {
       body *tempAddedCube;

		if(!cube_edit){
			if(ent!=NULL){				
				mSceneMgr->destroySceneNode(ent->getName());
				mSceneMgr->destroyEntity(ent);
				ent=NULL;
				if(mSceneMgr->hasEntity("cubeB")){
				   mSceneMgr->destroySceneNode("cubeB");
				   mSceneMgr->destroyEntity("cubeB");
				  }
				ent=NULL;
				}
		}

		// Handle user input
		if (targetBody == 0) {
			mGUI_rotate->setValue("ROTATION");	
			mGUI_setAngle->setValue("ANGLE");

		    // If no object is selected, hide all bounding boxes			
			for(std::vector<body*>::iterator iter = halfCubes.begin(); iter != halfCubes.end(); iter++) {
				if((*iter)!=NULL){
				  (*iter)->mNode->showBoundingBox(false);
				  if((((ActorUserData*)(*iter)->ActorUserData)->associatedBody)!=NULL)
				   ((ActorUserData*)(*iter)->ActorUserData)->associatedBody->mNode->showBoundingBox(false);
				}
			}
			if(ent!=NULL&&halfCubes.size()>0){				
				mSceneMgr->destroySceneNode(ent->getName());
				mSceneMgr->destroyEntity(ent);
				ent=NULL;
				if(mSceneMgr->hasEntity("cubeB")){
				   mSceneMgr->destroySceneNode("cubeB");
				   mSceneMgr->destroyEntity("cubeB");
				  }
				ent=NULL;
				}
		}
		
		
			if((cube_add&&cube_edit)&&(targetBody!=NULL||halfCubes.size()==0)){
			 
			  unsigned char facing;
			  unsigned char targetBodyID;
			  unsigned char molecubeID;
			  unsigned char molecubeClass;
				
			  if(targetBody!=NULL){
				targetBodyID=getID(targetBody);
				molecubeID=molecubeMorphology.searchVirtualID(targetBodyID);
				molecubeClass=molecubeMorphology.cubes[molecubeID]->getMolecubeClass();				
				facing=cube_Face1;				

				std::cout <<"ADD_CUBE : targetBodyID: "<<dec<<(int)targetBodyID << " molecubeID: "<<(int) molecubeID <<" class: " <<(int)molecubeClass<< "face: "<<dec<<(int)cube_Face1<<endl;
				std::cout <<"           cubeAdded: "<<dec<< " molecubeID: "<<(int)molecubeMorphology.getFreeID()<<" class: " <<(int)cube_typeCubeClass<< "face: "<<dec<<(int)cube_Face2<<" angle: "<<(int)cube_angle<<endl;

			   if(halfCubes.size()!=0)	
				if(tempAddedCube = addCube(targetBody,molecubeMorphology.getFreeID(),cube_typeCubeClass,facing,getTopFace(cube_Face1,cube_Face2),cube_orientation,cube_angle)) {

					halfCubes.push_back(tempAddedCube);
					designString.append("t"+StringConverter::toString((int)cube_orientation));
					designString.append(" ");
					designString.append(targetBody->getName());
					designString.append(" ");
				}
				else {
					std::cout << "Cube already added to that facing\n";
				}		
			  }
			  if(halfCubes.size()==0){
			   halfCubes.push_back(createCube(pose(Vector3(0,CHEIGHT,0), Quaternion(Radian(0), Vector3::UNIT_Y )),Vector3::UNIT_Y,cube_orientation,0,cube_typeCubeClass,faceOnTop,cube_angle,1,false));		
			   //Select the ActuatorClass after added a Base
			   if(cube_typeCubeClass==CLASS_BASE)
				cube_typeCubeID=0;
				cube_typeCube=true;
			  }

			  cube_add_parameter=true;
			  cube_add=false;
			}
					


		// The following if statements execute only if a cube is selected
		// Display the bounding box for the selected object
		if(targetBody!=0){

			targetBody->mNode->showBoundingBox(true);		


			// GUI Control
			if (cube_rotateClockwise){
				rotateClockwise(((ActorUserData*)(targetBody->ActorUserData))->D6joint);
				cube_rotateClockwise = false;
			}

			if (cube_rotateCounterClockwise) {
				rotateCounterClockwise(((ActorUserData*)(targetBody->ActorUserData))->D6joint);
				cube_rotateCounterClockwise = false;
			}

			if (cube_haltRotation) {
				haltRotation(((ActorUserData*)(targetBody->ActorUserData))->D6joint);
				cube_haltRotation = false;
			}


			
			if(molecubeMorphology.cubes[molecubeMorphology.searchVirtualID(getID(targetBody))]->getMolecubeClass()==CLASS_ACTUATOR){
			  if(getAngle(targetBody)!=cube_angleDisplay){				
					char str[4];
					cube_angleDisplay=getAngle(targetBody);
					itoa(cube_angleDisplay,str,10);
					mGUI_setAngle->setValue(str);
				}
			 }			
			else		
		      mGUI_setAngle->setValue("ANGLE");
			

			if(cube_angleDec){
				setAngle(targetBody,-1);

				cube_angleDec = false;
			}

			if(cube_angleInc){
				setAngle(targetBody,1);

				cube_angleInc = false;
			}


			//*****************************************************************
/*
		  if()
			 DynamicLines *lines = new DynamicLines(RenderOperation::OT_LINE_LIST);
				for (i=0; i<somePoints.size(); i++) {
				lines->addPoint(somePoints[i]);
			 }
			 lines->update();
	         SceneNode *linesNode = mScene->getRootSceneNode()->createChildSceneNode("lines");
		    linesNode->attachObject(lines);
			}
*/			
			if (cube_deleteCube&&cube_edit) {
				body *cube;						
				cube=targetBody;
				targetBody = 0;
				hasTargetBody = false;
				objectHasBeenSelected = false;	
				molecubeMorphology.display();
				std::cout <<"delete halflist ID"<<(int)getID(cube)<< " molecubeVirtualID:"<<(int)molecubeMorphology.searchVirtualID(getID(cube))<<" molecubeID: "<<molecubeMorphology.searchCubeID(getID(cube))<<endl;
				molecubeMorphology.deleteMolecube(molecubeMorphology.searchVirtualID(getID(cube)));
				molecubeMorphology.display();
				
				deleteCube(cube);				
				cube_deleteCube = false;				
			}
			
		}
		// End "if" statement that is dependent on a cube being selected
		//*****************************************************************
		
		if(cube_typeCube&&cube_edit){			  
			if(halfCubes.size()==0){
			  if(cube_typeCubeID<0)
				 cube_typeCubeID=4;
			  if(cube_typeCubeID>4)
				 cube_typeCubeID=0;
			}
			else{
			 if(cube_typeCubeID<0)
				 cube_typeCubeID=3;
			  if(cube_typeCubeID>3)
				 cube_typeCubeID=0;
			}
			std::cout <<"Type of cube selected "<<cube_typeCubeID<<endl;
			if (cube_typeCubeID==0){
				cube_typeCubeClass=CLASS_ACTUATOR;
			    mGUI_typeCube->setValue("ACTUATOR");
			}
			if (cube_typeCubeID==1){
			 cube_typeCubeClass=CLASS_BATTERY;
			 mGUI_typeCube->setValue("BATTERY");
			}
			if (cube_typeCubeID==2){
			 cube_typeCubeClass=CLASS_GRIPPER;
			 mGUI_typeCube->setValue("GRIPPER");
			}
			if (cube_typeCubeID==3){
			cube_typeCubeClass=CLASS_CONTROLLER;
			 mGUI_typeCube->setValue("CONTROLLER");		
			}
			if (cube_typeCubeID==4){
			 cube_typeCubeClass=CLASS_BASE;
			 mGUI_typeCube->setValue("BASE");		
			}


			 cube_typeCube=false;
			}

		   if(cube_orientationType&&cube_edit){			  
			  if(cube_orientation<0)
				 cube_orientation=3;
			  if(cube_orientation>3)
				 cube_orientation=0;
			std::cout <<"Type of orientation change"<<dec<<(int)cube_orientation<<endl;
			if (cube_orientation==0){
			 mGUI_CubeOrientation->setValue("PI");
			}
			if (cube_orientation==1){			
			 mGUI_CubeOrientation->setValue("PI.2/3");			
			}
			if (cube_orientation==2){
			  mGUI_CubeOrientation->setValue("0");
			}
			if (cube_orientation==3){
				mGUI_CubeOrientation->setValue("PI/2");						
			}
			 cube_orientationType=false;
			}

		
			if(cube_faceOnTop){			  
			  if(faceOnTop<0)
				 faceOnTop=5;
			  if(faceOnTop>5)
				 faceOnTop=0;
			std::cout <<"Face On top changed"<<(int)faceOnTop<<endl;
			if (faceOnTop==0)
			 mGUI_CubeFaceOnTop->setValue("BOTTOM");
			
			if (faceOnTop==1)
			 mGUI_CubeFaceOnTop->setValue("LEFT");
			
			if (faceOnTop==2)			
			 mGUI_CubeFaceOnTop->setValue("BACK");
			
			if (faceOnTop==3)		
			 mGUI_CubeFaceOnTop->setValue("TOP");
			
			if (faceOnTop==4)	
			 mGUI_CubeFaceOnTop->setValue("FRONT");
			
			if (faceOnTop==5)
			 mGUI_CubeFaceOnTop->setValue("RIGHT");		
			 cube_add_parameter=true;
			 cube_faceOnTop=false;
			}



       if (cube_restart) {
		    generationID = 0;
			timeInterNum = 0;
			numOfCubes = 1;
			restart(halfCubes); 
			ent=createCubeTranparence(pose(Vector3(0,CHEIGHT,0), Quaternion(Radian(0), Vector3::UNIT_Y )),Vector3::UNIT_Y,cube_orientation,0,cube_typeCubeClass,faceOnTop,cube_angle,1,false);		
			cube_restart = false;
		}

		if (cube_evolve) {
			if (timeInterNum == 0 || cube_newC == true){
				restart(halfCubes);
				loadDesign();
				timeInterNum = 0;
				cube_newC = false;

				if (generationID == 0){
					
			    
					numOfCubes = halfCubes.size();
					generationID = 1;
					groupID = 0;
					for (int a = 0; a < Population; a++){
						parent[a].creatGene(numOfCubes);
					}
					string name = mGUI_fileName->getValue().c_str();
					name = name.substr(0,(name.length() - 4));

					string st = name + "Genes" +  + ".txt";

					// null-call to get the size
					size_t needed = ::mbstowcs(NULL,&st[0],st.length());

					// allocate
					output0.resize(needed);
					::mbstowcs(&output0[0],&st[0],st.length());

					st = name + "BestMove" + ".txt";
					// null-call to get the size
					needed = ::mbstowcs(NULL,&st[0],st.length());

					output1.resize(needed);
					::mbstowcs(&output1[0],&st[0],st.length());	

					st = name + "EvoResult" + ".txt";
					// null-call to get the size
					needed = ::mbstowcs(NULL,&st[0],st.length());

					output2.resize(needed);
					::mbstowcs(&output2[0],&st[0],st.length());	

				}
				GetSystemTime(&now);
				curTime = static_cast<unsigned int> ((((now.wDay * 24 + now.wHour) * 60 + now.wMinute) * 60 + now.wSecond) * 1000 + now.wMilliseconds);
				iniTime = curTime;
			    timeAcc = curTime;
			}
			else {
				GetSystemTime(&now);
				curTime = static_cast<unsigned int> ((((now.wDay * 24 + now.wHour) * 60 + now.wMinute) * 60 + now.wSecond) * 1000 + now.wMilliseconds);
			}

			
			std::cout << "generationID: " << generationID << endl;
			std::cout << "current time: " << curTime << endl;
			std::cout << "initial time: " << iniTime << endl;
			std::cout << "time accumulater: " << timeAcc << endl;
			std::cout << "curTime - iniTime:" <<curTime - iniTime <<"=>"<< expeTime <<endl;
			
			mGUI_EvolutionGeneration->setValue(StringConverter::toString(generationID));
			mGUI_EvolutionTime->setValue(StringConverter::toString((curTime-iniTime)/1000));
			mGUI_EvolutionPopulation->setValue(StringConverter::toString(Population));

			if (curTime - iniTime >= expeTime) {
				for (int a = 0; a < numOfCubes; a++){
					haltRotation(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
				}
				
				std::cout << getDisplacement(halfCubes) << endl;
				
				double fitness;

				if (generationID == 1 || groupID != 0) {
					fitness = getDisplacement(halfCubes);
				    parent[groupID].setFitness(fitness);								
				}

				timeInterNum = 0;

				if (generationID == 1 && groupID == 0) bestMoveID = 0;
				else if (groupID == 1) bestMoveID = 1;
				else 
					if (parent[bestMoveID].getFitness() < fitness){
						bestMoveID = groupID;
						mGUI_EvolutionBest->setValue(StringConverter::toString((float)fitness));
					}				

				groupID++;

				float average=0;
				  for(int i=0;i<Population;i++)
					average+=parent[i].getFitness();
				mGUI_EvolutionAverage->setValue(StringConverter::toString((float)average/Population));		

				std::cout << "groupID: " << groupID << endl;
				mGUI_EvolutionIndividu ->setValue(StringConverter::toString(groupID));
				if (groupID == Population){
					saveEvolve(parent);
					creatSpring(parent, spring);
					
					for (int a = 0; a < Population; a++) {
						parent[a] = spring[a];
					}
					
					groupID = 0;

					generationID++;

				}
			}
			else if ((curTime - timeAcc >= timeInter) || timeInterNum == 0){
				

				int command, b;
				
				std::cout << "GenerationID: " << generationID << endl;
				

				timeAcc = curTime;
				
				
				if (timeInterNum == setUpInter){
					iPosition[0] = halfCubes[0]->getGlobalOrientation().x;
					iPosition[1] = halfCubes[0]->getGlobalOrientation().z;
				}

				for (int a = 0; a < numOfCubes; a++){
					if (timeInterNum > setUpInter)
						b = (timeInterNum-setUpInter) % (parent[groupID].getGene(a).length()-setUpInter) + setUpInter;
					else
						b = timeInterNum;

					command = parent[groupID].getGene(a)[b] - 48;
					std::cout << "Gene: " << parent[groupID].getGene(a) << endl;
					std::cout << "cube#: " << a << " time#: " << timeInterNum << " Command: "
						<< command << endl;
					
					switch (command) {
						case 1:
							{
								rotateClockwise(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
								break;}
						case 2:
							{
								rotateCounterClockwise(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
							break;
							}
						case 3:
							{
								haltRotation(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
							break;
							}
					}
				}

				timeInterNum++;
				std::cout << timeInterNum << endl;
			}
		}

		if (cube_playEvolve) {
			if (timeInterNum == 0 || cube_newC == true){
				restart(halfCubes);
				loadDesign();
				timeInterNum = 0;
			    cube_newC = false;

				if (generationID == 0){
					
					numOfCubes = halfCubes.size();
					generationID = 1;
					
					string st = mGUI_fileName->getValue().c_str();
					st = st.substr(0,(st.length() - 4));

					st = st + "BestMove" + ".txt";

					// null-call to get the size
					size_t needed = ::mbstowcs(NULL,&st[0],st.length());

					// allocate
					output0.resize(needed);
					::mbstowcs(&output0[0],&st[0],st.length());

					rBestMove.open(output0.c_str(), ios::out);
				}
		
				rBestMove >> gene[0];

				std::cout << generationID << " " << numOfCubes << " " << gene[0] << endl;

				if (gene[0] != ""){
					for (int a = 0; a < 3; a++) rBestMove >> gene[0];
					for (int a = 0; a < numOfCubes; a++){
						rBestMove >> gene[a];
						std::cout << gene[a] << endl;
					}

					GetSystemTime(&now);
					curTime = static_cast<unsigned int> ((((now.wDay * 24 + now.wHour) * 60 + now.wMinute) * 60 + now.wSecond) * 1000 + now.wMilliseconds);
					iniTime = curTime;
					timeAcc = curTime;
				} else
				{
					cube_playEvolve = false;
					cube_restart = true;
					rBestMove.clear();
					rBestMove.close();
				}
			}
			else {
				GetSystemTime(&now);
				curTime = static_cast<unsigned int> ((((now.wDay * 24 + now.wHour) * 60 + now.wMinute) * 60 + now.wSecond) * 1000 + now.wMilliseconds);
			}

			if (cube_playEvolve) {
			if (curTime - iniTime >= expeTime) {
				for (int a = 0; a < numOfCubes; a++){
					haltRotation(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
				}
				timeInterNum = 0;
				generationID++;
				gene[0] = "";
			}
			else if ((curTime - timeAcc >= timeInter) || timeInterNum == 0){
				int command, b;
				
				std::cout << "GenerationID: " << generationID << endl;

				timeAcc = curTime;
			
				for (int a = 0; a < numOfCubes; a++){
					if (timeInterNum > setUpInter)
						b = (timeInterNum-setUpInter) % (gene[a].length()-setUpInter) + setUpInter;
					else
						b = timeInterNum;

					std::cout << a << " " << b << endl;

						command = gene[a][b] - 48;
					std::cout << "Gene: " << gene[a] << endl;
					std::cout << "cube#: " << a << " time#: " << timeInterNum << " Command: "
						<< command << endl;
					
					switch (command) {
						case 1:
							{
								rotateClockwise(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
								break;}
						case 2:
							{
								rotateCounterClockwise(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
							break;
							}
						case 3:
							{
								haltRotation(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
							break;
							}
					}
				}

				timeInterNum++;
				std::cout << timeInterNum << endl;
			}
			}
		}
		
		//**************************************************************************
		if(recordStart){		   
		 std::cout <<"********************************** Start Record ***************************************"<<endl;
		   GetSystemTime(&now);
		   curTime = static_cast<unsigned int> ((((now.wDay * 24 + now.wHour) * 60 + now.wMinute) * 60 + now.wSecond) * 1000 + now.wMilliseconds);
		   iniTime = curTime;
		   timeAcc = curTime;
		   recAcc = curTime;
		   rCounter = 0;
		   recordStart=false;
		}
		
		if(record){
			GetSystemTime(&now);
		    curTime = static_cast<unsigned int> ((((now.wDay * 24 + now.wHour) * 60 + now.wMinute) * 60 + now.wSecond) * 1000 + now.wMilliseconds);
			if (curTime - recAcc >= recIni) {				
				saveData(halfCubes);
				recAcc = recAcc + recIni;
				//std::cout <<"********************************** Record ***************************************"<<endl;
		    }	
			
		}

		if(recordStop){
		   saveDataToMSF(rCounter,recIni);		   		  
		   recordStop=false;
		   std::cout <<"********************************** Stop Record ***************************************"<<endl;
		}

		//**************************************************************************

		if (cube_getData) {
			if (timeInterNum == 0 || cube_newC == true){
				restart(halfCubes);
				loadDesign();
			    timeInterNum = 0;
				
				if (cube_newC == true) {
					cube_newC = false;
					rCounter = 0;
				}

				numOfCubes = halfCubes.size();
				
				string st = mGUI_fileName->getValue().c_str();
				string name = st.substr(0,(st.length() - 4));

				st = name + "evoResult" + ".txt";
				// null-call to get the size
				size_t needed = ::mbstowcs(NULL,&st[0],st.length());

				// allocate
				output2.resize(needed);
				::mbstowcs(&output2[0],&st[0],st.length());

				rEvoResult.open(output2.c_str(), ios::out);

				st = name + "Data" + ".txt";
				// null-call to get the size
				needed = ::mbstowcs(NULL,&st[0],st.length());

				output3.resize(needed);
				::mbstowcs(&output3[0],&st[0],st.length());	
		
				rEvoResult >> gene[0];

				std::cout << generationID << " " << numOfCubes << " " << gene[0] << endl;

				if (gene[0] != ""){
					for (int a = 0; a < 3; a++) rEvoResult >> gene[0];
					for (int a = 0; a < numOfCubes; a++){
						rEvoResult >> gene[a];
						std::cout << gene[a] << endl;
					}

					GetSystemTime(&now);
					curTime = static_cast<unsigned int> ((((now.wDay * 24 + now.wHour) * 60 + now.wMinute) * 60 + now.wSecond) * 1000 + now.wMilliseconds);
					iniTime = curTime;
					timeAcc = curTime;
					recAcc = curTime;
				} else {
					cube_playEvolve = false;
					cube_restart = true;
					rEvoResult.clear();
					rEvoResult.close();
				}

			}
			else {
				GetSystemTime(&now);
				curTime = static_cast<unsigned int> ((((now.wDay * 24 + now.wHour) * 60 + now.wMinute) * 60 + now.wSecond) * 1000 + now.wMilliseconds);
			}

		if (curTime - recAcc >= recIni || timeInterNum == 0) {				
				saveData(halfCubes);
				recAcc = recAcc + recIni;
		}

		

			if (cube_getData) {
				if (curTime - iniTime >= expeTime) {
					for (int a = 0; a < numOfCubes; a++){
						haltRotation(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
					}
					cube_getData = false;
					saveDataToMSF(tRec,recIni);				
					timeInterNum = 0;
					generationID++;
					gene[0] = "";
					restart(halfCubes);
				}
				else if ((curTime - timeAcc >= timeInter) || timeInterNum == 0){
				int command, b;
				
				std::cout << "GenerationID: " << generationID << endl;

				timeAcc = curTime;
			
				for (int a = 0; a < numOfCubes; a++){
					if (timeInterNum > setUpInter)
						b = (timeInterNum-setUpInter) % (gene[a].length()-setUpInter) + setUpInter;
					else
						b = timeInterNum;

					std::cout << a << " " << b << endl;

					command = gene[a][b] - 48;
					std::cout << "Gene: " << gene[a] << endl;
					std::cout << "cube#: " << a << " time#: " << timeInterNum << " Command: "
						<< command << endl;
					
					switch (command) {
						case 1:
							{
								rotateClockwise(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
								break;}
						case 2:
							{
								rotateCounterClockwise(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
							break;
							}
						case 3:
							{
								haltRotation(((ActorUserData*)(halfCubes[a]->ActorUserData))->D6joint);
							break;
							}
					}
				}

				timeInterNum++;
				std::cout << timeInterNum << endl;
			}
			}
		}

		if (saveCube) {
			saveParameter();
			saveDesign();
			std::cout << "Saving to file\n";
			saveCube = false;
		}

		if (loadCube){
			loadParameter();
			cube_getData = false;
			cube_evolve = false;
			cube_playEvolve = false;
			loadCube = false;
			
			string s=mGUI_fileName->getValue().c_str();
			molecubeStar();
			molecubeStar2();
			molecubeStar3();
			molecubeStarG();
			/*
			if(s.find("spiral",0)!=string::npos&&s.length()>=9){
				molecubeSpiral((s[6]-'0')*10+s[7]-'0',s);
			}
			if(s.find("test",0)!=string::npos&&s.length()>=9){
			  molecubeTest(s[4]-'0',s[5]-'0',s[6]-'0',s);
			}		
			if(s.find("super",0)!=string::npos&&s.length()>=9){
			  molecubeSuperTest(s[5]-'0',s[6]-'0',s[7]-'0',s[8]-'0',s[9]-'0',s[10]-'0',s);
			}
			*/
			if(s.find(".MMF",0)!=string::npos||s.find(".mmf",0)!=string::npos){
				std::cout << "Loading from MMFfile\n";	
				loadDesign();				
			}						
			else{
				std::cout << "The file is not a MMF file\n";
			 //	loadDesign();				
			}			
			
		}

		// Keys are defined in "Eihort.h"
		// Example of keyboard input
		if (isKeyDown(I)) {
		}

		if (isKeyDown(U)) {
		//*************************
		// Example of some quaternion math and extracting the orientation of the cubes
		//Quaternion upperhalf, lowerhalf;
		//Vector3 upperhalfY, lowerhalfY, upperhalfZ, lowerhalfZ;
		//Vector3 jointOrientation, jointOrientation2;

		//upperhalf = targetBody->getGlobalOrientation();
		//lowerhalf = halfCubes[0]->getGlobalOrientation();
		//
		//upperhalfY = Vector3(upperhalf.yAxis().x, upperhalf.yAxis().y, upperhalf.yAxis().z);
		//lowerhalfY = Vector3(lowerhalf.yAxis().x, lowerhalf.yAxis().y, lowerhalf.yAxis().z);
		//upperhalfY.normalise();
		//lowerhalfY.normalise();
		//upperhalfZ = Vector3(upperhalf.zAxis().x, upperhalf.zAxis().y, upperhalf.zAxis().z);
		//lowerhalfZ = Vector3(lowerhalf.zAxis().x, lowerhalf.zAxis().y, lowerhalf.zAxis().z);
		//upperhalfZ.normalise();
		//lowerhalfZ.normalise();

		//jointOrientation = Quaternion(Radian(Math::PI/4), upperhalfY)
		//					*Quaternion(Radian(-.955316618), upperhalfZ)*upperhalfY;
		//jointOrientation2 = Quaternion(Radian(Math::PI*3/4), lowerhalfY)
		//					*Quaternion(Radian(-.955316618), lowerhalfZ)*lowerhalfY;
		//Radian angle;
		//Vector3 axis;
		//jointOrientation.getRotationTo(lowerhalfY).ToAngleAxis(angle, axis);
		//std::cout<< "Angle between y-axes: " << angle.valueDegrees() <<'\n';

		//*************************

			//Quaternion quatern = targetBody->getGlobalOrientation();
			//std::cout << "Orientation of upper half:"
			//	<< " x: " << jointOrientation.x
			//	<< " y: " << jointOrientation.y
			//	<< " z: " << jointOrientation.z << '\n';

			//Quaternion quatern = targetBody->getGlobalOrientation();
			//std::cout << "Orientation of upper half:"
			//	<< " x: " << quatern.zAxis().x
			//	<< " y: " << quatern.zAxis().y
			//	<< " z: " << quatern.zAxis().z << '\n';

			//quatern = halfCubes[0]->getGlobalOrientation();
			//std::cout << "Orientation of lower half:"
			//	<< " x: " << quatern.zAxis().x
			//	<< " y: " << quatern.zAxis().y
			//	<< " z: " << quatern.zAxis().z << '\n';
		}

		if(cube_add_parameter&&cube_edit){
				 bool find=false;
				 unsigned char targetBodyID;
				 unsigned char molecubeID;
				 unsigned char molecubeClass;
				 int n=0;
				if(targetBody){
				  targetBodyID=getID(targetBody);
				  molecubeID=molecubeMorphology.searchVirtualID(targetBodyID);
				  molecubeClass=molecubeMorphology.cubes[molecubeID]->getMolecubeClass();
								 				  
				  do {
				  if(molecubeClass==CLASS_BASE)
					cube_Face1=3;

				  if(molecubeClass==CLASS_ACTUATOR){
					if(cube_Face1<0)
						cube_Face1=5;
					if(cube_Face1>5)
						cube_Face1=0;
				  }
				  if(molecubeClass==CLASS_GRIPPER){
					if(cube_Face1<0)
						cube_Face1=2;
					if(cube_Face1>2)
						cube_Face1=0;
				  }
				  if(molecubeClass==CLASS_CONTROLLER){
				 	if(cube_Face1==4)
					  if(cube_Face1Inc)
					   cube_Face1=5;
					  else
					   cube_Face1=3;
					if(cube_Face1<0)
						cube_Face1=5;
					if(cube_Face1>5)
						cube_Face1=0;							
				  }
				  if(molecubeClass==CLASS_BATTERY){	
					if(cube_Face1==4)
						if(cube_Face1Inc)
						   cube_Face1=5;
						else
						   cube_Face1=3;
					if(cube_Face1<0)
						cube_Face1=5;
					if(cube_Face1>5)					
						cube_Face1=0;					
				  }				
									  
				  if(((ActorUserData*)(targetBody->ActorUserData))->fullCubeData->attachedCubes[cube_Face1]==NULL)
				    find=true;
				  else{
					if(cube_Face1Inc)
					 cube_Face1++;
				   else
				     cube_Face1--;
				  }
                  n++;

				}while(n<6&&!find);
		     }
			
				if(cube_typeCubeClass==CLASS_BASE)
					cube_Face2=3;

				if(cube_typeCubeClass==CLASS_ACTUATOR){
					if(cube_Face2<0)
						cube_Face2=5;
					if(cube_Face2>5)
						cube_Face2=0;
				  }
				  if(cube_typeCubeClass==CLASS_GRIPPER){
					if(cube_Face2<0)
						cube_Face2=2;
					if(cube_Face2>2)
						cube_Face2=0;
				  }
				  if(cube_typeCubeClass==CLASS_CONTROLLER){
				 	if(cube_Face2==4)
						if(cube_Face2Inc)
							cube_Face2=5;
						else
						   cube_Face2=3;
					if(cube_Face2<0)
						cube_Face2=5;
					if(cube_Face2>5)
						cube_Face2=0;							
				  }
				  if(cube_typeCubeClass==CLASS_BATTERY){	
					if(cube_Face2==4)
					  if(cube_Face2Inc)
					   cube_Face2=5;
					  else
					   cube_Face2=3;
					if(cube_Face2<0)
						cube_Face2=5;
					if(cube_Face2>5)
						cube_Face2=0;												
				  }				
				
				/*
			 switch(cube_Face1){
				case 0:
					mGUI_CubeFace1->setValue("face1 [0]");
				break;
				case 1:
					mGUI_CubeFace1->setValue("face1 [1]");
				break;
				case 2:
					mGUI_CubeFace1->setValue("face1 [2]");
				break;
				case 3:
					mGUI_CubeFace1->setValue("face1 [3]");
				break;
				case 4:
					mGUI_CubeFace1->setValue("face1 [4]");
				break;
				case 5:
					mGUI_CubeFace1->setValue("face1 [5]");	
				break;
			 }				
			 switch(cube_Face2){
				case 0:
					mGUI_CubeFace2->setValue("face2 [0]");
				break;
				case 1:
					mGUI_CubeFace2->setValue("face2 [1]");
				break;
				case 2:
					mGUI_CubeFace2->setValue("face2 [2]");
				break;
				case 3:
					mGUI_CubeFace2->setValue("face2 [3]");
				break;
				case 4:
					mGUI_CubeFace2->setValue("face2 [4]");
				break;
				case 5:
					mGUI_CubeFace2->setValue("face2 [5]");	
				break;
				
			 }	
			 */
			
			
			molecubeMorphology.display();
			
			if(targetBody)
			   std::cout <<"TARGET : targetBodyID: "<<dec<<(int)targetBodyID << " molecubeID: "<<(int) molecubeID <<" class: " <<(int)molecubeClass<< " face: "<<dec<<(int)cube_Face1<<endl;
			   std::cout <<" TRANS CUBE: "<<dec<< " newMolecubeID: "<<(int)molecubeMorphology.getFreeID()<<" class: " <<(int)cube_typeCubeClass<< " face: "<<(int)cube_Face2<<" cubeOrientation: "<<cube_orientation<<" angle: "<<(int)cube_angle<<endl;
			 					
				
				if(ent!=NULL){					
					mSceneMgr->destroySceneNode(ent->getName());
					mSceneMgr->destroyEntity(ent);
					if(mSceneMgr->hasEntity("cubeB")){
					  mSceneMgr->destroySceneNode("cubeB");
					  mSceneMgr->destroyEntity("cubeB");
					}
					ent=NULL;
				}
			   if(targetBody&&n<6&&find)
			    ent=addCubeTranparence(targetBody,molecubeMorphology.getFreeID(),cube_typeCubeClass,cube_Face1,getTopFace(cube_Face1,cube_Face2),cube_orientation,cube_angle);
			 
				
			   if(halfCubes.size()==0)
			    ent=createCubeTranparence(pose(Vector3(0,CHEIGHT,0), Quaternion(Radian(0), Vector3::UNIT_Y )),Vector3::UNIT_Y,cube_orientation,0,cube_typeCubeClass,faceOnTop,cube_angle,1,false);		
		
			   cube_Face1Inc=false;
			   cube_Face2Inc=false;
			   cube_add_parameter=false;

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
		cubeInterface->Shutdown();

		delete cubeInterface;
	} catch( Ogre::Exception& e ) {
		std::cout << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
	}
	std::cout << "--Done." << std::endl;
	return 0;
}
