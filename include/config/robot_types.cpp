#include "include/config/robot_types.h"
#include "include/config/simulated.h"

/* Robot types are differential, threeWheelOmni fourWheelOmni
 * Change one to have that ID registered as a robot of that type. */

#if 1
const RobotType robotIDTypes[] = {
 /*0*/	fourWheelOmni,
 /*1*/	differential,
 /*2*/	fourWheelOmni,
 /*3*/	fourWheelOmni,
 /*4*/	fourWheelOmni,
#if SIMULATED
 /*5*/	fourWheelOmni,
#else
 /*5*/	fourWheelOmni,
#endif
 /*6*/	fourWheelOmni,
 /*7*/	fourWheelOmni,
 /*8*/	fourWheelOmni,
 /*9*/	fourWheelOmni
};
#endif

//Goalie ID integer
float GOALIE_ID = 5;
