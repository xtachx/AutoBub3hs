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


#ifndef Utility_HPP_INCLUDED
#define Utility_HPP_INCLUDED


/*Includes*/
#include <opencv2/opencv.hpp>



void debugShow(cv::Mat&);
void showHistogramImage(cv::Mat& );



#endif