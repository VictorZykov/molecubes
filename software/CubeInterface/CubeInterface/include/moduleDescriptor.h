#ifndef MODULEDESCRIPTOR_H
#define MODULEDESCRIPTOR_H

#include "linkDescriptor.h"

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class ModuleDescriptor{
  unsigned char id;  //individual Molecube module identifier, address 0xFE is reserved for broadcasting  
  unsigned char virtualID; //individual Virtual Molecube module identifier for the simulator,
  unsigned char molecubeClass; //Molecube class identifier, such as AX-12 servo, South actuator half, North actuator half, etc.
  unsigned char numLinks; // Number of physical connections this module has in Molecube assembly
  unsigned short int angle; //0-36000 Initial angle between the half part of the cube
 public:
  vector<LinkDescriptor*> links; //Array of Link Descriptors -  structures with physical connection specifications - size of array specified by NumLinks	
  ModuleDescriptor(unsigned char id, 
		   unsigned char molecubeClass,unsigned int angle);
  ModuleDescriptor(unsigned char id, 
		   unsigned char molecubeClass,unsigned int angle,char virtualID);   
  ModuleDescriptor(ifstream *file);
  ~ModuleDescriptor();
  unsigned char getID();
  unsigned char getVirtualID();
  unsigned char getMolecubeClass();
  unsigned char getNumLinks();
  unsigned int getAngle();
  void setID(unsigned char id);
  void setVirtualID(unsigned char virtualID);
  void setMolecubeClass(unsigned char molecubeClass);
  void setNumLinks(unsigned char numLinks);  
  void setAngle(unsigned int angle); 
  void addLink(unsigned char selfSide,ModuleDescriptor *neighbor,unsigned char neighborSide,unsigned char orientation);
  void ModuleDescriptor::deleteLink(unsigned char id);
  unsigned char ModuleDescriptor::searchLinkId(unsigned char id);
  void saveMMFfile(ofstream *file); 
 private:
  void addLink(LinkDescriptor *link);
};

#endif
