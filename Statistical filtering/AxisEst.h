#pragma once
#include <iostream>
#include "opencv2/opencv.hpp"


class AxisEst
{
public:
	void initialize(cv::Mat src, cv::Mat original1, cv::Point2f beg, cv::Point2f end, float angle); 
	void estimateaxis(); 
	void axissearchstep(double angle_axis);
	void needleTipest(); 
	void Tipdetect(); 
	cv::Mat ret_image() { return m_original; };
	cv::Point2f tip_return() { return est_needleTip;  };
	double ret_angle() { return m_angle; }; 
private: 
	cv::Mat m_src; 
	cv::Mat original; 
	cv::Mat m_original;
	
	//ROugh Approximation of needle axis
	cv::Point2f pt1; 
	cv::Point2f pt2; 
	int s = 4; //range around ray iteration
	int b = 4; //Range around max_position
	int ROIheight = 100; //Height of ROI depends on Needle diameter
	int polynomialregrssion(std::vector<cv::Point2f>& needle); 
	std::vector<cv::Point2f> needlepoints;
	int w_max = 30; //for calculation of score value
	float m_slope;
	cv::Point2f needleTip; 
	cv::Point2f est_needleTip; 
	cv::Point2f final_tip; 

	double m_angle; 
};


