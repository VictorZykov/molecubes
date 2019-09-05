#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>

#include "neural.h"
#include "module.h"

#define FULL        3
#define PERCEPTRON  2 
#define FEEDBACK    1
#define NONE        0
#define FULL        3
#define PERCEPTRON  2 
#define FEEDBACK    1
#define NONE        0

using namespace std;

class Network;

class Network{

 public:
  vector <Module *> modules;
  Network();
  Network(int nbModule);
  ~Network();
  void init();
  void addModule(const int nbInput,
		 const int nbHiden,
		 const int nbOutput,
		 const char typeConnexion);
  void addSynapse();

  void activation();
  void transfer();
  void save();
  void load();

  Module *getModuleByRandom();
};

#endif
