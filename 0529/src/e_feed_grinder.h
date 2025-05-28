#ifndef E_FEED_GRINDER_H
#define E_FEED_GRINDER_H

#include "globals.h"
#include "pinnum.h"

extern bool isGrinding;

void initmotorGrinder();
void motorGrinder();
void rotateMotor(int speed);
bool isGrindingDone();
#endif