#include "stateDescriptor.h"

StateDescriptor::StateDescriptor(unsigned char id,
				 unsigned char molecubeClass,
				 unsigned int  timestamp,
				 unsigned char channel,
				 unsigned int value){
  this->id=id;
  this->molecubeClass=molecubeClass;
  this->timestamp=timestamp;
  this->channel=channel;
  this->value=value;
}

StateDescriptor::StateDescriptor(ifstream *file){
  file->read((char *)&id,sizeof(id));
  file->read((char *)&molecubeClass,sizeof(molecubeClass));
  file->read((char *)&timestamp,sizeof(timestamp));
  file->read((char *)&channel,sizeof(channel));
  file->read((char *)&value,sizeof(value));
}

unsigned char StateDescriptor::getId(){
  return id;
}

unsigned char StateDescriptor::getMolecubeClass(){
  return molecubeClass;
}

unsigned int StateDescriptor::getTimestamp(){
  return timestamp;
}

unsigned char StateDescriptor::getChannel(){
  return channel;
}

unsigned int StateDescriptor::getValue(){
  return value;
}

void StateDescriptor::setId(unsigned char id){
  this->id=id;
}

void StateDescriptor::setMolecubeClass(unsigned char molecubeClass){
  this->molecubeClass=molecubeClass;
}

void StateDescriptor::setTimestamp(unsigned int timestamp){
  this->timestamp=timestamp;
}

void StateDescriptor::setChannel(unsigned char channel){
  this->channel=channel;
}

void StateDescriptor::setValue(unsigned int value){
  this->value=value;
}

void StateDescriptor::saveMMFfile(ofstream *file){
  file->write((char *)&id,sizeof(id));
  file->write((char *)&molecubeClass,sizeof(molecubeClass));
  file->write((char *)&timestamp,sizeof(timestamp));
  file->write((char *)&channel,sizeof(channel));
  file->write((char *)&value,sizeof(value));
}













