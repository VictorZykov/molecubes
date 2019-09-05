#ifndef SYNAPSE_H
#define SYNAPSE_H

#include "neural.h"

class Neural;

class Synapse{
  double weight;
  double value;
  Neural *input;
  Neural *output;

 public:
  Synapse();
  Synapse(double weight);
  Synapse(Neural *input, Neural *output);
  ~Synapse();
  void init();

  void setWeight(const double weight);
  void setValue(const double value);
  void setInput(Neural *input);
  void setOutput(Neural *output);
  
  double getWeight(void);
  double getValue(void);
  Neural *getInput();
  Neural *getOutput();

  void transfer();
};

#endif 
