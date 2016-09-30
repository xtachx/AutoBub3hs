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


/*Memory Allocations*/

class bubble{

    private:

        /* cvRect describing the origin */
        std::vector<cv::Rect> KnownDescriptors;


        /*Temp storages*/
        int _dZdT;



    public:
        /*Constructor and deconstructor*/
        bubble(cv::Rect );
        ~bubble();


        float last_x;
        float last_y;

        cv::Rect GenesisPosition;

        /*Bubble descriptions*/
        std::vector<float> dz;

        void dSizedT(std::vector<std::pair<float,float>>&);
        int dRdT(void);
        int dZdT (void);


        /*Matching algorithms*/
        bool isNewPositionProbable(int&, int&);


        /*Print all X,Y*/
        void printAllXY(void);

        /*A fancy bubble adding thing ;-) */
        void operator << (cv::Rect );


};




#endif
