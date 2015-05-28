#include "include/config/team.h"
#include "behavior/behaviorassignment.h"
#include "behavior/genericmovementbehavior.h"
#include "behavior/defendfarfromball.h"
#include "behavior/attackmain.h"
#include "behavior/attacksupport.h"
#include "behavior/defendbehavior.h"
#include "behavior/simplebehaviors.h"
#include "utilities/comparisons.h"
#include "utilities/edges.h"
#include "strategy/normalgamestrategy.h"

bool NormalGameStrategy::isOnAttack = true;

/*************************************************/
/** CONFIGURATION **/
/* Defines the number of calls to considerSwitchCreiteria, in which the
 * switching condition is true, that must be made until the attack/defend
 * switch is actually made
 */
#define ATT_TO_DEF_SWITCH_COUNT 16
#define DEF_TO_ATT_SWITCH_COUNT 128

/* Defines the number of times the ball must be seen outside of the goal
 * to have the robots start moving again. Used to prevent jerkey movement
 * on the field, and causes a delay when the ball *does* come out of the goal.
 */
#define MIN_BALLINGOAL_COUNT 24


/*************************************************/
/** BEHAVIORS **/

/* OpBallBlocker
 * A behavior similar to the late DefendCloseToBall that gets in front
 * of the enemy robot with the ball.
 */
class OpBallBlocker : public GenericMovementBehavior
{
public:
    void perform(Robot *robot)
    {
        Robot* ballBot = gameModel->getHasBall();
        if(ballBot && !ballBot->isOnMyTeam())
        {
            Point bp = gameModel->getBallPoint();
            float Opang = Measurments::angleBetween(ballBot, bp);
            float myAng = Measurments::angleBetween(robot, ballBot);
            Point offset = Point(cos(Opang), sin(Opang)) * 500;
            Point target = bp + offset;
            setMovementTargets(target, myAng);
        } else {
            //Case to stop robot from floating
            setMovementTargets(robot->getRobotPosition());
        }
        GenericMovementBehavior::perform(robot);
    }
};


/* Behavior RetreatAfterGoal
 * Sends the robot to a point at a fixed x and a given y (in constructor)
 * Used to sit still when the ball is in either goal
 */
class RetreatAfterGoal : public GenericMovementBehavior
{
    int myYPos = 0;
public:
    RetreatAfterGoal(int whichYPosition)
        : myYPos(whichYPosition)
        { }
    void perform(Robot* robot) override
    {
        GameModel * gm = GameModel::getModel();
        Point wait_point = Point(gm->getMyGoal().x*0.5, myYPos);
        double wait_orientation = Measurments::angleBetween(robot->getRobotPosition(),gm->getBallPoint());

        setMovementTargets(wait_point, wait_orientation);
        GenericMovementBehavior::perform(robot);
    }
};

/*************************************************/
/** PUBLIC FUNCTIONS **/

bool NormalGameStrategy::hasStoppedForThisKickoff = false;

NormalGameStrategy::NormalGameStrategy()
    : currentMainAttacker(NULL)
    , currentSuppAttacker(NULL)
    , ballOriginalPos(GameModel::getModel()->getBallPoint())
    { }

/* So, here, the only way to get back into NGS is through a kickoff.
 * If StrategyController is deleting this because of a new gameModel
 * state, we make sure the NEXT kickoff is stopped for. This is needed
 * because StrategyController does delete this upon switching from attack
 * and defend, so this seperates those two deletion cases
 */
NormalGameStrategy::~NormalGameStrategy()
{
    if(gameModel->isNewCommand())
        hasStoppedForThisKickoff = false;
}

void NormalGameStrategy::assignBeh()
{
    if(gameModel->getMyTeam().size() >= 3) {
        isOnAttack = considerSwitchCreiteria();
        if(isOnAttack) {
            assignAttackBehaviors();
        } else {
            assignDefendBehaviors();
        }
    }
}


bool NormalGameStrategy::update()
{
    static int ballNotInGoalCount = 0;
    bool oldAttack = isOnAttack;
    GameModel* gm = GameModel::getModel();
    Point ball = gm->getBallPoint();
    Point opGoal = gm->getOpponentGoal();
    Point myGoal = gm->getMyGoal();
    bool ballMoved = !Measurments::isClose(ballOriginalPos, ball);

    //Ball near goals, used as checks to not do anything first
    static bool ballInOpGoal = false;
    static bool ballinMyGoal = false;
    ballInOpGoal = Measurments::isClose(ball, opGoal, 999);
    ballinMyGoal = Measurments::isClose(ball, myGoal, 999);

    isOnAttack = considerSwitchCreiteria();

    /* If the ball is close to opponent goal or the opponent goal,
     * the robots retreat back to their half, and no attack/defend
     * is run. ASSUMPTION: Ball never starts out near goal.
     */
    if(posedge(ballInOpGoal or ballinMyGoal))
        assignGoalKickBehaviors();

    if(ballInOpGoal or ballinMyGoal)
    {
        /* A count is kept to ensure robots don't move until the ball is
         * surely out of the goal
         */
        ballNotInGoalCount = 0;
    }
    else
    {
        /*
         * If we go to normal strategy after our kickoff
         *      we have attack behavior
         * if we go to normal stratgey after opponent kickoff
         *      we wait until opponent players move the ball
         * else
         *      we follow the general normal game strategy
         * */
        char prevGs = gm->getPreviousGameState();

        if(((prevGs == 'K' && TEAM == TEAM_BLUE) ||
            (prevGs == 'k' && TEAM == TEAM_YELLOW)) &&
            !ballMoved)
        {
            assignAttackBehaviors();
        }
        else if(((prevGs == 'k' && TEAM == TEAM_BLUE) ||
                 (prevGs == 'K' && TEAM == TEAM_YELLOW)) &&
                 !ballMoved)
        {
            BehaviorAssignment<SimpleBehaviors> haltAssignment;
            haltAssignment.setSingleAssignment(true);
            haltAssignment.assignBeh();
        }
        else
        {
            //The ball has moved and we have stopped for *this* kickoff.
            hasStoppedForThisKickoff = true;

            if(ballNotInGoalCount < MIN_BALLINGOAL_COUNT) {
                /* This ensures that the ball must be detected out of the goal
                 * a number of times before the robots move. Otherwise, we sould see
                 * very jerkey movement on the field with the ball being falsely
                 * detected outside of the goal
                 */
                ++ballNotInGoalCount;
                return false;
            }
            if(oldAttack != isOnAttack) {
                /* If the attack status has switched, we stop and return true so that
                 * StrategyController can assign this strategy again with the new mode
                 */
                return true;
            }
            else if(isOnAttack) {
                if(currentMainAttacker == NULL) {
                    assignBeh();
                    return false;
                }
                /* If the attacker has kicked (isFinished), it has made a pass or a goal,
                 * so we switch attack/defend behaviors. See assignAttackBehaviors
                 */
                if(currentMainAttacker->getCurrentBeh()->isFinished()) {
                    assignAttackBehaviors(true);
                }
            }
            else {
                /* Bug fix: This is entered if the ball comes out from the goal,
                 * and the robots should directly go into defend. This allows swithcing
                 * between modes to always work.
                 */
                //if(negedge(ballInOpGoal))
                    //assignDefendBehaviors();
            }
        }
        return false;
    }

    return false;
}

//Utility function to not duplicate this code
void NormalGameStrategy::assignGoalieIfOk()
{
    Robot* goalie = gameModel->findMyTeam(5);
    if(goalie)
        goalie->assignBeh<DefendFarFromBall>();
}

//Static function to be used in other places.
//Was added here to deal with simplebehaviors.cpp
void NormalGameStrategy::moveRobotToIdleLine(Robot* robot)
{
    Point wait_point = Point(0, -1500+3000*TEAM);

    if(Comparisons::distance(wait_point).minMyTeam() == robot) {
        //Closest guy to the wait point sits there instead
        robot->assignBeh<GenericMovementBehavior>( wait_point );
    }
    else {
        //Assigns the robot to sit along a line across the field
        int incSize = FIELD_WIDTH / (gameModel->getMyTeam().size() - 1);
        int y = -FIELD_WIDTH/2 + incSize * robot->getID();
        robot->assignBeh<RetreatAfterGoal>(y);
    }
}


/*************************************************/
/** PRIVATE FUNCTIONS **/
/* If the enemy has the ball, switch to defend. If we have the ball,
 * get the robot which now has the ball and go to Attack. The robot
 * that has the ball is driver, the other one is the defender.
 * The switch is only done when we're confident we should switch
 * (when we see the switch condition NORMAL_SWITCH_COUNT times)
 */
bool NormalGameStrategy::considerSwitchCreiteria()
{
    static int switchCounter = 0;

    GameModel* gm = GameModel::getModel();
    Robot* ballRobot = gm->getHasBall();

    if(ballRobot == NULL) {
        if(not(isOnAttack)) {
            ++switchCounter;
            return (switchCounter > DEF_TO_ATT_SWITCH_COUNT);
        }
        return true;
    }
    else if(ballRobot->isOnMyTeam() and not(isOnAttack)) {
        ++switchCounter;
        if(switchCounter > ATT_TO_DEF_SWITCH_COUNT) {
            /* We have seen the ball in our hands for long enough,
             * we will switch to attack. (retrun true)
             */
            switchCounter = 0;
            return true;
        }
    }
    else if(not(ballRobot->isOnMyTeam()) and isOnAttack){
        ++switchCounter;
        if(switchCounter > DEF_TO_ATT_SWITCH_COUNT) {
            /* We have not seen the ball in our hands for long enough,
             * we will switch to defend. (retrun false)
             */
            switchCounter = 0;
            return false;
        }
    }

    return NormalGameStrategy::isOnAttack;
}

/* What TwoVOne did, where two robots are on the other side of the
 * field and drive as far as possible, and at the end of that drive,
 * pass the ball to the other and repeat until halfway to the goal and
 * then kick to the goal.
 * EXPO: The third robot does "OpBallBlocker"
 */
void NormalGameStrategy::assignAttackBehaviors(bool switchSides)
{
    Robot* driverBot, *recvBot, *otherBot;
    findMostValidRobots(gameModel->getBallPoint(), driverBot, recvBot, otherBot);

    //First, makes all robots sit still (for now)
    assignGoalKickBehaviors();

    //Here we get a new attacker, because we are attacking
    currentMainAttacker = driverBot;

    if(switchSides) {
        driverBot->assignBeh<AttackSupport>(recvBot);
          recvBot->assignBeh<AttackMain>(driverBot);
    } else {
        driverBot->assignBeh<AttackMain>(recvBot);
          recvBot->assignBeh<AttackSupport>(driverBot);
    }
    if(otherBot)
     otherBot->assignBeh<OpBallBlocker>();
    assignGoalieIfOk();
}


/* Goalie stays at goal. One robot sits at the mid line ready to change
 * to attack. One other one gets in between the enemy passing robot
 * and enemy receiving robot.
 */
void NormalGameStrategy::assignDefendBehaviors()
{
    //We're using the new and amazing DefendBehavior instead of other stuff
    for(Robot* rob : gameModel->getMyTeam())
        rob->assignBeh<DefendBehavior>();

    //Here we remove the current attacker, because we are defending
    currentMainAttacker = NULL;

    //Usual special case for 5
    assignGoalieIfOk();
}


/* This runs when the ball is near in either penalty area.
 * Goalie is normal, and all robots are made to go to the idle line.
 * And attacker is removed because we are not attacking.
 */
void NormalGameStrategy::assignGoalKickBehaviors()
{
    for(Robot* robot : gameModel->getMyTeam())
        NormalGameStrategy::moveRobotToIdleLine(robot);
    assignGoalieIfOk();
    currentMainAttacker = NULL;
}


/* Utility function that finds three robots `a_out` `b_out` and `c_out` such that:
 * - a_out is closest to the `target`, and b_out and c_out are other unique robots.
 * - None are not the goalie robot
 */
void NormalGameStrategy::findMostValidRobots(Point target, Robot*& a_out, Robot*& b_out, Robot*& c_out)
{
    std::vector<Robot*>& myTeam = gameModel->getMyTeam();
    Robot* a_found = NULL, *b_found = NULL, *c_found = NULL;
	
	//Find robot closest to `target`
    a_found = *Comparisons::distance(target).ignoreID(5).min(myTeam);

    //b_found is another robot
    b_found = Comparisons::idNot(5).ignoreID(a_found).anyMyTeam();

    //c_found is also another robot
    c_found = Comparisons::idNot(5).ignoreIDs({a_found, b_found}).anyMyTeam();

    if(!a_found || !b_found)
        throw std::runtime_error("ERROR: Valid robots not found!");

    a_out = a_found;
    b_out = b_found;
    c_out = c_found;
}
