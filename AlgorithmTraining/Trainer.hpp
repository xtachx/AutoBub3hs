#ifndef TRAINER_HPP_INCLUDED
#define TRAINER_HPP_INCLUDED

#include <vector>
#include <string>

#include <opencv2/opencv.hpp>


class Trainer{

    private:
        void ParseAndSortFramesInFolder(std::string, std::string );

        std::vector<std::string> CameraFrames;


        int camera;
        int StatusCode;
        std::string EventDir;


    protected:


    public:

        /*direct access to the trained data*/
        cv::Mat TrainedAvgImage;
        cv::Mat TrainedSigmaImage;

        bool isLBPApplied = false;

        /*Directory structure required for the training set*/
        std::vector<std::string> EventList;

        /*which images int he snapshots are training images?*/
        std::vector<int> TrainingSequence {0, 1};

        /*Trainer instance initilized with the camera number*/
        Trainer(int,  std::vector<std::string>, std::string  );
        ~Trainer(void );

        /*Compute the mean and std*/
        void MakeAvgSigmaImage(bool );


};

/*Helper functions*/
bool frameSortFuncTrainer(std::string , std::string );


#endif // ANALYZERUNIT_HPP_INCLUDED
