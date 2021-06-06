#include "opencv2/opencv.hpp"
#include "TLD.h"

using namespace std;

//画点函数
void drawPoints(cv::Mat &frame, vector<cv::Point2f> points, cv::Scalar color) {
	for (auto i = points.begin(), ie = points.end(); i < ie; ++i) {
		cv::Point center(cvRound(i->x), cvRound(i->y));
		circle(frame, *i, 2, color, 1);
	}
}

//程序入口
int main(int argc, char* argv) {

	//step 1: open video
	cv::VideoCapture capture("david.mpg");
	if (!capture.isOpened()) {
		cout << "Could not read video file" << endl;
		return 1;
	}

	//step 2: initialize bounding box
	cv::Mat frame, pregray;
	capture >> frame;

	//cv::Rect2d box = selectROI(frame, false);
	BoundingBox box, bbnext;
	box.x = 150;
	box.y = 60;
	box.width = 90;
	box.height = 90;

	cvtColor(frame, pregray, CV_BGR2GRAY);

	cv::rectangle(frame, cv::Rect(150, 60, 90, 90), cv::Scalar(255, 0, 0), 3);
	imshow("img", frame);

	//step 3: initialize TLD
	TLD tld;
	tld.init(pregray, box);

	//step 4: initialize tracker parameters
	cv::Mat nowgray;
	vector<cv::Point2f> pts1;  
	vector<cv::Point2f> pts2;  

	bool tvalid = true;  

	while (capture.read(frame)) {

		cvtColor(frame, nowgray, CV_RGB2GRAY);

		BoundingBox bbnext = tld.processFrame(pregray, nowgray, pts1, pts2, box, tvalid);

		if (bbnext.width != 0 && bbnext.height != 0) {
			drawPoints(frame, pts1, cv::Scalar(0, 255, 255));
			drawPoints(frame, pts2, cv::Scalar(0, 255, 0));
			cv::rectangle(frame, bbnext, cv::Scalar(0, 0, 255), 2, 1);
			imshow("tld", frame);
		}
		else
			printf("TLD: bbnext Not Found\n");

		swap(pregray, nowgray);	
		pts1.clear();
		pts2.clear();

		if (cv::waitKey(33) == 'q')
			break;
	}

	//close video
	capture.release();
	return 0;
}

