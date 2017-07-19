/* The objective of this localizer is to shrink the image down
 * by some method and then get a "rough idea" as to where the bubble should be
 *
 * Input: CV::mat with image
 * Output: ROI pairs, 2 of them - bubble and mirror
 *
 *
 * Why this method: Instead of a wild goose chase around the entire frame
 * which never worked (See testbed folder with all the "detectbub.cpp" files), I
 * am convinced that that is the wrong way to go and wont work. If we can hone in
 * on where the bubble is, we dont have to use aggressive filters and may have
 * a better chance on "seeing" the bubble.
 *
 * Note: I am trying to use the Google C++ Style Guide. Follow it please.
 */



#include "opencv2/opencv.hpp"
#include "bubble.hpp"
#include <iostream>
#include <math.h>


/* ******************************************************************************
 * This function is step 1 to the problem. The lower bubble is gonna be
 * much bigger than the upper one. This will allow us to get an initial guess
 * on the position of the bubble and then use aggressive techniques for the top bubble.
 *
 * Not going by the mistakes earlier, this will be OO
 * ******************************************************************************/


bubble::bubble(BubbleImageFrame bubbleGenesisStructure){

    /*The first bubble in inserted */
    this->KnownDescriptors.push_back(bubbleGenesisStructure);

    this->last_x=bubbleGenesisStructure.MassCentres.x;
    this->last_y=bubbleGenesisStructure.MassCentres.y;
    this->GenesisPositionCentroid = bubbleGenesisStructure.MassCentres;

    this->GenesisPosition = bubbleGenesisStructure.newPosition;

    //printf("Checkpoint 1\n");
    this->lockThisIteration = true;

}


bubble::~bubble() {}


void bubble::operator<<(BubbleImageFrame newPosImgFrameStructure){

    /*Maybe check if the position is viable here  */

    //std::cout<<"Bubble added with RR cen X"<<newPosImgFrameStructure.newPosition.x<<" y "<<newPosImgFrameStructure.newPosition.y<<"\n";

    if (!this->lockThisIteration){

        this->KnownDescriptors.push_back(newPosImgFrameStructure);
        this->dz.push_back(this->last_x-newPosImgFrameStructure.newPosition.x);

        this->last_x=newPosImgFrameStructure.MassCentres.x;
        this->last_y=newPosImgFrameStructure.MassCentres.y;

        this->lockThisIteration=true;
    }


}




void bubble::printAllXY(void){

    for (int i=0; i<this->KnownDescriptors.size(); i++){
        std::cout<<"X: "<<this->KnownDescriptors[i].MassCentres.x<<" ";
    }
    std::cout<<"\n";
    for (int i=0; i<this->KnownDescriptors.size(); i++){
        std::cout<<"Y: "<<this->KnownDescriptors[i].MassCentres.y<<" ";
    }
    std::cout<<"\n";

}

bool bubble::isNewPositionProbable(int &x, int &y){

    if (fabs(this->last_y-y)<=4){
        if((this->last_x-x)<5) return true;
    }

    return false;
}



float bubble::dZdT (void){

    int numFrames=this->KnownDescriptors.size();
    float total_z;
    total_z=this->KnownDescriptors[0].newPosition.x-this->KnownDescriptors[numFrames-1].newPosition.x;
    return total_z/((float)numFrames-1.0);

}

float bubble::dRdT(void){

    int numFrames=this->KnownDescriptors.size();
    float dx=this->KnownDescriptors[0].newPosition.width-this->KnownDescriptors[numFrames-1].newPosition.width;
    float dy=this->KnownDescriptors[0].newPosition.height-this->KnownDescriptors[numFrames-1].newPosition.height;

    float dr = sqrt(dx*dx+dy*dy);
    return dr/((float)numFrames-1.0);
}


