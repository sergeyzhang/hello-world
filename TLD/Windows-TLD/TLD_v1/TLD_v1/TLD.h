#include "opencv2/opencv.hpp"
#include "help.h"
#include "Track.h"
#include "Classifier.h"
#include "PatchGenerator.h"

using namespace std;

class TLD {
public:
	//init
	void init(const cv::Mat &pregray, const BoundingBox &box);
	void buildGrid(const cv::Mat &pregray, const BoundingBox &box);
	float bbOverlap(const BoundingBox &box1, const BoundingBox &box2);
	void getOverlappingBoxes(const vector<BoundingBox> &grid, int num_closest);
	void getBBHull(vector<int> &good_boxes);
	void generatePositiveData(const cv::Mat& pregray, int num_warps);
	void getPattern(const cv::Mat &img, cv::Mat &pattern, cv::Scalar &mean, cv::Scalar &stdev);
	void generateNegativeData(const cv::Mat&pregray);
	double getVar(const BoundingBox &box, const cv::Mat &sum, const cv::Mat &sqsum);
	vector<int> index_shuffle(int begin, int end);

	//processFrame
	BoundingBox processFrame(const cv::Mat &pregray, const cv::Mat &nowgray, vector<cv::Point2f>& points1, vector<cv::Point2f> &points2, BoundingBox &box, bool tvalid);
	void detect(const cv::Mat &nowgray, vector<BoundingBox> &grid);
	void clusterConf(const vector<BoundingBox> &dbb, const vector<float> &dconf, vector<BoundingBox> &cbb, vector<float> &cconf);		
	void learn(const cv::Mat &nowgray, vector<BoundingBox> &grid, BoundingBox &lastbox, bool tvalid);

	int clusterBB(const vector<BoundingBox> &dbb, vector<int> &indexes); //if using bbcomp
private:

	//init
	TempStruct tmp;							//通过方差分类器的窗口
	DetStruct dt;							//通过方差分类器和集合分类器的窗口
	vector<BoundingBox> dbb;				//通过三个分类器的boundingbox
	vector<bool> dvalid;					//检测器检测到的boundingbox有效性
	vector<float> dconf;					//检测器检测到的boundingbox相似度

	int min_win = 15;						//最小矩形框size 15x15
	vector<BoundingBox> grid;				//扫描矢量窗口
	vector<cv::Size> scales;				//扫描窗口的尺度数组
	cv::Mat iisum;							//积分图
	cv::Mat iisqsum;						//平方积分图
	cv::PatchGenerator generator;		
	vector<int> good_boxes;					//overlap > 0.6
	std::vector<int> bad_boxes;				//overlap < 0.2
	BoundingBox bbhull;						//goodboxes的外接矩形
	BoundingBox best_box;					//重叠率最大的扫描窗口
	BoundingBox lastbox;					//上一次学习最好的框

	Classifier classifier;
	int patch_size = 15;					//归一化的框大小,15x15
	float var;								//方差

	vector<pair<std::vector<int>, int> > pX; //训练所需正样本
	vector<pair<std::vector<int>, int> > nX; //训练所需负样本

	float bad_patches = 100;				//用户定义100个
	cv::Mat p_patch;						//p_patch，1个，来自训练正样本集
	vector<cv::Mat> n_patch;				//n_patch, 个数等于bad_patches个数，即训练负样本集的个数

	vector<pair<vector<int>, int> > nXT;	//随机森林负样本测试集
	vector<cv::Mat> NNXT;					//NN负样本测试集

	//processFrame
	Track track;
	bool tracked;							//是否跟踪到
	BoundingBox tbb;						//boundingbox当前帧的位置和大小
	BoundingBox bbnext;
	float tconf;							//跟踪的相关度
	bool detected;							//通过方差分类器和森林分类器的扫描框个数是否大于0
};

