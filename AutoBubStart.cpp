/* OpenCV code to find Ellipses for
 * PICO Dark Matter search - the Geyser
 *
 * Written by Pitam Mitra for PICO Collaboration
 *
 * Latest Edit: 2014-07-11. See the HG Changelog
 */


#include <opencv2/opencv.hpp>
//#include "opencv2/cudaimgproc.hpp"
//#include <opencv/highgui/highgui.hpp>
//#include <opencv2/core/core.hpp>

/*C++ Stuff*/
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <stdlib.h>     /* exit, EXIT_FAILURE */


/*Geyser Image Analysis Stuff*/
#include "ImageEntropyMethods/ImageEntropyMethods.hpp"
#include "ParseFolder/ParseFolder.hpp"
//#include "common/CommonDatatypes.h"
//#include "SQLiteDBstorage/imageAnalysisResults.hpp"
#include "BubbleLocalizer/TplMatchLocalizer.hpp"
#include "BGS/bgs.hpp"
#include "BGS/methods/AdaptiveSelectiveBackgroundLearning.h"
#include "BGS/methods/jmo/MultiLayerBGS.h"



/*Namespaces - to depriciate*/
//using namespace cv;
//using namespace std;
//using std::vector;
//using namespace cv::cuda;

/*Forward declaration of debugShow*/
//void debugShow(cv::Mat&);

const int evalEntropyThresholdFrames = 2;
std::vector<int> badEvents;



/* Background Subtraction Methods */
IBGS *bgs0, *bgs1;
IBGS *bgs_special;
cv::Mat img_mask0, img_mask1;
cv::Mat img_bkgmodel0, img_bkgmodel1;




//Mat img; Mat templ;





bool sortFunc(std::string i, std::string j)
{

    unsigned int sequence_i, camera_i;
    int got_i = sscanf(i.c_str(),  "cam%dimage  %u.bmp",
                       &camera_i, &sequence_i
                      );


    assert(got_i == 2);

    unsigned int  sequence_j, camera_j;
    int got_j = sscanf(j.c_str(),  "cam%dimage  %u.bmp",
                       &camera_j, &sequence_j
                      );
    assert(got_j == 2);

    return sequence_i < sequence_j;

}


int ImageEntropyLauncher(cv::Mat& workingFrame, std::string imageDir, std::string imageFile, std::vector<std::string>::size_type &seq, bool &isTriggered, bool& imStability, int camera)
{


    /*Static variable to store the threshold entropy*/
    static float entropyThreshold = 0.0;
    /*Variable to store the current entropy in*/
    float singleEntropy;



    /*The names of the evalImage and the refImage*/
    std::string evalImg = imageDir + imageFile;


    /*Storage and pointers to the ROI in the image*/
    //cv::Mat buf_evalImg = cv::imread(evalImg.c_str());
    workingFrame = cv::imread(evalImg.c_str());
    //cv::Mat roiRaw = buf_evalImg(upROI);


    /*BackgroundSubtract*/
    if (camera == 0){
        bgs0->process(workingFrame, img_mask0, img_bkgmodel0);
        singleEntropy = calculateEntropyFrame(img_mask0);
    } else if (camera == 1) {
        bgs1->process(workingFrame, img_mask1, img_bkgmodel1);
        singleEntropy = calculateEntropyFrame(img_mask1);
    }


    //std::cout<<"Seq: "<<seq<<"| "<<"Entropy of BkgSub image: "<<singleEntropy<<"\n";
    //debugShow(img_mask);


    /*Calculate entropy of ROI*/
    if (seq==1 and singleEntropy>0.0005){
        printf("************ ---> WARNING <--******************\n");
        printf("Entropy is massive - something has triggered at the very first frame \n");
        printf("Autobub Image analysis is meaningless on this data set. Manual check recommended\n");
        printf(" *** Autobub skip --> *** \n");
        imStability = false;
        //exit (EXIT_FAILURE);
    }
    //std::cout<<"Frame Entropy: "<<singleEntropy<<std::endl;



    /*Nothing works better than manual entropy settings. :-(*/
    if (singleEntropy > 0.0005 and seq > evalEntropyThresholdFrames and imStability) {
        isTriggered = true;

    }



    //printf("Entropy of %s is %f\n", imageFile.c_str(), singleEntropy);

    return 0;

}



/* ************************************/


void SearchForBubbleInFrame( std::vector<std::string> SortedFileList, std::string imageDir, std::string eventSeq, std::vector<cv::RotatedRect>& BubblePixelPos, int& trigFrame, int camera, cv::Mat& workingFrame){
    //printf("FileListSize %d\n", FileList.size());
    bool imEntropyTrigger = false;
    bool notFound = false;
    bool imStability = true;
    std::vector<std::string>::size_type i=0;


    //static cv::Mat workingFrame;

    for (i = 0; i < SortedFileList.size(); i++) {
        //printf("%s ", SortedFileList[i].c_str());
        //std::cout<<"SListSize: "<<SortedFileList.size()<<"\n";

        /*Uncomment---------------*/
        ImageEntropyLauncher(workingFrame, imageDir, SortedFileList[i], i, imEntropyTrigger, imStability, camera);

        if  (imEntropyTrigger) {
            break;
        }

        if (not imStability) break;
    }


    if  (!imEntropyTrigger) {
        notFound = true;
        printf("\n**No bubbles were found. Manually check this folder**\n");
        //BubblePixelPos.push_back(cv::RotatedRect(cv::Point2f(0,0), cv::Size2f(0,0), 0));
        trigFrame = -1;
    }




    if (!notFound) {


        /****************************************************
         * Locate the Bubble Position
         *
         * This is not exactly an easy job
         * It was seen that the blue channel is very noisy
         * and the green channel is smoother and quieter
         * So, the green channel is taken instead of using
         * all 3 channels to make the grayscale image.
         *
        ****************************************************/

        /*The names of the evalImage and the refImage*/
        std::string srcImgPath = std::string(imageDir) + SortedFileList[i];
        //std::cout<<"Bubble Detected at: "<<srcImgPath<<" | Compare with: "<<cmpImgPath<<" Next Frame: "<<trigNextFramePath<<"\n";


        /*Debug flags again - to store IDed images*/
        //std::string imagesIDStoragePath = "checkrun/";

        //LocalizeOMatic(BubbleFrame, CmpFrame, trigNextFrame, BubblePixelPos, eventSeq.c_str(), imagesIDStoragePath);
        if (camera == 0){
            TPLLocalizer SearchTemplate(img_mask0, false);
            SearchTemplate.MarkBubbles(BubblePixelPos, workingFrame);
        } else if (camera == 1) {
            TPLLocalizer SearchTemplate(img_mask1, false);
            SearchTemplate.MarkBubbles(BubblePixelPos, workingFrame);
        }






         //******************************************

        //Zero-position fix
        trigFrame = i-1;
    }

}
/*Workaround because fermi grid is using old gcc*/
bool BubblePosZsort(cv::RotatedRect a, cv::RotatedRect b){return a.center.y<b.center.y;}
bool eventNameOrderSort(std::string a, std::string b){return std::stoi(a)<std::stoi(b);}

int main(int argc, char** argv)
{

    printf("This is AutoBub v2, the automatic unified bubble finder code for all chambers\n");

    if (argc < 3) {
        printf("Not enough parameters.\nUsage: abub <location of data> <run number> <directory for output file>\nEg: abub /coupp/data/30l-13/ 20140501_0 /home/coupp/recon/\n");
        printf("Note the trailing slashes.\n");
        return -1;
    }

    std::string dataLoc = argv[1];
    std::string run_number = argv[2];
    std::string out_dir = argv[3];

    std::string eventDir=dataLoc+run_number+"/";


    std::string abubOutFilename = out_dir+"abub2_"+run_number+".txt";
    const char* abub_out_file=abubOutFilename.c_str();
    FILE * pFile;
    pFile = fopen (abub_out_file,"w");
    fprintf(pFile, "%s\n", "Output of AutoBub v2 - the automatic unified bubble finder code by Pitam, using OpenCV");
    //run  ev  ibub  frame0  frame1  nbub0  nbub1  nbub  npix0  hori0  vert0  horivar0  vertvar0  npix1  hori1  vert1  horivar1  vertvar1

    //Original version of abub
    //fprintf(pFile, "%s\n", "run ev nbub_a ibub_a frame0_a nbub0_a frame1_a nbub1_a pixelx0_a pixely0_a smajdiam0_a smindiam0_a pixelx1_a pixely1_a smajdiam1_a smindiam1_a");

    // Hugh edit, to try to align with getBub (I know, I know)
    fprintf(pFile, "%s\n", "run ev ibub nbub frame0 nbub0 frame1 nbub1 hori0 vert0 smajdiam0_a smindiam0_a hori1 vert1 smajdiam1_a smindiam1_a");

    fprintf(pFile, "%s\n2\n\n\n", "%12s %5d %d %d %d %d %d %d %.02f %.02f %.02f %.02f %.02f %.02f %.02f %.02f");


    //std::string imageDir=eventDir+eventSeq+"/";


    /*Construct list of events*/
    std::vector<std::string> EventList;
    int* EVstatuscode = 0;
    GetEventDirLists(eventDir.c_str(), EventList, EVstatuscode);
    /*A sort is unnecessary at this level*/
    std::sort(EventList.begin(), EventList.end(), eventNameOrderSort);
    /*Event list is now constructed*/
    /***************************************/

    /*Memory allocations for file list vectors*/
    std::vector<std::string> FileList_cam0;
    std::vector<std::string> FileList_cam1;

    /* which kind of bgs to use */
    //bgs0 = new AdaptiveSelectiveBackgroundLearning;
    bgs0 = new MultiLayerBGS;
    bgs1 = new MultiLayerBGS;
    //bgs1 = new AdaptiveSelectiveBackgroundLearning;
    //bgs_special = new MultiLayerBGS;

    for (auto evi = 0; evi < EventList.size(); evi++) {
        std::string imageDir=eventDir+EventList[evi]+"/";
        //int eventSeq = std::stoi(EventList[evi]);
        int* statuscode = 0;
        int trigFrame_0, trigFrame_1;
        cv::Mat markedTriggerFrame0, markedTriggerFrame1;

        //std::cout<<" Image Dir: "<<imageDir<<std::endl; //debug

        /* ***************************
         * ***** Camera 0 Operations ******
         ********************************/

        //Generate File lists to process for this event
        GetFileLists(imageDir.c_str(), FileList_cam0, statuscode, "cam0image");
        std::sort(FileList_cam0.begin(), FileList_cam0.end(), sortFunc);
        //Memory to store the processed location
        std::vector<cv::RotatedRect> BubblePixelPos0;
        //Perform search
        SearchForBubbleInFrame(FileList_cam0, imageDir, EventList[evi], BubblePixelPos0, trigFrame_0, 0, markedTriggerFrame0);
        img_mask0.release();
        //img_bkgmodel0.release();

        //postprocess
        std::sort(BubblePixelPos0.begin(), BubblePixelPos0.end(), BubblePosZsort);

        int nbub_cam0 = BubblePixelPos0.size();





        /* ************************************
         ********* Camera 1 Operations ******
         **************************************/
        //Generate File lists to process for this event
        GetFileLists(imageDir.c_str(), FileList_cam1, statuscode, "cam1image");
        std::sort(FileList_cam1.begin(), FileList_cam1.end(), sortFunc);
        //Memory to store the processed locationstd::vector<cv::RotatedRect> BubblePixelPos1;
        std::vector<cv::RotatedRect> BubblePixelPos1;
        //Perform search
        SearchForBubbleInFrame(FileList_cam1, imageDir, EventList[evi], BubblePixelPos1, trigFrame_1, 1, markedTriggerFrame1);
        img_mask1.release();
        //img_bkgmodel1.release();

        //postprocess
        std::sort(BubblePixelPos1.begin(), BubblePixelPos1.end(), BubblePosZsort);
        //run ev nbub frame0 nbub0 frame1 nbub1 ibub pixelx pixely smajdiam smindiam pixelx pixely smajdiam smindiam

        int nbub_cam1 = BubblePixelPos1.size();

        //std::cout<<"Cent X: "<<BubblePixelPos1[0].center.x<<" | Cent Y: "<<BubblePixelPos1[0].center.y <<"\n";



        /* *************** Debug output pictures ************
        std::ostringstream cam1loc;
        cam1loc<<out_dir<<"1/"<<evi<<".png";
        cv::imwrite(cam1loc.str(), markedTriggerFrame0);

        std::ostringstream cam2loc;
        cam2loc<<out_dir<<"2/"<<evi<<".png";
        cv::imwrite(cam2loc.str(), markedTriggerFrame1);

        /* ***************************************************/


        /* *****************************************************
         * Common process involving decisions with both cameras
         *******************************************************/

        int nbubTotal = 0;

        /*If we see too many bubbles in a camera, then it is a bad trigger and set the total number of bubbles to the lesser of the two frames*/
        if ((nbub_cam0 > 3*nbub_cam1 or nbub_cam1 > 3*nbub_cam0) and (nbub_cam0!=0 and nbub_cam1!=0)){
            nbubTotal = nbub_cam0 <= nbub_cam1 ? nbub_cam0 : nbub_cam1;
        } else if(nbub_cam0==0 or nbub_cam1==0) {
            nbubTotal = (nbub_cam0 > 3 or nbub_cam1 > 3) ? 0 : std::max(nbub_cam0, nbub_cam1);
        } else {
            nbubTotal = nbub_cam0 >= nbub_cam1 ? nbub_cam0 : nbub_cam1;
        }

        /*IF it is a bad event, write that out --- To FIX into a proper method! *******/
        int print_nbubTotal=0;
        print_nbubTotal= (trigFrame_0 < 0 and trigFrame_1 < 0)?0:nbubTotal;



        //run ev nbub_a ibub_a frame0_a nbub0_a frame1_a nbub1_a pixelx0_a pixely0_a smajdiam0_a smindiam0_a pixelx1_a pixely1_a smajdiam1_a smindiam1_a


    /* ******** OUTPUT SECTION *************/

	/*Case where genuinely zero events were seen*/
	if (nbubTotal == 0){
		fprintf(pFile, "%s %s    %d %d     %d %d     %d %d   ", run_number.c_str(), EventList[evi].c_str(), 0, 0, -1, 0, -1, 0);
		fprintf(pFile, "%.02f %.02f %.02f %.02f   ",0.0,0.0,0.0,0.0);
		fprintf(pFile, "%.02f %.02f %.02f %.02f\n",0.0,0.0,0.0,0.0);
	}

	std::cout<<"Event: "<<EventList[evi]<<" | Camera 0: "<<nbub_cam0<<" Camera 1: "<<nbub_cam1<<" Total (after decisions): "<<print_nbubTotal<<"\n";

        for (auto j=0; j<nbubTotal; j++){


	  fprintf(pFile, "%s %s    %d %d     %d %d     %d %d   ", run_number.c_str(), EventList[evi].c_str(), j+1, print_nbubTotal, trigFrame_0, nbub_cam0, trigFrame_1,nbub_cam1);

                /*Camera 1*/
                if (j<nbub_cam0){
                        fprintf(pFile, "%.02f %.02f %.02f %.02f   ",BubblePixelPos0[j].center.x,  BubblePixelPos0[j].center.y,  BubblePixelPos0[j].size.width, BubblePixelPos0[j].size.height);
                } else {
                        fprintf(pFile, "%.02f %.02f %.02f %.02f   ",0.0,0.0,0.0,0.0);
                }
                /*camera 2*/
                if (j<nbub_cam1){
                        fprintf(pFile, "%.02f %.02f %.02f %.02f\n",BubblePixelPos1[j].center.x,  BubblePixelPos1[j].center.y,  BubblePixelPos1[j].size.width, BubblePixelPos1[j].size.height);
                } else{
                    fprintf(pFile, "%.02f %.02f %.02f %.02f\n",0.0,0.0,0.0,0.0);
                }

        }


        FileList_cam0.clear();
        FileList_cam1.clear();


    }

    fclose(pFile);

    printf("AutoBub done analyzing this run. Thank you.\n");


    return 0;

}

