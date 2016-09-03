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



#include "L3Localizer.hpp"
#include <stdio.h>


#include "../LBP/lbp.hpp"
#include "../LBP/LBPUser.hpp"
#include "../common/UtilityFunctions.hpp"
#include "../AnalyzerUnit.hpp"



/* ******************************************************************************
 * This function is step 1 to the problem. The lower bubble is gonna be
 * much bigger than the upper one. This will allow us to get an initial guess
 * on the position of the bubble and then use aggressive techniques for the top bubble.
 *
 * Not going by the mistakes earlier, this will be OO
 * ******************************************************************************/


L3Localizer::L3Localizer(std::string EventID, std::string ImageDir, int CameraNumber, bool nonStopPref):AnalyzerUnit(EventID, ImageDir, CameraNumber)
{


    /*User Prefs*/
    nonStopMode = nonStopPref;  /*Flag for non-stop operation vs debug*/
    color = cv::Scalar( 255, 255, 255); // White colour universal
    color_red = cv::Scalar( 0,0,255);  /*Red colour defined for drawing stuff*/
    color_orange = cv::Scalar( 0,140,255);  /*Orange colour defined for indicating level2 searc areaf*/
    color_green = cv::Scalar( 0, 255, 0);  /*Green colour defined for indicating Hough searc areaf*/

    /*set confidence flag to false at start*/
    Level1SuspicionFlag = false;

    /*The ROIs*/
    //topCutCornerX = 66;
    //topCutCornerY = 219;

    /*Info from fit*/
    numBubbleMultiplicity = 0;

}


L3Localizer::~L3Localizer() {}






/*This is the infamous ellipse - to - box area and eccentricity test to check whether the detection is a bubble or garbage.
 *Essentially what happens is that the ellipse should:
 *1. Not have an area less than 100 sq px
 *2. Not have an area bigger than the bounding box
 *3. Should not have a crazy eccentricity. The large bubbles are fairly roundish, the garbage is severely elongated
 */
void L3Localizer::EllipseTest(cv::Mat& frameDraw, cv::RotatedRect& minEllipse, cv::Rect& boundingBoxRect, cv::Scalar& color, std::vector<cv::RotatedRect>& bubbleLocations, int localizationDict, bool drawEllipses)
{

    float w,h, ellipseArea, boxArea;
    w = minEllipse.size.width;
    h = minEllipse.size.height;
    //minEllipse.center.x+=this->topCutCornerX;
    //minEllipse.center.y+=this->topCutCornerY;

    /*Area of Ellipse*/
    ellipseArea = 3.14159*w*h/4;
    boxArea = boundingBoxRect.width*boundingBoxRect.height;

    /*Debug Info
    std::cout<<"Rect Area: "<<boundingBoxRect.area()<<std::endl;
    */


    if (w/h >0.33 and w/h<3 and (ellipseArea > 0.5*boxArea and ellipseArea < 1.1*boxArea))
    {


        if (drawEllipses) cv::ellipse( frameDraw, minEllipse, color, 2, 8 );

        bubbleLocations.push_back(minEllipse);

        if (localizationDict== SEARCH_LEVEL_1)
        {



            /*Most bubbles shouldbe between 200-500. If not, start the Level 2 search*/
            ellipseArea >= 5000.0 ? this->Level1SuspicionFlag=true : this->Level1SuspicionFlag=false;
            this->numBubbleMultiplicity++;
        }


    }

}


/* Function to check for duplicates and overlap on ROI
 * It destroys the original vector but a new one
  * is made to take its place*/

void L3Localizer::rem_unique(std::vector<cv::Rect>& L2SearchAreas, std::vector<cv::Rect>& L2SearchAreasFixed)
{

    cv::Rect checkElem = L2SearchAreas[0];//.boundingRect();
    int cOrigHeight = checkElem.height;
    //checkElem.height += checkElem.height*1.0;

    if (L2SearchAreas.size()==1)
    {
        checkElem.height += cOrigHeight*1.0;
        L2SearchAreasFixed.push_back(checkElem);
        L2SearchAreas.erase(L2SearchAreas.begin());
        return;

    }
    bool foundOvrLap = false;
    for(std::vector<int>::size_type rects=1; rects<L2SearchAreas.size(); rects++)
    {
        cv::Rect thisElem = L2SearchAreas[rects];//.boundingRect();

        /*Backwards move*/
        checkElem.y -= cOrigHeight;
        /*Forward check*/
        checkElem.height += 2.0*cOrigHeight;


        cv::Rect OverLap = thisElem & checkElem;
        bool isOverlap = OverLap.area()!=0;


        if (isOverlap)
        {
            thisElem.height += thisElem.height*0.5;
            L2SearchAreasFixed.push_back(thisElem | checkElem);
            L2SearchAreas.erase(L2SearchAreas.begin()+rects);
            foundOvrLap = true;
        }

        /*Revert back*/
        checkElem.height -= 2.0*cOrigHeight;
        /*Backwards check*/
        checkElem.y += cOrigHeight*1.0;


    }
    /*Give a boost to height*/
    checkElem.height += cOrigHeight*1.0;
    if (!foundOvrLap) L2SearchAreasFixed.push_back(checkElem);
    L2SearchAreas.erase(L2SearchAreas.begin());

}



/* ******************************************************************************
 * This function is the primary localizer for the bubble finding algorithm.
 * However, it has no idea how big the bubble is and where to look for.
 * So the initial guess is the adaptive filtering.
 *
 * Why this strange name? It is a memory from the original CGeyserImageAnalysis code
 * written for the geyser, but now for COUPP-60.
 *
 * This routine is designed to look at the next frame and perform a guided search of the first frame
 * In case the bubbles are too small, this considerably narrows down the search areas
 * that the code has to look at. So this should make it more accurate
 *
 *  * *****************************************************************************/

void L3Localizer::CalculateInitialBubbleParams(void )
{

    //cv::Mat& TriggerFrame, ComparisonFrame, cv::Mat& FrameAfterTrigger

    /*Declare memory / variables that will be needed */
    cv::Mat LBPImageTrig, LBPImageTrigNext;
    cv::Mat FrameDiffTrig, FrameDiffTrigNext;
    cv::Mat FrameAfterTrigger = cv::imread(this->ImageDir + this->CameraFrames[this->MatTrigFrame+1],0);


    /*Temporary holder for the presentationFrame*/
    cv::Mat tempPresentation;
    cv::Mat tempInvertImage;
    tempPresentation = this->presentationFrame;//.clone();



    /*Construct the frame differences and LBPImage Frames*/
    FrameDiffTrigNext = FrameAfterTrigger - this->ComparisonFrame;
    cv::absdiff(FrameAfterTrigger, this->ComparisonFrame, FrameDiffTrig);
    //FrameDiffTrig =  this->triggerFrame - this->ComparisonFrame;


    //cv::normalize(FrameDiffTrig, FrameDiffTrig, 0, 255, NORM_MINMAX);
    //cv::normalize(FrameDiffTrigNext, FrameDiffTrigNext, 0, 255, NORM_MINMAX);

    std::cout<<"The next trigger is: "<<this->CameraFrames[this->MatTrigFrame]<<"\n";
    //debugShow(FrameAfterTrigger);
    //debugShow(FrameDiffTrigNext);

    //Denoising
    //int rows = FrameDiffTrig.rows;
    //int cols = FrameDiffTrig.cols;

    /*Kernel Definition*/
    int boxcar1_width = 3;
    int boxcar2_width = 50;
    int guard_band=2;
    cv::Mat boxCarKernel;

    cv::Mat boxCarKernel2 = -1.0*cv::Mat::ones(1,boxcar2_width,CV_8U);
    cv::Mat boxCarKernelG = cv::Mat::zeros(1, guard_band+1+guard_band,CV_8U);
    cv::Mat boxCarKernel1 = cv::Mat::ones(1, boxcar1_width,CV_8U);

    cv::hconcat(boxCarKernel2,boxCarKernelG,boxCarKernel); // horizontal concatenation
    cv::hconcat(boxCarKernel,boxCarKernel1,boxCarKernel); // horizontal concatenation

    //boxCarKernel /= (float)(boxcar2_width+boxcar1_width);
    /* **************************** */

    cv::filter2D(FrameDiffTrig, FrameDiffTrig, -1 , boxCarKernel, Point(boxcar2_width+guard_band-1, 0), 0, BORDER_DEFAULT );

    //std::cout << "M = "<< std::endl << " "  << boxCarKernel << std::endl << std::endl;
    //FrameDiffTrig /= (boxcar2_width+boxcar1_width);
    cv::threshold(FrameDiffTrig, FrameDiffTrig, 30, 255, CV_THRESH_TOZERO);
    //cv::normalize(FrameDiffTrig,FrameDiffTrig, 0, 255, NORM_L2);
    debugShow(this->presentationFrame);
    FrameDiffTrig=10.0*FrameDiffTrig;
    debugShow(FrameDiffTrig);



    LBPImageTrigNext = lbpImageSingleChan(FrameDiffTrigNext);
    LBPImageTrig = lbpImageSingleChan(FrameDiffTrig);


    debugShow(LBPImageTrig);
    //debugShow(LBPImageTrigNext);

    /*SAVE POINT 1*/
    if (!this->nonStopMode) cv::imwrite("SavePoint1_LBPImageTrig.jpg", LBPImageTrig);
    if (!this->nonStopMode) cv::imwrite("SavePoint1_LBPImageTrigNext.jpg", LBPImageTrigNext);

    //showHistogramImage(LBPImageTrigNext);

    //do
    //{
    //    this->rem_unique(minRect, L2SearchAreasFixed);
    //}
    //while (minRect.size()>=1);

    //L2SearchAreasFixed = minRect ;


}
/*Takes care of the localization completely. Just like it says... Localize-O-Matic!*/

void L3Localizer::LocalizeOMatic(std::string imageStorePath)
{

    if (!this->okToProceed) return;
    /*Assign the three useful frames*/
    this->triggerFrame = cv::imread(this->ImageDir + this->CameraFrames[this->MatTrigFrame],0);
    this->presentationFrame = triggerFrame.clone();
    this->ComparisonFrame = cv::imread(this->ImageDir + this->CameraFrames[0],0);

    /*Run the analyzer series*/
    this->numBubbleMultiplicity=0;
    this->CalculateInitialBubbleParams();


    /*Analyze results*/
    std::cout<<"Refined bubble multiplicity:  "<<this->numBubbleMultiplicity<<"\n";

    /*Store the finished image*/
    //cv::imwrite(imageStorePath+"/"+eventSeq+".jpg", BubbleFrame);


}
