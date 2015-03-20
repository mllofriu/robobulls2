#include <math.h>
#include <algorithm>
#include <utility>
#include <iostream>
#include "include/config/tolerances.h"
#include "utilities/measurments.h"
#include "utilities/comparisons.h"
#include "model/gamemodel.h"
#include "movement/pathfinding/fppa_pathfinding.h"

/* FPPA Pathfinding Constants */
#define FPPA_DEBUG 0
#define MAX_RECURSION_DEPTH 3
#define FRAME_UPDATE_COUNT 10

/* Implementation of the Fast Path Planning Algorithm
 * In a sense, this is a mostly a generalized implementation.
 * But it is currently hardcoded to work with robobulls only.
 * JamesW.
 */

namespace FPPA
{
namespace impl
{
    const Point fieldTopLeft = Point(-3000, -2000);
    const Point fieldBotRight = Point(3000, 2000);
    int framesUntilUpdate = 0;
    std::vector<Point> currentFrameObstacles;

    static bool insideRadiusRectangle(const Point& p0, const Point& p1, const Point& p2)
    {
        float rectTop = std::min(p1.y, p2.y) - ROBOT_RADIUS;
        float rectBottom = std::max(p1.y, p2.y) + ROBOT_RADIUS;
        float rectLeft = std::min(p1.x, p2.x) - ROBOT_RADIUS;
        float rectRight = std::max(p1.x, p2.x) + ROBOT_RADIUS;

        return (p0.x > rectLeft && p0.x < rectRight)
            && (p0.y > rectTop  && p0.y < rectBottom);
    }

    /*********************************************************/

    static std::pair<bool, Point>
    isObstacleinLine(const Point& beginPos, const Point& endPos, bool avoidBall)
    {
        bool  obstacle_found = false;
        Point obstacle_position;
        /* As much as I'd hate doing this, it's needed for now. */
        GameModel* gm = GameModel::getModel();

        for(const Point& pt : currentFrameObstacles)
        {
            /* With the new architecture, we need to manually exclude points that are
             * close to the start or ending point in this function.
             * Also... now it _always_ avoids the ball, even if it is close to the start/end
             * when avoidBall is on
             */
            if(Measurments::isClose(pt, beginPos, ROBOT_RADIUS)
               //Measurments::isClose(pt, endPos, ROBOT_RADIUS)
                    )
            {
                if(Measurments::isClose(gm->getBallPoint(), pt, 80)) {
                    if(not(avoidBall))
                        continue;
                } else {
                    continue;
                }
            }

            obstacle_found = Measurments::lineDistance(pt, beginPos, endPos) < ROBOT_SIZE*0.75 &&
                insideRadiusRectangle(pt, beginPos, endPos);

            if(obstacle_found) {
                obstacle_position = pt;
                break;
            }
        }

    #if FPPA_DEBUG
        //printf("[FPPA] Checking Line (%f,%f)-(%f,%f)\n", beginPos.x, beginPos.y, endPos.x, endPos.y);
        if(obstacle_found)
            printf("[FPPA] Found: %.0f,%.0f\n", obstacle_position.x, obstacle_position.y);
    #endif

        return std::pair<bool, Point>(obstacle_found, obstacle_position);
    }


    static bool isObstacleAtPoint(const Point& toCheck)
    {
        return Comparisons::isDistanceToLess(toCheck, ROBOT_SIZE).any_of(currentFrameObstacles);
    }

    /*********************************************************/

    static Point findSearchDirection(const Point& initialPos, const Point& endPos, int sign)
    {
        float theta = Measurments::angleBetween(initialPos, endPos);

        /* Reducing the search size by a factor can help eliminate
         * the jagged edges in the path, but risks cutting corners
         * too close around obstacles.
         */
        float dx = 0.25 * ROBOT_RADIUS * cos(theta + M_PI_2);
        float dy = 0.25 * ROBOT_RADIUS * sin(theta + M_PI_2);

        return Point(sign * dx, sign * dy);
    }

    /*********************************************************/

    static void buildPathimpl(Path* results, const Point& beginPos,
                              const Point& endPos,int sign, int depth, bool avoidBall)
    {
        if(depth >= MAX_RECURSION_DEPTH)
            return;

        auto obstacle_info = isObstacleinLine(beginPos, endPos, avoidBall);

        if(obstacle_info.first == true)  //Obstacle was found
        {
            Point deltaPoint = findSearchDirection(beginPos, endPos, sign);
            Point subGoal;
            float i = 1;

            while(obstacle_info.first && (i < 10)) {
                subGoal = obstacle_info.second + Point(i * deltaPoint.x,  i * deltaPoint.y);
                obstacle_info.first = isObstacleAtPoint(subGoal);
                i += 0.5;
            }

            /* Find the ending point and insert the new point before it
             * (Possibly update this knowing the pos of the point)
             */
            auto it = std::find(results->begin(), results->end(), endPos);
            results->insert(it, subGoal);

            /* Recursively test the paths from the beginning to the subGoal and from
             * the subGoal to the end
             */
            buildPathimpl(results, beginPos, subGoal, sign, depth + 1, avoidBall);
            buildPathimpl(results, subGoal,  endPos, sign, depth + 1, avoidBall);
        }
    }

    /*********************************************************/

    /* This is the function that communicates outside the FPPA namespace to define
     * obstacle information. In our case, that is getting the GameModel and all the robots
     * from that. However, I would like to have this more generalized in the future.
     */
    static void buildObstacleCollection()
    {
        GameModel* mod = GameModel::getModel();
        const auto& myTeam = mod->getMyTeam();
        const auto& opTeam = mod->getOponentTeam();

        currentFrameObstacles.clear();
        currentFrameObstacles.reserve(myTeam.size() + opTeam.size() + 1);

        for(Robot* rob : myTeam)
            currentFrameObstacles.push_back(rob->getRobotPosition());
        for(Robot* rob : opTeam)
            currentFrameObstacles.push_back(rob->getRobotPosition());

    #if FPPA_DEBUG
           // std::cout << "[FPPA] All Obstacles: " << std::endl;
            //for(Point pt : *obstacles) std::cout << pt.toString() << std::endl;
    #endif
    }

    /*********************************************************/

    static std::pair<Path, Path>
    findBothPaths(const Point& start, const Point& end, bool avoidBall)
    {
        Path topPath, bottomPath;

        topPath.push_back(start);
        topPath.push_back(end);
        bottomPath.push_back(start);
        bottomPath.push_back(end);

        impl::buildPathimpl(&topPath, start, end, -1, 0, avoidBall);
        impl::buildPathimpl(&bottomPath, start, end,  1, 0, avoidBall);

        return std::move(std::make_pair(std::move(topPath), std::move(bottomPath)));
    }

    /*********************************************************/

    static bool isValidPath(const Path& p)
    {
        return std::none_of(p.begin(), p.end(), Comparisons::isPointOutsideField());
    }

    static void sanitizePoint(Point& pt)
    {
        pt.x = Measurments::clamp(pt.x, -2900.0f, 2900.0f);
        pt.y = Measurments::clamp(pt.y, -1900.0f, 1900.0f);
    }

    static float getPathLength(const Path& p)
    {
        float totalDist = 0;
        for(auto it = p.begin(); it != p.end()-1; ++it)
            totalDist += Measurments::distance(*it, *(it+1));
        return totalDist;
    }

} //namespace impl


    void pathfindingBegin()
    {
        if(impl::framesUntilUpdate == 0) {
            impl::buildObstacleCollection();
            impl::framesUntilUpdate = FRAME_UPDATE_COUNT;
        }
    }


    void pathfindingEnd()
    {
        --impl::framesUntilUpdate;
    }


    PathInfo findShortestPath(const Point& start, const Point& end, bool avoidBall,
                              PathDirection pathHint, float unlessValue)
    {
        if(avoidBall)
            impl::currentFrameObstacles.push_back(GameModel::getModel()->getBallPoint());

        std::pair<Path, Path> foundPaths = impl::findBothPaths(start, end, avoidBall);

        if(avoidBall)
            impl::currentFrameObstacles.pop_back();

        //Create PathInfos: pair  of {points vector, direction top/bottom}
        //Get distance and valid status of each
        PathInfo topPathInfo = std::make_pair(foundPaths.first, PathDirection::Top);
        PathInfo botPathInfo = std::make_pair(foundPaths.second, PathDirection::Bottom);
        bool topValidStatus  = impl::isValidPath(topPathInfo.first);
        bool botValidStatus  = impl::isValidPath(botPathInfo.first);
        float topTotalDist   = impl::getPathLength(topPathInfo.first);
        float botTotalDist   = impl::getPathLength(botPathInfo.first);

        //Top path is valid and bottom isn't; defaut top
        if(topValidStatus and not(botValidStatus)) {
            return topPathInfo;
        }

        //Bottom path is valid and top isn't
        if(botValidStatus and not(topValidStatus)) {
            return botPathInfo;
        }

        //User requested top or bottom path explicitly
        if(pathHint != PathDirection::None) {
            PathInfo* chosenPathInfo = &topPathInfo;
            PathInfo* otherPathInfo  = &botPathInfo;
            float selectedDist = topTotalDist;
            float otherDist = botTotalDist;

            if(pathHint == PathDirection::Bottom) {
                chosenPathInfo = &botPathInfo;
                otherPathInfo  = &topPathInfo;
                selectedDist = botTotalDist;
                otherDist = topTotalDist;
            }

            if(unlessValue != -1.0) {
                //The other path is at least "unlessValue" percent shorter
                if(selectedDist - (selectedDist * unlessValue) > otherDist)
                    return *otherPathInfo;
            }
            return *chosenPathInfo;
         }

        //Otherwise, selects PathInfo top or bottom strictly based on best distance.
        //Sanitizes Points in case both are invalid.
        PathInfo& chosenPathInfo = topPathInfo;
        if(botTotalDist < topTotalDist)
            chosenPathInfo = botPathInfo;
        Path& chosenPath = chosenPathInfo.first;

        //Limit all points in the "chosen" path to being inside the field
        std::for_each(chosenPath.begin(), chosenPath.end(), impl::sanitizePoint);

        return chosenPathInfo;
    }


    bool isObstacleInLine(const Point& start, const Point& end, Point* obsPosOut, bool avoidBall)
    {
        if(avoidBall)
            impl::currentFrameObstacles.push_back(GameModel::getModel()->getBallPoint());

        auto obstacle_info = impl::isObstacleinLine(start, end, avoidBall);

        if(avoidBall)
            impl::currentFrameObstacles.pop_back();

        if(obstacle_info.first) {
            if(obsPosOut != nullptr) *obsPosOut = obstacle_info.second;
            return Measurments::isClose(start, obstacle_info.second, 2500);
        } else {
            return false;
        }
    }


    bool isPointInLine(const Point& start, const Point& end, const Point& question)
    {
        return Measurments::lineDistance(question, start, end) < ROBOT_SIZE &&
               impl::insideRadiusRectangle(question, start, end);
    }


    const std::vector<Point>& getCurrentObstacles()
    {
        return impl::currentFrameObstacles;
    }

} //namespace FPPA
