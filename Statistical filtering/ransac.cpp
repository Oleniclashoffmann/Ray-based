#include "ransac.h"

ransac::ransac(const cv::Mat src)
{
    m_src = src;
}

//This function calculates the estimated axis of the needle with the RANSAC algorithm to determine an ROI for further investigation
void ransac::algorithm()
{
    //First the data set is created upon which further analysis is done
    this->create_data();

    //Variable Initialization
    int ran_numx, ran_numy;
    int iterations = 0;
    int max_inliers = 0;
    int pt3_x, pt3_y;
    float a, b, abs;
    estimatedSlope = 0;
    estimatedIntercept = 0;
    int confirmed_inlier = 0;
    col_image = m_src;

    //Convert image from Gray to BGR for further investigation 
    cv::cvtColor(col_image, col_image, cv::COLOR_GRAY2BGR);

    //If the created dataset is not zero the RANSAC algorithm starts
    if (data_x.size() != 0)
    {
        //WHile loop for all iterations in the algorithm -> m_k iterations
        while (iterations < m_k)
        {
            //Get two random data points from dataset, Depending on underlying model has to be adjusted to more dataspoints (e.g. for curved needles)
            for (int i = 0; i < 2; i++)
            {
                ran_numx = rand() % data_x.size();
                ran_numy = rand() % data_y.size();
                ran_datax.push_back(data_x[ran_numx]);
                ran_datay.push_back(data_y[ran_numy]);
            }

            //In this case straight needle is assumed, therefore linear regression is chosen to fit a line (isnt necessary since only 2 Points, but when using more points) 
            this->linear_regression(a, b);
            
            //Needle line is defined
            cv::Point starting(0, a);
            cv::Point ending(460, 460 * b + a);
            cv::Scalar line_color(255, 0, 0);
            cv::line(col_image, starting, ending, line_color, 2);

            //Now the inliers of every data point in the created dataset are calculated
            confirmed_inlier = 0;
            for (int j = 0; j < data_x.size(); j++)
            {
                pt3_x = data_x[j];
                pt3_y = data_y[j];
                abs = (std::abs(b * pt3_x - pt3_y + a)) / (sqrt(b * b + 1));
                
                //if distance to line is smaller than specified value m_t then inlier
                if (abs <= m_t)
                {
                    confirmed_inlier = confirmed_inlier + 1;
                    confirmed_inlierx.push_back(pt3_x);
                    confirmed_inliery.push_back(pt3_y);
                }
            }

            //After inlier count, count is compared to highest inlier count in whole iteration to get the line with most inliers
            if (confirmed_inlier > max_inliers)
            {
                max_inliers = confirmed_inlier;
                if (estimatedx.size() != 0)
                {
                    estimatedx.clear();
                    estimatedy.clear();
                }
                estimatedx = confirmed_inlierx;
                estimatedy = confirmed_inliery;
                estimatedIntercept = a;
                estimatedSlope = b;
            }

            //Vectors are cleared
            confirmed_inlierx.clear();
            confirmed_inliery.clear();
            ran_datax.clear();
            ran_datay.clear();

            iterations++;
        }

    }

    //Start and end point of the final line are computed
    starting.x = 0;
    starting.y = estimatedIntercept;
    ending.x = 300; //300 is randomly chosen to siplay the line 
    ending.y = 300 * estimatedSlope + estimatedIntercept;

    cv::Point starting(0, estimatedIntercept);
    cv::Point ending(300, 300 * estimatedSlope + estimatedIntercept);

    confirmed_inlierx.clear();
    confirmed_inliery.clear();
    ran_datax.clear();
    ran_datay.clear();
    estimatedx.clear();
    estimatedy.clear();
    data_x.clear();
    data_y.clear();
}

//In this function the dataset is created
//The algorithm iterates through all pixel values and saves x and y coordinates for every pixel that is above 0
void ransac::create_data()
{
        std::cout << "-----------------------In 460 schleife!!------------------- " << std::endl;
        for (int x = 0; x < m_src.cols; x++)
        {
            for (int y = 0; y < m_src.rows; y++)
            {
                if (y >= 0 && y < m_src.rows)
                {

                    if (m_src.at<float>(y, x) > 0)
                    {
                        data_x.push_back(x);
                        data_y.push_back(y);
                    }
                }
            }
        }

}

//linear regression function to fit the line to random points
void ransac::linear_regression(float& a, float& b)
{
    float sum_x = 0, sum_x2 = 0, sum_y = 0, sum_xy = 0;
    int n = ran_datax.size();

    for (int i = 0; i < ran_datax.size(); i++)
    {
        sum_x = sum_x + ran_datax[i];
        sum_x2 = sum_x2 + (ran_datax[i] * ran_datax[i]);
        sum_y = sum_y + ran_datay[i];
        sum_xy = sum_xy + (ran_datax[i] * ran_datay[i]);
    }

    float meanX = sum_x / n;
    float meanY = sum_y / n;

    b = (sum_xy - (n * meanX * meanY)) / (sum_x2 - n * meanX * meanX);
    a = meanY - (b * meanX);
}