#pragma once
#include "help.h"
#include<opencv2/opencv.hpp>

using namespace std;

class Track {
public:
	bool trackP2P(const cv::Mat &pregray, const cv::Mat nowgray, vector<cv::Point2f> &pts1, vector<cv::Point2f> &pts2, BoundingBox &box);
	BoundingBox				tbb;

private:
	vector<cv::Point2f> bbPoints(vector<cv::Point2f> &pts1, const BoundingBox &box);
	bool trackf2f(const cv::Mat &pregray, const cv::Mat &nowgray, vector<cv::Point2f> &pts1, vector<cv::Point2f> &pts2);
	void normCrossCorrelation(const cv::Mat &pregray, const cv::Mat &nowgray, vector<cv::Point2f>& pts1, vector<cv::Point2f> &pts2);
	float median(vector<float> v);
	void bbPredict(const vector<cv::Point2f>& pts1, const vector<cv::Point2f>& pts2, const BoundingBox &lastbox, BoundingBox &tbb);

	vector<uchar>			status;
	vector<uchar>			FB_status;
	vector<cv::Point2f>		pointsFB;
	vector<float>			FB_error;
	cv::Size				window_size = cv::Size(4, 4);
	int						level = 5;
	cv::TermCriteria		term_criteria = cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 20, 0.03);
	vector<float>			similarity;
	float					lambda = 0.5;
	
	BoundingBox				lastbox;
};
