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
	TempStruct tmp;							//ͨ������������Ĵ���
	DetStruct dt;							//ͨ������������ͼ��Ϸ������Ĵ���
	vector<BoundingBox> dbb;				//ͨ��������������boundingbox
	vector<bool> dvalid;					//�������⵽��boundingbox��Ч��
	vector<float> dconf;					//�������⵽��boundingbox���ƶ�

	int min_win = 15;						//��С���ο�size 15x15
	vector<BoundingBox> grid;				//ɨ��ʸ������
	vector<cv::Size> scales;				//ɨ�贰�ڵĳ߶�����
	cv::Mat iisum;							//����ͼ
	cv::Mat iisqsum;						//ƽ������ͼ
	cv::PatchGenerator generator;		
	vector<int> good_boxes;					//overlap > 0.6
	std::vector<int> bad_boxes;				//overlap < 0.2
	BoundingBox bbhull;						//goodboxes����Ӿ���
	BoundingBox best_box;					//�ص�������ɨ�贰��
	BoundingBox lastbox;					//��һ��ѧϰ��õĿ�

	Classifier classifier;
	int patch_size = 15;					//��һ���Ŀ��С,15x15
	float var;								//����

	vector<pair<std::vector<int>, int> > pX; //ѵ������������
	vector<pair<std::vector<int>, int> > nX; //ѵ�����踺����

	float bad_patches = 100;				//�û�����100��
	cv::Mat p_patch;						//p_patch��1��������ѵ����������
	vector<cv::Mat> n_patch;				//n_patch, ��������bad_patches��������ѵ�����������ĸ���

	vector<pair<vector<int>, int> > nXT;	//���ɭ�ָ��������Լ�
	vector<cv::Mat> NNXT;					//NN���������Լ�

	//processFrame
	Track track;
	bool tracked;							//�Ƿ���ٵ�
	BoundingBox tbb;						//boundingbox��ǰ֡��λ�úʹ�С
	BoundingBox bbnext;
	float tconf;							//���ٵ���ض�
	bool detected;							//ͨ�������������ɭ�ַ�������ɨ�������Ƿ����0
};

