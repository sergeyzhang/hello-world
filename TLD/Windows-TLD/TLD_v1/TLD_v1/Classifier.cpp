#include "Classifier.h"

using namespace std;

void Classifier::prepare(const vector<cv::Size> &scales) {

	int totalFeatures = nstructs*structSize;  //nstructs = 10; structSize = 13;

	features = vector<vector<Feature> >(scales.size(), vector<Feature>(totalFeatures));

	
	cv::RNG& rng = cv::theRNG(); //OpenCV自带的随机种子发生器
	float x1f, x2f, y1f, y2f;
	int x1, x2, y1, y2;

	//用随机数去充填每一个尺度扫描窗口的特征
	for (int i = 0; i < totalFeatures; i++) {
		x1f = (float)rng;
		y1f = (float)rng;
		x2f = (float)rng;
		y2f = (float)rng;
		for (int s = 0; s < scales.size(); s++) {
			x1 = x1f * scales[s].width;
			y1 = y1f * scales[s].height;
			x2 = x2f * scales[s].width;
			y2 = y2f * scales[s].height;

			features[s][i] = Feature(x1, y1, x2, y2);  //features[s][i] = true/false 返回布尔值

		}
	}
	
	thrN = 0.5*nstructs; 

	//后验概率指每一个分类器对传入的图像进行像素对比，每个像素对比得到0或1
	//一共totalFeatures个对比，连成一个totalFeatures位的二进制码x，然后索引到一个记录了后验
	//概率的数组P(y,x)。y为0或1(二分类),意思指在出现x的基础上，该图片为y的概率. 对n个基本分类
	//器的后验概率做平均，大于0.5的则被判定为含有目标
	for (int i = 0; i < nstructs; i++) {
		posteriors.push_back(vector<float>(pow(2.0, structSize), 0));
		pCounter.push_back(vector<int>(pow(2.0, structSize), 0));
		nCounter.push_back(vector<int>(pow(2.0, structSize), 0));
	}
}

void Classifier::getFeatures(const cv::Mat &patch, const int &grid_idx, vector<int> &fern) {

	int leaf;										//叶子，树的最终节点

	for (int t = 0; t < nstructs; t++) {			//森林树的个数，共计10棵
		leaf = 0;
		for (int f = 0; f < structSize; f++) {		//树上的每个节点进行操作，共13个节点
													//struct feature特征结构体有一个运算符重载bool operator()
													//返回的patch图像片在(y1,x1)和(y2,x2)点的像素比较值，返回布尔值
													//然后，leaf记录了目标区域图像的特征
													//leaf = 0 移位仍是0；不为0时，变为22,34,55,983...

			leaf = (leaf << 1) + features[grid_idx][t*nstructs + f](patch);
		}
		fern[t] = leaf;								//leaf是正整数0,7,3343,...;
	}
}

void Classifier::trainF(const vector<std::pair<vector<int>, int> > &ferns_data, int resample) {
	//所有树的正样本阈值之和
	thrP = thr_fern * nstructs;

	for (int i = 0; i < ferns_data.size(); i++) {

		//1表示正样本
		if (ferns_data[i].second == 1) {

			//输入的是正样本，却被分类成正样本
			if (measure_forest(ferns_data[i].first) <= thrP)
				update(ferns_data[i].first, 1, 1);
		}
		else { //为0表示负样本
			   //输入的是负样本，却被分成正样本
			if (measure_forest(ferns_data[i].first) >= thrN)  //thrN在tld.init()中的Classifier.prepare()算出
				update(ferns_data[i].first, 0, 1);
		}
	}
}

float Classifier::measure_forest(vector<int> ferns_data) {
	float votes = 0;
	for (int i = 0; i < nstructs; i++) {

		//后验概率posteriors[i][idx] = ((float)pCounter[i][idx]) / (pCounter[i][idx]+nCounter[i][idx]);
		votes += posteriors[i][ferns_data[i]];	//每课树的每个特征值对应的后验概率累加值
	}
	return votes;
}

void Classifier::update(const vector<int> &ferns_data, int C, int N) {
	int idx;
	for (int i = 0; i < nstructs; i++) {
		idx = ferns_data[i];

		//如果C==1，正样本+1；如果C==0,负样本+1
		(C == 1) ? pCounter[i][idx] += N : nCounter[i][idx] += N;

		if (pCounter[i][idx] == 0) {
			posteriors[i][idx] = 0;
		}
		else {
			posteriors[i][idx] = ((float)(pCounter[i][idx])) / (pCounter[i][idx] + nCounter[i][idx]);
		}
	}
}

void Classifier::trainNN(const vector<cv::Mat> &nn_examples) {
	float conf, dummy;
	vector<int> y(nn_examples.size(), 0);				//y数组初始化为0
	y[0] = 1;											//传入trainNN这个函数的nn_examples样本集，

	vector<int> isin;									//计算NNConf所需参数
	for (int i = 0; i < nn_examples.size(); i++) {
		onlineNNModel(nn_examples[i], isin, conf, dummy);      //计算输入图片与在线模型之间的相关相似度Conf               
		if (y[i] == 1 && conf <= thr_nn) {              //标签是正样本，如果相关相似度<0.65,则它没有前景目标，属于分类错误，添加到正样本库                  
			if (isin[1] < 0) {							//没有发现正样本相似度大于0.95的                     
				NN_pX = vector<cv::Mat>(1, nn_examples[i]);
				continue;
			}

			NN_pX.push_back(nn_examples[i]);				//正样本入栈
		}
		if (y[i] == 0 && conf > 0.5)                    //如果是负样本且相关相似           
			NN_nX.push_back(nn_examples[i]);              //负样本入栈

	}


	printf("Trained NN examples: %d positive %d negative\n", (int)NN_pX.size(), (int)NN_nX.size());
}


void Classifier::onlineNNModel(const cv::Mat &example, vector<int> &isin, float &rsconf, float &csconf) {
	/*Inputs:
	* -NN Patch
	* Outputs:
	* -Relative Similarity (rsconf), Conservative Similarity (csconf), In pos. set|Id pos set|In neg. set (isin)
	*/
	isin = vector<int>(3, -1);
	if (NN_pX.empty()) { //if isempty(tld.pex) % IF positive examples in the model are not defined THEN everything is negative
		rsconf = 0; //    conf1 = zeros(1,size(x,2));
		csconf = 0;
		return;
	}
	if (NN_nX.empty()) { //if isempty(tld.nex) % IF negative examples in the model are not defined THEN everything is positive
		rsconf = 1;   //    conf1 = ones(1,size(x,2));
		csconf = 1;
		return;
	}
	cv::Mat ncc(1, 1, CV_32F);
	float nccP, csmaxP, maxP = 0;
	bool anyP = false;
	int maxPidx, validatedPart = ceil(NN_pX.size()*valid); //float valid = 0.5; .yml文件读出的
	

	float nccN, maxN = 0;
	bool anyN = false;
	for (int i = 0; i < NN_pX.size(); i++) {
		matchTemplate(NN_pX[i], example, ncc, CV_TM_CCORR_NORMED);      // measure NCC to positive examples
		nccP = (((float*)ncc.data)[0] + 1)*0.5;
		if (nccP > ncc_thesame)
			anyP = true;
		if (nccP > maxP) {
			maxP = nccP;
			maxPidx = i;
			if (i < validatedPart)
				csmaxP = maxP;
		}
	}
	for (int i = 0; i < NN_nX.size(); i++) {
		matchTemplate(NN_nX[i], example, ncc, CV_TM_CCORR_NORMED);     //measure NCC to negative examples
		nccN = (((float*)ncc.data)[0] + 1)*0.5;
		if (nccN > ncc_thesame)
			anyN = true;
		if (nccN > maxN)
			maxN = nccN;
	}
	//set isin
	if (anyP) isin[0] = 1;  //if he query patch is highly correlated with any positive patch in the model then it is considered to be one of them
	isin[1] = maxPidx;      //get the index of the maximall correlated positive patch
	if (anyN) isin[2] = 1;  //if  the query patch is highly correlated with any negative patch in the model then it is considered to be one of them
	
	//Measure Relative Similarity
	float dN = 1 - maxN;
	float dP = 1 - maxP;
	rsconf = (float)dN / (dN + dP);
	//Measure Conservative Similarity
	dP = 1 - csmaxP;
	csconf = (float)dN / (dN + dP);

}

void Classifier::evaluateTh(const vector<pair<vector<int>, int> > &nXT, const vector<cv::Mat> &nExT) {
	float fconf;
	for (int i = 0; i < nXT.size(); i++) {

		//所有基本分类器的后验概率的平均值如果大于thr_fern,则认为含有前景目标
		//mearsure_forest返回的是所有后验概率的累加和，nstructs为树的个数
		fconf = (float)measure_forest(nXT[i].first) / nstructs;
		if (fconf > thr_fern)
			thr_fern = fconf;
	}

	vector <int> isin;
	float conf, dummy;
	for (int i = 0; i < nExT.size(); i++) {
		onlineNNModel(nExT[i], isin, conf, dummy);
		if (conf > thr_nn)
			thr_nn = conf;
	}
	if (thr_nn > thr_nn_valid)
		thr_nn_valid = thr_nn;
}
