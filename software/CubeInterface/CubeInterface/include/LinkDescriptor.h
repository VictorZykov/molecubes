#ifndef LINKDESCRIPTOR_H
#define LINKDESCRIPTOR_H

#include <iostream>
#include <fstream>

using namespace std;

class LinkDescriptor{
  unsigned char selfID; //0-253 individual Molecube module identifier of the first joint module
  unsigned char selfClass; //0-255 Molecube class identifier of the first joint module
  unsigned char selfSide;  //0-n The joint interface identifier for the first joint module (see Module Connector Identification Conventions below)
  unsigned char selfOrient; //0-3 Orientation of the first module relative to the second (see Orientation Convention below)
  unsigned char neighborID; //0-253 individual Molecube module identifier of the second joint module					 
  unsigned char neighborClass; //0-255  Molecube class identifier of the second joint module				
  unsigned char neighborSide; //0-n The joint side identifier for the second joint module 				
 public:
  LinkDescriptor(unsigned char selfID,
		 unsigned char selfClass,
		 unsigned char selfSide,
		 unsigned char selfOrient,
		 unsigned char neighborID,
		 unsigned char neighborClass,
		 unsigned char neighborSide);
  LinkDescriptor::LinkDescriptor(ifstream *file);
  void setSelfID(unsigned char selfID);
  void setSelfClass(unsigned char selfClass);
  void setSelfSide(unsigned char selfSide);	
  void setSelfOrient(unsigned char selfOrient);
  void setNeighborID(unsigned char neighborID);
  void setNeighborClass(unsigned char neighborClass);
  void setNeighborSide(unsigned char neighborSide);
  unsigned char getSelfID();
  unsigned char getSelfClass();
  unsigned char getSelfSide();
  unsigned char getSelfOrient();
  unsigned char getNeighborID();
  unsigned char getNeighborClass();
  unsigned char getNeighborSide();
  void saveMMFfile(ofstream *file);
};

#endif



