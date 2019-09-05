#ifndef STATEDESCRIPTOR_H
#define STATEDESCRIPTOR_H

#include <iostream>
#include <fstream>

using namespace std;

class StateDescriptor{
  unsigned char id;             // Molecube adress
  unsigned char molecubeClass;  // Molecube class
  unsigned int timestamp;       // Time of state validity
  unsigned char channel;        // Telemetry Channel represented
  unsigned int value;         // Telemetry Channel value

 public:
 StateDescriptor(unsigned char id,
		 unsigned char molecubeClass,
		 unsigned int  timestamp,
		 unsigned char channel,
		 unsigned int value);
 StateDescriptor(ifstream *file);
 unsigned char  getId();
 unsigned char  getMolecubeClass();
 unsigned int   getTimestamp();
 unsigned char  getChannel();
 unsigned int getValue();
 void setId(unsigned char id);
 void setMolecubeClass(unsigned char moledcubeClass);
 void setTimestamp(unsigned int timestamp);
 void setChannel(unsigned char channel);
 void setValue(unsigned int value);
 void saveMMFfile(ofstream *file);

};
#endif
