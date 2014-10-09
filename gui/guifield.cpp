#include "guifield.h"

GuiField::GuiField()
{
//    setFlag(ItemIsMovable); // makes it movable
}

QRectF GuiField::boundingRect() const
{
    return QRectF(0,0,6000,4000);
}

void GuiField::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rec = boundingRect();
    QPen borderPen(QPen(Qt::white, 20, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
    // Lines
    QLineF centerLine(3000,0, 3000,4000);
    // Center circle
    int centerDiam = rec.height() / 4;
    int centerCornerX = (rec.width()/2) - (centerDiam/2);
    int centerCornerY = (rec.height()/2) - (centerDiam/2);
    QRectF centerCircle(centerCornerX,centerCornerY, centerDiam,centerDiam);
    // Penalty areas
    int boxWidth = centerDiam/2;
    int boxHeight = centerDiam*1.3333;
    int BoxY = (rec.height()/2) - (boxHeight/2);
    int yellBoxX = rec.width()-boxWidth;
    QRectF blueBox(0,BoxY, boxWidth,boxHeight);
    QRectF yellBox(yellBoxX,BoxY,boxWidth,boxHeight);
    // Goals
    int goalThick = 40;
    int goalWidth = boxWidth/2.5;
    int goalHeight = boxHeight/1.8;
    int goalY = (rec.height()/2)-(goalHeight/2);
    int blueGoalX = goalWidth*-1;
    QPen goalPen(QPen(Qt::darkGray, goalThick, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
    //int yellGoalX = rec.width();
    QRectF blueGoal(blueGoalX,goalY, goalWidth,goalHeight);
    QRectF yellGoal(rec.width(),goalY, goalWidth,goalHeight);
    // Drawing field
    QBrush plainBrush(Qt::darkGreen, Qt::SolidPattern);
    QBrush crossBrush(Qt::cyan, Qt::SolidPattern);
    QBrush brush(Qt::white, Qt::SolidPattern);
//    if (plain) {
//        painter->fillRect(rec,plainBrush);
//    } else {
//        painter->fillRect(rec,crossBrush);
//    }
    if (colorScheme == "Default") {
        brush = (QBrush(Qt::darkGreen, Qt::SolidPattern));
        painter->setPen(QPen(Qt::white, 30, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        borderPen = (QPen(Qt::white, 50, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        goalPen = (QPen(Qt::lightGray, goalThick, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        painter->fillRect(rec, brush);
    } else if (colorScheme == "Ice Rink") {
            brush = (QBrush(Qt::cyan, Qt::SolidPattern));
            painter->setPen(QPen(Qt::red, 30, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
            borderPen = (QPen(Qt::red, 50, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
            goalPen = (QPen(Qt::darkRed, goalThick, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
            painter->fillRect(rec, brush);

    } else if (colorScheme == "Air Hockey") {
        brush = (QBrush(QColor::fromRgb(0,0,200,255), Qt::SolidPattern));
        painter->setPen(QPen(Qt::white, 30, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        borderPen = (QPen(Qt::white, 50, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        goalPen = (QPen(QColor::fromRgb(56,56,56,255), goalThick, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        painter->fillRect(rec, brush);

    } else if (colorScheme == "Basketball") {
        brush = (QBrush(QColor::fromRgb(205,133,63,255), Qt::SolidPattern));
        painter->setPen(QPen(Qt::white, 30, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        borderPen = (QPen(Qt::white, 50, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        goalPen = (QPen(QColor::fromRgb(152,0,0,255), goalThick, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        painter->fillRect(rec, brush);

    } else if (colorScheme == "White") {
        brush = (QBrush(Qt::white, Qt::SolidPattern));
        painter->setPen(QPen(Qt::darkGray, 30, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        borderPen = (QPen(Qt::darkGray, 50, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        goalPen = (QPen(QColor::fromRgb(32,32,32,255), goalThick, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        painter->fillRect(rec, brush);

    } else if (colorScheme == "Game Boy") {
        brush = (QBrush(QColor::fromRgb(152,251,152,255), Qt::SolidPattern));
        painter->setPen(QPen(QColor::fromRgb(85,107,47,255), 30, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        borderPen = (QPen(QColor::fromRgb(85,107,47,255), 50, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        goalPen = (QPen(QColor::fromRgb(37,57,0,255), goalThick, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        painter->fillRect(rec, brush);

    }
    // Drawing field lines
    //painter->setPen(QPen(Qt::white, 30, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
    painter->drawLine(centerLine);
    painter->drawEllipse(centerCircle);
    painter->drawRect(blueBox);
    painter->drawRect(yellBox);
    // Drawing Grid
    if (grid) {
        painter->setPen(QPen(Qt::black, 5, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        int longDenom = (rec.width()/2) / 100;
        int latiDenom = (rec.height()/2) / 100;
        for (int i=1; i<longDenom; i++) {
            int gridX = i*rec.width()/longDenom;
            QLineF longitude(gridX,0, gridX,4000);
            painter->drawLine(longitude);
        }
        for (int j=1; j<latiDenom; j++) {
            int gridY = j*rec.height()/latiDenom;
            QLineF latitude(0,gridY, 6000,gridY);
            painter->drawLine(latitude);
        }
    }
    // Drawing goals
    QBrush goalBrush(Qt::white, Qt::DiagCrossPattern);
    painter->fillRect(blueGoal,goalBrush);
    painter->fillRect(yellGoal,goalBrush);
    if (coloredGoals) {
        painter->setPen(QPen(Qt::blue, goalThick, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        painter->drawRect(blueGoal);
        painter->setPen(QPen(Qt::yellow, goalThick, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        painter->drawRect(yellGoal);
    } else {
        painter->setPen(goalPen);
        painter->drawRect(blueGoal);
        painter->drawRect(yellGoal);
    }

    painter->setPen(borderPen);
    painter->drawRect(rec);
}