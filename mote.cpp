#include "mote.h"

// No arguments
Mote::Mote() {
    battery=batteryCapacity;
}
// Just battery
Mote::Mote(float newBattery) {
    battery=newBattery;
}
// Just position
Mote::Mote(float newX, float newY){
    battery=batteryCapacity;
    position.x=newX;
    position.y=newY;
}
// Just position passing a position struct
Mote::Mote(int newId,position_t newPosition){
    battery=batteryCapacity;
    id=newId;
    position.x=newPosition.x;
    position.y=newPosition.y;
}

// Everything
Mote::Mote(float newBattery, float newX, float newY){
    battery=newBattery;
    position.x=newX;
    position.y=newY;
}

float Mote::batteryPerCent() { return battery/batteryCapacity*100; }


void Mote::addNeighbor(int id, int type){
    int *data=new int[2];
    *data=id;
    *(data+1)=type;
    this->neighbors.push_back(data);
}

void Mote::deleteNeighbors(){
    for (unsigned int i = 0; i<neighbors.size(); i++) delete [] neighbors.at(i);
    neighbors.clear();
}

































