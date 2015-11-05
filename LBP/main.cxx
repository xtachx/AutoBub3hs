#include <opencv2/opencv.hpp>

#include "lbp.hpp"
#include <string>
#include "opencv2/photo/photo.hpp"
using namespace cv;
void debugShow(cv::Mat& );


cv::Mat lbpImage(cv::Mat frame){

// initial values
    int radius = 2;
    int neighbors = 8;

// matrices used
    //Mat frame; // always references the last frame
    Mat dst; // image after preprocessing
    Mat lbp; // lbp image


    int lbp_operator=2;



    //frame = cv::imread(framename);
	//dst = frame;
	cvtColor(frame, dst, CV_BGR2GRAY);

    //GaussianBlur(dst, dst, Size(7,7), 5, 3, BORDER_CONSTANT); // tiny bit of smoothing is always a good idea
    // comment the following lines for original size
    //resize(frame, frame, Size(), 0.5, 0.5);
    //resize(dst,dst,Size(), 0.5, 0.5);
    //
    switch(lbp_operator) {
    	case 0:
    		lbp::ELBP(dst, lbp, radius, neighbors); // use the extended operator
    		break;
    	case 1:
    		lbp::OLBP(dst, lbp); // use the original operator
    		break;
    	case 2:
    		lbp::VARLBP(dst, lbp, radius, neighbors);
    		break;
    	}
    	// now to show the patterns a normalization is necessary
    	// a simple min-max norm will do the job...
    	normalize(lbp, lbp, 0, 255, NORM_MINMAX, CV_8UC1);

    	//imshow("original", frame);
    	//imshow("lbp", lbp);
        return lbp;

}




int main(int argc, const char *argv[]) {

    std::string oFrame, cFrame;

    //[pitam@pitbrane AutoBub2]$ ./abub /home/pitam/2l-15-data/ 20150421_0 c

    oFrame = "/home/pitam/2l-15-data/20150418_1/12/cam0image  5.bmp";
    cFrame = "/home/pitam/2l-15-data/20150418_1/12/cam0image  0.bmp";



    cv::Mat OriginalFrame, CmpFrame, prelbp, lbpmap, lbpmap2;

    OriginalFrame = cv::imread(oFrame);
    CmpFrame = cv::imread(cFrame);

    ////////////
    //lbpmap = lbpImage(OriginalFrame);
    //cv::threshold(lbpmap, lbpmap, 10, 255, CV_THRESH_BINARY);
    //debugShow(lbpmap);
    ///////////


    prelbp = OriginalFrame - CmpFrame;
    lbpmap = lbpImage(prelbp);


    cv::threshold(lbpmap, lbpmap, 10, 255, CV_THRESH_BINARY);

    lbpmap2=lbpmap.clone();
    debugShow(lbpmap2);

    /*Use contour / canny edge detection to find contours of interesting objects*/
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(lbpmap, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1);

    /*Make two vectors to store the fitted rectanglse and ellipses*/
    std::vector<cv::RotatedRect> minEllipse( contours.size() );
    std::vector<cv::Rect> minRect;


    cv::Scalar color = cv::Scalar( 255,255,255);

    //std::cout<<"Number of Objects: "<<contours.size();

    /*Generate the ellipses and rectangles for each contours*/
    for( int i = 0; i < contours.size(); i++ ) {
        if (cv::contourArea(contours[i])>=2){
            minRect.push_back(cv::boundingRect( contours[i]));
        }
        //cv::rectangle(OriginalFrame, minRect[i], color,2,8,0);
    }

    debugShow(OriginalFrame);

    cv::Mat imgBotROI, cmpFrameSmall;


    //cv::cvtColor(OriginalFrame, dist_src, CV_BGR2GRAY);
    //cv::threshold(dist_src, dist_src, 0, 255, CV_THRESH_BINARY+CV_THRESH_OTSU);
    //debugShow(dist_src);
    //cv::distanceTransform(OriginalFrame, dist, CV_DIST_L12, 3);
    //cv::erode(lbpmap2, lbpmap2, 3, 7, 21 );

    //cv::threshold(prelbp, prelbp, 3, 255, CV_THRESH_BINARY);
    /*Draw each contour and rectangle and apply the ellipse test!*/
    for( int i = 0; i< minRect.size(); i++ ) {
        //for (std::vector<int>::size_type i = 0; i != L2SearchAreasFixed.size(); i++){


        /*Get the boundingRect*/
        cv::Rect brect = minRect[i];

        /*extend boundaries*/
        brect.x -= brect.width;
        brect.y -= brect.height;
        brect.height *= 5;
        brect.width *= 5;



        /*Draw the boundingRect*/
        cv::rectangle(lbpmap2, brect, color,1,8,0);

        //cv::Point offsets(brect.x, brect.y);
        //int maxCircleRadius = brect.width > brect.height ? brect.width : brect.height;


        //cv::blur(OriginalFrame, OriginalFrame, cv::Size(3,3));

        static cv::Mat bigger_frame, lbpmap22, bigger_frame_cmp, showFrameDiff;


        imgBotROI = OriginalFrame.clone();
        imgBotROI = imgBotROI(brect);

        cmpFrameSmall = CmpFrame.clone();
        cmpFrameSmall = cmpFrameSmall(brect);



        cv::resize(imgBotROI, bigger_frame, cv::Size(5*brect.width, 5*brect.height ), INTER_LANCZOS4);
        cv::resize(cmpFrameSmall, bigger_frame_cmp, cv::Size(5*brect.width, 5*brect.height ), INTER_LANCZOS4);
        //cv::blur(bigger_frame, bigger_frame, cv::Size(9,9));

        debugShow(bigger_frame);


        showFrameDiff = bigger_frame - bigger_frame_cmp;
        debugShow(showFrameDiff);

        lbpmap22 = lbpImage(showFrameDiff);
        cv::threshold(lbpmap22, lbpmap22, 10, 255, CV_THRESH_BINARY);

        debugShow(lbpmap22);



        static cv::Mat dist, dist_src;
        //dist_src=imgBotROI;
        //cv::cvtColor(imgBotROI, dist_src, CV_BGR2GRAY);
        //cv::threshold(dist_src, dist_src, 0, 255, CV_THRESH_BINARY+CV_THRESH_OTSU);

        //debugShow(dist_src);

        //cv::distanceTransform(dist_src, dist, CV_DIST_L12, 5);
        //debugShow(dist);

        //cv::threshold(imgBotROI, imgBotThresholdCut,0, 255, CV_THRESH_BINARY+CV_THRESH_OTSU);
        //debugShow(imgBotThresholdCut);
    }

    debugShow(lbpmap2);

    return 0; // success




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
