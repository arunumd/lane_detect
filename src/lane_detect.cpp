/*
 * lane_detect.cpp
 *
 *  Created on: May 24, 2018
 *      Author: arun
 */

#include <iostream>
#include <ostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <vector>
#include "lane_detect.hpp"
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
using namespace cv;
using namespace std;
// Ref: https://stackoverflow.com/questions/3024197/what-does-int-argc-char-argv-mean

//Hardcoding OpenCV minimum and maximum values for Hue, Saturation and Values slider
const int max_possible_h = 180, max_possible_s = 255, max_possible_v = 255;
int max_hue = max_possible_h, max_sat = max_possible_s, max_val = max_possible_v; //These are the maximum possible values
int min_hue = 0, min_sat = 0, min_val = 0; //These are the minimum possible values which are mandatory
const string color_adjust_window = "HSV Adjustment Window";

/**********************************************************************
 *                                                                    *
 *      Defining our callback functions for HSV threshold trackbar    *
 *                                                                    *
 **********************************************************************/
/* Trackbar functions Deactivated since we are not using interactive thresholding but hardcoding thresholds for yellow and white lanes
static void min_hue_f(int, void *)
{
    min_hue = min(max_hue-1, min_hue);
    setTrackbarPos("Low H", color_adjust_window, min_hue);
}
static void max_hue_f(int, void *)
{
    max_hue = max(max_hue, min_hue+1);
    setTrackbarPos("High H", color_adjust_window, max_hue);
}
static void min_sat_f(int, void *)
{
    min_sat = min(max_sat-1, min_sat);
    setTrackbarPos("Low S", color_adjust_window, min_sat);
}
static void max_sat_f(int, void *)
{
    max_sat = max(max_sat, min_sat+1);
    setTrackbarPos("High S", color_adjust_window, max_sat);
}
static void min_val_f(int, void *)
{
   min_val = min(max_val-1, min_val);
   setTrackbarPos("Low V", color_adjust_window, min_val);
}
static void max_val_f(int, void *)
{
   max_val = max(max_val, min_val+1);
   setTrackbarPos("High V", color_adjust_window, max_val);
}*/

void mouse_click(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if  ( event == EVENT_RBUTTONDOWN )
    {
        cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
}

int main(int argc, char *argv[]){
   /**************************************************************************
    *                                                                        *
    * Defining the Camera Parameters and Distortion coefficients obtained    *
    * from the given text file                                               *
    *                                                                        *
    **************************************************************************/
    string videolocation;
    string filestring;
    cv::Mat cleanimage, hsvimage, threshimage;
    // Hard-coding the Camera Parameters below
    cv::Mat cameraParameters = (cv::Mat_<double>(3,3) << 1.15422732e+03, 0.00000000e+00, 6.71627794e+02, 0.00000000e+00, 1.14818221e+03, 3.86046312e+02, 0.00000000e+00, 0.00000000e+00, 1.00000000e+00);
    // Hard-coding the distortion coefficients below
    cv::Mat distCoeffs = (cv::Mat_<double>(1,8) << -2.42565104e-01, -4.77893070e-02, -1.31388084e-03, -8.79107779e-05, 2.20573263e-02, 0, 0, 0);
    cv::Mat outputimages;
    /**************************************************************************
     *                                                                        *
     * Code for opening video file based on full path of file from user input.*
     * This code relies on boost::filesystem from the boost library for C++.  *
     * More information about boost library can be found on this website :    *
     * https://www.boost.org/doc/libs/1_67_0/libs/filesystem/doc/index.htm    *
     *                                                                        *
     **************************************************************************/
    //Lines 93 to 127 are only for user input of video filepath and file verification
    if (argc < 2){
        std::cout<<"You have not given the input video filepath in command-line arguments\nPlease provide the video filepath below:\n";
        std::getline (std::cin,videolocation);//User input prompt for file path
        fs::path p(videolocation);//Boost filesystem object for path
        filestring = p.string();
        while(!is_regular_file(p)){ //Boost filesystem function for checking if there is a real file in the mentioned path
            std::cout<<"You have not given a valid input video filepath in command-line arguments\nPlease provide the video filepath below:\n";
            std::getline (std::cin,videolocation);//User input prompt for file path
            fs::path p(videolocation);//Boost filesystem object for path
            filestring = p.string();
        }
    }
    else if (argc = 2){
        fs::path p(argv[1]);
        filestring = p.string();
        while(!is_regular_file(p)){ //Boost filesystem function for checking if there is a real file in the mentioned path
            std::cout<<"You have not given a valid input video filepath in command-line arguments\nPlease provide the video filepath below:\n";
            std::getline (std::cin,videolocation);//User input prompt for file path
            fs::path p(videolocation);//Boost filesystem object for path
            filestring = p.string();
        }
    }
    else{
        std::cout<<"The input file path cannot contain any empty spaces\nPlease enter input filepath in the following manner:";
        std::cout<<"Linux and Windows formats go here";
        std::getline (std::cin,videolocation);//User input prompt for file path
        fs::path p(videolocation);//Boost filesystem object for path
        filestring = p.string();
        while(!is_regular_file(p)){ //Boost filesystem function for checking if there is a real file in the mentioned path
            std::cout<<"You have not given a valid input video filepath in command-line arguments\nPlease provide the video filepath below:\n";
            std::getline (std::cin,videolocation);//User input prompt for file path
            fs::path p(videolocation);//Boost filesystem object for path
            filestring = p.string();
        }
    }


    //string filestring = videolocation.string();
    //Moving on to frame extraction from input video file which was obtained from path above
    VideoCapture videofile(filestring);
    std::cout<<filestring;

    if(!videofile.isOpened()){
        cout << "Error opening input video file" << endl;
        return -1;
      }

    namedWindow("Original Image",CV_WINDOW_AUTOSIZE);
    //resizeWindow("Original Image",500,250);
    //namedWindow(color_adjust_window,CV_WINDOW_AUTOSIZE);

    while(1){

        Mat frame;
        // Capture frame-by-frame
        videofile >> frame;

        // If the frame is empty, break immediately
        if (frame.empty())
          break;

        //Calling matrix of same size and type as our frame
        cv::Mat cleanimage = cv::Mat::zeros(frame.size(), frame.type());

        //Undistortion of video frames
        undistort(frame, cleanimage, cameraParameters, distCoeffs);

        Mat blurredimage = cleanimage.clone();
        GaussianBlur(cleanimage, blurredimage,Size(7,7),0,0);//Applying a Gaussian blur to reduce the noise

        //Concatenating multiple images
        //cv::hconcat(frame,cleanimage,outputimages);
        //imshow( "Concatenated Images", outputimages );
        //Converting BGR image to HSV image by using OpenCV inbuilt function
        cvtColor(blurredimage, hsvimage, COLOR_BGR2HSV);

        //Creating trackbars for HSV on the output frames for user control
        /* Interactive trackbar to set min and max values for HS and V below
        createTrackbar("Hue - Low", color_adjust_window, &min_hue, max_possible_h, min_hue_f);
        createTrackbar("Hue - High", color_adjust_window, &max_hue, max_possible_h, max_hue_f);
        createTrackbar("Saturation - Low", color_adjust_window, &min_sat, max_possible_s, min_sat_f);
        createTrackbar("Saturation - High", color_adjust_window, &max_sat, max_possible_s, max_sat_f);
        createTrackbar("Value - Low", color_adjust_window, &min_val, max_possible_v, min_val_f);
        createTrackbar("Value - High", color_adjust_window, &max_val, max_possible_v, max_val_f);
        */

        /****************************************
         *                                      *
         *        Detection of Yellow lane      *
         *                                      *
         ****************************************/

        // Creating a HSV color threshold mask for yellow lane
        cv::Mat yellow_mask/*Mask for yellow color*/, yellow_lane/*Yellow lane detected after bitwise_and*/;
        yellow_mask = cv::Mat::zeros(hsvimage.size(), hsvimage.type());
        yellow_lane = cv::Mat::zeros(hsvimage.size(), hsvimage.type());
        cv::Scalar yellow_min = cv::Scalar(18, 102, 204); //Minimum HSV range for yellow lane
        cv::Scalar yellow_max = cv::Scalar(25, 255, 255); //Maximum HSV range for yellow lane
        cv::inRange(hsvimage, yellow_min, yellow_max, yellow_mask); // Masking yellow color on the HSV image
        //cv::bitwise_and(hsvimage, hsvimage, yellow_mask, yellow_lane); // Detection of yellow lane on the HSV image

        /****************************************
        *                                       *
        *        Detection of White lane        *
        *                                       *
        *****************************************/

        // Creating a HSV color threshold mask for white lane
        cv::Mat white_mask /*Mask for white color*/, white_lane /*White lane detected after bitwise_and*/;
        white_mask = cv::Mat::zeros(hsvimage.size(), hsvimage.type());
        white_lane = cv::Mat::zeros(hsvimage.size(), hsvimage.type());
        cv::Scalar white_min = cv::Scalar(0, 0, 204);     //Minimum HSV range for white lane
        cv::Scalar white_max = cv::Scalar(255, 51, 255);  //Maximum HSV range for white lane
        cv::inRange(hsvimage, white_min, white_max, white_mask); // Masking white color on the HSV image
        //cv::bitwise_and(hsvimage, hsvimage, white_mask, white_lane); // Detection of white lane on the HSV image

        // Creating a HSV color threshold mask for white lane
        //Applying thresholded color values to our input video frames
        //inRange(hsvimage, Scalar(min_hue, min_sat, min_val), Scalar(max_hue, max_sat, max_val), threshimage);

        /*************************************************
         *                                               *
         * Combining White and Yellow lanes into one mask*
         *                                               *
         *************************************************/
        //cv::Mat yellow_and_white; //Matrix for yellow and white lanes detection
        //yellow_and_white = yellow_mask + white_mask; //Combining both the masks together

        // Display the resulting frame
        //imshow("Original Image",hsvimage);
        cv::Mat mask = white_mask | yellow_mask;
        cv::Mat colorlines;
        cvtColor(mask, colorlines, COLOR_GRAY2BGR);
        imshow("White & Yellow - Thresholded Lanes", mask);

        /**************************************************
         *      Edge detection for lanes using Canny      *
         **************************************************/
        cv::Mat edges, edge_map;
        Canny(mask, edges, 10, 20, 3);

        edge_map = Scalar::all(0);

        hsvimage.copyTo(edge_map, edges);

        imshow("Edges", edge_map);
        //Creating a matrix of zeros like hsvimage

        cv::Mat empty(hsvimage.rows, hsvimage.cols, CV_8U, Scalar(0));

        vector<Point> pts;
        pts.push_back(Point(522, 472));    //top-left
        pts.push_back(Point(795, 472));    //top-right
        pts.push_back(Point(1190, 684));   //bottom-right
        pts.push_back(Point(251, 684));    //bottom-left


        fillConvexPoly(empty, pts, 255);

        imshow("Polygonal region selected", empty);
        //setMouseCallback("White & Yellow - Thresholded Lanes", mouse_click, NULL);

        //Mat dummy(2, 4, CV_32FC1 );

        //Point2f inputpts[4], outputpts[4];

        //dummy = Mat::zeros(mask.rows, mask.cols, mask.type());


        /*
        //Hardcoding four vertices for region of interest with yellow and white lanes in Source image
        inputpts[0] = Point2f(522, 472); //Top-left
        inputpts[1] = Point2f(795, 472); //Top-Right
        inputpts[2] = Point2f(251, 684); //Bottom-Left
        inputpts[3] = Point2f(1190, 684); //Bottom-Right

        //Hardcoding four vertices for region of interest with yellow and white lanes in Wrapped image
        outputpts[0] = Point2f(251, 472); //Top-left
        outputpts[1] = Point2f(1190, 472); //Top-Right
        outputpts[2] = Point2f(251, 684); //Bottom-Left
        outputpts[3] = Point2f(1190, 684); //Bottom-Right

        */
        //Mat dummy = getPerspectiveTransform(inputpts, outputpts);

        //Mat output;

        //warpPerspective(mask, output, dummy, output.size(), INTER_LINEAR, BORDER_CONSTANT);

        //imshow("Wrapped Image", output);

        //imshow("Yellow Lane", yellow_mask);
        //imshow("All lanes", yellow_and_white);

        //imshow(color_adjust_window, threshimage);

        /**********************************************************
         *                                                        *
         *              Hough Lines detection steps               *
         *                                                        *
         **********************************************************/
        vector<Vec2f> lines; //Hough lines
        HoughLines(mask, lines, 1, CV_PI/180, 150, 0, 0);
        //Drawing lines
        for( size_t i = 0; i < lines.size(); i++ )
        {
            float rho = lines[i][0], theta = lines[i][1];
            Point pt1, pt2;
            double a = cos(theta), b = sin(theta);
            double x0 = a*rho, y0 = b*rho;
            pt1.x = cvRound(x0 + 1000*(-b));
            pt1.y = cvRound(y0 + 1000*(a));
            pt2.x = cvRound(x0 - 1000*(-b));
            pt2.y = cvRound(y0 - 1000*(a));
            line( colorlines, pt1, pt2, Scalar(0,0,255), 3, LINE_AA);
        }

        //imshow("Hough Lines", colorlines);
        // Press  ESC on keyboard to exit
        char c=(char)waitKey(0);
        if(c==27)
          break;
      }

    // When everything done, release the video capture object
    videofile.release();

    // Closes all the frames
    destroyAllWindows();

    return 0;
}

