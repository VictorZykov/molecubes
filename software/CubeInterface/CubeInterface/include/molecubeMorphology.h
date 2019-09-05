#ifndef MOLECUBEMORPHOLOGY_H
#define MOLECUBEMORPHOLOGY_H

#include "linkDescriptor.h"
#include "moduleDescriptor.h"
#include "stateDescriptor.h" 

#include <iostream>
#include <fstream>
#include <vector>

#define GRAVITY_CONST 9.80665f

#define CLASS_GRIPPER    0xF8
#define CLASS_CONTROLLER 0xFC
#define CLASS_BATTERY    0xF7
#define CLASS_ACTUATOR	 0xFA
#define CLASS_BASE		 0xFB

#define FILENAME_GRIPPER     "gripper"
#define FILENAME_CONTROLLER	 "controller"
#define FILENAME_BATTERY	 "battery"
#define FILENAME_ACTUATOR	 "cube"
#define FILENAME_BASE		 "base"

#define WEIGHT_GRIPPER      0.19469f //kg
#define WEIGHT_CONTROLLER   0.17015f
#define WEIGHT_ACTUATOR		0.217f
#define WEIGHT_BATTERY		0.240f
#define WEIGHT_BASE		    0.960f

#define MASS_GRIPPER        WEIGHT_GRIPPER/GRAVITY_CONST  //kg
#define MASS_CONTROLLER     WEIGHT_CONTROLLER/GRAVITY_CONST
#define MASS_ACTUATOR       WEIGHT_ACTUATOR/GRAVITY_CONST
#define MASS_BATTERY        WEIGHT_BATTERY/GRAVITY_CONST
#define MASS_BASE           WEIGHT_BASE/GRAVITY_CONST

#define VOLUME_GRIPPER		0.000185f //m3 
#define VOLUME_CONTROLLER	0.000125f
#define VOLUNE_ACTUATOR		0.000125f
#define VOLUNE_BATTERY		0.000125f
#define VOLUNE_BASE		    0.000960f

#define DENSITY_GRIPPER		MASS_GRIPPER/VOLUME_GRIPPER       //0.827*4.62  //weight= 194.69 (with 2caches) mass=19.86 volume=185cm3  virtualVolume=27cm3  size=
#define DENSITY_CONTROLLER  MASS_CONTROLLER/VOLUME_CONTROLLER //0.966*4.62  //weight= 170.15g (with 2caches) mass=17.39g volume=125cm3 virtualVolume=27cm3 size=	
#define DENSITY_ACTUATOR    MASS_ACTUATOR/VOLUNE_ACTUATOR     //1.23*4.62   //weight= 217.00 (with 3caches) mass=22.183 volume=125cm3	virtualVolume=27cm3  size=
#define DENSITY_BATTERY		MASS_BATTERY/VOLUNE_BATTERY       //1.361*4.62  //weight= 240.17 (with 2caches) mass=24.50 volume=125cm3  virtualVolume=27cm3  size=6m VirtualSize=
#define DENSITY_BASE		MASS_BASE/VOLUNE_BASE 

using namespace std;

void molecubeSpiral(char size,string filename);
void molecubeTest(char selfFace, char selfNeighbor, char orientation,string filename);
void molecubeSuperTest(char selfFace, char selfNeighbor, char orientation, char selfFace2, char selfNeighbor2, char orientation2,string filename);
void molecubeStar();
void molecubeStar2();
void molecubeStar3();
void molecubeStarG();

class MolecubeMorphology{
  unsigned char cubeCount; //0-255  Number of cubes in the morphology, this corresponds ot the number of module descriptors
  unsigned char morphologyChecksum;  //0-255
  unsigned int stateCount; //0-2^32 Number of state descriptors in the state array
  vector<StateDescriptor*> states;  //state Array of state descriptors 
  unsigned char stateChecksum; //0-255 
 public:
  vector<ModuleDescriptor*> cubes; //Array of module descriptors
  MolecubeMorphology();
  ~MolecubeMorphology();
  void addMolecube(ModuleDescriptor *cube);
  void MolecubeMorphology::addState(StateDescriptor *state);  
  void deleteMolecube(unsigned char id);
  bool loadMMFfile(const char *filename);
  void saveMMFfile(const char *filename);
  void MolecubeMorphology::display(void);
  unsigned char getCubeCount();
  unsigned int  getStateCount();
  unsigned char MolecubeMorphology::getFreeID();
  unsigned char searchCubeID(unsigned char ID);
  unsigned char searchVirtualID(unsigned char virtualID);
  void clear(void);
  void clearState(void);
};

#endif
