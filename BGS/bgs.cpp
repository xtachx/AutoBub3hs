/*
This file is part of BGSLibrary.

BGSLibrary is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

BGSLibrary is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BGSLibrary.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <opencv2/opencv.hpp>


#include "methods/AdaptiveSelectiveBackgroundLearning.h"
#include "methods/DPZivkovicAGMMBGS.h"

int main(int argc, char **argv)
{
  std::cout << "Using OpenCV " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION << "." << CV_SUBMINOR_VERSION << std::endl;

  /* Background Subtraction Methods */
  IBGS *bgs;

  /*** Default Package Routines which cam be used***/
  bgs = new AdaptiveSelectiveBackgroundLearning;


  /*** Donovan Parks packages which work ***/
  //bgs = new DPZivkovicAGMMBGS;

  int frameNumber = 0;
  int key = 0;
  while(key != 'q')
  {
    std::stringstream ss;
    ss << frameNumber;
    std::string fileName = "./45/cam1image  " + ss.str() + ".bmp";
    std::cout << "reading " << fileName << std::endl;

    cv::Mat img_input = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);

    if (img_input.empty())
      break;

    cv::imshow("input", img_input);

    cv::Mat img_mask;
    cv::Mat img_bkgmodel;
    bgs->process(img_input, img_mask, img_bkgmodel); // by default, it shows automatically the foreground mask image

    //if(!img_mask.empty())
    //  cv::imshow("Foreground", img_mask);
    //  do something

    key = cvWaitKey(33);
    frameNumber++;
  }
  cvWaitKey(0);
  delete bgs;

  cvDestroyAllWindows();

  return 0;
}
