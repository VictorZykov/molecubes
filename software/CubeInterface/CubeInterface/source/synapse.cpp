#include "synapse.h"

Synapse::Synapse(){
  weight=0;
  value=0;
  input=NULL;
  output=NULL;
}

Synapse::Synapse(double weight){
  this->weight=weight;
  value=0;
  input=NULL;
  output=NULL;
}
Synapse::Synapse(Neural *input, Neural *output){
  weight=0;
  value=0;
  this->input=input;
  this->output=output;
}

Synapse::~Synapse(){
}

void Synapse::init(){
  weight=0;
  value=0;
}

void Synapse::setWeight(const double weight){
  this->weight=weight;
}

void Synapse::setValue(const double value){
  this->value=value;
}

void Synapse::setInput(Neural *input){
  this->input=input;
}

void Synapse::setOutput(Neural *output){
  this->output=output;
}
  
double Synapse::getWeight(void){
  return weight;
}

double Synapse::getValue(void){
  return value;
}

Neural *Synapse::getInput(){
  return input;
}

Neural *Synapse::getOutput(){
  return output;
}

void Synapse::transfer(){
  value=input->getOutput()*weight;
}
