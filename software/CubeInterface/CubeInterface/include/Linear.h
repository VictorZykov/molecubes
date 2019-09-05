/*

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

const double maxError = 0.01 * 2 * 3.1415926;
const int tRec = expeTime / recIni;				// Total number of record cycles

void linearlization(int tCounter,int numOfCubes, double y[][tRec], double** outY)
{
	int x1 = 0;
	int x2 = 2;
	int cx;
	double cy;			// cx, cy is the current point under testing
	double y1, y2;		// y1 is the value of first point, y2 is the value
	double s, i, d;		// s is slope of the linear funciton formed by point 1 and 2, 
						// i is the intersecpt on y axis
						// d is the Y-distance between current point and the line
	
	for (int c = 0; c < numOfCubes; c++){
		x1 = 0;
		x2 = 2;
	
		while (x2 < tCounter) {
			y1 = y[c][x1];
			y2 = y[c][x2];

			s = (y2 - y1)/(x2 - x1);
			i = y1 - s * x1;

			cx = x1 + 1;
			
			while (cx != x2){
				cy = s * cx + i;
				d = abs(cy - y[c][cx]);

				if (d > maxError) break;
				else cx = cx + 1;
			}
		
			if (cx == x2 && (x2 < (tCounter - 1)) && (abs(y2 - y1) <= 3.1415926*12/18))
				x2 = x2 + 1;
			else {
				if (x2 < (tCounter - 2)) {
					for (int a = x1; a <= x2; a++) {
						if ((a == x1) || (a == x2)) {
							//if (y[c][x2-1] = y[c][x2-2])
							//	outY[c][x2] = y[c][x2];
							//else
							//	cout << "";
							outY[c][a] = y[c][a];
						}
						else
							outY[c][a] = 0;
					}
					
					

					x1 = x2;
					x2 = x2 + 2;
				}
				else {
					x2 = x2 + 1;
				}
			}
		}
		outY[c][tCounter-1] = y[c][tCounter-1];
	}
}
/*
void linearlization(int tCounter,int numOfCubes, double y[][tRec], double** outY)
{
	int x1 = 0;
	int x2 = 2;
	int cx;
	double cy;			// cx, cy is the current point under testing
	double y1, y2;		// y1 is the value of first point, y2 is the value
	double s, i, d;		// s is slope of the linear funciton formed by point 1 and 2, 
						// i is the intersecpt on y axis
						// d is the Y-distance between current point and the line
	
	for (int c = 0; c < numOfCubes; c++){
		x1 = 0;
		x2 = 2;
	
		while (x2 < tRec) {
			y1 = y[c][x1];
			y2 = y[c][x2];

			s = (y2 - y1)/(x2 - x1);
			i = y1 - s * x1;

			cx = x1 + 1;
			
			while (cx != x2) {
				cy = s * cx + i;
				d = abs(cy - y[c][cx]);

				if (d > maxError) break;
				else cx = cx + 1;
			}
		
			if (cx == x2 && (x2 < (tRec - 1)) && (abs(y2 - y1) <= 3.1415926*12/18))
				x2 = x2 + 1;
			else {
				if (x2 < (tRec - 2)) {
					for (int a = x1; a <= x2; a++) {
						if ((a == x1) || (a == x2)) {
							//if (y[c][x2-1] = y[c][x2-2])
							//	outY[c][x2] = y[c][x2];
							//else
							//	cout << "";
							outY[c][a] = y[c][a];
						}
						else
							outY[c][a] = 0;
					}
					
					

					x1 = x2;
					x2 = x2 + 2;
				}
				else {
					x2 = x2 + 1;
				}
			}
		}
		outY[c][tRec-1] = y[c][tRec-1];
	}
}
*/