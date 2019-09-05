#include "molecubeMorphology.h"


MolecubeMorphology::MolecubeMorphology(){
  cubeCount=0;
  morphologyChecksum=0;
  stateCount=0;  
  stateChecksum=0;
}

MolecubeMorphology::~MolecubeMorphology(){
	ModuleDescriptor *cube;
	while(cubes.size()!=0){
	  cube=cubes[cubes.size()-1];	
	  cubes.pop_back();
	  delete cube;
	}
	cubes.clear();	
}

void MolecubeMorphology::clear(void){
	cubeCount=0;
	morphologyChecksum=0;
    stateCount=0;  
    stateChecksum=0;
	while(cubes.size()!=0){
	  delete cubes[cubes.size()-1];	
	  cubes.pop_back();
	}
	while(states.size()!=0){
	  delete states[states.size()-1];	
	  states.pop_back();
	}
	states.clear();
	cubes.clear();		
}

void MolecubeMorphology::clearState(void){	
    stateCount=0;  
    stateChecksum=0;
	while(states.size()!=0){
	  delete states[states.size()-1];	
	  states.pop_back();
	}
	states.clear();
}


unsigned char MolecubeMorphology::getFreeID(){
	int id;
	for(id=0;id<250;id++){
	 bool freeID=true;
	 for(int n=0;n<cubeCount;n++)
	    if(cubes[n]->getID()==id)
		  freeID=false;
	 if(freeID==true){
		// std::cout<<"   MolecubeMorphology::getFreeID return->ID"<<dec<<(int)id<<endl;
		return id;
	 }
	} 
  std::cout<<"ERROR: MolecubeMorphology::getFreeID 255 return->ID"<<dec<<id<<endl;
  return 255;
}


void MolecubeMorphology::addMolecube(ModuleDescriptor *cube){
  cubeCount++;
  cubes.push_back(cube);
}

void MolecubeMorphology::addState(StateDescriptor *state){
  stateCount++;
  states.push_back(state);
}


void MolecubeMorphology::deleteMolecube(unsigned char id){

  for(int n=0;n<cubes[id]->getNumLinks();n++)
	  cubes[cubes[id]->links[n]->getNeighborID()]->deleteLink(cubes[id]->getID());
  
  for(int n=id;n<cubeCount-1;n++){
     cubes[n]=cubes[n+1];
	 cubes[n]->setVirtualID(cubes[n]->getVirtualID()-1);
  }
 
  cubes.pop_back();
  cubeCount--;
}

bool MolecubeMorphology::loadMMFfile(const char *filename){
  ModuleDescriptor *cube;
  StateDescriptor *state;
  ifstream file(filename);
  if(file.is_open()){
   file.read((char *)&cubeCount,sizeof(cubeCount)); 
   for(int n=0;n<cubeCount;n++){
    cube=new ModuleDescriptor(&file);  
    cubes.push_back(cube);
   }
  file.read((char *)&morphologyChecksum,sizeof(morphologyChecksum));
  file.read((char *)&stateCount,sizeof(stateCount));

   for(unsigned int n=0;n<stateCount;n++){
    state=new StateDescriptor(&file);
	states.push_back(state);
   }
   file.read((char *)&stateChecksum,sizeof(stateChecksum));
   file.close(); 
   for(unsigned int n=0;n<stateCount;n++){
	  if(states[n]->getTimestamp()==0)
		  cubes[searchCubeID(states[n]->getId())]->setAngle(states[n]->getValue());
    }
 
   return 1;
  }
  std::cout <<"File does not exit: "<<filename<<endl;
  return 0;
}

unsigned char MolecubeMorphology::getCubeCount(){
	return cubeCount;
}

unsigned int MolecubeMorphology::getStateCount(){
	return cubeCount;
}

void MolecubeMorphology::display(){
 std::cout <<endl<<"==============================================================================="<<endl;
 std::cout <<"nbcubes: "<<(int)cubeCount<<endl;
 for(int n=0;n<cubeCount;n++){		
		std::cout <<dec<<"n="<<n<<" molecube VirtualID="<<(int)cubes[n]->getVirtualID()<<" ID="<<(int)cubes[n]->getID()<<" angle="<<(unsigned int)cubes[n]->getAngle()/10<< " nbLink="<<(int)cubes[n]->getNumLinks()<<" class:"<<hex<< (int)cubes[n]->getMolecubeClass()<<endl;					
	    for(int i=0;i<cubes[n]->getNumLinks();i++)
			std::cout<<"  link:"<<(int)cubes[n]->links[i]->getSelfID()<<"->"<<(int)cubes[n]->links[i]->getNeighborID()<<endl; 
 }
 std::cout <<endl<<"==============================================================================="<<endl<<endl;
}

void MolecubeMorphology::saveMMFfile(const char *filename){
  ofstream file (filename,ios::binary|ios::out);  
  file.write((char *)&cubeCount,sizeof(cubeCount));
  clearState(); 
  for(unsigned int n=0;n<cubeCount;n++){
   cubes[n]->saveMMFfile(&file);
   if(cubes[n]->getMolecubeClass()==CLASS_ACTUATOR)
	  addState(new StateDescriptor(cubes[n]->getID(),cubes[n]->getMolecubeClass(),0,0,cubes[n]->getAngle()));
  }  
  file.write((char *)&morphologyChecksum,sizeof(morphologyChecksum));
  file.write((char *)&stateCount,sizeof(stateCount));
  for(unsigned int n=0;n<stateCount;n++)
	states[n]->saveMMFfile(&file);
  file.write((char *)&stateChecksum,sizeof(stateChecksum));
  file.close();
}

unsigned char MolecubeMorphology::searchCubeID(unsigned char ID){
  for (int n=0;n<cubeCount;n++)
	if(cubes[n]->getID()==ID)
	 return n;
  //display();
  //std::cout <<"ERROR searchCubeID: "<<(int)ID<<endl;  
  return 255;
}

unsigned char MolecubeMorphology::searchVirtualID(unsigned char virtualID){
  for (int n=0;n<cubeCount;n++)
	if(cubes[n]->getVirtualID()==virtualID)
	 return n;
  display();
  std::cout <<"ERROR searchVirtualID: "<<(int)virtualID<<endl; 
  return 255;
}

void molecubeStarG(){
 MolecubeMorphology *molecubeMorphology;
 molecubeMorphology=new MolecubeMorphology();
	
 molecubeMorphology->addMolecube(new ModuleDescriptor(0,CLASS_BATTERY,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(1,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(2,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(3,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(4,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(5,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(6,CLASS_ACTUATOR,0));

 /*
 molecubeMorphology->addMolecube(new ModuleDescriptor(7,CLASS_GRIPPER,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(8,CLASS_GRIPPER,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(9,CLASS_GRIPPER,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(10,CLASS_GRIPPER,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(11,CLASS_GRIPPER,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(12,CLASS_GRIPPER,0));
*/
 molecubeMorphology->addMolecube(new ModuleDescriptor(7,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(8,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(9,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(10,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(11,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(12,CLASS_ACTUATOR,0));

 molecubeMorphology->cubes[0]->addLink(1,molecubeMorphology->cubes[1],5,0);
 molecubeMorphology->cubes[0]->addLink(2,molecubeMorphology->cubes[2],4,1);
 molecubeMorphology->cubes[0]->addLink(3,molecubeMorphology->cubes[3],0,1);
 molecubeMorphology->cubes[0]->addLink(4,molecubeMorphology->cubes[4],2,1);
 molecubeMorphology->cubes[0]->addLink(5,molecubeMorphology->cubes[5],1,1);
 molecubeMorphology->cubes[0]->addLink(0,molecubeMorphology->cubes[6],3,1);

 molecubeMorphology->cubes[1]->addLink(1,molecubeMorphology->cubes[7],4,3);
 molecubeMorphology->cubes[2]->addLink(2,molecubeMorphology->cubes[8],4,3);
 molecubeMorphology->cubes[3]->addLink(3,molecubeMorphology->cubes[9],4,3);
 molecubeMorphology->cubes[4]->addLink(4,molecubeMorphology->cubes[10],4,3);
 molecubeMorphology->cubes[5]->addLink(5,molecubeMorphology->cubes[11],4,3);
 molecubeMorphology->cubes[6]->addLink(0,molecubeMorphology->cubes[12],4,3);


  molecubeMorphology->saveMMFfile("starG.MMF");
}


void molecubeStar(){
 MolecubeMorphology *molecubeMorphology;
 molecubeMorphology=new MolecubeMorphology();

 molecubeMorphology->addMolecube(new ModuleDescriptor(0,CLASS_CONTROLLER,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(1,CLASS_BATTERY,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(2,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(3,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(4,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(5,CLASS_ACTUATOR,0));
 
 molecubeMorphology->cubes[0]->addLink(2,molecubeMorphology->cubes[1],2,1);
 molecubeMorphology->cubes[1]->addLink(0,molecubeMorphology->cubes[2],4,1);
 molecubeMorphology->cubes[1]->addLink(1,molecubeMorphology->cubes[3],4,1);
 molecubeMorphology->cubes[1]->addLink(4,molecubeMorphology->cubes[4],4,1);
 molecubeMorphology->cubes[1]->addLink(3,molecubeMorphology->cubes[5],4,1);

 molecubeMorphology->saveMMFfile("star.MMF");
}



void molecubeStar2(){
 MolecubeMorphology *molecubeMorphology;
 molecubeMorphology=new MolecubeMorphology();

 molecubeMorphology->addMolecube(new ModuleDescriptor(0,CLASS_CONTROLLER,0));	
 molecubeMorphology->addMolecube(new ModuleDescriptor(1,CLASS_BATTERY,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(2,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(3,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(4,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(5,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(6,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(7,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(8,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(9,CLASS_ACTUATOR,0));
	
 molecubeMorphology->cubes[0]->addLink(2,molecubeMorphology->cubes[1],2,1);
 molecubeMorphology->cubes[1]->addLink(0,molecubeMorphology->cubes[2],3,1);
 molecubeMorphology->cubes[1]->addLink(1,molecubeMorphology->cubes[3],5,1);
 molecubeMorphology->cubes[1]->addLink(4,molecubeMorphology->cubes[4],2,1);
 molecubeMorphology->cubes[1]->addLink(3,molecubeMorphology->cubes[5],0,1);

 molecubeMorphology->cubes[2]->addLink(0,molecubeMorphology->cubes[6],4,1);
 molecubeMorphology->cubes[3]->addLink(1,molecubeMorphology->cubes[7],4,1);
 molecubeMorphology->cubes[4]->addLink(4,molecubeMorphology->cubes[8],4,1);
 molecubeMorphology->cubes[5]->addLink(3,molecubeMorphology->cubes[9],4,1);

  molecubeMorphology->saveMMFfile("star2.MMF");
}

void molecubeStar3(){
 MolecubeMorphology *molecubeMorphology;
 molecubeMorphology=new MolecubeMorphology();
	
 molecubeMorphology->addMolecube(new ModuleDescriptor(0,CLASS_BATTERY,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(1,CLASS_CONTROLLER,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(2,CLASS_ACTUATOR,3000));
 molecubeMorphology->addMolecube(new ModuleDescriptor(3,CLASS_ACTUATOR,600));
 molecubeMorphology->addMolecube(new ModuleDescriptor(4,CLASS_ACTUATOR,600));
 molecubeMorphology->addMolecube(new ModuleDescriptor(5,CLASS_ACTUATOR,3000));
 molecubeMorphology->addMolecube(new ModuleDescriptor(6,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(7,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(8,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(9,CLASS_ACTUATOR,0));
	
 molecubeMorphology->cubes[0]->addLink(1,molecubeMorphology->cubes[1],1,0);
 molecubeMorphology->cubes[0]->addLink(2,molecubeMorphology->cubes[2],4,1);
 molecubeMorphology->cubes[0]->addLink(3,molecubeMorphology->cubes[3],0,3);
 molecubeMorphology->cubes[0]->addLink(4,molecubeMorphology->cubes[4],2,2);
 molecubeMorphology->cubes[0]->addLink(0,molecubeMorphology->cubes[5],3,0);
 
 molecubeMorphology->cubes[2]->addLink(2,molecubeMorphology->cubes[6],4,1);
 molecubeMorphology->cubes[3]->addLink(3,molecubeMorphology->cubes[7],0,3);
 molecubeMorphology->cubes[4]->addLink(4,molecubeMorphology->cubes[8],2,2);
 molecubeMorphology->cubes[5]->addLink(0,molecubeMorphology->cubes[9],3,0);


  molecubeMorphology->saveMMFfile("star3.MMF");
}


//test link, oriention and faces between two cubes
void molecubeTest(char selfFace, char selfNeighbor, char orientation,string filename){
 MolecubeMorphology *molecubeMorphology;
 molecubeMorphology=new MolecubeMorphology();
	
 molecubeMorphology->addMolecube(new ModuleDescriptor(0,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(1,CLASS_ACTUATOR,0));
	
 molecubeMorphology->cubes[0]->addLink(selfFace,molecubeMorphology->cubes[1],selfNeighbor,orientation);

  molecubeMorphology->saveMMFfile(&filename[0]);
}

void molecubeSuperTest(char selfFace, char selfNeighbor, char orientation, char selfFace2, char selfNeighbor2, char orientation2,string filename){
 MolecubeMorphology *molecubeMorphology;
 molecubeMorphology=new MolecubeMorphology();
	
 molecubeMorphology->addMolecube(new ModuleDescriptor(0,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(1,CLASS_ACTUATOR,0));
 molecubeMorphology->addMolecube(new ModuleDescriptor(2,CLASS_ACTUATOR,0));
	
 molecubeMorphology->cubes[0]->addLink(selfFace,molecubeMorphology->cubes[1],selfNeighbor,orientation);
 molecubeMorphology->cubes[1]->addLink(selfFace2,molecubeMorphology->cubes[2],selfNeighbor2,orientation2);

  molecubeMorphology->saveMMFfile(&filename[0]);
}



void molecubeSpiral(char size,string filename){
	MolecubeMorphology *molecubeMorphology;
	molecubeMorphology=new MolecubeMorphology();

	for(char n=0;n<size;n++)
		molecubeMorphology->addMolecube(new ModuleDescriptor(n,CLASS_ACTUATOR,450));
	
	for(char n=0;n<size-1;n++)
	  molecubeMorphology->cubes[n]->addLink(3,molecubeMorphology->cubes[n+1],0,2);

  molecubeMorphology->saveMMFfile(&filename[0]);
}


/*
int main(int argc, char *argv[]){
  molecubeSpiral(10);
}
*/