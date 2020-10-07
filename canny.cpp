#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <queue>

using namespace cv;
using namespace std;

// Step 1: complete gradient and threshold
// Step 2: complete sobel
// Step 3: complete canny (recommended substep: return Max instead of C to check it) 
// Step 4 (facultative, for extra credits): implement a Harris Corner detector






// Raw gradient. No denoising
void gradient(const Mat&Ic, Mat& G2)
{
	Mat I;
	cvtColor(Ic, I, COLOR_BGR2GRAY);

	int m = I.rows, n = I.cols;
	G2 = Mat(m, n, CV_32F);

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			// Compute squared gradient (except on borders)
			// ...
			float ix, iy;												// FLAVIEN
			// without this checks, it would crash, as the image would be accessed outside of its domain		// FLAVIEN
			// Give a 0 gradient, whenever any of the looked up value would be outside the image domain		// FLAVIEN

			// Using forward differences:
			// if (i == m - 1)											// FLAVIEN
			// 	iy = 0;												// FLAVIEN
			// else													// FLAVIEN
			// 	iy = (float(I.at<uchar>(i + 1, j)) - float(I.at<uchar>(i, j)));					// FLAVIEN
			// if (j == n - 1)											// FLAVIEN
			// 	ix = 0;												// FLAVIEN
			// else													// FLAVIEN
			// 	ix = (float(I.at<uchar>(i, j + 1)) - float(I.at<uchar>(i, j)));					// FLAVIEN
			// G2.at<float>(i, j) = (ix*ix + iy*iy);								// FLAVIEN

			// Using central differences:
			if (i == 0 || i == m - 1)										// FLAVIEN
				iy = 0;												// FLAVIEN
			else													// FLAVIEN
				iy = (float(I.at<uchar>(i + 1, j)) - float(I.at<uchar>(i-1, j)))/2;				// FLAVIEN
			if (j == 0 || j == n - 1)										// FLAVIEN
				ix = 0;												// FLAVIEN
			else													// FLAVIEN
				ix = (float(I.at<uchar>(i, j + 1)) - float(I.at<uchar>(i, j-1)))/2;				// FLAVIEN
			G2.at<float>(i, j) = (ix*ix + iy*iy);									// FLAVIEN

		}
	}



}

// Gradient (and derivatives), Sobel denoising
void sobel(const Mat&Ic, Mat& Ix, Mat& Iy, Mat& G2)
{
	Mat I;
	cvtColor(Ic, I, COLOR_BGR2GRAY);

	int m = I.rows, n = I.cols;
	Ix = Mat(m, n, CV_32F);
	Iy = Mat(m, n, CV_32F);
	G2 = Mat(m, n, CV_32F);

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			float ix, iy;
			if (i == 0 || i == m - 1 || j == 0 || j == n - 1)							// FLAVIEN
				iy = 0;												// FLAVIEN
			else													// FLAVIEN
				iy = ((float(I.at<uchar>(i + 1, j - 1)) - float(I.at<uchar>(i - 1, j - 1))) + 2 * (float(I.at<uchar>(i + 1, j)) - float(I.at<uchar>(i - 1, j))) + (float(I.at<uchar>(i + 1, j - 1)) - float(I.at<uchar>(i - 1, j - 1)))) / 8;
			if (i == 0 || i == m - 1 || j == 0 || j == n - 1)							// FLAVIEN
				ix = 0;												// FLAVIEN
			else													// FLAVIEN
				ix = ((float(I.at<uchar>(i - 1, j + 1)) - float(I.at<uchar>(i - 1, j - 1))) + 2 * (float(I.at<uchar>(i, j + 1)) - float(I.at<uchar>(i, j - 1))) + (float(I.at<uchar>(i + 1, j + 1)) - float(I.at<uchar>(i + 1, j - 1)))) / 8;
			Ix.at<float>(i, j) = ix;										// FLAVIEN
			Iy.at<float>(i, j) = iy;										// FLAVIEN
			G2.at<float>(i, j) = (ix*ix + iy*iy);									// FLAVIEN
		}
	}
}

// Gradient thresholding, default = do not denoise
Mat threshold(const Mat& Ic, float s, bool denoise = false)
{
	Mat Ix, Iy, G2;
	if (denoise)
		sobel(Ic, Ix, Iy, G2);
	else
		gradient(Ic, G2);
	int m = Ic.rows, n = Ic.cols;
	Mat C(m, n, CV_8U);
	for (int i = 0; i < m; i++){
		for (int j = 0; j < n; j++){
			// C.at<uchar>(i, j) = ...
			if (G2.at<float>(i, j) > s*s)				// FLAVIEN
				C.at<uchar>(i, j) = 255;			// FLAVIEN
			else							// FLAVIEN
				C.at<uchar>(i, j) = 0;				// FLAVIEN
		}		
	}
	return C;
}

// Gradient thresholding and denoising: X-Derivative
Mat threshold_X_derivative(const Mat& Ic, float s)
{
	Mat Ix, Iy, G2;
	sobel(Ic, Ix, Iy, G2);
	return Ix;
}

// Gradient thresholding and denoising: Y-Derivative
Mat threshold_Y_derivative(const Mat& Ic, float s)
{
	Mat Ix, Iy, G2;
	sobel(Ic, Ix, Iy, G2);
	return Iy;
}


// Canny edge detector, with thresholds s1<s2
Mat canny(const Mat& Ic, float s1, float s2)
{
	Mat Ix, Iy, G2;
	sobel(Ic, Ix, Iy, G2);

	int m = Ic.rows, n = Ic.cols;
	Mat Max(m, n, CV_8U);									// Binary black&white image with white pixels when ( G2 > s1 && max in the direction of the gradient )
												// http://www.cplusplus.com/reference/queue/queue/
	queue<Point> Q;										// Enqueue seeds ( Max pixels for which G2 > s2 )
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {


			uchar mvar = 0;
			float G0 = G2.at<float>(i, j);

			float PI = 3.14159265;
			float thetatest = atan(Iy.at<float>(i, j)/Ix.at<float>(i, j)) * 180/PI;	// FLAVIEN
			// cout << "thetatest is: " << thetatest << endl;				// FLAVIEN
			float theta = atan2(Iy.at<float>(i, j), Ix.at<float>(i, j)) * 180/PI;	// FLAVIEN
			// cout << "theta is: " << theta << endl;					// FLAVIEN

			if (i > 0 && j > 0 && i < m - 1 && j<n - 1 && G0 > s1*s1){

				if ((-22.5<theta) && (theta<22.5)) {
					if ((G0 > G2.at<float>(i, j+1)) && (G0 > G2.at<float>(i, j-1))) {
						mvar = 255;
					}
					if (G0 > s2*s2){
    						Q.push(Point(j, i));
					}
				}
				else if ((22.5<theta) && (theta<67.5)) {
					if ((G0 > G2.at<float>(i+1, j+1)) && (G0 > G2.at<float>(i-1, j-1))) {
						mvar = 255;
					}
					if (G0 > s2*s2){
    						Q.push(Point(j, i));
					}
				}
				else if ((67.5<theta) && (theta<112.5)) {
					if ((G0 > G2.at<float>(i+1, j)) && (G0 > G2.at<float>(i-1, j))) {
						mvar = 255;
					}
					if (G0 > s2*s2){
    						Q.push(Point(j, i));
					}
				}
				else if ((112.5<theta) && (theta<157.5)) {
					if ((G0 > G2.at<float>(i+1, j-1)) && (G0 > G2.at<float>(i-1, j+1))) {
						mvar = 255;
					}
					if (G0 > s2*s2){
    						Q.push(Point(j, i));
					}
				}

			}

			else 
				mvar = 0;


			Max.at<uchar>(i, j) = mvar;

		}
	}

	// Propagate seeds
	Mat C(m, n, CV_8U);
	C.setTo(0);
	while (!Q.empty()) {
		int i = Q.front().y, j = Q.front().x;
		Q.pop();

		if (i > 0 && j > 0 && i < m - 1 && j < n - 1) {
			if (C.at<uchar>(i, j) != 255) {
				C.at<uchar>(i, j) = 255;
			
				for (int a = -1; a<2; a++) {
					for (int b = -1; b<2; b++) {
					
						if (Max.at<uchar>(i+a, j+b) == 255) {
							Q.push(Point(j+b, i+a));
							//C.at<uchar>(i+a, j+b) = 255;	
						}
					}
				}
			}
		}
	} 
	return C;
	// return Max;
}











// facultative, for extra credits (and fun?)
// Mat harris(const Mat& Ic, ...) { ... }

int main()
{
	Mat I = imread("../road.jpg");
	
	Mat I_color2bgray;
	cvtColor(I, I_color2bgray, COLOR_BGR2GRAY);

	imshow("Input", I); waitKey();
	imshow("Input in gray", I_color2bgray); waitKey();
	imshow("Threshold", threshold(I, 15)); waitKey();
	imshow("Threshold + denoising", threshold(I, 15, true)); waitKey();
	imshow("Threshold + denoising: threshold_X_derivative", threshold_X_derivative(I, 15)); waitKey();
	imshow("Threshold + denoising: threshold_Y_derivative", threshold_Y_derivative(I, 15)); waitKey();

	// imshow("Non Maximum Suppression", Max); waitKey();	
	// imshow("Canny", canny(I, 15, 45)); waitKey();		
	imshow("Non Maximum Suppression", canny(I, 15, 45)); waitKey();	

	// imshow("Input", I);
	// imshow("Threshold", threshold(I, 15));
	// imshow("Threshold + denoising", threshold(I, 15, true));
	// imshow("Canny", canny(I, 15, 45));
	// imshow("Harris", harris(I, 15, 45));

	waitKey();

	return 0;
}

















