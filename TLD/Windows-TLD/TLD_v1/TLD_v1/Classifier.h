#include "opencv2/opencv.hpp"
#include "stdio.h"

using namespace std;

struct Feature{
	uchar x1, y1, x2, y2;
	Feature() : x1(0), y1(0), x2(0), y2(0) {}
	Feature(int _x1, int _y1, int _x2, int _y2)
		: x1((uchar)_x1), y1((uchar)_y1), x2((uchar)_x2), y2((uchar)_y2)
	{}
	bool operator ()(const cv::Mat& patch) const
	{
		//返回patch图片在（y1,x1）和（y2,x2）点的像素比较值，返回0或1
		return patch.at<uchar>(y1, x1) > patch.at<uchar>(y2, x2);
	}
};


class Classifier {

public:

	void prepare(const vector<cv::Size> &scales);
	void getFeatures(const cv::Mat &patch, const int &grid_idx, std::vector<int> &fern);
	void trainF(const vector<pair<vector<int>, int> > &ferns_data, int resample);
	void trainNN(const vector<cv::Mat> &nn_examples);
	float measure_forest(vector<int> ferns_data);
	void update(const vector<int> &ferns_data, int C, int N);
	void onlineNNModel(const cv::Mat &example, std::vector<int> &isin, float &rsconf, float &csconf);
	void evaluateTh(const vector<pair<vector<int>, int> > &nXT, const vector<cv::Mat> &nExT);
			

	float thr_nn_valid;  //检测器检测到的与跟踪器预测到的box的重叠度的阈值，0.7 


	float getFernTh() { 
		return thr_fern; //得到森林分类器的阈值
	} 

	float getNNTh() { 
		return thr_nn; //得到最近邻分类器的阈值
	} 

private:
	float thr_fern	= 0.6;					//集合分类器（森林）的10个后验概率阈值，0.6，大于该值，被认为有前景目标
	int structSize	= 13;					//每个分类器的节点数（树的深度），13
	int nstructs	= 10;					//森林树的个数，即分类器的个数，10
	float valid		= 0.5;					//阈值？0.5
	float ncc_thesame = 0.95;				//最近邻相似度正样本的阈值，0.95，大于该值是好样本
	float thr_nn	= 0.65;					//最近邻分类器的阈值，0.65			

	std::vector<std::vector<Feature> > features;	//特征结构体
	std::vector< std::vector<int> > nCounter;		//负样本计数器，10组
	std::vector< std::vector<int> > pCounter;		//正样本计数器，10组
	std::vector< std::vector<float> > posteriors;	//森林的后验概率数组，10组
	float thrN;										//随机森林负样本阈值
	float thrP;										//随机森林正样本阈值
													
	vector<cv::Mat> NN_pX;							//NN正样本
	vector<cv::Mat> NN_nX;							//NN负样本
};
