// Statistical filtering.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programm
#include <iostream>
#include "opencv2/opencv.hpp"
#include "AxisEst.h"
#include "preprocessing.h"
#include "save_file.h"
#include "ransac.h"

int main()
{
    cv::Mat image; 
	int i = 0; 

    //Axis
    AxisEst axisest; 

    //preprocessing
    preprocessing preprocessing; 


	while (i < 100)
	{
        //Start timer for Computational time analysis
        auto start_time = std::chrono::high_resolution_clock::now();

        //Initialize data path for the Ultrasound images
        std::string path = "C:/Users/ohoff/Documents/Bachelorarbeit/Datensets/Pictures/liver/liv01_pos01_oth01/";
        path += "name"; 
        path += std::to_string(i);
        path = path + ".jpg";
        image = cv::imread(path);
        std::cout << "name" << i << std::endl; 

        //define Rotation angle for, angled insertion simulation
        double angle = 95;

        
        /////////Preprocessing/////////
        preprocessing.init(image, angle);
        cv::Mat process = preprocessing.process(); 
        //preprocessing.linedetection();

        //rotate the preprocessed image according to defined rotation angle
        cv::Mat rotation;
        cv::Point pt(image.cols / 2, image.rows / 2);
        rotation = cv::getRotationMatrix2D(pt, angle, 1.0);
        cv::warpAffine(image, image, rotation, cv::Size(image.cols, image.rows));
        

       
        
        /////////ROI identification/////////
        ransac algorithm1(process); 
        algorithm1.algorithm(); 

        //Define the ROI coordinates
        int y_intercept = algorithm1.ret_est_intercept(); 
        int y_end;
        int x_end;
        float slope = -algorithm1.ret_est_slope();
        int y_end_help = slope * process.cols - y_intercept;
        if (y_end_help >= 0)
        {
            y_end = 0;
            x_end = y_intercept / slope;
        }
        if (0 > y_end_help && y_end_help > -process.cols)
        {
            y_end = abs(slope * process.cols - y_intercept);
            x_end = process.cols;
        }
        if (y_end_help < -process.cols)
        {
            y_end = process.rows;
            x_end = abs((process.rows + y_intercept) / slope);

        }
        cv::Point2f beg(0, y_intercept);
        cv::Point2f end(x_end, y_end); 

        /////////Axis Estimation/////////
        axisest.initialize(process,image, beg, end, slope);
        axisest.estimateaxis();
        cv::Point2f final_tip=axisest.tip_return();
        double alg_est_angle = axisest.ret_angle(); 

        //Get final time for Computational time analysis
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;

        //Save data in txt document
        save_file file("name" +std::to_string(i), final_tip, elapsed, alg_est_angle);
        
        //Print results
        std::cout << elapsed.count() << std::endl;
        std::cout << "-----------------------------------" << std::endl; 
        image = axisest.ret_image(); 
        cv::circle(image, final_tip, 4, cv::Scalar(0, 0, 255), 2);
        cv::imshow("line", image); 
        cv::waitKey(1);

        i++;
	}
}
