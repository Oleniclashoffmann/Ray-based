#include "Preprocessing.h"

preprocessing::preprocessing()
{

}

preprocessing::~preprocessing()
{

}

//Function initializes the class with the current image
void preprocessing::init(cv::Mat src, int angle)
{
	m_src = src.clone();
	original = src.clone();
	m_angle = angle;
}

//This function is the main prerpocessing function that is called from the main file 
//It converts the image into grayscale applies Gaussian blur and a predefined threshold value 
//And it calls connected components algorithm for further image preprocessing
cv::Mat preprocessing::process()
{
	cv::cvtColor(m_src, m_src, cv::COLOR_BGR2GRAY);
	m_src.convertTo(m_src, CV_32FC1);
	cv::GaussianBlur(m_src, m_src, cv::Size(7, 7), 0, 0);
	cv::threshold(m_src, m_src, m_threshold_value, m_max_binary_value, m_threshold_type);
	cv::Point pt(m_src.cols / 2, m_src.rows / 2);
	cv::Mat rotation;
	rotation = cv::getRotationMatrix2D(pt, m_angle, 1.0);
	cv::warpAffine(m_src, m_src, rotation, cv::Size(m_src.cols, m_src.rows));

	m_src = this->connected_components(m_src); 

	m_src_hugh = m_src;
	return m_src;
}


//this is a connected components algorithm
//it analyzes the structures in each image and erases structures that are beneath a specific size to reduce noise in the image
cv::Mat preprocessing::connected_components(cv::Mat filteredImage)
{
	cv::imshow("Before COnnected", filteredImage); 

	//Defines threshold for Clustersize that is erased
	int minClusterSize = 130;
	filteredImage.convertTo(filteredImage, CV_8U, 255.0);

	//perform connected component analysis
	cv::Mat labels, stats, centroids;
	int imageType = filteredImage.type();
	if (imageType != CV_8U)
		std::cout << "\nImage type ist not 8-bit " << std::endl;
	else {
		int numLabels = cv::connectedComponentsWithStats(filteredImage, labels, stats, centroids);

		//Iterate through the connected components 
		for (int label = 1; label < numLabels; label++)
		{
			int area = stats.at<int>(label, cv::CC_STAT_AREA);

			//Check if the cluster is small and erase it
			if (area < minClusterSize)
			{
				for (int row = 0; row < filteredImage.rows; row++)
				{
					for (int col = 0; col < filteredImage.cols; col++)
					{
						if (labels.at<int>(row, col) == label)
						{ 
							filteredImage.at<uchar>(row, col) = 0;
						}
					}
				}
			}
		}
	}
	filteredImage.convertTo(filteredImage, CV_32FC1);
	return filteredImage;

}

