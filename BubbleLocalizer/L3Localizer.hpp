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


#ifndef ImageLocalization_Rough_HPP_INCLUDED
#define ImageLocalization_Rough_INCLUDED


/*Includes*/
#include <opencv2/opencv.hpp>


/*Memory Allocations*/

class L3Localizer{

    private:



        /*User Params*/
        bool nonStopMode;
        cv::Scalar color, color_orange, color_green, color_red;
        #define SEARCH_LEVEL_1 0
        #define SEARCH_LEVEL_2 1



        /*The top and bottom dark area cut*/
        int topCutCornerX;
        int topCutCornerY;


        /*Matrices to store the frames.*/
        cv::Mat presentationFrame, frameDiffTrig;

        /*Test functions and pass criteria*/
        void EllipseTest(cv::Mat&, cv::RotatedRect&, cv::Rect&, cv::Scalar&, std::vector<cv::RotatedRect>&,  int, bool drawEllipses=true);

        /*Fix dupes in search ROIs*/
        void reFixSearchROIs(std::vector<cv::RotatedRect>& );
        void rem_unique(std::vector<cv::Rect>&, std::vector<cv::Rect>& );




    public:
        /*Constructor and deconstructor*/
        L3Localizer(cv::Mat&, bool nonStopPref = true );
        ~L3Localizer();

        /*Public functions exposing the interface*/
        //void LocalizeBottomBubble(cv::Mat&, cv::Mat&, std::vector<cv::RotatedRect>& );
        void TemporalGuidedSearch2(cv::Mat&, cv::Mat&, cv::Mat&);

        /*Public variables exposed for direct manipulation*/
        int numBubbleMultiplicity;
        bool Level1SuspicionFlag;

        std::vector<cv::RotatedRect> bubbleRects;








};

void LocalizeOMatic(cv::Mat &, cv::Mat&, cv::Mat&, std::vector<cv::RotatedRect>&, std::string, std::string);
bool bubbleBRectSort(cv::RotatedRect , cv::RotatedRect );



#endif
