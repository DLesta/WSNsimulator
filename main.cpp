#include <iostream>
#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <unistd.h>
#include <iomanip>
#include <fstream>
#include <vector>
#include <math.h>
#include <random>
#include <mote.h>
using namespace std;

// Constants
static const int numberOfMotes = 100; // Only perfect squares please
static const int distanceBetweenMotes = 50; // m
static const float maxCurrentRecharge = 100; // mA
float coef = 0.01; // Coefficient maxCurrent-actualCurrent
float radioRange = 120; // m
float timeInterval = 2; // h
static const int baseMoteId=45;
static const float batteryConsumptionInSleep = 0.2; //mA
static const float batteryConsumptionInOperation = 4.1; //mA
static const float batteryConsumptionInFormingNetwork = 150; //mA
static const float batteryConsumptionInTransmission = 250; //mA
static const int routerProportion=5; // One router every routerProportion motes
static const float transmissionTime=0.0006; //hours

// Global variables
vector<Mote> motes;
short ClLUT[numberOfMotes][numberOfMotes];
int costPrueba=15000000;
vector<int> swap;
unsigned int aliveMotes=numberOfMotes;
unsigned int elapsedTime;
unsigned int totalTimeOfSimulation=24*365*5;
bool baseIsolated=false;
//Functions
void motesDeployment();
void recharge();
void recalculateNeighborTable();
void ClLUTcal();
void calulatePaths();
void subtractBatteryToAll(float value);
bool notIsolated(int id2);
int getIndex(int id);
float sim(float newcoeff);


int main()
{
    sim(0.05);
    return 0;
}


float sim(float newcoeff){

    coef=newcoeff;

    // Reinit of variables
    motes.clear();
    baseIsolated=false;
    aliveMotes=numberOfMotes;
    costPrueba=15000000;
    float D=0.3/3; //Duty cycle
    float timeFormingNetwork=20.0/3600;
    motesDeployment();
    motes.at(baseMoteId).setBattery(999999999); // Base can not die. WHAT IS DEAD MAY NEVER DIE

    ClLUTcal();
    calulatePaths();
    //Simulation
    for (elapsedTime=0; elapsedTime<totalTimeOfSimulation && motes.size()>10 && !baseIsolated; elapsedTime+=timeInterval){
        // Energy recharge
        recharge();
        // Sleep and op energy consumption
        subtractBatteryToAll((batteryConsumptionInSleep*(1.0-D) + batteryConsumptionInOperation*D)*timeInterval);
        // Path discovery (ZIGBEE)
        if (motes.size()<aliveMotes){
            calulatePaths();
            aliveMotes=motes.size();

        }
        if (!baseIsolated && motes.size()>0){
            // Alive data transmission
            subtractBatteryToAll(batteryConsumptionInFormingNetwork*timeFormingNetwork);
            // Sensing data transmission
            // Two random motes send information every cycle
            for (int i=0; i<2; i++) {
                int a=rand();
                int b=motes.size();
                int idr=a%b;
                if (motes.at(idr).getRouter()){
                    vector<int> pathr = motes.at(idr).getPath();
                    for (unsigned int i=0; i<pathr.size(); i++) if (notIsolated(pathr.at(i)))
                        motes.at(getIndex(pathr.at(i))).addBattery(-transmissionTime*batteryConsumptionInTransmission);
                }
            }
            // Delete dead motes
            for (unsigned int i=0; i<motes.size(); i++) {
                if (motes.at(i).getBattery()<0){
                    motes.erase(motes.begin()+i--);
                }
            }

        } else if( motes.size()==0) baseIsolated=true;
    }
    return ((float)elapsedTime);
}


void motesDeployment(){
    // Number of motes per line
    int s=sqrt(numberOfMotes);
    // Maximum distance from the origin at one axis
    int xyM=s*distanceBetweenMotes;
    // Seed generation
    srand(time(NULL));
    // Variables needed for the deployment
    float rx,ry;
    position_t position;
    // Creation and positioning the motes
    for (int xi=0,c=0; xi<xyM; xi+=distanceBetweenMotes) for (int yj=0; yj<xyM; yj+=distanceBetweenMotes,c++) {
        rx = (-500.0+(rand() % 1000))/50.0;
        ry = (-500.0+(rand() % 1000))/50.0;
        position.x=rx+xi;
        position.y=ry+yj;
        motes.push_back(Mote(c,position));
        // One of each routerProportion motes is a router
        if (rand() % routerProportion == 0) motes.at(c).setRouter(true);
    }
    motes.at(baseMoteId).setRouter(true);       // baseMote

    // Assign coefHarvesting
    default_random_engine generator;
    normal_distribution<float> distribution(0.7,0.3);
    float value;
    for (unsigned int i=0; i<motes.size(); i++){
        value=distribution(generator);
        if (value>=0 && value<=1) motes.at(i).setCoefHarvesting(value);
        else if (value<0.1) motes.at(i).setCoefHarvesting(0.1);
        else if (value>1) motes.at(i).setCoefHarvesting(1);
    }


}

void recharge(){
    if((elapsedTime%24)/12>0){
        float coeffRandom;
        float value;
        for (unsigned int i=0; i<motes.size(); i++) {
            coeffRandom=((float)(rand()%1000)+500.0)/1000;
            value=motes.at(i).getCoefHarvesting()*maxCurrentRecharge*coef*timeInterval*coeffRandom;
            if (motes.at(i).getBattery()+value<batteryCapacity || motes.at(i).getId()==baseMoteId) motes.at(i).addBattery(value);
            else (motes.at(i).setBattery(batteryCapacity));
        }
    }
}

int getIndex(int id){
    int index=-1;
    while (++index < motes.size() && motes.at(index).getId()!=id);
    if (index!=motes.size()) return index;
    else return -1;
}

float Mote::getDistanceWith(int id2){
    position_t pos=motes.at(getIndex(id2)).getPosition();
    return sqrt((position.x-pos.x)*(position.x-pos.x)+(position.y-pos.y)*(position.y-pos.y));

}

void Mote::calculateNeighbors(){
    for (unsigned int j=0; j<motes.size(); j++) {
        int id2=motes.at(j).getId();
        // float distancee=this->getDistanceWith(id2);
        if (id!=id2 && this->getDistanceWith(id2)<radioRange){
            this->addNeighbor(id2,motes.at(j).getRouter());
        }
    }

    // If it has no neighbors it is an isolated mote
    if (this->getNeighbors().empty()) this->addNeighbor(-1,0);
    else{
        unsigned int i=0;
        while (i<this->getNeighbors().size() && *(this->getNeighbors().at(i++)+1)<1);
        // If i is equal to size only the last one could be the router, so it is
        // rechecked. Else, it has a router in its range and it is no isolated
        if (i==this->getNeighbors().size() && *(this->getNeighbors().at(--i)+1)==0) {
            this->deleteNeighbors();
            this->addNeighbor(-1,0);
        }
    }
}

void recalculateNeighborTable(){
    for (unsigned int i=0; i<motes.size(); i++) {

        if (motes.at(i).getNeighbors().size()) motes.at(i).deleteNeighbors();
        motes.at(i).calculateNeighbors();
        //Mote m=motes.at(i);
        if (*(motes.at(i).getNeighbors().at(0))<0){
            if (motes.at(i).getId()==baseMoteId){
                //vector<Mote> a =motes;
                baseIsolated=true;
            }
            else{
                //cout<<motes.at(i).getId()<<" isolated deleted "<<endl;
                motes.erase(motes.begin()+i);
                i--;
            }
        }
    }
}

void ClLUTcal(){
    float dist = 0;
    float m=0.8/radioRange;
    int value =0;
    for (int i=0;i<numberOfMotes;i++) for (int j=0;j<numberOfMotes;j++){
        position_t pos1=motes.at(getIndex(i)).getPosition();
        position_t pos2=motes.at(getIndex(j)).getPosition();
        dist = sqrt((pos1.x-pos2.x)*(pos1.x-pos2.x)+(pos1.y-pos2.y)*(pos1.y-pos2.y));
        float pl=1.0-m*dist+(rand()%100-50)/500;
        if (pl>1)pl=1;
        else if(pl<=0.1) pl=0.1;
        if (pl>0) {
            value = round(1/pow(pl,4));
            if (value>=7) ClLUT[i][j]=6;
            else ClLUT[i][j]=value;
        } else if(pl<=0)ClLUT[i][j]=6;
    }

}

int Mote::calculateNextHop(int *previousPath, int len, int acumulatedCost){
    if (router){
        for (unsigned int i=0; i<neighbors.size(); i++){

            int id2=*(neighbors.at(i));
            bool router=*(neighbors.at(i)+1);
            bool flag=true;
            for (int ii=0; ii<len; ii++){
                if (previousPath[ii]==id) flag=false;
            }

            // Backward avoided
            if (flag){
                if (id2==baseMoteId){
                    // Base reached
                    if ((acumulatedCost + ClLUT[id][id2])<costPrueba) {
                        costPrueba=acumulatedCost + ClLUT[id][id2];
                        swap.clear();
                        for (int k=0; k<len; k++){
                            swap.push_back(previousPath[k]);
                        }
                        swap.push_back(id);

                    }
                    return 1;
                }
                else if (router){
                    int newpath[len+1];

                    for (int k=0; k<len; k++) newpath[k]=previousPath[k];
                    newpath[len]=id;
                    motes.at(getIndex(id2)).calculateNextHop(newpath,len+1,ClLUT[id][id2]+acumulatedCost);

                }
            }
        }
    } else {
        swap.clear();
        float dist=999999999;
        int8_t p=-1;
        for (unsigned int i=0; i<neighbors.size(); i++){
            int id2=*(neighbors.at(i));
            bool router2=*(neighbors.at(i)+1);

            if (notIsolated(id2) && router2 && this->getDistanceWith(id2)<dist ){
                p=id2;
            }
        }

        if(p>0) swap.push_back(p);
        else return-1;
    }

    return 1;
}

void calulatePaths(){
    recalculateNeighborTable();
    int result;
    for (unsigned int i=0; i<motes.size() && !baseIsolated; i++){
        costPrueba=15000000;
        result=motes.at(i).calculateNextHop(NULL,0,0);
        if(result>0) motes.at(i).setPath(swap);
        else{
            motes.erase(motes.begin()+i);
            --i;
        }

    }
}

void subtractBatteryToAll(float value){
    for (unsigned int i = 0; i<motes.size(); i++) motes.at(i).addBattery(-value);
}

bool notIsolated(int id2){
    unsigned int i=0;
    while(motes.at(i++).getId()!=id2 && i<motes.size());
    if(i<motes.size() || (i==motes.size() && motes.at(--i).getId()==id2)) return true;
    else return false;
}






























