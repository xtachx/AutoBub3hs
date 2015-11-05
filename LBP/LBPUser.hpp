/* *****************************************************************************
 * This file contains the necessary functions for applying an LBP Operator
 * to a given an image.
 *
 * by Pitam Mitra for the PICO Geyser ImageAnalysis Algorithm.
 *
 * Created: 12 May 2014
 *
 * Issues: Needs tuning
 *
 *******************************************************************************/

#include "lbp.hpp"




cv::Mat lbpImage(cv::Mat &);
cv::Mat lbpImageSingleChan(cv::Mat &);
cv::Mat lbpImageSingleChanExperimental(cv::Mat &);
