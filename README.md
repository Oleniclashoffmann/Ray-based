# Ray-Based Needle Detection in Ultrasound Images

> **Bachelor's Thesis Project** — 1 of 3 Real-time needle axis and tip estimation in ultrasound images using a ray-based statistical filtering algorithm.

## Problem Statement

Accurate needle tip localization during ultrasound-guided medical procedures (e.g. biopsies, regional anesthesia) is critical for patient safety. However, needles are often poorly visible in ultrasound images due to noise, speckle artifacts, and tissue interference. Manual tracking by clinicians is error-prone and limits procedural efficiency. This project addresses the challenge by implementing a **fully automated, multi-stage image processing pipeline** that enhances needle visibility, detects the needle trajectory, and precisely estimates the tip position — all in real time.

## Algorithm Overview

The algorithm follows a multi-stage pipeline inspired by ray-based line detection techniques:

```
Input Image → Preprocessing → RANSAC ROI Estimation → Ray Casting & Scoring → Needle Axis Fit → Tip Detection
```

### Pipeline Steps

1. **Preprocessing** — Convert to grayscale, apply Gaussian blur, threshold the image, and remove small noise clusters using connected component analysis.
2. **RANSAC-Based ROI Estimation** — Extract non-zero pixel coordinates and apply the RANSAC algorithm with linear regression to estimate a rough needle axis, defining a region of interest (ROI).
3. **Ray Casting** — Cast rays perpendicular to the estimated needle axis across the ROI. For each pixel along each ray, compute average intensities in a neighborhood parallel to the needle line.
4. **Edge Detection & Scoring** — Compute the discrete derivative along each ray to detect edges. A sliding window score function identifies needle-like profiles:
   
   $$\text{score} = S_{\text{lag}} - S_{\text{lead}} - |S_{\text{lag}} + S_{\text{lead}}|$$
   
   where $S_{\text{lead}}$ and $S_{\text{lag}}$ are the summed derivative values in each half of the window.
5. **Histogram Accumulation (Hough-like)** — Project the highest-scoring points from each ray along the needle direction into a 1D histogram. The peak bin identifies the strongest linear feature.
6. **Needle Tip Detection** — Identify the deepest detected point along the needle axis as the estimated tip.
7. **Polynomial Regression** — Fit a polynomial curve through the detected needle points for the final axis representation.

## Tech Stack

| Technology | Purpose |
|---|---|
| **C++17** | Core implementation language |
| **OpenCV** | Image processing, connected components, line drawing, matrix operations |

## Project Structure

```
Statistical filtering/
├── Statistical filtering.cpp   # Main entry point — loads images, orchestrates the pipeline
├── preprocessing.cpp/.h        # Image preprocessing (grayscale, blur, threshold, connected components)
├── ransac.cpp/.h               # RANSAC algorithm for initial needle axis estimation & ROI
├── AxisEst.cpp/.h              # Ray-based axis estimation, scoring, histogram, and tip detection
├── save_file.cpp/.h            # Output results (tip position, angle, computation time) to file
```

### Component Responsibilities

| Module | Description |
|---|---|
| `preprocessing` | Converts images to grayscale, applies Gaussian blur and binary thresholding, removes small noise clusters via connected component analysis |
| `ransac` | Creates a dataset of non-zero pixels, applies RANSAC with linear regression to find the best-fit line, and returns the estimated slope and intercept for ROI definition |
| `AxisEst` | Core detection — defines a rotated rectangular ROI, casts perpendicular rays, computes intensity derivatives, applies the scoring function, builds a Hough-like histogram, detects needle points and tip, and fits a polynomial curve |
| `save_file` | Logs needle tip coordinates, estimated angle, and computation time to output files |

## Prerequisites

- **C++17** compatible compiler
- **OpenCV 4.x** (tested with OpenCV 4.5+)
- **Visual Studio 2019/2022** (`.vcxproj` included) or any C++ build system with OpenCV linked

## Getting Started

1. **Clone the repository**
   ```bash
   git clone https://github.com/olehoffmann/Ray-based.git
   cd Ray-based
   ```

2. **Configure image paths**  
   Update the image directory path in `Statistical filtering.cpp` (line 29) to point to your ultrasound image dataset:
   ```cpp
   std::string path = "path/to/your/ultrasound/images/";
   ```

3. **Configure output paths**  
   Update output file paths in `save_file.cpp` to your desired output directory.

4. **Build & Run**  
   Open `Statistical filtering.sln` in Visual Studio with OpenCV configured, then build and run.

## Configuration Parameters

Key algorithm parameters that can be tuned for different datasets:

| Parameter | Location | Default | Description |
|---|---|---|---|
| Threshold value | `preprocessing.h` | `180` | Binary threshold for image segmentation |
| Min cluster size | `preprocessing.cpp` | `130` | Minimum pixel cluster size (smaller clusters = noise) |
| RANSAC iterations | `ransac.h` | `700` | Number of RANSAC iterations |
| Inlier threshold | `ransac.h` | `3.0` | Max distance from line to be considered an inlier |
| ROI height | `AxisEst.h` | `100` | Height of the search ROI (depends on needle diameter) |
| Window size | `AxisEst.h` | `30` | Sliding window size for score calculation |
| Ray spacing | `AxisEst.cpp` | `10 px` | Distance between perpendicular rays |

## Context

This project was developed as part of a **Bachelor's thesis** on automated needle detection in ultrasound imaging. The algorithm is designed for real-time processing of sequential ultrasound frames and outputs the estimated needle tip position, axis angle, and per-frame computation time.

## License

This project was developed as part of a Bachelor's thesis. Feel free to use it as a reference or starting point for your own work.
