# Statistical filtering
# Ray-Based Statistical Filter Algorithm

## Introduction
This repository hosts the implementation of a ray-based algorithm, primarily aimed at processing medical imaging data. The focus of this algorithm is on the estimation of axes and tips of needles within the ultrasound images. 

## Structure of the Project
The codebase is divided into several key components, each responsible for different aspects of the algorithm's pipeline:
- `StatisticalFilter`: Serves as the main file.
- `AxisEst`: A class dedicated to estimating the axes of the structures and the tips within the images.
- `Preprocessing`: Handles the initial processing of the images to prepare them for analysis.
- `ransac`: Computes the ROI by applying the RANSAC algorithm to the preprocessed data.
- `save_file`: Manages the output, facilitating the saving of processed data and results.

## Prerequisites
To run this algorithm, you need the following dependencies installed on your system:
- OpenCv
- Change the image repostiory in main file to read in images
