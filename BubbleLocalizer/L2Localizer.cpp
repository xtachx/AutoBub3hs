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



#include "L2Localizer.hpp"
#include <stdio.h>


/* ******************************************************************************
 * This function is step 1 to the problem. The lower bubble is gonna be
 * much bigger than the upper one. This will allow us to get an initial guess
 * on the position of the bubble and then use aggressive techniques for the top bubble.
 *
 * Not going by the mistakes earlier, this will be OO
 * ******************************************************************************/


L2Localizer::L2Localizer(cv::Mat& prFrame, bool nonStopPref){

    /*Assign the Presentation frame
     * Rest of them have been moved
     * due to expansion of the code
     */
    presentationFrame = prFrame;

    /*User Prefs*/
    nonStopMode = nonStopPref;  /*Flag for non-stop operation vs debug*/
    color = cv::Scalar( 0,0,255);  /*Red colour defined for drawing stuff*/
    color_orange = cv::Scalar( 0,140,255);  /*Orange colour defined for indicating level2 searc areaf*/
    color_green = cv::Scalar( 0, 255, 0);  /*Green colour defined for indicating Hough searc areaf*/

    /*set confidence flag to false at start*/
    Level1SuspicionFlag = false;

    /*The ROIs*/
    topCutCornerX = 66;
    topCutCornerY=219;

    /*Info from fit*/
    numBubbleMultiplicity = 0;

}


L2Localizer::~L2Localizer() {}






/*This is the infamous ellipse - to - box area and eccentricity test to check whether the detection is a bubble or garbage.
 *Essentially what happens is that the ellipse should:
 *1. Not have an area less than 100 sq px
 *2. Not have an area bigger than the bounding box
 *3. Should not have a crazy eccentricity. The large bubbles are fairly roundish, the garbage is severely elongated
 */
void L2Localizer::EllipseTest(cv::Mat& frameDraw, cv::RotatedRect& minEllipse, cv::Rect& boundingBoxRect, cv::Scalar& color, std::vector<cv::RotatedRect>& bubbleLocations, int localizationDict, bool drawEllipses){

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


    if (w/h >0.33 and w/h<3 and (ellipseArea > 0.5*boxArea and ellipseArea < 1.1*boxArea)) {


        if (drawEllipses) cv::ellipse( frameDraw, minEllipse, color, 2, 8 );

        bubbleLocations.push_back(minEllipse);

        if (localizationDict== SEARCH_LEVEL_1){



            /*Most bubbles shouldbe between 200-500. If not, start the Level 2 search*/
            ellipseArea >= 5000.0 ? this->Level1SuspicionFlag=true : this->Level1SuspicionFlag=false;
            this->numBubbleMultiplicity++;
        }


    }

}


/* Function to check for duplicates and overlap on ROI
 * It destroys the original vector but a new one
  * is made to take its place*/

void L2Localizer::rem_unique(std::vector<cv::RotatedRect>& L2SearchAreas, std::vector<cv::Rect>& L2SearchAreasFixed){

    cv::Rect checkElem = L2SearchAreas[0].boundingRect();
    int cOrigHeight = checkElem.height;
    //checkElem.height += checkElem.height*1.0;

    if (L2SearchAreas.size()==1){
        checkElem.height += cOrigHeight*1.0;
        L2SearchAreasFixed.push_back(checkElem);
        L2SearchAreas.erase(L2SearchAreas.begin());
        return;

    }
    bool foundOvrLap = false;
    for(std::vector<int>::size_type rects=1; rects<L2SearchAreas.size(); rects++){
            cv::Rect thisElem = L2SearchAreas[rects].boundingRect();

            /*Backwards move*/
            checkElem.y -= cOrigHeight;
            /*Forward check*/
            checkElem.height += 2.0*cOrigHeight;


            cv::Rect OverLap = thisElem & checkElem;
            bool isOverlap = OverLap.area()!=0;


            if (isOverlap){
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

void L2Localizer::TemporalGuidedSearch2(cv::Mat& FrameBeforeTrigger, cv::Mat& frameTrigger, cv::Mat& FrameAfterTrigger){

    std::vector<cv::RotatedRect> bubbleLocations2;

    /*Declare memory / variables that will be needed */
    cv::Mat DarkImg, BrightImg, imgBotROI, imgBotThresholdCut, imgArea;
    cv::Mat DarkImg_Trig, BrightImg_Trig, imgArea_Trig;

    /* Create the "bright" and "dark" frame subtractions
     * This is needed so that darker areas of the bubble are resolved in the "dark"
     * frame and brighter areas in the "bright frame". This is like getting the negative values
     * from an image subtraction.
     */
     //cv::GaussianBlur(FrameAfterTrigger, FrameAfterTrigger, cv::Size(11,11), 0, 0 );
     //cv::GaussianBlur(FrameBeforeTrigger, FrameBeforeTrigger, cv::Size(11,11), 0, 0 );


    BrightImg = FrameAfterTrigger - FrameBeforeTrigger;
    BrightImg_Trig = frameTrigger - FrameBeforeTrigger;

    DarkImg  =  FrameBeforeTrigger - FrameAfterTrigger;
    DarkImg_Trig  =  FrameBeforeTrigger - frameTrigger;

    /* Adding them gives the "complete picture*/
    imgArea = DarkImg;// + BrightImg;
    imgArea_Trig = DarkImg_Trig + BrightImg_Trig;


    /*SAVE POINT 1*/
    if (!this->nonStopMode) cv::imwrite("SavePoint1_FullFrameSub.jpg", imgArea);

    /*Temporary holder for the presentationFrame*/
    cv::Mat tempPresentation;
    tempPresentation = this->presentationFrame;//.clone();

    /*Now cut out the bottom area to work with it*/
    cv::Rect upROI(66, 219, 1002-66, 1563-219);

    imgBotROI = imgArea(upROI);//( this->botROI );
    //tempPresentation = tempPresentation(upROI);

    /*Either blur and threshold it or just threshold it to extract features*/
    //cv::blur(imgBotROI, imgBotROI, cv::Size(3,3));
    double sigmaX=0;
    double sigmaY=0;


    cv::GaussianBlur(imgBotROI, imgBotROI, cv::Size(13,13), sigmaX, sigmaY );
    //debugShow(imgBotROI);

    /*The first line is to get rid of extra pixel noise time to time*/
    cv::threshold(imgBotROI, imgBotThresholdCut,3, 255, CV_THRESH_TOZERO);
    cv::threshold(imgBotThresholdCut, imgBotThresholdCut,0, 255, CV_THRESH_BINARY+CV_THRESH_OTSU);

    //cv::threshold(imgBotROI, imgBotThresholdCut,0, 255, CV_THRESH_BINARY+CV_THRESH_OTSU);
    //debugShow(imgBotThresholdCut);

    /*SAVE POINT 2*/
    if (!this->nonStopMode) cv::imwrite("SavePoint2_FrameThresholded.jpg", imgBotThresholdCut);


    /*Use contour / canny edge detection to find contours of interesting objects*/
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(imgBotThresholdCut, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1);

    /*Make two vectors to store the fitted rectanglse and ellipses*/
    std::vector<cv::RotatedRect> minEllipse( contours.size() );
    std::vector<cv::Rect> minRect( contours.size() );

    /*Generate the ellipses and rectangles for each contours*/
    for( int i = 0; i < contours.size(); i++ ) {
        minRect[i] = cv::boundingRect( contours[i]);
         if( contours[i].size() > 5 ) {
            minEllipse[i] = cv::fitEllipse( cv::Mat(contours[i]) );
        }
    }

    /*Draw each contour and rectangle and apply the ellipse test!*/
    for( int i = 0; i< contours.size(); i++ ) {
        if (cv:: contourArea(contours[i]) > 5) {

            /*SAVE POINT 3*/
            if (!this->nonStopMode) cv::imwrite("SavePoint3_FullFrameContourBox.jpg", this->presentationFrame);

            cv::rectangle(tempPresentation, minRect[i], this->color, 2,1,8);
            /*Ellipse Test*/
            this->EllipseTest(tempPresentation, minEllipse[i], minRect[i], this->color, bubbleLocations2, SEARCH_LEVEL_2, false);

        }
    }
    //debugShow(tempPresentation);

    /*****************************/
    //printf("Found numBubbles: %d\n", bubbleLocations2.size());
    /*Get rid of dupes - small subroutine*/
    std::vector<cv::Rect> L2SearchAreasFixed;  /*Assign memory for fixed ROIs*/
    /*Sort the ROIs by descending area, so we can extend bigger areas and see if the smaller ones overlap
     *Note: the sort function is a lambda, so we need c++11 for this.
     *I will use C++11 anyways so this isnt a big deal now*/
    std::sort(bubbleLocations2.begin(), bubbleLocations2.end(), bubbleBRectSort);


    /*Use for debugging
     */
     for (std::vector<int>::size_type i=0; i<bubbleLocations2.size(); i++)
       //printf("x: %f y: %f | w: %f h: %f | BBoxArea: %0.2f \n",  bubbleLocations2[i].center.x,  bubbleLocations2[i].center.y,  bubbleLocations2[i].size.width, bubbleLocations2[i].size.height, bubbleLocations2[i].boundingRect().area());
     //std::cout << '\n';


    do{
        this->rem_unique(bubbleLocations2, L2SearchAreasFixed);
    } while (bubbleLocations2.size()>=1);

    /*------*/
     //printf("ROIs remaining - after dupe cleaning: %d\n", L2SearchAreasFixed.size());


    for (std::vector<int>::size_type i = 0; i != L2SearchAreasFixed.size(); i++){


        /*Get the boundingRect*/
        cv::Rect brect = L2SearchAreasFixed[i];

        /*Offset from main image by*/
        brect.x+=this->topCutCornerX;
        brect.y+=this->topCutCornerY;

        /*Accounting for the fact that bubble moves up*/
        //brect.height += brect.height*0.5;

        /*Draw the boundingRect*/
        cv::rectangle(tempPresentation, brect, this->color_orange,1,8,0);

        cv::Point offsets(brect.x, brect.y);
        int maxCircleRadius = brect.width > brect.height ? brect.width : brect.height;


        imgBotROI = imgArea_Trig.clone();
        imgBotROI = imgBotROI(brect);
        //debugShow(imgBotROI);
        cv::threshold(imgBotROI, imgBotThresholdCut,0, 255, CV_THRESH_BINARY+CV_THRESH_OTSU);
        //debugShow(imgBotThresholdCut);


        std::vector<cv::Vec4i> hierarchy;
        std::vector<std::vector<cv::Point> > contours2;
        std::vector<cv::RotatedRect> minEllipse;

        /* Find contours - with Canny edge detection*/
        findContours( imgBotThresholdCut, contours2, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );


        for( int i = 0; i < contours2.size(); i++ ) {
            int contourArea = cv::contourArea(contours2[i]);
            if (contours2[i].size() > 5){
                minEllipse.push_back(fitEllipse( cv::Mat(contours2[i]) ));

                //debugShow(imgBotThresholdCut);
            }
        }

        for( int i = 0; i < minEllipse.size(); i++ ) {

            /*Account for the multiplicity*/
            this->numBubbleMultiplicity++;


            /*Fix the centres of the ellipses for the presentation frame*/
            minEllipse[i].center.x+=brect.x;
            //minEllipse[i].center.x+=this->topCutCornerX;

            minEllipse[i].center.y+=brect.y;
            //minEllipse[i].center.y+=this->topCutCornerY;

            cv::ellipse( tempPresentation, minEllipse[i], this->color, 1, 8 );


            //printf("x: %f y: %f | w: %f h: %f\n",  minEllipse[i].center.x,  minEllipse[i].center.y,  minEllipse[i].size.width, minEllipse[i].size.height);
        }
        /*Copy over the data*/
        //printf("Min Ellipse Vec size: %d\n", minEllipse.size());
        bubbleRects.insert(bubbleRects.end(), minEllipse.begin(), minEllipse.end());

    }
    /*Debug Flags*/
    //debugShow(tempPresentation);
    //cv::imwrite("FinalResult2.jpg", tempPresentation);




}
/*Takes care of the localization completely. Just like it says... Localize-O-Matic!*/

void LocalizeOMatic(cv::Mat& BubbleFrame, cv::Mat& CmpFrame, cv::Mat& trigNextFrame, std::vector<cv::RotatedRect>& botBubbleLoc, std::string eventSeq, std::string imageStorePath){

    std::vector<cv::RotatedRect> bubbleLocations;
    cv::Mat BFChan[3], CmpChan[3],  trigNextChan[3];

    /*-----------To optimize later*******/
    cv::split(BubbleFrame, BFChan);
    cv::split(CmpFrame, CmpChan);
    cv::split(trigNextFrame, trigNextChan);

    L2Localizer LocalizeMe(BubbleFrame, 1);
    LocalizeMe.numBubbleMultiplicity=0;

    /*And launch!*/
    LocalizeMe.TemporalGuidedSearch2(BFChan[0], CmpChan[0], trigNextChan[0]);

    /*Analyze results*/
    //std::cout<<"Refined bubble multiplicity:  "<<LocalizeMe.numBubbleMultiplicity<<"\n";

    /*Store the finished image*/
    //cv::imwrite(imageStorePath+"/"+eventSeq+".jpg", BubbleFrame);

    /*Copy data over*/
    botBubbleLoc = LocalizeMe.bubbleRects;

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
bool bubbleBRectSort(cv::RotatedRect a, cv::RotatedRect b){return a.boundingRect().area()>b.boundingRect().area();}
