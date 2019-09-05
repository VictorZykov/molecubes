#ifndef _LEDDRIVER_H_
#define _LEDDRIVER_H_

#include "global.h"

// PCA addresses (to address individual LED drivers on each of the three triplet PCBs)
#define     M_PCA   0b11000000
#define     L_PCA   0b11000010
#define     R_PCA   0b11000100
#define     All_PCA 0b11100000

void RGB_LED_init(u08 addr, u08 red, u08 green, u08 blue);
void RGB_LED_PWMx(u08 addr, u08 red, u08 green, u08 blue);

#endif
