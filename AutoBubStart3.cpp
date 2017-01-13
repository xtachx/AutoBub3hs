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
#include <stdexcept>

/*Geyser Image Analysis Stuff*/
#include "ParseFolder/ParseFolder.hpp"
//#include "common/CommonDatatypes.h"
//#include "SQLiteDBstorage/imageAnalysisResults.hpp"
#include "BubbleLocalizer/L3Localizer.hpp"
#include "LBP/lbp.hpp"
#include "LBP/LBPUser.hpp"

#include "AnalyzerUnit.hpp"
#include "AlgorithmTraining/Trainer.hpp"
#include "PICOFormatWriter/PICOFormatWriterV3.hpp"
#include "bubble/bubble.hpp"
#include "common/UtilityFunctions.hpp"


const int evalEntropyThresholdFrames = 2;
std::vector<int> badEvents;





/*Workaround because fermi grid is using old gcc*/
bool BubblePosZsort(cv::RotatedRect a, cv::RotatedRect b)
{
    return a.center.y<b.center.y;
}
bool eventNameOrderSort(std::string a, std::string b)
{
    return std::stoi(a)<std::stoi(b);
}


/*The main autobub code starts here*/
int main(int argc, char** argv)
{

    printf("This is AutoBub v3, the automatic unified bubble finder code for all chambers\n");

    if (argc < 3)
    {
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

    try
    {
        GetEventDirLists(eventDir.c_str(), EventList, EVstatuscode);

    /*Crash handler at the begining of the program - writes -5 if the folder could not be read*/
    }
    catch (...)
    {
        std::cout<<"Failed to read the images from the run. Autobub cannot continue.\n";
        PICO60Output->stageCameraOutputError(0,-5, -1);
        PICO60Output->stageCameraOutputError(1,-5, -1);
        PICO60Output->stageCameraOutputError(2,-5, -1);
        PICO60Output->stageCameraOutputError(3,-5, -1);
        PICO60Output->writeCameraOutput();
        return -5;
    }

    /*A sort is unnecessary at this level, but it is good practice and does not cost extra resources*/
    std::sort(EventList.begin(), EventList.end(), eventNameOrderSort);
    /*Event list is now constructed*/


    /*Learn Mode
     *Train on a given set of images for background subtract
     */
    printf("**Starting training. AutoBub is in learn mode**\n");

    Trainer *TrainC0 = new Trainer(0, EventList, eventDir);
    Trainer *TrainC1 = new Trainer(1, EventList, eventDir);
    Trainer *TrainC2 = new Trainer(2, EventList, eventDir);
    Trainer *TrainC3 = new Trainer(3, EventList, eventDir);



    try {
        TrainC0->MakeAvgSigmaImage(false);
        TrainC1->MakeAvgSigmaImage(false);
        TrainC2->MakeAvgSigmaImage(false);
        TrainC3->MakeAvgSigmaImage(false);

    } catch (...) {
        std::cout<<"Failed to train on images from the run. Autobub cannot continue.\n";
        PICO60Output->stageCameraOutputError(0,-7, -1);
        PICO60Output->stageCameraOutputError(1,-7, -1);
        PICO60Output->stageCameraOutputError(2,-7, -1);
        PICO60Output->stageCameraOutputError(3,-7, -1);
        PICO60Output->writeCameraOutput();
        return -7;
    }



    printf("**Training complete. AutoBub is now in detect mode**\n");


    /*Detect mode
     *Iterate through all the events in the list and detect bubbles in them one by one
     *A seprate procedure will store them to a file at the end
     */

    for (int evi = 0; evi < EventList.size(); evi++)
    {
        std::string imageDir=eventDir+EventList[evi]+"/Images/";
        //std::cout<<"Processing event: "<<EventList[evi]<<" / "<<EventList.size()<<"\n";
        printf("\rProcessing event: %s / %d  ... ", EventList[evi].c_str(), EventList.size()-1);
        AnalyzerUnit *AnalyzerC0 = new L3Localizer(EventList[evi], imageDir, 0, true, &TrainC0); /*EventID, imageDir and camera number*/
        AnalyzerUnit *AnalyzerC1 = new L3Localizer(EventList[evi], imageDir, 1, true, &TrainC1); /*EventID, imageDir and camera number*/
        AnalyzerUnit *AnalyzerC2 = new L3Localizer(EventList[evi], imageDir, 2, true, &TrainC2); /*EventID, imageDir and camera number*/
        AnalyzerUnit *AnalyzerC3 = new L3Localizer(EventList[evi], imageDir, 3, true, &TrainC3); /*EventID, imageDir and camera number*/

        /*We need the actual event number in case folders with events are missing*/
        int actualEventNumber = atoi(EventList[evi].c_str());

        /*Fancy coursors!*/
        advance_cursor();


        /* ***************************
         * ***** Camera 0 Operations ******
         ********************************/

        /*Exception handling - per camera*/
        try
        {
            AnalyzerC0->ParseAndSortFramesInFolder();
            AnalyzerC0->FindTriggerFrame();
            //cout<<"Trigger Frame: "<<AnalyzerC0->MatTrigFrame<<"\n";
            if (AnalyzerC0->okToProceed)
            {
                AnalyzerC0->LocalizeOMatic(out_dir);  //uncomment for full run
                if (AnalyzerC0->okToProceed) PICO60Output->stageCameraOutput(AnalyzerC0->BubbleList,0, AnalyzerC0->MatTrigFrame, actualEventNumber);
                else PICO60Output->stageCameraOutputError(0,-8, actualEventNumber);
            }
            else
            {
                PICO60Output->stageCameraOutputError(0,AnalyzerC0->TriggerFrameIdentificationStatus, actualEventNumber);
            }

        /*The exception block for camera 0 specific crashes. outputs -6 for the error*/
        }
        catch (...)
        {
            PICO60Output->stageCameraOutputError(0,-6, actualEventNumber);
        }

        /*Fancy coursors!*/
        advance_cursor();

        /* ***************************
         * ***** Camera 1 Operations ******
         ********************************/
        try
        {
            AnalyzerC1->ParseAndSortFramesInFolder();
            AnalyzerC1->FindTriggerFrame();

            if (AnalyzerC1->okToProceed)
            {
                AnalyzerC1->LocalizeOMatic(out_dir);  //uncomment for full run
                if (AnalyzerC1->okToProceed) PICO60Output->stageCameraOutput(AnalyzerC1->BubbleList,1, AnalyzerC1->MatTrigFrame, actualEventNumber);
                else PICO60Output->stageCameraOutputError(0,-8, actualEventNumber);
            }
            else
            {
                PICO60Output->stageCameraOutputError(1,AnalyzerC1->TriggerFrameIdentificationStatus, actualEventNumber);
            }

        /*The exception block for camera 1 specific crashes. outputs -6 for the error*/
        }
        catch (...)
        {
            PICO60Output->stageCameraOutputError(1,-6, actualEventNumber);
        }
        /*Fancy coursors!*/
        advance_cursor();

        /* ***************************
         * ***** Camera 2 Operations ******
         ********************************/
        try
        {
            AnalyzerC2->ParseAndSortFramesInFolder();
            AnalyzerC2->FindTriggerFrame();
            //cout<<"Trigger Frame: "<<AnalyzerC2->MatTrigFrame<<"\n";
            if (AnalyzerC2->okToProceed)
            {
                AnalyzerC2->LocalizeOMatic(out_dir);
                if (AnalyzerC2->okToProceed) PICO60Output->stageCameraOutput(AnalyzerC2->BubbleList,2, AnalyzerC2->MatTrigFrame, actualEventNumber);
                else PICO60Output->stageCameraOutputError(0,-8, actualEventNumber);
            }
            else
            {
                PICO60Output->stageCameraOutputError(2,AnalyzerC2->TriggerFrameIdentificationStatus, actualEventNumber);
            }

        /*The exception block for camera 2 specific crashes. outputs -6 for the error*/
        }
        catch (...)
        {
            PICO60Output->stageCameraOutputError(2,-6, actualEventNumber);
        }

        /*Fancy coursors!*/
        advance_cursor();

        /* ***************************
         * ***** Camera 3 Operations ******
         ********************************/
        try
        {

            AnalyzerC3->ParseAndSortFramesInFolder();
            AnalyzerC3->FindTriggerFrame();
            //cout<<"Trigger Frame: "<<AnalyzerC3->MatTrigFrame<<"\n";
            if (AnalyzerC3->okToProceed)
            {
                AnalyzerC3->LocalizeOMatic(out_dir);  //uncomment for full run
                if (AnalyzerC3->okToProceed) PICO60Output->stageCameraOutput(AnalyzerC3->BubbleList,3, AnalyzerC3->MatTrigFrame, actualEventNumber);
                else PICO60Output->stageCameraOutputError(0,-8, actualEventNumber);
            }
            else
            {
                PICO60Output->stageCameraOutputError(3,AnalyzerC3->TriggerFrameIdentificationStatus, actualEventNumber);
            }

        /*The exception block for camera 3 specific crashes. outputs -6 for the error*/
        }
        catch (...)
        {
            PICO60Output->stageCameraOutputError(3,-6, actualEventNumber);
        }

        /*Fancy coursors!*/
        advance_cursor();

        /*Write and commit output after each iteration, so in the event of a crash, its not lost*/
        PICO60Output->writeCameraOutput();
        delete AnalyzerC0;
        delete AnalyzerC1;
        delete AnalyzerC2;
        delete AnalyzerC3;
    }

    printf("run complete.\n");

    /*GC*/
    delete TrainC0;
    delete TrainC1;
    delete TrainC2;
    delete TrainC3;
    delete PICO60Output;


    printf("AutoBub done analyzing this run. Thank you.\n");
    return 0;

}

