#include "module.h"


Module::Module(){
}

Module::~Module(){
}

Module::Module(const int nbInput,
	       const int nbHiden,
	       const int nbOutput,
	       const char typeConnexion){

  for(int n=0;n<nbInput;n++)
    inputs.push_back(new Neural(this));

  for(int n=0;n<nbHiden;n++)
    hidens.push_back(new Neural(this));
  
  for(int n=0;n<nbOutput;n++)
    outputs.push_back(new Neural(this));

  switch (typeConnexion){
  case FULL:

  case FEEDBACK:
    for(int i=0;i<(int)outputs.size();i++)
      for(int j=0;j<(int)inputs.size();j++)
	inputs[j]->synapses.push_back(new Synapse(outputs[i],inputs[j]));

  case PERCEPTRON:
    for(int i=0;i<(int)inputs.size();i++)
      for(int j=0;j<(int)hidens.size();j++)
	hidens[j]->synapses.push_back(new Synapse(inputs[i],hidens[j]));
    
    for(int i=0;i<(int)hidens.size();i++)
      for(int j=0;j<(int)outputs.size();j++)
	outputs[j]->synapses.push_back(new Synapse(hidens[i],outputs[j]));
    
  case NONE:
    break;
  }
}

void Module::init(){
  for(int n=0;n<(int)inputs.size();n++)
    inputs[n]->init();

  for(int n=0;n<(int)hidens.size();n++)
    hidens[n]->init();
      
  for(int n=0;n<(int)outputs.size();n++)
    outputs[n]->init();
}

void Module::activation(){
 for(int n=0;n<(int)inputs.size();n++)
    inputs[n]->activation();

  for(int n=0;n<(int)hidens.size();n++)
    hidens[n]->activation();
      
  for(int n=0;n<(int)outputs.size();n++)
    outputs[n]->activation();
}

void Module::transfer(){
  for(int n=0;n<(int)inputs.size();n++)
    inputs[n]->transfer();

  for(int n=0;n<(int)hidens.size();n++)
    hidens[n]->transfer();
      
  for(int n=0;n<(int)outputs.size();n++)
    outputs[n]->transfer();
}

void Module::save(){

}

void Module::load(){

}

void Module::setX(double x){
	this->x=x;
}

void Module::setY(double y){
	this->y=y;
}

void Module::setZ(double z){
	this->z=z;
}

double Module::getX(){
	return x;
}

double Module::getY(){
	return y;
}

double Module::getZ(){
	return z;
}

Neural *Module::getNeuralInOutByRandom(){
	Neural *neural;
	if(rand()%2)
		neural=inputs[rand()%inputs.size()];
	else
		neural=outputs[rand()%outputs.size()];
	return neural;
}