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
#include "PICOFormatWriter/PICOFormatWriter.hpp"


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


    /*I anticipate the object to become large with many bubbles, so I wanted it on the heap*/
    OutputWriter *PICO60Output = new OutputWriter(out_dir, run_number);
    PICO60Output->writeHeader();

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
    Trainer *TrainC0 = new Trainer(0, EventList, eventDir);
    TrainC0->MakeAvgSigmaImage(false);

    Trainer *TrainC1 = new Trainer(1, EventList, eventDir);
    TrainC1->MakeAvgSigmaImage(false);

    Trainer *TrainC2 = new Trainer(2, EventList, eventDir);
    TrainC2->MakeAvgSigmaImage(false);

    Trainer *TrainC3 = new Trainer(3, EventList, eventDir);
    TrainC3->MakeAvgSigmaImage(false);

    /*Detect mode
     *Iterate through all the events in the list and detect bubbles in them one by one
     *A seprate procedure will store them to a file at the end
     */

    for (int evi = 0; evi < EventList.size(); evi++) {
        std::string imageDir=eventDir+EventList[evi]+"/Images/";
        std::cout<<"Processing event: "<<evi<<" / "<<EventList.size()<<"\n";
        AnalyzerUnit *AnalyzerC0 = new L3Localizer(EventList[evi], imageDir, 0, false, &TrainC0); /*EventID, imageDir and camera number*/
        AnalyzerUnit *AnalyzerC1 = new L3Localizer(EventList[evi], imageDir, 1, false, &TrainC1); /*EventID, imageDir and camera number*/
        AnalyzerUnit *AnalyzerC2 = new L3Localizer(EventList[evi], imageDir, 2, false, &TrainC2); /*EventID, imageDir and camera number*/
        AnalyzerUnit *AnalyzerC3 = new L3Localizer(EventList[evi], imageDir, 3, false, &TrainC3); /*EventID, imageDir and camera number*/




        int actualEventNumber = atoi(EventList[evi].c_str());
        /* ***************************
         * ***** Camera 0 Operations ******
         ********************************/

        //Generate File lists to process for this event
        AnalyzerC0->ParseAndSortFramesInFolder();
        AnalyzerC0->FindTriggerFrame();
        //cout<<"Trigger Frame: "<<AnalyzerC0->MatTrigFrame<<"\n";
        AnalyzerC0->LocalizeOMatic(out_dir);
        PICO60Output->stageCameraOutput(AnalyzerC0->bubbleRects,0, AnalyzerC0->MatTrigFrame, actualEventNumber);


        /* ***************************
         * ***** Camera 1 Operations ******
         ********************************/

        //Generate File lists to process for this event
        AnalyzerC1->ParseAndSortFramesInFolder();
        AnalyzerC1->FindTriggerFrame();
        //cout<<"Trigger Frame: "<<AnalyzerC1->MatTrigFrame<<"\n";
        AnalyzerC1->LocalizeOMatic(out_dir);
        PICO60Output->stageCameraOutput(AnalyzerC1->bubbleRects,1, AnalyzerC1->MatTrigFrame, actualEventNumber);



        /* ***************************
         * ***** Camera 2 Operations ******
         ********************************/

        //Generate File lists to process for this event
        AnalyzerC2->ParseAndSortFramesInFolder();
        AnalyzerC2->FindTriggerFrame();
        //cout<<"Trigger Frame: "<<AnalyzerC2->MatTrigFrame<<"\n";
        AnalyzerC2->LocalizeOMatic(out_dir);
        PICO60Output->stageCameraOutput(AnalyzerC2->bubbleRects,2, AnalyzerC2->MatTrigFrame, actualEventNumber);


        /* ***************************
         * ***** Camera 3 Operations ******
         ********************************/

        //Generate File lists to process for this event
        AnalyzerC3->ParseAndSortFramesInFolder();
        AnalyzerC3->FindTriggerFrame();
        //cout<<"Trigger Frame: "<<AnalyzerC3->MatTrigFrame<<"\n";
        AnalyzerC3->LocalizeOMatic(out_dir);
        PICO60Output->stageCameraOutput(AnalyzerC3->bubbleRects,3, AnalyzerC3->MatTrigFrame, actualEventNumber);





        PICO60Output->writeCameraOutput();
        delete AnalyzerC0;
        delete AnalyzerC1;
        delete AnalyzerC2;
        delete AnalyzerC3;
    }

    /*Write staged output*/


    /*GC*/
    delete TrainC0;
    delete TrainC1;
    delete TrainC2;
    delete TrainC3;
    delete PICO60Output;


    printf("AutoBub done analyzing this run. Thank you.\n");
    return 0;

}

