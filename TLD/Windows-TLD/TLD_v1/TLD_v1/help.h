#pragma once
#include "opencv2/opencv.hpp"

using namespace std;

//Bounding Boxes
struct BoundingBox : public cv::Rect {			//公有继承自Rect，有两个成员：重叠率和扫描窗口的尺度
	BoundingBox() {}
	BoundingBox(cv::Rect r) : cv::Rect(r) {}
public:
	float overlap;								//两个窗口的交和并
	int sidx;									//扫描窗口尺度序号，scale index
};

//Detection structure
struct DetStruct {								//方差分类器检测模块的窗口结构体
	vector<int> bb;
	vector<float> conf1;					//存放相关相似度
	vector<float> conf2;					//存放保守相似度
	vector<vector<int> > isin;
	vector<cv::Mat> patch;					//选定框
	
	//vector<vector<int> > patt;
};

//Temporal structure
struct TempStruct {								//暂存的结构体，只有patt()和conf(相关相似度)两个成员
	vector<vector<int> > patt;
	vector<float> conf;
};

struct OComparator {							//比较两者的重合度，有一个vector<BoundingBox>成员
	OComparator(const vector<BoundingBox>& _grid) :grid(_grid) {}
	vector<BoundingBox> grid;
	bool operator()(int idx1, int idx2) {
		return grid[idx1].overlap > grid[idx2].overlap;
	}
};


struct CComparator {							//比较两者置信度
	CComparator(const vector<float>& _conf) :conf(_conf) {}
	vector<float> conf;
	bool operator()(int idx1, int idx2) {
		return conf[idx1] > conf[idx2];
	}
};

