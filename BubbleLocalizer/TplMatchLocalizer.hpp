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


#ifndef ImageLocalization_TPL_HPP_INCLUDED
#define ImageLocalization_TPL_HPP_INCLUDED


/*Includes*/
#include <opencv2/opencv.hpp>


/*Memory Allocations*/

class TPLLocalizer{

    private:



        /*User Params*/
        bool nonStopMode;

        /*The in-process stepping frames*/
        cv::Mat probabilityMapFrame;
        cv::Mat MaskFrame, presentationFrame;
        cv::Mat templ = cv::imread("BTem.png", 0);



    public:
        /*Constructor and deconstructor*/
        TPLLocalizer(cv::Mat&, bool nonStopPref = true );
        ~TPLLocalizer();

        /*Public functions exposing the interface*/
        void BlobMatch(std::vector<cv::RotatedRect>&);
        void MarkBubbles(std::vector<cv::RotatedRect>&, cv::Mat&);
        void DrawCrosshairs(cv::Mat&, cv::Rect& );


        //void LocalizeBottomBubble(cv::Mat&, cv::Mat&, std::vector<cv::RotatedRect>& );

        /*Public variables exposed for direct manipulation*/
        //int numBubbleMultiplicity;
        //bool Level1SuspicionFlag;

        std::vector<cv::RotatedRect> bubbleRects;






};

void debugShow(cv::Mat&);
//void LocalizeOMatic(cv::Mat &, cv::Mat&, cv::Mat&, std::vector<cv::RotatedRect>&, std::string, std::string);
//bool bubbleBRectSort(cv::RotatedRect , cv::RotatedRect );




#endif
