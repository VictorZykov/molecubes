#ifndef MODULE_H
#define MODULE_H

#include "neural.h"

#include <iostream>
#include <fstream>
#include <vector>

#define FULL        3
#define PERCEPTRON  2 
#define FEEDBACK    1
#define NONE        0


class Module;

class Module{
 double x;
 double y;
 double z;

 public:
  vector <Neural*> inputs;
  vector <Neural*> hidens;
  vector <Neural*> outputs;
  
  Module();
  ~Module();
  Module(const int nbInput,
	 const int nbHiden,
	 const int nbOutput,
	 const char typeConnexion);
  void init();
  void activation();
  void transfer();
  void save();
  void load();
  void setX(const double x);
  void setY(const double y);
  void setZ(const double z);
  double getX();
  double getY();
  double getZ();
  Neural *getNeuralInOutByRandom();
};

#endif
