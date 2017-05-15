#include <vector>
#ifndef MOTE_H
#define MOTE_H

using namespace std;
static const float batteryCapacity = 2000; // mAh

struct position_t {
    float x,y;
};


class Mote {
    int id;
    float battery;
    float coefHarvesting;
    bool router=false;
    vector<int> path;
    vector<int*> neighbors;
    position_t position;



public:
    //------------------------------CONSTRUCTORS---------------------------------------------------------
    // No arguments
    Mote();
    // Just battery
    Mote(float newBattery);
    // Just position
    Mote(float newX, float newY);
    // Just positions passing a position structure
    Mote(int newId,position_t newPosition);
    // Everything
    Mote(float newBattery, float newX, float newY);

    //-----------------------------Setters and Getters----------------------------------------------------

    int getId() {return id;}
    float getBattery() {return battery;}
    void setBattery(float value) {battery=value;}
    void setCoefHarvesting(float newCoef) {coefHarvesting = newCoef;}
    float getCoefHarvesting() {return coefHarvesting;}
    void setRouter(bool value) {router=value;}
    bool getRouter() {return router;}
    vector<int> getPath(){return path;}
    void setPath(vector<int> newPath) {path=newPath;}
    position_t getPosition() { return position;}
    vector<int*> getNeighbors() {return neighbors;}

    //-----------------------------Functions--------------------------------------------------------------

    // Function to get the remain battery in per cent mode
    float batteryPerCent();

    float addBattery(float value){
        battery+=value;
        return battery;
    }
    void addNeighbor(int id, int type);

    void calculateNeighbors();

    void deleteNeighbors();

    int calculateNextHop(int *previousPath, int len, int acumulatedCost);

    float getDistanceWith(int id2);
};

#endif // MOTE_H
