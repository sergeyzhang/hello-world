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
		//����patchͼƬ�ڣ�y1,x1���ͣ�y2,x2��������رȽ�ֵ������0��1
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
			

	float thr_nn_valid;  //�������⵽���������Ԥ�⵽��box���ص��ȵ���ֵ��0.7 


	float getFernTh() { 
		return thr_fern; //�õ�ɭ�ַ���������ֵ
	} 

	float getNNTh() { 
		return thr_nn; //�õ�����ڷ���������ֵ
	} 

private:
	float thr_fern	= 0.6;					//���Ϸ�������ɭ�֣���10�����������ֵ��0.6�����ڸ�ֵ������Ϊ��ǰ��Ŀ��
	int structSize	= 13;					//ÿ���������Ľڵ�����������ȣ���13
	int nstructs	= 10;					//ɭ�����ĸ��������������ĸ�����10
	float valid		= 0.5;					//��ֵ��0.5
	float ncc_thesame = 0.95;				//��������ƶ�����������ֵ��0.95�����ڸ�ֵ�Ǻ�����
	float thr_nn	= 0.65;					//����ڷ���������ֵ��0.65			

	std::vector<std::vector<Feature> > features;	//�����ṹ��
	std::vector< std::vector<int> > nCounter;		//��������������10��
	std::vector< std::vector<int> > pCounter;		//��������������10��
	std::vector< std::vector<float> > posteriors;	//ɭ�ֵĺ���������飬10��
	float thrN;										//���ɭ�ָ�������ֵ
	float thrP;										//���ɭ����������ֵ
													
	vector<cv::Mat> NN_pX;							//NN������
	vector<cv::Mat> NN_nX;							//NN������
};
