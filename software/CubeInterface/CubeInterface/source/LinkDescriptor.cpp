#include "linkDescriptor.h"

LinkDescriptor::LinkDescriptor(unsigned char selfID,
		 unsigned char selfClass,
		 unsigned char selfSide,
		 unsigned char selfOrient,
		 unsigned char neighborID,
		 unsigned char neighborClass,
		 unsigned char neighborSide){
  this->selfID=selfID;	
  this->selfClass=selfClass;
  this->selfSide=selfSide;
  this->selfOrient=selfOrient;
  this->neighborID=neighborID;
  this->neighborClass=neighborClass;
  this->neighborSide=neighborSide;
}

LinkDescriptor::LinkDescriptor(ifstream *file){
   file->read((char *)&selfID,sizeof(selfID));
   file->read((char *)&selfClass,sizeof(selfClass));
   file->read((char *)&selfSide,sizeof(selfSide));
   file->read((char *)&selfOrient,sizeof(selfOrient));
   file->read((char *)&neighborID,sizeof(neighborID));
   file->read((char *)&neighborClass,sizeof(neighborClass));
   file->read((char *)&neighborSide,sizeof(neighborSide));  
}

void LinkDescriptor::setSelfID(unsigned char selfID){
  this->selfID=selfID;
}

void LinkDescriptor::setSelfClass(unsigned char selfClass){
  this->selfClass=selfClass;
}

void LinkDescriptor::setSelfSide(unsigned char selfSide){
  this->selfSide=selfSide;
}

void LinkDescriptor::setSelfOrient(unsigned char selfOrient){
  this->selfOrient=selfOrient;
}

void LinkDescriptor::setNeighborID(unsigned char neighborID){
  this->neighborID=neighborID;
}

void LinkDescriptor::setNeighborClass(unsigned char neighborClass){
  this->neighborClass=neighborClass;
}
  
void LinkDescriptor::setNeighborSide(unsigned char neighborSide){
  this->neighborSide=neighborSide;
}

unsigned char LinkDescriptor::getSelfID(){
  return selfID;
}

unsigned char LinkDescriptor::getSelfClass(){
  return selfClass;
}

unsigned char LinkDescriptor::getSelfSide(){
  return selfSide;
}

unsigned char LinkDescriptor::getSelfOrient(){
  return selfOrient;
}

unsigned char LinkDescriptor::getNeighborID(){
  return neighborID;
}

unsigned char LinkDescriptor::getNeighborClass(){
  return neighborClass;
}

unsigned char LinkDescriptor::getNeighborSide(){
  return neighborSide;
}

void LinkDescriptor::saveMMFfile(ofstream *file){
   file->write((char *)&selfID,sizeof(selfID));
   file->write((char *)&selfClass,sizeof(selfClass));
   file->write((char *)&selfSide,sizeof(selfSide));
   file->write((char *)&selfOrient,sizeof(selfOrient));
   file->write((char *)&neighborID,sizeof(neighborID));
   file->write((char *)&neighborClass,sizeof(neighborClass));
   file->write((char *)&neighborSide,sizeof(neighborSide));  
}
