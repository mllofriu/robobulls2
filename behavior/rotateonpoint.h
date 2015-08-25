#ifndef ROTATEONPOINT_H
#define ROTATEONPOINT_H

#include "behavior/genericmovementbehavior.h"

/*! @brief Behavior to rotate around a stationary point
 * @author James W
 * @details This behavior is omni-only. It allows a robot to approach a point
 * from any angle, and rotate on a circle, facing the center,
 * to face a final direction of `faceAngle`. Additionally, the `dist`
 * argument specifies the dist to stay from `where` when rotating.
 * isFinished() returns true when the robot is near the target point
 * on the circle. */

class RotateOnPoint : public GenericMovementBehavior
{
public:
    /*! @brief Constructor
     * @param center Point to rotate around
     * @param faceAngle and angle to face when done rotating
     * @param dist The Distance to stay away from the point while rotating */
    RotateOnPoint(Point center, float faceAngle, float dist);

    void perform(Robot *robot) override;
    bool isFinished() override;

private:
    double difang_sign(double x, double y);

    enum {init, closing, rotating} state;
    Point center_point; //!<Point to rotate around
    float center_ang;   //!<Angle to face to around point
    float center_dist;  //!<Radius from center_point to rotate around
    Point offset_point; //!<Initial point on circle to initially go to
    float offset_ang;   //!<Initial angle between center_point and offset_point
    Point final_point;  //!<Point where the robot should end up on the circle
    bool  finished;     //!<Finished?
    int   sign;         //!<Sign, 1 or -1, to rotate in the shorter direction
};

#endif // ROTATEONPOINT_H
