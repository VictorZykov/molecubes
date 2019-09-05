#ifndef PARAMETER_H
#define PARAMETER_H

#include <iostream>
#include <fstream>

using namespace std;

class  Parameter{
  char faceOnTop;
  float height_star;
 public:
  Parameter();
  ~Parameter();

  void setFaceOnTop(char faceOnTop);	
  void setHeight_star(float height_star);	
  char getFaceOnTop();
  float getHeight_star();
  bool load(const char *filename);
  void save(const char *filename);  
};
#endif
