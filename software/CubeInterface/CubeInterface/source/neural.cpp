#include "neural.h"

double Neural::sum(){
  double result=0;
  for(int n=0;n<(int)synapses.size();n++)
    result+=synapses[n]->getValue();
  return result/synapses.size();
}

Neural::Neural(){
  output=0;
  lambda=1;
  attachedModule=NULL;
}

Neural::Neural(Module *attachedModule){
  output=0;
  lambda=1;
  this->attachedModule=attachedModule;
}


Neural::~Neural(){
}

void Neural::init(){
  output=0;
  for(int n=0;n<(int)synapses.size();n++)
    synapses[n]->init();
}

double Neural::sigmoid(double x,double lambda){
  return 1.0/(1.0+exp(-lambda*x));
}

double Neural::getOutput(){
  return output;
}

void Neural::activation(){
  output=sigmoid(sum(),lambda);
}

void Neural::transfer(){
  for(int n=0;n<(int)synapses.size();n++)
    synapses[n]->transfer();
}

void Neural::save(ofstream *file){
  file->write((char *) &lambda, sizeof(lambda));
  
  int weightSize=(int)synapses.size();
  file->write((char *) &weightSize, sizeof(weightSize));
  for( int n=0; n<(int)weightSize; n++){
    double weight=synapses[n]->getWeight();
    file->write((char *) &weight, sizeof(weight));
  }
}

void Neural::load(ifstream *file){
  file->read((char *) &lambda, sizeof(lambda));
  
  int weightSize;
  file->read((char *) &weightSize, sizeof(weightSize)); 
  double weight;  
  for( int n=0; n<weightSize; n++){
    file->read((char *) &weight, sizeof(weight));
    synapses.push_back(new Synapse(weight));
  }
}

void Neural::setLambda(const double lambda){
  this->lambda=lambda;
}

void Neural::setX(double x){
	this->x=x;
}

void Neural::setY(double y){
	this->y=y;
}

void Neural::setZ(double z){
	this->z=z;
}

double Neural::getX(){
	return x;
}

double Neural::getY(){
	return y;
}

double Neural::getZ(){
	return z;
}


Module *Neural::getAttachedModule(){
	return attachedModule;
}

void Neural::setAttachedModule(Module *attachedModule){
	this->attachedModule=attachedModule;
}

void Neural::addSynapse(Neural *input){
	synapses.push_back(new Synapse(input,this));
}