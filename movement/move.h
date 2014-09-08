#ifndef MOVEMENT_MOVE_H
#define MOVEMENT_MOVE_H

#include <deque>
#include "include/config/globals.h"
#include "model/robot.h"
#include "model/gamemodel.h"
#include "movement/pathfinding/fppa_pathfinding.h"
#include "movement/movetype.h"

namespace Movement
{

/* Separate from a skill, Movement::Move represents
 * a layer lower than those in Skill. Movement contains
 * routines for robot movement, including pathfinding,
 * GotoPosition, three/four wheel omni movement, and the differential
 * control modules "closed loop control (CLC)."
 *
 * This is the Movement's core base class, it has some standardized features now:
 * - Velocity multiplier via setVelocityMultiplier
 *   1.0 is normal velocity, 0.0 is no velocity. This is different from before.
 * - Obstacle avoidance or standard movement via withObstacleAvoid bool;
 *   However, this is abstracted from the velocity calculators; it only
 *   asks the calculators to compute velocity to the next points.
 */
#define MOVEMENT_MOVE_DEBUG 0

class Move
{
public:
    Move();
    Move(Point targetPoint, float targetAngle = UNUSED_ANGLE_VALUE, bool withObstacleAvoid = true);
    virtual ~Move();
	
    void recreate(Point targetPoint, float targetAngle = UNUSED_ANGLE_VALUE, bool withObstacleAvoid = true);
    
    void setVelocityMultiplier(float newMultiplier);
    
    bool perform(Robot* robot, Type moveType = Type::Default);
	
protected:
    float lfront, lback, rfront, rback;  //rob->type() == fourWheelOmni
    float left, right;                   //rob->type() == differential;
    float back;                          //rob->type() == threeWheelOmni;

	/* Override this function is a derived class to provide the calculated
	 * velocities to a general point targetPoint, and general angle targetAngle.
	 * Store the results in *this (lfront, lback... etc); but in this function
	 * do not set the velocities on the robot.
	 */
    virtual void calculateVels
        (Robot* rob, Point targetPoint, float targetAngle, Type moveType) = 0;

private:
    Point m_targetPoint      = Point(9999, 9999);
    float m_targetAngle      = UNUSED_ANGLE_VALUE;
    float velMultiplier      = 1.0;
    bool  isInitialized      = false; 
    bool  useObstacleAvoid   = true;

    bool  hasFoundPathEnd    = false;
    bool  currentPathIsClear = false;
    float nextTargetAngle    = UNUSED_ANGLE_VALUE;
    float nextDistTolerance  = 250;
    Point lastObsPoint       = Point(9999, 9999);
    FPPA::PathInfo      pathInfo;
    std::deque<Point>   pathQueue;
    FPPA::PathDirection lastDirection;
    std::vector<Point>  lastObstacles;

    bool calcObstacleAvoidance(Robot* rob, Type moveType);
    bool calcRegularMovement(Robot* rob, Type moveType);
    void assignNewPath(const Point& robotPoint);
    
    void setVels(Robot* robot);
};


}

/*
Movement::Move* myMove;
...
myMove = new Movement::GoToPosition(targetPoint, targetAngle);
...
myMove->setVelocityMultiplier(2);
myMove->perform(robot);
myMove->perform(robot, Movement::Differential::SharpTurns);
*/

#endif