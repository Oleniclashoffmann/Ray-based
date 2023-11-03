#include "AxisEst.h"

//Initialize function that initializes images and ROI information for each image
void AxisEst::initialize(cv::Mat src, cv::Mat original1,cv::Point2f beg, cv::Point2f end, float angle)
{
	m_src = src.clone(); 
	m_original = original1.clone(); 
	

	pt1 = beg; 
	pt2 = end; 
	needleTip = end; 
	m_slope = angle; 

}

void AxisEst::estimateaxis()
{
	axissearchstep(m_slope); 
}

//Main function that is called from main file
void AxisEst::axissearchstep(double slope)
{
	cv::Mat canvas = m_src.clone(); 
	cv::cvtColor(canvas, canvas, cv::COLOR_GRAY2BGR); 

	//define ROI using angle and Needle Position  
	cv::Point2f center((pt1.x + pt2.x)/2, (pt1.y + pt2.y)/2);
	cv::circle(canvas, center, 3, cv::Scalar(255, 0, 0), -1); 
	double length_rect = 440; //length of rect
	double angle_rad = atan(slope);

	// Convert the angle to degrees
	double angle_deg = -(angle_rad * (180.0 / CV_PI));
	
	// Calculate the four points of the rectangle
	cv::RotatedRect roi(center, cv::Size2f(length_rect, ROIheight), angle_deg);

	//define Rays
	cv::Point2f vertices[4];
	roi.points(vertices);

	// Draw the rotated rectangle on the canvas
	for (int i = 0; i < 4; i++)
	{
		cv::line(canvas, vertices[i], vertices[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
	}

	//Create Rays, perpendicular to needle axis
	float stepSize = 5;
	float x_2 = vertices[1].x;
	std::vector<std::pair<cv::Point2f, cv::Point2f>> rays;
	cv::LineIterator it_bottom(vertices[0], vertices[3], 8); 
	cv::LineIterator it_top(vertices[1], vertices[2], 8); 

	//Draw rays and save rays to vector
	for (int i = 0; i < it_bottom.count; i++, it_bottom++, it_top++)
	{
		if (i % 10 == 0)//defines distance of each ray to each other
		{
			cv::Point2f start(it_bottom.pos());
			cv::Point2f end(it_top.pos()); 
			cv::line(canvas, start, end, cv::Scalar(0, 0, 150), 1, 4, 0);
			rays.push_back(std::make_pair(start, end));
		}
	}
	
	//Define vectors for score calculation
	std::vector<std::pair<int, std::vector<double>>> S_pair; 
	std::vector<double> S_rk_y;
	int i = 0; 
	int bin_size = 0; 

	//iterate through all rays
	for (const auto& ray : rays)
	{
		//line iterator for each ray
		cv::LineIterator it(ray.first, ray.second, 8); 

		std::vector<double> R; 
		std::vector<double> R_der; 
		int totalpixels = 0;
		bin_size = 0; 

		//iterate alongside each ray
		for (int y = 0; y < it.count; y++, it++)
		{
			R.push_back(0); 

			//iterate in predefined region s (not just one line along ray put area of +/-s around ray)
			for (int i = it.pos().x - s; i < it.pos().x + s; i++)
			{
				if (i < m_src.cols && i >= 0 && it.pos().y < m_src.rows && it.pos().y >= 0)
				{
					//Add all pixel values up at respective y position 
					uchar pixelval_char = m_src.at<float>(it.pos().y, i);
					int pixelval = static_cast<int>(pixelval_char);
					R[y] += pixelval;
					totalpixels += 1; 
				}
			}

			//devide pixel values through totalpixels
			R[y] /= totalpixels; 
			bin_size++; 
		}

		//Use derivative of pixel values to find contrast differences in intensity levels
		for (int y = 0; y < it.count; y++)
		{
			if (y + 1 < it.count)
			{
				R_der.push_back(R[y + 1] - R[y]);
			}
			else {
				R_der.push_back(0);
			}
		}

		//Claculate score value described in Bachelor thesis using derivative 
		for (int y = 0; y < it.count; y++)
		{
			double S_lead = 0; 
			double S_lagg = 0; 
			
			for (int w = 0; w < w_max; w++)
			{
				if (y + w < it.count) 
				{
					S_lead = R_der[y + w] + S_lead;
				}
			}
			for (int w = 0; w < w_max; w++)
			{
				if (y - w - 1 >= 0)
				{
					S_lagg = R_der[y - w - 1] + S_lagg;
				}
			}
			S_rk_y.push_back(S_lagg - S_lead - abs(S_lagg + S_lead));
		}
		
		//Save values in vector with ray and score value 
		S_pair.push_back(std::make_pair(i, S_rk_y)); 

		S_rk_y.clear(); 
		i++; 
	}

	std::vector<double> H_b(bin_size); 
	int iterator = 0; 
	 
	//Create Histogram
	for (const auto &ray : rays)
	{
		cv::LineIterator it(ray.first, ray.second, 8);
		for (int y = 0; y < it.count; y++)
		{
			if(y < H_b.size() && iterator < S_pair.size())
				H_b[y] += S_pair[iterator].second[y]; 
		}
		iterator++; 
	}

	//Smooth Histogram
	std::vector<double> H_bS(bin_size);
	for (int y = 0; y < bin_size; y++)
	{
		if (y - 1 >= 0 && y + 1 < bin_size) 
		{
			H_bS[y] = H_b[y] + 1 / 2 * H_b[y - 1] + 1 / 2 * H_b[y + 1]; 
		}
		else
		{
			H_bS[y] = H_b[y]; 
		}
	}

	//Find max of H_bS
	auto max_element_iterator = std::max_element(H_bS.begin(), H_bS.end());
	std::size_t max_position = 0; 

	if (max_element_iterator != H_bS.end()) 
	{
		max_position = std::distance(H_bS.begin(), max_element_iterator);
	}

	//Find points that lie on the needle using S_pair
	iterator = 0; 
	int second_pos; 
	int first_pos; 
	std::vector<cv::Point2f> points; 
	cv::Point2f pk; 

	//Iterate thorugh all rays
	for (const auto& ray : rays)
	{
		pk.x =  0;
		pk.y = 0; 
		double max_val = 0;
		double compare_val = 0;

		//iterate through range arounf max_position to find optimal point that lies on needle
		for (int i = max_position - 3 * b; i < max_position + 3 * b; i++)
		{
			if (i < ROIheight + 1 && i >= 0) {
				if(i < S_pair[iterator].second.size())
				{

					compare_val = S_pair[iterator].second[i];
					cv::LineIterator iter(ray.first, ray.second, 8);

					if (compare_val > max_val)
					{
						max_val = compare_val;
						second_pos = i;
						first_pos = iterator;

						for (int x = 0; x < i; ++x)
						{
							iter++;
						}

						//final point that is chosen for the specific ray
						pk = iter.pos();
					}
				}
			}
		}
		
		points.push_back(pk);
		iterator++; 
	}

	//draw collection of needle points
	for (const cv::Point2f& point : points)
	{
		if (point.x != 0 && point.y != 0) {
			cv::circle(canvas, point, 4, cv::Scalar(255, 165, 0), -1); // You can customize the marker size and thickness
			cv::imshow("Rotated Rectangle", canvas);
		}
		needlepoints.push_back(point);
	}

	//draw line between points
	for (size_t i = 1; i < points.size(); i++)
	{
		if (points[i-1].x != 0 && points[i-1].y != 0 && points[i].x != 0 && points[i].y != 0) {
			cv::line(canvas, points[i - 1], points[i], cv::Scalar(255, 165, 0), 2);	
		}
	}

	cv::imshow("Rotated Rectangle", canvas);
	cv::waitKey(1);

	//call tip detection
	this->Tipdetect(); 
}

//this function identifies the needle tip
void AxisEst::Tipdetect()
{
	cv::Point2f estimated_needletip; 
	std::vector<cv::Point2f> final_curve; 

	//iterates through all estimated points that lie on needle
	for (size_t i = 1; i < needlepoints.size()-1; i++)
	{
		if(needlepoints[i].x != 0 && needlepoints[i].y != 0)
			final_curve.push_back(needlepoints[i]); 

		//identifies needle tip if ray does not contain a needle point estimation that selects previous point as needle tip
		if (needlepoints[i].x == 0 && needlepoints[i].y == 0 && needlepoints[i+ 1].x == 0 && needlepoints[i+ 1].y == 0)
		{
			estimated_needletip = needlepoints[i - 1]; 
			break; 
		}
	}
	
	cv::cvtColor(m_src, m_src, cv::COLOR_GRAY2BGR);
	cv::circle(m_src, estimated_needletip, 4, cv::Scalar(0, 255, 0), 2);
	cv::imshow(" Fitted Line", m_src);
	std::cout << estimated_needletip << std::endl; 

	//call polynomial regression to draw curved line
	this->polynomialregrssion(final_curve); 

	est_needleTip = estimated_needletip; 
	std::cout << est_needleTip << std::endl;

	final_curve.clear(); 
	needlepoints.clear();
}

//polynomial regression function
int AxisEst::polynomialregrssion(std::vector<cv::Point2f>& needle)
{
	int degree = 1;
	cv::Mat image = m_src.clone(); 
	size_t N = needle.size();

	if (N < degree + 1) {
		std::cerr << "Error: Insufficient data points for the given degree of the polynomial." << std::endl;
		return 0;
	}

	// Create matrices for the normal equations (X and Y) to perform least squares
	cv::Mat X(N, degree + 1, CV_64FC1);
	cv::Mat Y(N, 1, CV_64FC1);

	for (size_t i = 0; i < N; ++i) {
		for (int j = 0; j <= degree; ++j) {
			X.at<double>(i, j) = pow(needle[i].x, j);
		}
		Y.at<double>(i, 0) = needle[i].y;
	}

	// Calculate the coefficients using least squares
	cv::Mat coeffs;
	cv::solve(X, Y, coeffs, cv::DECOMP_NORMAL | cv::DECOMP_LU);

	// Convert the coefficients matrix to a vector
	std::vector<double> coefficients(coeffs.ptr<double>(), coeffs.ptr<double>() + coeffs.rows);

	double x_min = needle.front().x;
	double x_max = needle.back().x;

	// Draw the fitted polynomial curve into the image
	for (int x = static_cast<int>(x_min); x <= static_cast<int>(x_max); ++x) {
		
		double y = 0.0;
		for (int i = 0; i <= degree; ++i) {
			y += coefficients[i] * pow(x, i);
		}
		cv::Point2f pt(x, y);
		cv::circle(m_original, pt, 2, cv::Scalar(255,0 , 0), -1); // Draw a white pixel for each point on the curve
	}
	cv::imshow(" curved Fitted Line", image);

	if (degree == 1)
	{
		double slope = coefficients[1]; 
		m_angle = atan(slope); 
		m_angle = m_angle * (180 / CV_PI); 
		std::cout << m_angle << std::endl; 
	}

	return 1; 
}

