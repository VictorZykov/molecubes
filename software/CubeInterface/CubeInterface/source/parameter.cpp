#include "parameter.h"


void Parameter::setFaceOnTop(char faceOnTop){
	this->faceOnTop=faceOnTop;
}
	
void Parameter::setHeight_star(float height_star){
	this->height_star=height_star;
}
	
char Parameter::getFaceOnTop(){	 
	return faceOnTop;
}

float Parameter::getHeight_star(){
	return height_star;
}

bool Parameter::load(const char *filename){
  ifstream file(filename);
  if(file.is_open()){
		file.read((char *)&faceOnTop,sizeof(faceOnTop));
		file.read((char *)&height_star,sizeof(height_star));	
  return 1;
  }
  std::cout <<"File does not exit: "<<filename<<endl;
  return 0;
}

void Parameter::save(const char *filename){
	ofstream file (filename,ios::binary|ios::out);  
	file.write((char *)&faceOnTop,sizeof(faceOnTop));
	file.write((char *)&height_star,sizeof(height_star));
	file.close();
}


Parameter::Parameter(){
}

Parameter::~Parameter(){
}