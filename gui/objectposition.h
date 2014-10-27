#ifndef OBJECTPOSITION_H
#define OBJECTPOSITION_H

#include <QString>
#include <QLabel>
#include "mainwindow.h"
#include "utilities/point.h"

class ObjectPosition
{
public:
    ObjectPosition(MainWindow * mw);
    MainWindow * dash;
    int teamSize = 6;
    int threadTicker = 0;
    Point pastPos;
    Point pastBotPoints[6];
    Point newBotPoints[6];
    void setupPastBotPoints();
    void getPastBotPoints();
    void getNewBotPoints();
    int botSpeeds[6];
    int oldSpeeds[6];
    void getBotSpeeds();
    void getOldSpeeds();
//    void getThreadTicker(int tick);
    QString getBotCoord(int id);        // Returns the specified robot's x/y position as a QString
    QString getBotOrientString(int id);
    double  getBotOrientDouble(bool myTeam, int id);
    int getBotCoordX(bool myTeam, int id);
    int getBotCoordY(bool myTeam,int id);
    float     getBotSpeed(bool myTeam, int id);
    QString getBallCoord();
    int  getBallCoordX();
    int  getBallCoordY();
    // Mouse stuff
    int getMouseCoordX();
    int getMouseCoordY();

};

#endif // OBJECTPOSITION_H
