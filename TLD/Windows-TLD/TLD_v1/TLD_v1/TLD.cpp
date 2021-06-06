#include "TLD.h"

using namespace std;

//init
void TLD::init(const cv::Mat &pregray, const BoundingBox &box) {
	
	//产生全局网格grid	
	buildGrid(pregray, box); 						
	
	iisum.create(pregray.rows + 1, pregray.cols + 1, CV_32F);	 
	iisqsum.create(pregray.rows + 1, pregray.cols + 1, CV_64F);
	dconf.reserve(100);		//检测得到的目标相似度向量组，方便push_back操作
	dbb.reserve(100);		//detect bounding box
	tmp.conf = vector<float>(grid.size());
	tmp.patt = vector<vector<int> >(grid.size(), vector<int>(10, 0));
	
	dt.bb.reserve(grid.size());
	good_boxes.reserve(grid.size());
	bad_boxes.reserve(grid.size());
	p_patch.create(patch_size, patch_size, CV_64F); //NN唯一正样本来自pregray[best_box]
	
	//初始化种仿射变换发生器generator
	int noise_init = 5;								
	float angle_init = 20;		
	float scale_init = 0.02;
	generator = cv::PatchGenerator(0, 0, noise_init, true, 1 - scale_init, 1 + scale_init, -angle_init*CV_PI / 180, angle_init*CV_PI / 180, -angle_init*CV_PI / 180, angle_init*CV_PI / 180);
	
	//产生good_boxes,bad_boxes,best_box,lastbox
	int num_closest_init = 10; //最初最近的10个框
	getOverlappingBoxes(grid, num_closest_init);		

	printf("init: found %d good boxes, %d bad boxes\n", (int)good_boxes.size(), (int)bad_boxes.size());
	printf("init: best_box: %d %d %d %d\n", best_box.x, best_box.y, best_box.width, best_box.height);
	printf("init: box hull: %d %d %d %d\n", bbhull.x, bbhull.y, bbhull.width, bbhull.height);
	
	//准备分类器
	classifier.prepare(scales);						

	//产生训练所需的正样本
	int num_warps_init = 20; 
	generatePositiveData(pregray, num_warps_init);	
	printf("init: training positve samples (all from good_boxes) pX.size: %d\n", pX.size());
	

	cv::Scalar stdev, mean;									
	meanStdDev(pregray(best_box), mean, stdev);	//计算best_box的均值和方差
	var = pow(stdev.val[0], 2)*0.5;				
	cout << "init: pregray[best_box] variance: " << var << endl;
	
	
	integral(pregray, iisum, iisqsum);
	double vr = getVar(best_box, iisum, iisqsum)*0.5; //自写方差函数
	cout << "init: pregray[best_box] check variance: " << vr << endl;
	
	//产生训练所需的负样本
	generateNegativeData(pregray);	

	int half = (int)nX.size()*0.5f;
	nXT.assign(nX.begin() + half, nX.end());		//森林分类器负样本测试集
	nX.resize(half);

	printf("init: training negative samples (all from bad_boxes) nX.size: %d\n", nX.size());
	printf("init: Ramdom Forest Testing Sampels(all from bad_boxes) nXT.size: %d\n", nXT.size());

	half = (int)n_patch.size()*0.5f;
	NNXT.assign(n_patch.begin() + half, n_patch.end());		//NN负样本测试集
	n_patch.resize(half);
	
	printf("init: Nearest Neighbourhood negative sampels (all from pregray[bad_boxes[i]]) NNExT.size: %d\n", NNXT.size());


	//Merge Negative Data with Positive Data and shuffle it
	vector<pair<vector<int>, int> > ferns_data(nX.size() + pX.size());
	vector<int> idx = index_shuffle(0, ferns_data.size()); //乱序数组
	int a = 0;
	for (int i = 0; i < pX.size(); i++) {
		ferns_data[idx[a]] = pX[i];
		a++;
	}
	for (int i = 0; i < nX.size(); i++) {
		ferns_data[idx[a]] = nX[i];
		a++;
	}
	//Data already have been shuffled, just putting it in the same vector
	vector<cv::Mat> nn_data(n_patch.size() + 1);
	nn_data[0] = p_patch;
	for (int i = 0; i < n_patch.size(); i++) {
		nn_data[i + 1] = n_patch[i];
	}
	///Training：训练集合分类器和最近邻分类器
	classifier.trainF(ferns_data, 2); //bootstrap = 2
	classifier.trainNN(nn_data);

	///Threshold Evaluation on testing sets
	classifier.evaluateTh(nXT, NNXT);

	float thr_fern = classifier.getFernTh();
	printf("init: RF thr_fern: %f\n", thr_fern);

	float thr_NN = classifier.getFernTh();
	printf("init: NN thr_NN: %f\n", thr_NN);
	printf("--------init is DONE-------\n");

}

void TLD::buildGrid(const cv::Mat &pregray, const BoundingBox &box) {
	const float SHIFT = 0.1; //检测时，扫描框移动的步长值，可以更改大一点
	const float SCALES[] = { 0.16151,0.19381,0.23257,0.27908,0.33490,0.40188,0.48225,
		0.57870,0.69444,0.83333,1,1.20000,1.44000,1.72800,
		2.07360,2.48832,2.98598,3.58318,4.29982,5.15978,6.19174 };  //产生21个不同比例的扫描窗口，e.g. 目标框的0.16151倍
	int width, height, min_bb_side;
	//Rect bbox;
	BoundingBox bbox;
	cv::Size scale;

	int sc = 0;
	for (int s = 0; s < 21; s++) { //依次判断扫描窗口的大小，排除小于min_win窗口大小的window
		width = round(box.width*SCALES[s]);
		height = round(box.height*SCALES[s]);
		min_bb_side = min(height, width);
		if (min_bb_side < min_win || width > pregray.cols || height > pregray.rows)
			continue;
		scale.width = width;
		scale.height = height;
		scales.push_back(scale);
		for (int y = 1; y < pregray.rows - height; y += round(SHIFT*min_bb_side)) {
			for (int x = 1; x < pregray.cols - width; x += round(SHIFT*min_bb_side)) {
				bbox.x = x;
				bbox.y = y;
				bbox.width = width;
				bbox.height = height;
				bbox.overlap = bbOverlap(bbox, box);
				bbox.sidx = sc;
				grid.push_back(bbox);
			}
		}
		sc++;
	}
}

float TLD::bbOverlap(const BoundingBox &box1, const BoundingBox &box2) {
	if (box1.x > box2.x + box2.width) { return 0.0; }
	if (box1.y > box2.y + box2.height) { return 0.0; }
	if (box1.x + box1.width < box2.x) { return 0.0; }
	if (box1.y + box1.height < box2.y) { return 0.0; }

	float colInt = min(box1.x + box1.width, box2.x + box2.width) - max(box1.x, box2.x);
	float rowInt = min(box1.y + box1.height, box2.y + box2.height) - max(box1.y, box2.y);

	float intersection = colInt * rowInt;
	float area1 = box1.width*box1.height;
	float area2 = box2.width*box2.height;
	return intersection / (area1 + area2 - intersection);
}

void TLD::getOverlappingBoxes(const vector<BoundingBox> &grid, int num_closest) {
	float max_overlap = 0;
	for (int i = 0; i < grid.size(); i++) {
		if (grid[i].overlap > max_overlap) {
			max_overlap = grid[i].overlap;
			best_box = grid[i];
		}
		if (grid[i].overlap > 0.6) {
			good_boxes.push_back(i);
		}
		else if (grid[i].overlap < 0.2) {
			bad_boxes.push_back(i);
		}
	}

	printf("init: grid.size: %d\n", grid.size());
	printf("init: good_boxes.size1: %d\n", good_boxes.size());

	//Get the best num_closest (10) boxes and puts them in good_boxes
	if (good_boxes.size() > num_closest) {
		std::nth_element(good_boxes.begin(), good_boxes.begin() + num_closest, good_boxes.end(), OComparator(grid));
		good_boxes.resize(num_closest);
	}

	printf("init: good_boxes.size2: %d\n", good_boxes.size());

	getBBHull(good_boxes);
}

void TLD::getBBHull(vector<int> &good_boxes) {
	int x1 = INT_MAX, x2 = 0;
	int y1 = INT_MAX, y2 = 0;
	int idx;
	for (int i = 0; i < good_boxes.size(); i++) {

		idx = good_boxes[i];

		x1 = min(grid[idx].x, x1);
		y1 = min(grid[idx].y, y1);
		x2 = max(grid[idx].x + grid[idx].width, x2);
		y2 = max(grid[idx].y + grid[idx].height, y2);
	}
	bbhull.x = x1;
	bbhull.y = y1;
	bbhull.width = x2 - x1;
	bbhull.height = y2 - y1;
}

void TLD::generatePositiveData(const cv::Mat &pregray, int num_warps) {
	cv::Scalar mean;			
	cv::Scalar stdev;		

	//将best_box的图片归一化为15x15大小的patch，存在唯一pEx正样本中
	getPattern(pregray(best_box), p_patch, mean, stdev);

	cv::Mat img;
	cv::Mat warped;
	GaussianBlur(pregray, img, cv::Size(9, 9), 1.5);
	warped = img(bbhull); 					
	
	cv::RNG& rng = cv::theRNG();
	cv::Point2f pt(bbhull.x + (bbhull.width - 1)*0.5f, bbhull.y + (bbhull.height - 1)*0.5f); //获得bbhull窗口的中心坐标
	vector<int> fern(10);   //建立10棵树(正样本)的后验概率数组
	

	pX.clear();			
	cv::Mat patch;
	if (pX.capacity() < num_warps*good_boxes.size())
		pX.reserve(num_warps*good_boxes.size()); 

	int idx;
	for (int i = 0; i < num_warps; i++) {

		if (i > 0)
			generator(pregray, pt, warped, bbhull.size(), rng); //产生变换后的正样本
		
		for (int b = 0; b < good_boxes.size(); b++) {
			idx = good_boxes[b];		
			patch = img(grid[idx]);     
			classifier.getFeatures(patch, grid[idx].sidx, fern); //得到输入的patch的特征fern，即13位的二进制码
			pX.push_back(make_pair(fern, 1)); //正例，标签为1
		}
	}
}

void TLD::getPattern(const cv::Mat &img, cv::Mat &pattern, cv::Scalar &mean, cv::Scalar &stdev) {
	//Output: resized Zero-Mean patch
	resize(img, pattern, cv::Size(patch_size, patch_size)); 
	meanStdDev(pattern, mean, stdev);					
	pattern.convertTo(pattern, CV_32F);
	pattern = pattern - mean.val[0];				
}

void TLD::generateNegativeData(const cv::Mat &pregray) {
	/* Inputs:
	 * - Image
	 * - bad_boxes (Boxes far from the bounding box)
	 * - variance (pEx variance)
	 * Outputs
	 * - Negative fern features (nX)
	 * - Negative NN examples (nEx)
	 */
	random_shuffle(bad_boxes.begin(), bad_boxes.end());//Random shuffle bad_boxes indexes
	int idx;
	//Get Fern Features of the boxes with big variance (calculated using integral images)
	int a = 0;

	printf("init: negative data generation started.\n");
	vector<int> fern(10);

	nX.reserve(bad_boxes.size());

	cv::Mat patch;
	for (int j = 0; j < bad_boxes.size(); j++) {
		idx = bad_boxes[j];
		if (getVar(grid[idx], iisum, iisqsum) < var*0.5f)
			continue;
		patch = pregray(grid[idx]);
		classifier.getFeatures(patch, grid[idx].sidx, fern);
		nX.push_back(make_pair(fern, 0));
		a++;
	}
	printf("init: nX Negative examples generated: ferns: %d\n", a);
	
	//random_shuffle(bad_boxes.begin(),bad_boxes.begin()+bad_patches);//Randomly selects 'bad_patches' and get the patterns for NN;
	cv::Scalar dum1, dum2;

	n_patch = vector<cv::Mat>(bad_patches);

	for (int i = 0; i < bad_patches; i++) {
		idx = bad_boxes[i];
		patch = pregray(grid[idx]);
		getPattern(patch, n_patch[i], dum1, dum2);
	}
	printf("init: n_patch Negative Examples: %d\n", (int)n_patch.size());
}


double TLD::getVar(const BoundingBox &box, const cv::Mat &sum, const cv::Mat &sqsum) {
	double brs = sum.at<int>(box.y + box.height, box.x + box.width);
	double bls = sum.at<int>(box.y + box.height, box.x);
	double trs = sum.at<int>(box.y, box.x + box.width);
	double tls = sum.at<int>(box.y, box.x);
	double brsq = sqsum.at<double>(box.y + box.height, box.x + box.width);
	double blsq = sqsum.at<double>(box.y + box.height, box.x);
	double trsq = sqsum.at<double>(box.y, box.x + box.width);
	double tlsq = sqsum.at<double>(box.y, box.x);
	double mean = (brs + tls - trs - bls) / ((double)box.area());
	double sqmean = (brsq + tlsq - trsq - blsq) / ((double)box.area());
	return sqmean - mean*mean;
}

vector<int> TLD::index_shuffle(int begin, int end) {
	vector<int> indexes(end - begin);					//初始化vector<int>型indexs数组
	for (int i = begin; i<end; i++) {
		indexes[i] = i;
	}
	random_shuffle(indexes.begin(), indexes.end());	//内置函数
	return indexes;
}


//processFrame
BoundingBox TLD::processFrame(const cv::Mat &pregray, const cv::Mat &nowgray, vector<cv::Point2f> &points1, vector<cv::Point2f> &points2, BoundingBox &box, bool tvalid) {
	
	vector<BoundingBox> cbb;					//聚类得到的目标框
	vector<float> cconf;						//聚类得到的目标的相似度
	int confident_detections = 0;				//满足一定条件的聚类扫描框个数
	int didx;									//检测得到的目标框索引

	///跟踪模块
	if (tvalid) {
		tracked = track.trackP2P(pregray, nowgray, points1, points2, box);	//跟踪下一帧
	}
	else {
		tracked = false;						//跟不上就是False
	}

	if (tracked) {
		//Estimate Confidence and Validity
		cv::Mat tpattern;
		cv::Scalar tmean, tstdev;
		BoundingBox bb;

		tbb = track.tbb;

		bb.x = max(tbb.x, 0);
		bb.y = max(tbb.y, 0);
		bb.width = min(min(nowgray.cols - tbb.x, tbb.width), min(tbb.width, tbb.br().x));
		bb.height = min(min(nowgray.rows - tbb.y, tbb.height), min(tbb.height, tbb.br().y));

		//归一化img2(bb)对应的patch的size，缩放到15x15，存入pattern
		getPattern(nowgray(bb), tpattern, tmean, tstdev);
		vector<int> tisin;
		float tdummy;

		//计算图片pattern到在线模型M的保守相似度
		classifier.onlineNNModel(tpattern, tisin, tdummy, tconf); //Conservative Similarity

		//保守相似度大于阈值，则评估跟踪有效
		if (tconf > classifier.thr_nn_valid) {
			tvalid = true;
		}
	}
	else
		tvalid = false;

	///检测模块
	detect(nowgray, grid);

	
	///综合模块
	if (tracked) {
		
		bbnext = tbb; //processFrame得到的TLD结果
		
		printf("processFrame: Tracked\n");

		if (detected) {      

			//通过重叠度对检测器检测到的目标bbox进行聚类
			clusterConf(dbb, dconf, cbb, cconf);                     
			printf("processFrame: clusterConf found %d clusters\n", (int)cbb.size());

			//检测到的框置信度cconf>跟踪得来的框的置信度tconf;检测到的框与跟踪得来的框重合度小
			//以上即对比检测模块和跟踪模块的结果,如果框重合度小，但检测到的框置信度更高，则使用检测结果当bbnext
			for (int i = 0; i < cbb.size(); i++) {
				if (bbOverlap(tbb, cbb[i]) < 0.5 && cconf[i] > tconf) {
					confident_detections++;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
					didx = i; //detection index索引值
				}
			}

			//如果只有一个confident_detection满足条件，那么就用检测结果来初始化跟踪器
			if (confident_detections == 1) {                               
				printf("processFrame: clusterConf found only one better box but far away frame that of track module..reinitializing tracking\n");
				bbnext = cbb[didx];
				tvalid = false;
			}
			else {
				printf("processFrame: detector find %d confident boxes but different to that of track module\n", confident_detections);
				int cx = 0, cy = 0, cw = 0, ch = 0;
				int close_detections = 0;
				for (int i = 0; i < dbb.size(); i++) {
					if (bbOverlap(tbb, dbb[i]) > 0.7) {                    
						cx += dbb[i].x;
						cy += dbb[i].y;
						cw += dbb[i].width;
						ch += dbb[i].height;
						close_detections++;
						printf("processFrame: weighted detection of tbb and dbb: %d %d %d %d\n", dbb[i].x, dbb[i].y, dbb[i].width, dbb[i].height);
					}
				}
				if (close_detections > 0) {

					//对与跟踪器预测到的box距离很近的box和跟踪器本身预测到的box进行坐标与大小平均
					bbnext.x = cvRound((float)(10 * tbb.x + cx) / (float)(10 + close_detections));   // weighted average trackers trajectory with the close detections
					bbnext.y = cvRound((float)(10 * tbb.y + cy) / (float)(10 + close_detections));
					bbnext.width = cvRound((float)(10 * tbb.width + cw) / (float)(10 + close_detections));
					bbnext.height = cvRound((float)(10 * tbb.height + ch) / (float)(10 + close_detections));
					printf("processFrame: Tracker bb: %d %d %d %d\n", tbb.x, tbb.y, tbb.width, tbb.height);
					printf("processFrame: Average bb: %d %d %d %d\n", bbnext.x, bbnext.y, bbnext.width, bbnext.height);
					printf("processFrame: Weighting %d close detection(s) with tracker..\n", close_detections);
				}
				else 
					printf("processFrame: use bbnext from track module and we find %d close detections\n", close_detections);
			}
		}
	}
	else {                                        //   If NOT tracking
		printf("processFrame: Not tracking..\n");
		tvalid = false;

		//如果跟踪器没有跟踪到目标，但是检测器检测到了一些可能的目标box，那么同样对其进行聚类，但只是简单的
		//将聚类的cbb[0]作为新的跟踪目标
		if (detected) {                            //  and detector is defined
			clusterConf(dbb, dconf, cbb, cconf);   //  cluster detections
			printf("Not tracking but using %d clusters obtained from detector\n", (int)cbb.size());
			if (cconf.size() == 1) {
				bbnext = cbb[0];
				printf("Not tracking but will reinitialize tracker obtained by detector\n");
				tvalid = true;
			}
		}
	}
	
	//学习模块
	if (tvalid) {
		lastbox = bbnext;
		learn(nowgray, grid, lastbox, tvalid);
	}

	return bbnext;
}


void TLD::detect(const cv::Mat &nowgray, vector<BoundingBox> &grid) {
	//cleaning
	dbb.clear();
	dconf.clear();
	dt.bb.clear();
	
	cv::Mat img(nowgray.rows, nowgray.cols, CV_8U);
	integral(nowgray, iisum, iisqsum);
	GaussianBlur(nowgray, img, cv::Size(9, 9), 1.5);
	int numtrees = 10;
	float fern_th = classifier.getFernTh();
	vector <int> ferns(10);
	float conf;
	int a = 0;
	cv::Mat patch;

	//级联分类器模块一：方差检测模块
	for (int i = 0; i < grid.size(); i++) {
		if (getVar(grid[i], iisum, iisqsum) >= var) {
			a++;

			//级联分类器模块二：集合分类器检测模块
			patch = img(grid[i]);
			classifier.getFeatures(patch, grid[i].sidx, ferns);
			conf = classifier.measure_forest(ferns);
			tmp.conf[i] = conf;
			tmp.patt[i] = ferns;
			if (conf > numtrees*fern_th) {
				dt.bb.push_back(i);
			}
		}
		else
			tmp.conf[i] = 0.0;
	}
	int detections = dt.bb.size();

	printf("detect: %d Bounding boxes passed the variance filter\n", a);
	printf("detect: %d initial detection from Fern Classifier\n", detections);

	if (detections > 100) {
		nth_element(dt.bb.begin(), dt.bb.begin() + 100, dt.bb.end(), CComparator(tmp.conf));
		dt.bb.resize(100);
		detections = 100;
	}

	if (detections == 0) {
		detected = false;
		return;
	}

	printf("detect: Fern detector made %d detections ", detections);

	//  Initialize detection structure
	//dt.patt = vector<vector<int> >(detections, vector<int>(10, 0));        //  Corresponding codes of the Ensemble Classifier
	
	dt.conf1 = vector<float>(detections);                                  //  Relative Similarity (for final nearest neighbour classifier)
	dt.conf2 = vector<float>(detections);                                  //  Conservative Similarity (for integration with tracker)
	dt.isin = vector<vector<int> >(detections, vector<int>(3, -1));        //  Detected (isin=1) or rejected (isin=0) by nearest neighbour classifier
	dt.patch = vector<cv::Mat>(detections, cv::Mat(patch_size, patch_size, CV_32F));//  Corresponding patches
	int idx;
	cv::Scalar mean, stdev;
	float nn_th = classifier.getNNTh();

	//级联分类器模块三：最近邻分类器检测模块
	for (int i = 0; i < detections; i++) {                                    //  for every remaining detection
		idx = dt.bb[i];                                                       //  Get the detected bounding box index
		patch = nowgray(grid[idx]);
		getPattern(patch, dt.patch[i], mean, stdev);						  //  Get pattern within bounding box
		classifier.onlineNNModel(dt.patch[i], dt.isin[i], dt.conf1[i], dt.conf2[i]);  //  Evaluate nearest neighbour classifier
		//dt.patt[i] = tmp.patt[idx];
		//printf("Testing feature %d, conf:%f isin:(%d|%d|%d)\n",i,dt.conf1[i],dt.isin[i][0],dt.isin[i][1],dt.isin[i][2]);
	
		if (dt.conf1[i] > nn_th) {                                               //  idx = dt.conf1 > tld.model.thr_nn; % get all indexes that made it through the nearest neighbour
			dbb.push_back(grid[idx]);                                         //  BB    = dt.bb(:,idx); % bounding boxes
			dconf.push_back(dt.conf2[i]);                                     //  Conf  = dt.conf2(:,idx); % conservative confidences
		}
	}  
	
	//通过三个级联器的目标框
	printf("detect: dbb.size: %d\n", dbb.size());

	if (dbb.size() > 0) {
		printf("detect: Found %d NN matches\n", (int)dbb.size());
		detected = true;
	}
	else {
		printf("detect: No NN matches found.\n");
		detected = false;
	}
}

bool bbcomp(const BoundingBox &b1, const BoundingBox &b2) {
	TLD t;
	if (t.bbOverlap(b1, b2) < 0.5)
		return false;
	else
		return true;
}

void TLD::clusterConf(const vector<BoundingBox> &dbb, const vector<float> &dconf, vector<BoundingBox> &cbb, vector<float> &cconf) {
	int numbb = dbb.size();
	vector<int> T;
	float space_thr = 0.5;
	int c = 1;
	switch (numbb) {
	case 1:
		cbb = vector<BoundingBox>(1, dbb[0]);
		cconf = vector<float>(1, dconf[0]);
		return;
		break;
	case 2:
		T = vector<int>(2, 0);
		if (1 - bbOverlap(dbb[0], dbb[1]) > space_thr) {
			T[1] = 1;
			c = 2;
		}
		break;
	default:
		T = vector<int>(numbb, 0);
		//c = partition(dbb, T, (*bbcomp));
		c = clusterBB(dbb,T);
		break;
	}
	cconf = vector<float>(c);
	cbb = vector<BoundingBox>(c);
	printf("Cluster indexes: ");
	BoundingBox bx;
	for (int i = 0; i < c; i++) {
		float cnf = 0;
		int N = 0, mx = 0, my = 0, mw = 0, mh = 0;
		for (int j = 0; j < T.size(); j++) {
			if (T[j] == i) {
				printf("%d ", i);
				cnf = cnf + dconf[j];
				mx = mx + dbb[j].x;
				my = my + dbb[j].y;
				mw = mw + dbb[j].width;
				mh = mh + dbb[j].height;
				N++;
			}
		}
		if (N > 0) {
			cconf[i] = cnf / N;
			bx.x = cvRound(mx / N);
			bx.y = cvRound(my / N);
			bx.width = cvRound(mw / N);
			bx.height = cvRound(mh / N);
			cbb[i] = bx;
		}
	}
	printf("\n");
}

void TLD::learn(const cv::Mat &nowgray, vector<BoundingBox> &grid, BoundingBox &lastbox, bool tvalid) {
	printf("[Learning]\n ");
	///Check consistency
	BoundingBox bb;
	bb.x = max(lastbox.x, 0);
	bb.y = max(lastbox.y, 0);
	bb.width = min(min(nowgray.cols - lastbox.x, lastbox.width), min(lastbox.width, lastbox.br().x));
	bb.height = min(min(nowgray.rows - lastbox.y, lastbox.height), min(lastbox.height, lastbox.br().y));
	cv::Scalar learn_mean, learn_stdev;
	cv::Mat learn_pattern;

	//归一化img(bb)对应的patch
	getPattern(nowgray(bb), learn_pattern, learn_mean, learn_stdev);

	vector<int> learn_isin;
	float learn_dummy, learn_conf;

	//计算输入图片(跟踪器的目标box)与在线模型之间的相关相似度conf
	classifier.onlineNNModel(learn_pattern, learn_isin, learn_conf, learn_dummy);
	
	
	if (learn_conf < 0.5) {
		printf("Learning: fast change..not training\n");
		tvalid = false;
		return;
	}
	if (pow(learn_stdev.val[0], 2) < var) {
		printf("Learning: low variance..not training\n");
		tvalid = false;
		return;
	}
	if (learn_isin[2] == 1) {
		printf("Learning: patch in negative data..not traing");
		tvalid = false;
		return;
	}
	/// Data generation
	for (int i = 0; i < grid.size(); i++) {
		grid[i].overlap = bbOverlap(lastbox, grid[i]);
	}
	vector<pair<vector<int>, int> > fern_examples;
	good_boxes.clear();
	bad_boxes.clear();

	int num_closest_update = 10;
	int num_warps_update = 10;

	getOverlappingBoxes(grid, num_closest_update);  //num_closest_update = 10;
	
	if (good_boxes.size() > 0)
		generatePositiveData(nowgray, num_warps_update);	//num_warps_update = 10;
	else {
		tvalid = false;
		printf("Learning: no good boxes..Not training");
		return;
	}

	fern_examples.reserve(pX.size() + bad_boxes.size());
	fern_examples.assign(pX.begin(), pX.end());
	int idx;
	for (int i = 0; i < bad_boxes.size(); i++) {
		idx = bad_boxes[i];
		if (tmp.conf[idx] >= 1) {
			fern_examples.push_back(make_pair(tmp.patt[idx], 0));
		}
	}
	vector<cv::Mat> nn_examples;
	nn_examples.reserve(dt.bb.size() + 1);
	nn_examples.push_back(p_patch);
	float bad_overlap = 0.2;
	for (int i = 0; i < dt.bb.size(); i++) {
		idx = dt.bb[i];
		if (bbOverlap(lastbox, grid[idx]) < bad_overlap)
			nn_examples.push_back(dt.patch[i]);
	}
	// Classifiers update
	classifier.trainF(fern_examples, 2);
	classifier.trainNN(nn_examples);
}


int TLD::clusterBB(const vector<BoundingBox> &dbb, vector<int>& indexes) {
	//FIXME: Conditional jump or move depends on uninitialised value(s)
	const int c = dbb.size();
	//1. Build proximity matrix
	cv::Mat D(c, c, CV_32F);
	float d;

	for (int i = 0; i < c; i++) {
		for (int j = i + 1; j < c; j++) {
			d = 1 - bbOverlap(dbb[i], dbb[j]);
			D.at<float>(i, j) = d;
			D.at<float>(j, i) = d;
		}
	}

	//2. Initialize disjoint clustering
	float *L = new float[c - 1];
	int   **nodes = new int *[c - 1];
	for (int i = 0; i < 2; i++)
		nodes[i] = new int[c - 1];
	int *belongs = new int[c];

	int m = c;
	for (int i = 0; i < c; i++) {
		belongs[i] = i;
	}
	for (int it = 0; it < c - 1; it++) {
		//3. Find nearest neighbor
		float min_d = 1;
		int node_a, node_b;
		for (int i = 0; i < D.rows; i++) {
			for (int j = i + 1; j < D.cols; j++) {
				if (D.at<float>(i, j) < min_d && belongs[i] != belongs[j]) {
					min_d = D.at<float>(i, j);
					node_a = i;
					node_b = j;
				}
			}
		}
		if (min_d > 0.5) {
			int max_idx = 0;
			bool visited;
			for (int j = 0; j < c; j++) {
				visited = false;
				for (int i = 0; i < 2 * c - 1; i++) {
					if (belongs[j] == i) {
						indexes[j] = max_idx;
						visited = true;
					}
				}
				if (visited)
					max_idx++;
			}
			return max_idx;
		}

		//4. Merge clusters and assign level
		L[m] = min_d;
		nodes[it][0] = belongs[node_a];
		nodes[it][1] = belongs[node_b];
		for (int k = 0; k < c; k++) {
			if (belongs[k] == belongs[node_a] || belongs[k] == belongs[node_b])
				belongs[k] = m;
		}
		m++;
	}

	/*========================================================*/
	delete[] L;
	L = NULL;
	for (int i = 0; i < 2; i++) {
		delete[] nodes[i];
		nodes[i] = NULL;
	}									  //modified by xian
	delete[] nodes;
	nodes = NULL;
	delete[] belongs;
	belongs = NULL;

	return 1;
	/*========================================================*/
}
