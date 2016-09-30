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


bubble::bubble(cv::Rect bubbleGenesisRect){

    /*The first bubble in inserted */
    this->KnownDescriptors.push_back(bubbleGenesisRect);

    this->last_x=bubbleGenesisRect.x;
    this->last_y=bubbleGenesisRect.y;

    this->GenesisPosition = bubbleGenesisRect;

    //printf("Checkpoint 1\n");
    this->lockThisIteration = true;

}


bubble::~bubble() {}

void bubble::operator<<(cv::Rect newPosition){

    /*Maybe check if the position is viable here  */



    if (!this->lockThisIteration){

        this->KnownDescriptors.push_back(newPosition);
        this->dz.push_back(this->last_x-newPosition.x);

        this->last_x=newPosition.x;
        this->last_y=newPosition.y;

        this->lockThisIteration=true;
    }


}

void bubble::printAllXY(void){

    for (int i=0; i<this->KnownDescriptors.size(); i++){
        std::cout<<"X: "<<this->KnownDescriptors[i].x<<" ";
    }
    std::cout<<"\n";
    for (int i=0; i<this->KnownDescriptors.size(); i++){
        std::cout<<"Y: "<<this->KnownDescriptors[i].y<<" ";
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
    float total_z=this->KnownDescriptors[0].x-this->KnownDescriptors[numFrames-1].x;
    return total_z/((float)numFrames-1.0);

}

float bubble::dRdT(void){

    int numFrames=this->KnownDescriptors.size();
    float dx=this->KnownDescriptors[0].width-this->KnownDescriptors[numFrames-1].width;
    float dy=this->KnownDescriptors[0].height-this->KnownDescriptors[numFrames-1].height;

    float dr = sqrt(dx*dx+dy*dy);
    return dr/((float)numFrames-1.0);
}


