#include <vector>
#include <string>
#include <dirent.h>
#include <iostream>


#include <opencv2/opencv.hpp>


#include "AnalyzerUnit.hpp"
#include "ImageEntropyMethods/ImageEntropyMethods.hpp"
#include "LBP/LBPUser.hpp"
#include "AlgorithmTraining/Trainer.hpp"




AnalyzerUnit::AnalyzerUnit(std::string EventID, std::string ImageDir, int CameraNumber)
{
    /*Give the properties required to make the object - the identifiers i.e. the camera number, and location*/
    this->ImageDir=ImageDir;
    this->CameraNumber=CameraNumber;
    this->EventID=EventID;

}

AnalyzerUnit::~AnalyzerUnit(void ){}



/*! \brief Parse and sort a list of triggers from an event for a specific camera
 *
 * \param searchPattern
 * \return this->CameraFrames
 *
 * This function makes a list of all the file names matching the trigger
 * from a certain camera in a file and then makes aa sorted
 * list of all the images that can then be processed one by one .
 */


void AnalyzerUnit::ParseAndSortFramesInFolder(std::string searchPattern )
{

    /*Function to Generate File Lists*/
    {
        DIR *dir  = opendir (this->ImageDir.c_str()) ;
        if (dir)
        {
            /* print all the files and directories within directory */
            struct dirent* hFile;
            while (( hFile = readdir( dir )) != NULL )
            {
                if ( !strcmp( hFile->d_name, "."  )) continue;
                if ( !strcmp( hFile->d_name, ".." )) continue;

                // in linux hidden files all start with '.'
                if  ( hFile->d_name[0] == '.' ) continue;

                // dirFile.name is the name of the file. Do whatever string comparison
                // you want here. Something like:
                if ( strstr( hFile->d_name, searchPattern.c_str() ))
                    //printf( "found an .bmp file: %s\n", hFile->d_name );
                    this->CameraFrames.push_back(std::string(hFile->d_name));
            }



            closedir (dir);

            std::sort(this->CameraFrames.begin(), this->CameraFrames.end(), frameSortFunc);

        }
        else
        {
            /* could not open directory */
            //perror ("");
            this->StatusCode = 1;
        }


    }

}

/*! \brief Find the trigger frame
 *
 * \return int this->TriggerFrame
 *
 * Function uses the Image Entropy algorithm to search for
 * and decide the frame where the bubble can be first seen. (Trigger frame)
 */


void AnalyzerUnit::FindTriggerFrame(void ){

//ImageEntropyLauncher(           workingFrame,             imageDir,     SortedFileList[i],                                        i,      imEntropyTrigger,     imStability, camera);
//int ImageEntropyLauncher(cv::Mat& workingFrame, cv::Mat& comparisonFrame, std::string imageDir, std::string imageFile, std::string comparisonFile, std::vector<std::string>::size_type &seq, bool &isTriggered, bool& imStability, int camera)
// ImageEntropyLauncher(workingFrame, cmpFrame, imageDir, SortedFileList[i], SortedFileList[0], i, imEntropyTrigger, imStability, camera);

    cv::Mat workingFrame, img_mask0, img_mask1;
    cv::Mat comparisonFrame;

    /*Static variable to store the threshold entropy WHERE USED??*/
    static float entropyThreshold = 0.0;
    /*Variable to store the current entropy in*/
    float singleEntropy;

    /*The reference image does not change, it is the first frame*/
    std::string refImg = this->ImageDir + this->CameraFrames[0];
    std::cout<<"Ref Image: "<<refImg<<"\n";
    comparisonFrame = cv::imread(refImg.c_str());

    /*Start by flagging that a bubble wasnt found, flag gets changed to 0 if all goes well*/
    this->TriggerFrameIdentificationStatus=2;

    for (int i = 1; i < this->CameraFrames.size(); i++) {

        /*The name and load to memory evalImage*/
        std::string evalImg = this->ImageDir + this->CameraFrames[i];
        workingFrame = cv::imread(evalImg.c_str());

        /*BackgroundSubtract*/
        img_mask0 = workingFrame - comparisonFrame;

        /*Find LBP and then calculate Entropy*/
        img_mask1 = lbpImage(img_mask0);
        singleEntropy = calculateEntropyFrame(img_mask1);

        /* ****************
         * Debug Point here
         * **************** */
        //debugShow(img_mask1);
        std::cout<<"Entropy of BkgSub image: "<<singleEntropy<<"\n";


        /*Calculate entropy of ROI*/
        if (i==1 and singleEntropy>5.0005){
            printf("************ ---> WARNING <--******************\n");
            printf("Entropy is massive - something has triggered at the very first frame \n");
            printf("Autobub Image analysis is meaningless on this data set. Manual check recommended\n");
            printf(" *** Autobub skip --> *** \n");
            this->TriggerFrameIdentificationStatus = 1;
            this->okToProceed=false;
            break;
            //exit (EXIT_FAILURE);
        }
        //std::cout<<"Frame Entropy: "<<singleEntropy<<std::endl;



        /*Nothing works better than manual entropy settings. :-(*/
        if (singleEntropy < 0.02 and i > this->minEvalFrameNumber) {
            this->TriggerFrameIdentificationStatus = 0;
            this->MatTrigFrame = i;
            break;
        }
    }


    if  (this->TriggerFrameIdentificationStatus==2) {
        printf("\n**No bubbles were found. Manually check this folder**\n");
        this->okToProceed=false;
        this->MatTrigFrame;
    }

}

/*Misc functions*/

/*! \brief Sorting function for camera frames
 *
 * To be used by std::sort for sorting files associated with
 * the camera frames. Not to be used otherwise.
 */

bool frameSortFunc(std::string i, std::string j)
{

    unsigned int sequence_i, camera_i;
    int got_i = sscanf(i.c_str(),  "cam%d_image%u.png",
                       &camera_i, &sequence_i
                      );


    assert(got_i == 2);

    unsigned int  sequence_j, camera_j;
    int got_j = sscanf(j.c_str(),  "cam%d_image%u.png",
                       &camera_j, &sequence_j
                      );
    assert(got_j == 2);

    return sequence_i < sequence_j;

}
