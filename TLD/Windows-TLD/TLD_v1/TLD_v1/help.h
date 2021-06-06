#pragma once
#include "opencv2/opencv.hpp"

using namespace std;

//Bounding Boxes
struct BoundingBox : public cv::Rect {			//���м̳���Rect����������Ա���ص��ʺ�ɨ�贰�ڵĳ߶�
	BoundingBox() {}
	BoundingBox(cv::Rect r) : cv::Rect(r) {}
public:
	float overlap;								//�������ڵĽ��Ͳ�
	int sidx;									//ɨ�贰�ڳ߶���ţ�scale index
};

//Detection structure
struct DetStruct {								//������������ģ��Ĵ��ڽṹ��
	vector<int> bb;
	vector<float> conf1;					//���������ƶ�
	vector<float> conf2;					//��ű������ƶ�
	vector<vector<int> > isin;
	vector<cv::Mat> patch;					//ѡ����
	
	//vector<vector<int> > patt;
};

//Temporal structure
struct TempStruct {								//�ݴ�Ľṹ�壬ֻ��patt()��conf(������ƶ�)������Ա
	vector<vector<int> > patt;
	vector<float> conf;
};

struct OComparator {							//�Ƚ����ߵ��غ϶ȣ���һ��vector<BoundingBox>��Ա
	OComparator(const vector<BoundingBox>& _grid) :grid(_grid) {}
	vector<BoundingBox> grid;
	bool operator()(int idx1, int idx2) {
		return grid[idx1].overlap > grid[idx2].overlap;
	}
};


struct CComparator {							//�Ƚ��������Ŷ�
	CComparator(const vector<float>& _conf) :conf(_conf) {}
	vector<float> conf;
	bool operator()(int idx1, int idx2) {
		return conf[idx1] > conf[idx2];
	}
};

