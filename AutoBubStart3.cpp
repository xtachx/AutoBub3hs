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
#include "ParseFolder/ParseFolder.hpp"
//#include "common/CommonDatatypes.h"
//#include "SQLiteDBstorage/imageAnalysisResults.hpp"
#include "BubbleLocalizer/L3Localizer.hpp"
#include "LBP/lbp.hpp"
#include "LBP/LBPUser.hpp"

#include "AnalyzerUnit.hpp"
#include "AlgorithmTraining/Trainer.hpp"



/*Forward declaration of debugShow*/
//void debugShow(cv::Mat&);

const int evalEntropyThresholdFrames = 2;
std::vector<int> badEvents;





/* ************************************/



/*Workaround because fermi grid is using old gcc*/
bool BubblePosZsort(cv::RotatedRect a, cv::RotatedRect b){return a.center.y<b.center.y;}
bool eventNameOrderSort(std::string a, std::string b){return std::stoi(a)<std::stoi(b);}

int main(int argc, char** argv)
{

    printf("This is AutoBub v3, the automatic unified bubble finder code for all chambers\n");

    if (argc < 3) {
        printf("Not enough parameters.\nUsage: abub <location of data> <run number> <directory for output file>\nEg: abub /coupp/data/30l-13/ 20140501_0 /home/coupp/recon/\n");
        printf("Note the trailing slashes.\n");
        return -1;
    }

    std::string dataLoc = argv[1];
    std::string run_number = argv[2];
    std::string out_dir = argv[3];

    std::string eventDir=dataLoc+run_number+"/";

    FILE * pFile;
    std::string abubOutFilename = out_dir+"abub3_"+run_number+".txt";
    pFile = fopen (abubOutFilename.c_str(),"w");
    fprintf(pFile, "%s\n", "Output of AutoBub v3 - the automatic unified bubble finder code by Pitam, using OpenCV.");

    /*The output needs to be in the PICO output format, that goes as
     *run  ev  ibub  frame0  frame1  nbub0  nbub1  nbub  npix0  hori0  vert0  horivar0  vertvar0  npix1  hori1  vert1  horivar1  vertvar1
     */

    // Hugh edit, to try to align with getBub (I know, I know)
    fprintf(pFile, "%s\n", "run ev ibub nbub frame0 nbub0 frame1 nbub1 hori0 vert0 smajdiam0_a smindiam0_a hori1 vert1 smajdiam1_a smindiam1_a");
    fprintf(pFile, "%s\n2\n\n\n", "%12s %5d %d %d %d %d %d %d %.02f %.02f %.02f %.02f %.02f %.02f %.02f %.02f");


    /*Construct list of events*/
    std::vector<std::string> EventList;
    int* EVstatuscode = 0;
    GetEventDirLists(eventDir.c_str(), EventList, EVstatuscode);

    /*A sort is unnecessary at this level, but it is good practice and does not cost extra resources*/
    std::sort(EventList.begin(), EventList.end(), eventNameOrderSort);
    /*Event list is now constructed*/
    /* ************************************* */



    /*Learn Mode
     *Train on a given set of images for background subtract
     */
    printf("Eventlist 1: %s\n",EventList[0].c_str());
    Trainer *TrainC0 = new Trainer(0, EventList, eventDir);
    TrainC0->MakeAvgSigmaImage();


    /*Detect mode
     *Iterate through all the events in the list and detect bubbles in them one by one
     *A seprate procedure will store them to a file at the end
     */

    for (auto evi = 0; evi < EventList.size(); evi++) {
        std::string imageDir=eventDir+EventList[evi]+"/Images/";
        std::cout<<"Processing: "<<"\n";
        AnalyzerUnit *AnalyzerC0 = new L3Localizer(EventList[evi], imageDir, 0, false); /*EventID, imageDir and camera number*/



        /* ***************************
         * ***** Camera 0 Operations ******
         ********************************/

        //Generate File lists to process for this event
        AnalyzerC0->ParseAndSortFramesInFolder("cam0_image");
        AnalyzerC0->FindTriggerFrame();
        cout<<"Trigger Frame: "<<AnalyzerC0->MatTrigFrame<<"\n";
        AnalyzerC0->LocalizeOMatic("checkrun/");



//        /* ************************************
//         ********* Camera 1 Operations ******
//         **************************************/
//        //Generate File lists to process for this event
//        GetFileLists(imageDir.c_str(), FileList_cam1, statuscode, "cam1image");
//        std::sort(FileList_cam1.begin(), FileList_cam1.end(), sortFunc);
//        //Memory to store the processed locationstd::vector<cv::RotatedRect> BubblePixelPos1;
//        std::vector<cv::RotatedRect> BubblePixelPos1;
//        //Perform search
//        SearchForBubbleInFrame(FileList_cam1, imageDir, EventList[evi], BubblePixelPos1, trigFrame_1, 1, markedTriggerFrame1);
//        img_mask1.release();
//        //img_bkgmodel1.release();
//
//        //postprocess
//        std::sort(BubblePixelPos1.begin(), BubblePixelPos1.end(), BubblePosZsort);
//        //run ev nbub frame0 nbub0 frame1 nbub1 ibub pixelx pixely smajdiam smindiam pixelx pixely smajdiam smindiam
//
//        int nbub_cam1 = BubblePixelPos1.size();
//
//
//        /* Debug code to check the bubble output ont he fly ******
//        if (nbub_cam0 != 0){
//            for (int i=0; i<nbub_cam0; i++)
//                std::cout<<"C0 Cent X: "<<BubblePixelPos0[i].center.x<<" | Cent Y: "<<BubblePixelPos0[i].center.y <<"\n";
//        }
//
//        if (nbub_cam1 != 0){
//            for (int i=0; i<nbub_cam1; i++)
//                std::cout<<"C1 Cent X: "<<BubblePixelPos1[i].center.x<<" | Cent Y: "<<BubblePixelPos1[i].center.y <<"\n";
//        }
//
//        /* ************** */
//
//
//        /* *************** Debug output pictures ************/
//        std::ostringstream cam1loc;
//        cam1loc<<out_dir<<"1/"<<evi<<".png";
//        cv::imwrite(cam1loc.str(), markedTriggerFrame0);
//
//        std::ostringstream cam2loc;
//        cam2loc<<out_dir<<"2/"<<evi<<".png";
//        cv::imwrite(cam2loc.str(), markedTriggerFrame1);
//
//        /* ***************************************************/
//



//       delete AnalyzerC0;
    }

    /*GC*/
    delete TrainC0;


    fclose(pFile);
    printf("AutoBub done analyzing this run. Thank you.\n");
    return 0;

}

