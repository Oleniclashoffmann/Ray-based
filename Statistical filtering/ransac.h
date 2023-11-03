#pragma once
#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include <numeric>

class ransac
{
public:
	ransac(const cv::Mat src);
	void algorithm();
	float ret_est_slope() { return estimatedSlope; };
	float ret_est_intercept() { return estimatedIntercept; };


private:

	int m_k = 700; //Maximum iterations allowed 
	int m_n = 10;  //Minimum number of data points to estimate parameters 
	float m_t = 3.0;  //Threshold value to determine if points are fit well 
	int m_d = 10;  //Number of close data points required to assert model 

	float m_slope;

	float estimatedSlope, estimatedIntercept;

	cv::Mat m_src, m_original, col_image;

	std::vector<int> data_x;
	std::vector<int> data_y;

	std::vector<int> ran_datax;
	std::vector<int> ran_datay;

	std::vector<int> confirmed_inlierx;
	std::vector<int> confirmed_inliery;

	std::vector<int> estimatedx;
	std::vector<int> estimatedy;


	cv::Point starting;
	cv::Point ending;

	void create_data();
	void linear_regression(float& a, float& b);

	int m_angle;

};

