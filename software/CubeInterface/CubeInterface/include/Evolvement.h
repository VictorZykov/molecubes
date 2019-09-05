/*

#########################################################################################

Cube Interface ver 0.1

Computational Synthesis Lab
Cornell University
Mar 26, 2008

Written by Hang Li

*/

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string>
#include <windows.h>

using namespace std;

const int maxCubes = 20;
const int Population = 100;
const int timeInter = 100;	// set time for each move state as 0.1s (sleep(33) = 0.1)
const int expeTime = 40000; // set experimental time for each gene as 10s(1000 = 1s) 20000
const int setUpInter = 1000 / timeInter;
const int inLength = setUpInter + 5;
const int recIni = 20;
/*
const double Pc = 0.3;
const double Pbc = 0.1;
const double Pm = 0.3;
const double Pi = 0.3;
const double Pr = 0.1;
*/
const double Pc = 0.7;
const double Pbc = 0.7;
const double Pm = 0.05;
const double Pi = 0.05;
const double Pr = 0.1;

static int groupID;
static int numOfCubes;
static int bestMoveID;
static int generationID;
static int timeInterNum;
static int iniTime;
static int timeAcc;
static int recAcc;
static int curTime;

static string gene[maxCubes];

class geneType
{
public:
	/*int timeInterNum;
	int iniTime;
	int timeAcc;
	int timeDiff;
	int totalTime;
	int curTime;*/

	void setGene (int cubeID, string newGene) {gene[cubeID] = newGene;};
	void setFitness (double a) {fitness = a;};
	string getGene(int cubeID) {return gene[cubeID];};
	double getFitness() {return fitness;};
	
	// creatGene for the first generation
	void creatGene (int num){
		for (int a = 0; a < num; a++) {
				gene[a] = geGene ();
		}
	}
	
	// rand a string formed by 0, 1, 2, 3
	string geGene ()
	{
		string s = "";
		int a;
		char b;
		
		for (a = 0; a < inLength; a++){		
			do {
				//srand ( time(NULL) );
				// 4 is the number of states of a cube 
				// (0: keep current state; 1: halt; 2: clockwise; 3: counterclockwise)
				b = static_cast<unsigned char> (48 + rand() % 9);	
			}while (a != 0 && s[a - 1] == b);
		    
			if (b >= 52)
				s.append(1, '0');
			else
				s.append(1, b);
		}
		return s;
	}

private:
	double fitness;
	string gene[maxCubes];
};

void Fit (string Gene[Population], int Fitness[Population]);
void creatSpring (geneType parent[], geneType spring[]);
void cross (geneType &spring1, geneType pa2);
int selectPa (int ranN, int possibility[Population]);
void mutate (geneType &spring);
void bigCross (geneType spring[]);

//string geGene ();

/*int main ()
{
	int cubeID[numOfCube];
	int length, maxFitness;
	geneType parent[Population];
	geneType spring[Population];
	
	int a, b, c;

	//****************************************************************
	// set cube pointer to cubeID
	for (a = 0; a < numOfCube; a++) {
		cubeID[a] = a;
	}
	
	srand (0);
	//srand(static_cast<unsigned int> (time(0)));
	
	for (a = 0; a < Population; a++) {
		for (b = 0; b < numOfCube; b++) {
				parent[a].setGene(b, geGene ());
		}
	}

	// ****************************************
	// case for evolvement to stop
	do {

	// run each gene and calculate the fitness
	for (a = 0; a < Population; a++) {
		string s[numOfCube];
		
		// read the gene into s
		for (b = 0; b < numOfCube; b++) {
			s[b] = parent[a].getGene(b);
		}

		length = s[0].size();
		
		//***********************************************************************
		for (int t = 0; t < expeTime; t += timeInter)
		{
			for (b = 0; b < length; b++) {
				for (c = 0; c < numOfCube; c++) {
					switch (s[c][b]) {
					case '0':
						break;
					case '1':
						// call function to halt a cube
						break;
					case '2':
						// call function to clockwise a cube
						break;
					case '3':
						// call function to counterclockwise a cube
					break;
					}
				}
			
			// time for each movement
			//Sleep(moveTime);
			}
		}
			
		//****************************************************
		// calculate fitness for this Gene
	    parent[a].setFitness(length);
	}



	for (a = 0; a < Population; a++) {
		if (maxFitness < parent[a].getFitness())
			maxFitness = parent[a].getFitness();

		for (b = 0; b < numOfCube; b++) {
			cout << a << "-" << b << ": " << parent[a].getGene(b) 
				 << " " << parent[a].getFitness() << endl;
		}
	}

	creatSpring(parent, spring);
	
	for (a = 0; a < Population; a++) {
		parent[a] = spring[a];
		for (b = 0; b < numOfCube; b++) {
			cout << a << "-" << b << ": " << spring[a].getGene(b)<< endl;
		}
	}

	}while (maxFitness <= 5);

	for (a = 0; a < Population; a++) {
		if (spring[a].getFitness() == 0) {
			for (b = 0; b < numOfCube; b++) {
				cout << a << "-" << b << ": " << spring[a].getGene(b) 
					 << " " << spring[a].getFitness() << endl;
			}
		}
	}

	

	/*GePo (Gene);
	
	do
	{
		Fit (Gene, Fitness);
		
		for (a = 0; a < Population; a++)
		{
			cout << Gene[a] << " Fitness: " << Fitness[a] << endl;
			if (Fitness[a] == Length) 
				b = true;
		}
		
		if (b == false)
		{
			CSpring (Gene, Spring, Fitness);
	
			for (a = 0; a < Population; a++)
			{
				Gene[a] = Spring[a];
				cout << Gene[a] << " " << Spring[a] << endl;
			}
		}
	
	} while (!b);

  
	system("pause");

	return 0;
}*/

//	function that used to generate a initial gene for a cube


/*
void Fit (string Gene[Population], int Fitness[Population])
{
	int a, b;

	for (a = 0; a < Population; a++)
	{
		Fitness[a] = 0;

		for (b = 0; b < Length; b++)
			Fitness[a] += (Gene[a][b] - 48);
	}
}*/

void creatSpring (geneType parent[], geneType spring[])
{
	int a, pa1, pa2, crossPo;
	int total = 0;
	int possibility[Population];
	string P1, P2;
	
	for (a = 0; a < Population; a++)
	{
		if (a == bestMoveID) total += static_cast<unsigned int> (parent[a].getFitness() * 200);
		else total += static_cast<unsigned int> (parent[a].getFitness() * 100);

		possibility[a] = total;
	}

	if (parent[0].getFitness() < parent[bestMoveID].getFitness())
		spring[0] = parent[bestMoveID];
	else
		spring[0] = parent[0];

	spring[1] = parent[bestMoveID];

	for (a = 2; a < Population; a++)
	{
		int rPo = static_cast<unsigned int> (1000 * Pr);
		//srand ( time(NULL) );
		if (rand() % 1000 <= rPo)	{
			for (int b = 0; b < numOfCubes; b++) {
				spring[a].setGene(b, geneType().geGene());
			}
		}
		else {
			pa1 = selectPa(rand() % total, possibility);
		
			pa2 = selectPa(rand() % total, possibility);
			
			spring[a] = parent[pa1];
			
			crossPo = static_cast<unsigned int> (1000 * Pc);
			//srand ( time(NULL) );
			if (rand() % 1000 <= crossPo)
				cross(spring[a], parent[pa2]);

			mutate (spring[a]);
		}
	}

	bigCross (spring);
}
	
int selectPa (int ranN, int possibility[])
{
	int a;
	for (a = 0; a < Population; a++)
	{
		if (ranN <= possibility[a])
			break;
	}
	return a;
}


void cross(geneType &spring, geneType pa2)
{
	int crossPoint;
	int shortLength;
	int pa1ID, pa2ID;
	string gene[2];
	
	//srand ( time(NULL) );
	pa1ID = rand() % numOfCubes;
	pa2ID = (pa1ID + rand() % 100) % numOfCubes;

	gene[0] = spring.getGene(pa1ID);
	gene[1] = pa2.getGene(pa2ID);

	if ((spring.getGene(pa1ID)).size() <= (pa2.getGene(pa2ID)).size())
		shortLength = (gene[0]).size();
	else
		shortLength = (gene[1]).size();
	
	crossPoint = 1 + rand() % (shortLength - 1);
	
	for (int b = 0; b < crossPoint; b++) {
		gene[0][b] = gene[1][b];
	}
		
	spring.setGene(pa1ID, gene[0]);
}

void mutate (geneType &spring)
{
	int a;
	int mutatPo, possibility, mutatPl, mutatTo, appendPo;
	int length = spring.getGene(0).size();
	string gene;
	//srand ( time(NULL) );

	for (a = 0; a < numOfCubes; a++) {
		mutatPo = static_cast<unsigned int> (Pm * 1000) * length;
		//srand ( time(NULL) );
		possibility = rand() % 1000;

		if (possibility <= mutatPo)
		{
			gene = spring.getGene(a);
			
			mutatPl = possibility / (static_cast<unsigned int> (Pm * 1000));

			do {
				mutatTo = 48 + rand() % 9;				
			} while (mutatTo == gene[mutatPl]);
			
			if (mutatTo >= 52)
				gene[mutatPl] = 48;
			else
				gene[mutatPl] = mutatTo;
			
			spring.setGene(a, gene);
		}
	}
	
	//srand ( time(NULL) );
		appendPo = static_cast<unsigned int> (Pi * 1000);
		possibility = rand() % 1000;

		if (possibility <= appendPo) {
			char b;
			
			for (int c = 0; c < numOfCubes; c++)
			{
				gene = spring.getGene(c);
				////srand ( time(NULL) );
				b = 48 + rand() % 4;

				spring.setGene(c, gene.append(1,b));
			}
		}
}

void bigCross (geneType spring[])
{
	int crossPo, possibility, group1, group2, cGene[2];
	string gene;

	crossPo = static_cast<unsigned int> (Pbc * 1000) * Population;
	possibility = rand() % 1000;

	if (possibility <= crossPo){
		
		srand ( time(NULL) );
		group1 = rand() % (Population - 2) + 2;
		
		do{
			//srand ( time(NULL) );
			group2 = rand() % (Population - 2) + 2;
		}while (group1 == group2);

		//srand ( time(NULL) );
		cGene[0] = rand() % numOfCubes;

		do{
			//srand ( time(NULL) );
			cGene[1] = rand() % numOfCubes;
		}while (cGene[0] == cGene[1]);

		spring[group1].setGene(cGene[0], spring[group1].getGene(cGene[1]));

		/*for (int a = 0; a < rand() % numOfCubes; a++){
			gene = spring[group1].getGene(a);
			spring[group1].setGene(a, spring[group2].getGene(a));
			spring[group2].setGene(a, gene);
		}*/
	}
}