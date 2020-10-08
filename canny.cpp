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
			// Squared gradient computation (except on borders)

			float ix, iy;

			// Using forward differences:
			// if (i == m - 1)
			// 	iy = 0;	
			// else	
			// 	iy = (float(I.at<uchar>(i + 1, j)) - float(I.at<uchar>(i, j)));	
			// if (j == n - 1)
			// 	ix = 0;	
			// else
			// 	ix = (float(I.at<uchar>(i, j + 1)) - float(I.at<uchar>(i, j)));	
			// G2.at<float>(i, j) = (ix*ix + iy*iy);			

			// Using central differences:
			if (i == 0 || i == m - 1)
				iy = 0;
			else
				iy = (float(I.at<uchar>(i + 1, j)) - float(I.at<uchar>(i-1, j)))/2;
			if (j == 0 || j == n - 1)
				ix = 0;	
			else
				ix = (float(I.at<uchar>(i, j + 1)) - float(I.at<uchar>(i, j-1)))/2;
			G2.at<float>(i, j) = (ix*ix + iy*iy);

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
			if (i == 0 || i == m - 1 || j == 0 || j == n - 1)
				iy = 0;	
			else
				iy = ((float(I.at<uchar>(i+1, j-1)) - float(I.at<uchar>(i-1, j-1))) + 2*(float(I.at<uchar>(i+1, j)) - float(I.at<uchar>(i-1, j))) + (float(I.at<uchar>(i+1, j-1)) - float(I.at<uchar>(i-1, j-1)))) / 8;
			if (i == 0 || i == m - 1 || j == 0 || j == n - 1)
				ix = 0;	
			else				
				ix = ((float(I.at<uchar>(i - 1, j + 1)) - float(I.at<uchar>(i - 1, j - 1))) + 2 * (float(I.at<uchar>(i, j + 1)) - float(I.at<uchar>(i, j - 1))) + (float(I.at<uchar>(i + 1, j + 1)) - float(I.at<uchar>(i + 1, j - 1)))) / 8;
			Ix.at<float>(i, j) = ix;
			Iy.at<float>(i, j) = iy;
			G2.at<float>(i, j) = (ix*ix + iy*iy);
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
			if (G2.at<float>(i, j) > s*s)
				C.at<uchar>(i, j) = 255;
			else
				C.at<uchar>(i, j) = 0;
		}		
	}
	return C;
}


// Canny edge detector, with thresholds s1<s2
Mat canny(const Mat& Ic, float s1, float s2)
{
	Mat Ix, Iy, G2;
	sobel(Ic, Ix, Iy, G2);

	int m = Ic.rows, n = Ic.cols;
	Mat Max(m, n, CV_8U);	

	queue<Point> Q;	
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {


			uchar mvar = 0;
			float G0 = G2.at<float>(i, j);

			float PI = 3.14159265;
			// float thetatest = atan(Iy.at<float>(i, j)/Ix.at<float>(i, j)) * 180/PI;	
			// cout << "thetatest is: " << thetatest << endl;
			float theta = atan2(Iy.at<float>(i, j), Ix.at<float>(i, j)) * 180/PI;
			// cout << "theta is: " << theta << endl;

			
			if (theta < 0){
                		theta = theta + 180; 
 			} 
			//cout << "modified theta is: " << theta << endl;

			if (i > 0 && j > 0 && i < m - 1 && j<n - 1 && G0 > s1*s1){

				if ((theta < 22.5) || (theta > 157.5)) {
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

	// imshow("Non Maximum Suppression", canny(I, 15, 45)); waitKey();	
	imshow("Canny edge detection", canny(I, 15, 45)); waitKey();		

	// imshow("Harris", harris(I, 15, 45));

	waitKey();

	return 0;
}

















