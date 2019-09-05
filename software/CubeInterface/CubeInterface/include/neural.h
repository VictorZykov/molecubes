#ifndef NEURAL_H
#define NEURAL_H

#include "synapse.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

class Synapse;
class Module;

class Neural{
  double x;
  double y;
  double z;
  double output;
  double lambda;
  Module *attachedModule;

 public:
  vector <Synapse*> synapses;
  
  Neural();
  Neural(Module *attachedModule);
  ~Neural();
  
  void init();
  
  double sum();
  double getOutput();
  void setLambda(const double lambda);
  double sigmoid(const double x,const double lambda);
  void load(ifstream *file);
  void save(ofstream *file);
  void activation();
  void transfer();
  void setX(const double x);
  void setY(const double y);
  void setZ(const double z);
  double getX();
  double getY();
  double getZ();
  Module *getAttachedModule();
  void setAttachedModule(Module *attachedModule);
  void Neural::addSynapse(Neural *input);
};

#endif
