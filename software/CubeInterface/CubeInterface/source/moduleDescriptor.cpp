#include "moduleDescriptor.h"

ModuleDescriptor::ModuleDescriptor(unsigned char id,
  unsigned char molecubeClass, unsigned int angle){
  this->id=id;
  this->virtualID=0;
  this->molecubeClass=molecubeClass;
  this->numLinks=0;
  this->angle=angle;
}

ModuleDescriptor::ModuleDescriptor(unsigned char id,
  unsigned char molecubeClass, unsigned int angle,char virtualID){
  this->id=id;
  this->virtualID=virtualID;
  this->molecubeClass=molecubeClass;
  this->numLinks=0;
  this->angle=angle;
}


ModuleDescriptor::ModuleDescriptor(ifstream *file){
  LinkDescriptor *link;
  file->read((char *)&id,sizeof(id));
  virtualID=0;
  file->read((char *)&molecubeClass,sizeof(molecubeClass));
  angle=0;
  file->read((char *)&numLinks,sizeof(numLinks));  
  for(int n=0;n<numLinks;n++){
    link=new LinkDescriptor(file);  
    links.push_back(link);
  }
}

ModuleDescriptor::~ModuleDescriptor(){
	while(links.size()!=0){
	  delete links[links.size()-1];	
	  links.pop_back();
	}
	links.clear();
}

unsigned char ModuleDescriptor::getID(){
  return id;
}

unsigned char ModuleDescriptor::getVirtualID(){
  return virtualID;
}

unsigned char ModuleDescriptor::getMolecubeClass(){
  return molecubeClass;
}

unsigned char ModuleDescriptor::getNumLinks(){
  return numLinks;
}

unsigned int ModuleDescriptor::getAngle(){
  return angle;
}

void ModuleDescriptor::setID(unsigned char id){
  this->id=id;
}

void ModuleDescriptor::setVirtualID(unsigned char virtualID){
  this->virtualID=virtualID;
}

void ModuleDescriptor::setMolecubeClass(unsigned char molecubeClass){
  this->molecubeClass=molecubeClass;
}

void ModuleDescriptor::setNumLinks(unsigned char numLinks){
  this->numLinks=numLinks;
}

void ModuleDescriptor::setAngle(unsigned int angle){
 this->angle=angle;
}

void ModuleDescriptor::addLink(unsigned char selfSide, ModuleDescriptor *neighbor,
  unsigned char neighborSide,unsigned char orientation){
  LinkDescriptor *selfLink,*neighborLink;
  selfLink=new LinkDescriptor(this->id,this->molecubeClass,selfSide,orientation,neighbor->getID(),neighbor->getMolecubeClass(),neighborSide);
  neighborLink=new LinkDescriptor(neighbor->getID(),neighbor->getMolecubeClass(),neighborSide,orientation,this->id,this->molecubeClass,selfSide);
  this->addLink(selfLink);
  neighbor->addLink(neighborLink);
}

void ModuleDescriptor::addLink(LinkDescriptor *link){
  numLinks++;
  links.push_back(link);
}

void ModuleDescriptor::deleteLink(unsigned char id){
  LinkDescriptor *link;
  id=searchLinkId(id);
  link=links[id];
	  for(int i=id;i<numLinks-1;i++)
   links[i]=links[i+1];
  delete(link);
  links.pop_back();
  numLinks--;
}

unsigned char ModuleDescriptor::searchLinkId(unsigned char id){
	for(int n=0;n<numLinks;n++)
	  if(links[n]->getNeighborID()==id)
		return n;
  return 0;
}

void ModuleDescriptor::saveMMFfile(ofstream *file){
  file->write((char *)&id,sizeof(id));
  file->write((char *)&molecubeClass,sizeof(molecubeClass));
  file->write((char *)&numLinks,sizeof(numLinks));
  vector<LinkDescriptor*>::iterator it;
  if(links.size()!=0)   
    for(it=links.begin();it!=links.end();it++)
      ((LinkDescriptor *)*it)->saveMMFfile(file);
}

 
