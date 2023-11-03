#include "save_file.h"

save_file::save_file(std::string path, cv::Point2f tip, std::chrono::duration<double> time, double angle)
{
    std::ofstream outfile;
    std::ofstream file2; 

   outfile.open("C:/Users/ohoff/Documents/Bachelorarbeit/Schreiben/Erro analysis/Angle/Statistical/Fulneedle-judith/Tip/20grad.txt", std::ios_base::app); // append instead of overwrite
   file2.open("C:/Users/ohoff/Documents/Bachelorarbeit/Schreiben/Erro analysis/Angle/Statistical/Fulneedle-judith/Angle/20grad.txt", std::ios_base::app);
    if (!outfile) {
        std::cerr << "Could not open the file!" << std::endl;

    }
    outfile << path << ", " << tip.x << ", " << tip.y << ", " << time.count() << std::endl;
    file2 << angle << std::endl; 
    outfile.close();
    file2.close(); 

}
