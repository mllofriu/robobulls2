#ifndef DRIVEBALL_H
#define DRIVEBALL_H

#include "skill/skill.h"
#include "utilities/point.h"
#include "model/robot.h"
#include "include/globals.h"

namespace Skill {

    class DriveBall:public Skill
    {
    public:
        DriveBall(Point, double);
        bool perform(Robot*);
    private:
        Point targetPosition;
        double direction;
        enum states {initial, moveBehindBall, moveTowardBall, driveBall, haveTheBall} state;
        Skill * skill;
        Point *behindBall;
        Point *closeToBall;
        Point *goal;

    };
}

#endif // DRIVEBALL_H
