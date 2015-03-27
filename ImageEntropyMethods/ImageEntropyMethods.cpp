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

#include "ImageEntropyMethods.hpp"

/*Definitions
Note: DECLARATIONS GO TO HPP
DEFINITIONS go here
*/
/*Memory for the image greyscale and histogram*/
cv::Mat image_greyscale, img_histogram;

/*Histogram sizes and bins*/
const int histSize[] = {16};
float range[] = { 0, 256 };
const float* histRange[] = { range };

/*This function calculates ImageEntropy
based on CPU routines. It converts the image to
BW first. This is assumed that the image frames are
subtracted already */
float calculateEntropyFrame(cv::Mat& ImageFrame){

    float ImgEntropy=0.0;
    /*Check if image is BW or colour*/
    if (ImageFrame.channels() > 1){
        /*Convert to BW*/
        cv::cvtColor(ImageFrame, image_greyscale, cv::COLOR_BGR2GRAY);
    } else {
        /*The = operator assigns pointers so no memory is wasted*/
        image_greyscale = ImageFrame;
    }


    /*Calculate Histogram*/
    cv::calcHist(&image_greyscale, 1, 0,        cv::Mat(), img_histogram, 1, histSize, histRange, true, false);
    /*Normalize Hist*/
    img_histogram = img_histogram/(ImageFrame.rows*ImageFrame.cols);
    /*Calculate Entropy*/
    for (int i=0; i<img_histogram.rows; i++){
            float binEntry = img_histogram.at<float>(i, 0);
            if (binEntry !=0)
                ImgEntropy -= binEntry * log2(binEntry);
    }

    return ImgEntropy;
}
