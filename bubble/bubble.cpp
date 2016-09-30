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

}


bubble::~bubble() {}

void bubble::operator<<(cv::Rect newPosition){

    /*Maybe check if the position is viable here  */

    this->KnownDescriptors.push_back(newPosition);



    this->dz.push_back(this->last_x-newPosition.x);

    this->last_x=newPosition.x;
    this->last_y=newPosition.y;


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

    if (fabs(this->last_y-y)<=5){
        if((this->last_x-x)<15) return true;
    }

    return false;
}





