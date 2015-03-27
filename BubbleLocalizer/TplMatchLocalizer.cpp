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



#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/features2d/features2d.hpp>



#include "TplMatchLocalizer.hpp"
#include <stdio.h>


/* ******************************************************************************
 * This function is step 1 to the problem. The lower bubble is gonna be
 * much bigger than the upper one. This will allow us to get an initial guess
 * on the position of the bubble and then use aggressive techniques for the top bubble.
 *
 * Not going by the mistakes earlier, this will be OO
 * ******************************************************************************/


TPLLocalizer::TPLLocalizer(cv::Mat& prFrame, bool nonStopPref)
{

    /*Assign the Presentation frame
     * Rest of them have been moved
     * due to expansion of the code
     */
    presentationFrame = prFrame.clone();

    /*WorkingFrame - this is the foreground mask*/
    MaskFrame = prFrame;

    /*User Prefs*/
    nonStopMode = nonStopPref;  /*Flag for non-stop operation vs debug*/

    /*read the bubble template*/
    templ = cv::imread("BTem.png", 0);


}


TPLLocalizer::~TPLLocalizer() {}





/*
 * This is the code for the template matching Algorithms
 * It will make a probability map based on a given template and then find maxima
 * above a certain cut
 */

void TPLLocalizer::BlobMatch(std::vector<cv::RotatedRect>& botBubbleLoc)
{



    //MatchingMethod( 0, 0, img_mask, templ );
    int match_method = CV_TM_CCOEFF_NORMED;


    //mask: MaskFrame
    //probs: probabilityMapFrame

    std::cout<<"IMG_TYPE: "<<this->MaskFrame.type()<<" | Templ type: "<<this->templ.type()<<"\n" ;

    /// Create the result matrix
    int result_cols =  this->MaskFrame.cols - this->templ.cols + 1;
    int result_rows = this->MaskFrame.rows - this->templ.rows + 1;

    //this->probabilityMapFrame.create( result_rows, result_cols, CV_32FC1 );

    /// Do the Matching and Normalize
    //cv::matchTemplate( this->MaskFrame, this->templ, this->probabilityMapFrame, match_method );
    //cv::normalize( this->probabilityMapFrame, this->probabilityMapFrame, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );

    //debugShow(this->probabilityMapFrame);

    cv::SimpleBlobDetector::Params params;
    params.minDistBetweenBlobs = 50.0f;
    params.filterByInertia = false;
    params.filterByConvexity = false;
    params.filterByColor = false;
    params.filterByCircularity = false;
    params.filterByArea = true;
    params.minArea = 20.0f;
    params.maxArea = 500.0f;
    // ... any other params you don't want default value

    // set up and create the detector using the parameters
    cv::SimpleBlobDetector blob_detector(params);
    // or cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params)

    // detect!
    std::vector<cv::KeyPoint> keypoints;
    blob_detector.detect(this->MaskFrame, keypoints);

    // extract the x y coordinates of the keypoints:

    for (int i=0; i<keypoints.size(); i++) {
        float X = keypoints[i].pt.x;
        float Y = keypoints[i].pt.y;
        std::cout<<"X: "<<X<<" | Y: "<<Y<<"\n";
        cv::drawKeypoints( this->MaskFrame, keypoints, this->presentationFrame, cv::Scalar(0,0,255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
    }

    debugShow(this->presentationFrame);


}

/*
* Count and segment overlapping objects with Watershed and Distance Transform.
* Watershed transform is used to resolve overlapping objects.
*
* See the tutorial at:
* http://docs.opencv.org/trunk/doc/py_tutorials/py_imgproc/py_watershed/py_watershed.html
*
* Really neat example:
*/

void TPLLocalizer::MarkBubbles(std::vector<cv::RotatedRect>& bubbleRects, cv::Mat& bubbleFrame)
{

    // Find total markers
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(this->MaskFrame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    int ncomp = contours.size();
    int nBubbles = 0;

    /*Make two vectors to store the fitted rectanglse and ellipses*/
    std::vector<cv::RotatedRect> minEllipse( contours.size() );
    std::vector<cv::Rect> minRect( contours.size() );

    /*Generate the ellipses and rectangles for each contours*/
    for( int i = 0; i < ncomp; i++ ) {
        /*Min bounding rectangle*/
        minRect[i] = cv::boundingRect( contours[i]);
        //This is for fun
        if( contours[i].size() > 5 and (minRect[i].width >= 5 and minRect[i].height >= 5) ) {
            minEllipse[i] = cv::fitEllipse( cv::Mat(contours[i]) );
            //std::cout<<"Size of bounding rect: W "<<minRect[i].width<<" H "<<minRect[i].height<<"\n";
            /*Code for drawing crosshairs*/
            //this->DrawCrosshairs(bubbleFrame, minRect[i]);
            nBubbles++;
        }


    }


    /*Copy over the data*/
    //printf("Min Ellipse Vec size: %d\n", minEllipse.size());
    bubbleRects.insert(bubbleRects.end(), minEllipse.begin(), minEllipse.end());

    //debugShow(bubbleFrame);

}

void TPLLocalizer::DrawCrosshairs(cv::Mat& bubbleFrame, cv::Rect& TargetRect){

    int radius = TargetRect.height > TargetRect.width ? TargetRect.height : TargetRect.width;
    radius *= 1.5;
    radius = radius < 50 ? radius : 50;

    cv::Scalar XHairColour = cv::Scalar(0,0,255);

    //cv::Point2d c(r.x + r.width / 2, r.y + r.height / 2);
    int X0 = TargetRect.x + TargetRect.width/2;
    int Y0 = TargetRect.y + TargetRect.height/2;

    cv::circle(bubbleFrame, cv::Point(X0,Y0), radius , cv::Scalar(0,0,255), 1, 8, 0);

    //X-Hair x axis lines
    cv::line(bubbleFrame, cv::Point(X0 - 0.5*radius, Y0),  cv::Point(X0 - 1.5*radius, Y0), XHairColour, 1, 8, 0);
    cv::line(bubbleFrame, cv::Point(X0 + 0.5*radius, Y0),  cv::Point(X0 + 1.5*radius, Y0), XHairColour, 1, 8, 0);
    //X-Hair y axis lines
    cv::line(bubbleFrame, cv::Point(X0, Y0 - 0.5*radius),  cv::Point(X0, Y0 - 1.5*radius), XHairColour, 1, 8, 0);
    cv::line(bubbleFrame, cv::Point(X0, Y0 + 0.5*radius),  cv::Point(X0, Y0 + 1.5*radius), XHairColour, 1, 8, 0);


}

/* *****************************************************/
/* DebugShow*/

void debugShow(cv::Mat& frame)
{

    cv::Mat cvFlippedImg;
    //cv::bitwise_not(frame, cvFlippedImg);
    cvFlippedImg = frame;

    const char* source_window = "DebugSource";
    cv::namedWindow( source_window, CV_WINDOW_NORMAL );

    cv::imshow(source_window, cvFlippedImg);
    cv::waitKey(0);

}

/*Workarounds for fermi grid old GCC*/
bool bubbleBRectSort(cv::RotatedRect a, cv::RotatedRect b)
{
    return a.boundingRect().area()>b.boundingRect().area();
}
