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
        rx = (-500.0+(rand() % 1000))/250.0;
        ry = (-500.0+(rand() % 1000))/250.0;
        position.x=rx+xi;
        position.y=ry+yj;
        motes.push_back(Mote(c,position));
        // One of each 7 motes is a router
        if (c % 7 == 0) motes.at(c).setRouter(true);
    }
    motes.at(baseMoteId).setRouter(true);       // baseMote
    /*for (unsigned int i=0; i<motes.size(); i++) {
        cout << motes.at(i).batteryPerCent() << " " << motes.at(i).getRouter() << endl;
    }*/
    // Assign coefHarvesting
    default_random_engine generator;
    normal_distribution<float> distribution(0.5,0.3);
    float value;
    for (unsigned int i=0; i<motes.size(); i++){
        value=distribution(generator);
        if (value>=0 && value<=1) motes.at(i).setCoefHarvesting(value);
        else if (value<0) motes.at(i).setCoefHarvesting(0);
        else if (value>1) motes.at(i).setCoefHarvesting(1);
    }


}

void recharge(){
    for (unsigned int i=0; i<motes.size(); i++) {
        motes.at(i).addBattery(motes.at(i).getCoefHarvesting()*maxCurrentRecharge*coef*timeInterval);
    }
}

int getIndex(int id){
    int index=-1;
    while (motes.at(++index).getId()!=id);
    return index;
}

float Mote::getDistanceWith(int id2){
    position_t pos=motes.at(getIndex(id2)).getPosition();
    return sqrt((position.x-pos.x)*(position.x-pos.x)+(position.y-pos.y)*(position.y-pos.y));

}

void Mote::calculateNeighbors(){

    for (unsigned int j=0; j<motes.size(); j++) {
        int id2=motes.at(j).getId();
        if (id!=id2 && this->getDistanceWith(id2)<radioRange){
            this->addNeighbor(id2,motes.at(j).getRouter());
        }
    }

}

void recalculateNeighborTable(){
    for (unsigned int i=0; i<motes.size(); i++) {
        if (motes.at(i).getNeighbors().size()) motes.at(i).deleteNeighbors();
        motes.at(i).calculateNeighbors();
    }
}

void ClLUTcal(){
    float dist = 0;
    float m=0.8/radioRange;
    short value =0;
    for (int i=0;i<numberOfMotes;i++) for (int j=0;j<numberOfMotes;j++){
        position_t pos1=motes.at(getIndex(i)).getPosition();
        position_t pos2=motes.at(getIndex(j)).getPosition();
        dist = sqrt((pos1.x-pos2.x)*(pos1.x-pos2.x)+(pos1.y-pos2.y)*(pos1.y-pos2.y));
        float pl=1.0-m*dist+(rand()%100-50)/500;
        if (pl>1)pl=1;


        if (pl>0) {
            value = round(1/pow(pl,4));
            if (value>=7) ClLUT[i][j]=6;
            else ClLUT[i][j]=value;
        }
    }

}
