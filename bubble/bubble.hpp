/* *****************************************************************************
 * This file contains the necessary functions for ImageEntropy
 * ALGORITHM given an image, expect a number which is the ImageEntropy.
 *
 * by Pitam Mitra for the PICO Geyser ImageAnalysis Algorithm.
 *
 * Created: 30 Jan 2014
 *
 * Issues: We still dont know how to handle colour images with 3 channel_type entropy.
 * Sum them?
 *
 *******************************************************************************/


#ifndef Bubble_HPP_INCLUDED
#define Bubble_HPP_INCLUDED


/*Includes*/
#include <opencv2/opencv.hpp>

struct BubbleImageFrame{

    cv::Rect newPosition;
    double ContArea;
    double ContRadius;
    cv::Moments moments;
    cv::Point2f MassCentres;


};

/*Memory Allocations*/

class bubble{

    private:



        /*Temp storages*/
        int _dZdT;



    public:
        /*Constructor and deconstructor*/
        bubble(BubbleImageFrame );
        ~bubble();

        /* cvRect describing the origin */
        std::vector<BubbleImageFrame> KnownDescriptors;

        float last_x;
        float last_y;

        cv::Rect GenesisPosition;
        cv::Point2f GenesisPositionCentroid;


        /*Bubble descriptions*/
        std::vector<float> dz;

        void dSizedT(std::vector<std::pair<float,float>>&);
        float dRdT(void);
        float dZdT (void);

        bool lockThisIteration;


        /*Matching algorithms*/
        bool isNewPositionProbable(int&, int&);


        /*Print all X,Y*/
        void printAllXY(void);

        /*A fancy bubble adding thing ;-) */
        void operator << (BubbleImageFrame );


};




#endif
