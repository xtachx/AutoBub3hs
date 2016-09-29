#include <vector>
#include <string>
#include <dirent.h>
#include <iostream>


#include <opencv2/opencv.hpp>
#include "PICOFormatWriter.hpp"



//#include "ImageEntropyMethods/ImageEntropyMethods.hpp"
//#include "LBP/LBPUser.hpp"




OutputWriter::OutputWriter(std::string OutDir, std::string run_number)
{
    /*Give the properties required to make the object - the identifiers i.e. the camera number, and location*/
    this->OutputDir = OutDir;
    this->run_number = run_number;

    this->abubOutFilename = this->OutputDir+"abub3_"+this->run_number+".txt";
    //this->OutFile.open(this->abubOutFilename);

}

OutputWriter::~OutputWriter(void ) {}



/*! \brief Function writes headers for the output file
 *
 * \param void
 * \return void
 *
 * This function writes the header file for abub3 output
 */

void OutputWriter::writeHeader(void ){

    this->OutFile.open(this->abubOutFilename);
    this->OutFile<<"Output of AutoBub v3 - the automatic unified bubble finder code by Pitam, using OpenCV.\n";
    this->OutFile<<"run ev ibubimage TotalBub4CamImg camera frame0 hori vert smajdiam smindiam\n";
    this->OutFile<<"%12s %5d %d %d %d %d %d %d %.02f %.02f\n8\n\n\n";
    this->OutFile.close();
}

/*! \brief Function stages and sorts the Rects as they come from the bubble identifier if there was no error int he process
 *
 * \param std::vector<cv::Rect> BubbleData
 * \param int camera
 * \return void
 *
 * This function writes the header file for abub3 output
 */

void OutputWriter::stageCameraOutput(std::vector<cv::Rect> BubbleRectIn, int camera, int frame0, int event){

    int tempStatus;
    if (BubbleRectIn.size()==0) tempStatus = -1;
    else tempStatus = 0;

    BubbleData* thisBubbleData;
    if (camera==0) thisBubbleData = &this->BubbleData0;
    else if (camera==1) thisBubbleData = &this->BubbleData1;
    else if (camera==2) thisBubbleData = &this->BubbleData2;
    else if (camera==3) thisBubbleData = &this->BubbleData3;


    thisBubbleData->RectData = BubbleRectIn;
    thisBubbleData->StatusCode = tempStatus;
    thisBubbleData->frame0 = frame0;
    thisBubbleData->event = event;


}

/*! \brief Function stages error
 *
 * \param std::vector<cv::Rect> BubbleData
 * \param int camera
 * \return void
 *
 * This function writes the header file for abub3 output
 */


void OutputWriter::stageCameraOutputError(int camera, int error){

    if (camera==0) {
        this->BubbleData0.StatusCode = error;
    } else if (camera==1) {
        this->BubbleData1.StatusCode = error;
    } else if (camera==2) {
        this->BubbleData2.StatusCode = error;
    } else if (camera==3) {
        this->BubbleData3.StatusCode = error;
    }

}

OutputWriter::BubbleData::BubbleData(){};


void OutputWriter::formEachBubbleOutput(int camera, int &ibubImageStart, int nBubTotal){

    this->_StreamOutput.clear();

    BubbleData *workingData;

    if (camera==0) workingData = &this->BubbleData0;
    else if (camera==1) workingData = &this->BubbleData1;
    else if (camera==2) workingData = &this->BubbleData2;
    else if (camera==3) workingData = &this->BubbleData3;

    //int event;
    //int frame0=50;
    //run ev ibubimage TotalBub4CamImg camera frame0 hori vert smajdiam smindiam\n";
    if (workingData->StatusCode !=0) {
        this->_StreamOutput<<this->run_number<<" "<<workingData->event<<"    "<<0.0<<" "<<0.0<<"    "<<camera<<" "<<workingData->StatusCode<<"    "<<0.0<<" "<<0.0<<" "<<0.0<<" "<<0.0<<"\n";
    } else {
    /*Write all outputs here*/
        for (int i=0; i<workingData->RectData.size(); i++){
            //run ev iBubImage TotalBub4CamImage camera
            this->_StreamOutput<<this->run_number<<" "<<workingData->event<<"    "<<ibubImageStart+i<<" "<<nBubTotal<<"    "<<camera<<" ";
            //frame0
            this->_StreamOutput<<workingData->frame0<<"    ";
            //hori vert smajdiam smindiam
            float width=workingData->RectData[i].width;
            float height=workingData->RectData[i].height;
            float x = (float)workingData->RectData[i].x+width/2.0;
            float y = (float)workingData->RectData[i].y+height/2.0;
            this->_StreamOutput<<x<<" "<<y<<" "<<width<<" "<<height<<"\n";
        }

        ibubImageStart += workingData->RectData.size();

    }



}



void OutputWriter::writeCameraOutput(void){

    int ibubImageStart = 1;
    int nBubTotal = this->BubbleData0.RectData.size()+this->BubbleData1.RectData.size()+this->BubbleData2.RectData.size()+this->BubbleData3.RectData.size();

    this->formEachBubbleOutput(0, ibubImageStart, nBubTotal);
    this->formEachBubbleOutput(1, ibubImageStart, nBubTotal);
    this->formEachBubbleOutput(2, ibubImageStart, nBubTotal);
    this->formEachBubbleOutput(3, ibubImageStart, nBubTotal);

    this->OutFile.open(this->abubOutFilename, std::fstream::out | std::fstream::app);
    this->OutFile<<this->_StreamOutput.rdbuf();
    this->OutFile.close();
}


