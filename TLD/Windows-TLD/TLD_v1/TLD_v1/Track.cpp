#include"Track.h"
bool Track::trackP2P(const cv::Mat &pregray, const cv::Mat nowgray, vector<cv::Point2f> &pts1, vector<cv::Point2f> &pts2, BoundingBox &box) {
	
	lastbox.x = box.x;
	lastbox.y = box.y;
	lastbox.width = box.width;
	lastbox.height= box.height;

	vector<cv::Point2f> point1 = bbPoints(pts1, box);  //points1����100����

	if (pts1.size() < 1) {
		printf("Track: track::bbPoints error that points are not generated\n");
		return false;
	}

	vector<cv::Point2f> points = point1;

	bool tracked = trackf2f(pregray, nowgray, points, pts2);

	if (tracked) {

		bbPredict(points, pts2, lastbox, tbb); //���tbb�����ٵ���boundingbox

		if (tbb.x > nowgray.cols || tbb.y > nowgray.rows || tbb.br().x < 1 || tbb.br().y < 1) {
			tracked = false;
			printf("Track: Too unstable predictions FB error=%f\n");
			return false;
		}
	}
	else
		printf("Track: No points tracked\n");

	return tracked;
}

vector<cv::Point2f> Track::bbPoints(vector<cv::Point2f> &pts1, const BoundingBox &box) {

	int max_pts = 10;

	//�����߽�
	int margin_h = 0;
	int margin_v = 0;

	//�����������
	int stepx = ceil(double((box.width - 2 * margin_h) / max_pts));	
	int stepy = ceil(double((box.height - 2 * margin_v) / max_pts));

	//�������㣬��10x10��������
	for (int y = box.y + margin_v; y < box.y + box.height - margin_v; y += stepy) {
		for (int x = box.x + margin_h; x < box.x + box.width - margin_h; x += stepx) {
			pts1.push_back(cv::Point2f(x, y));
		}
	}
	return pts1;
}

bool Track::trackf2f(const cv::Mat &pregray, const cv::Mat &nowgray, vector<cv::Point2f> &pts1, vector<cv::Point2f> &pts2) {
	
	//Forward tracking:  ��tʱ��ͼ���е�A����ٵ�t+1ʱ��ͼ���е�B��
	calcOpticalFlowPyrLK(pregray, nowgray, pts1, pts2, status, similarity, window_size, level, term_criteria, lambda, 0);
	//Backward tracking: ��t+1ʱ��ͼ���е�B����ٵ�tʱ��ͼ���е�C��
	calcOpticalFlowPyrLK(nowgray, pregray, pts2, pointsFB, FB_status, FB_error, window_size, level, term_criteria, lambda, 0);

	for (int i = 0; i < pts1.size(); ++i) {
		FB_error[i] = norm(pointsFB[i] - pts1[i]);
	}

	normCrossCorrelation(pregray, nowgray, pts1, pts2);

	//Filter out points with FB_error[i] < median(FB_error) && points with sim_error[i] > median(sim_error)
	float simmed = median(similarity);
	size_t i, k;
	for (i = k = 0; i < pts2.size(); ++i) {
		if (!status[i])
			continue;
		if (similarity[i] > simmed) {
			pts1[k] = pts1[i];
			pts2[k] = pts2[i];
			FB_error[k] = FB_error[i];
			k++;
		}
	}

	if (k == 0)
		return false;
	pts1.resize(k);
	pts2.resize(k);
	FB_error.resize(k);

	//�ڶ���ɸѡ���ҳ�FB_error < median(FB_error)�ĵ㣬��ɾ����Щ��
	float fbmed = median(FB_error);
	for (i = k = 0; i < pts2.size(); ++i) {
		if (!status[i])
			continue;
		if (FB_error[i] <= fbmed) {
			pts1[k] = pts1[i];
			pts2[k] = pts2[i];
			k++;
		}
	}
	pts1.resize(k);
	pts2.resize(k);
	if (k > 0)
		return true;
	else
		return false;
}

void Track::normCrossCorrelation(const cv::Mat &pregray, const cv::Mat &nowgray, vector<cv::Point2f> &pts1, vector<cv::Point2f> &pts2) {
	cv::Mat rec0(10, 10, CV_8U);
	cv::Mat rec1(10, 10, CV_8U);
	cv::Mat res(1, 1, CV_32F);

	for (int i = 0; i < pts1.size(); i++) {
		if (status[i] == 1) {

			//getRectSubPix����ԭ�ͣ�getRectSubPix( InputArray image, Size patchSize, Point2f center, OutputArray patch��
			//Input Array image: ����ͼ��
			//Size patchSize:    ��ȡͼ���С��
			//Point2f center:    ����ͼ���е�λ�ã�
			//OutputArray patch: ���ͼ��
			getRectSubPix(pregray, cv::Size(10, 10), pts1[i], rec0);
			getRectSubPix(nowgray, cv::Size(10, 10), pts2[i], rec1);

			//ģ��ƥ�䣬ƥ��ǰһ֡�͵�ǰ֡����ȡ��10x10���ؾ��Σ��Ƚ�ƥ����ӳ��ͼ��
			matchTemplate(rec0, rec1, res, CV_TM_CCOEFF_NORMED); //CV_TM_CCOEFF��opencv���ú���
			similarity[i] = ((float *)(res.data))[0];			 //res�Ǹ�1x1�����飬��res.data[0],����vector����Ϊ�˸��ǽ�Լ�ռ�

		}
		else {
			similarity[i] = 0.0;
		}
	}
	rec0.release();												
	rec1.release();
	res.release();
}

float Track::median(vector<float> v) {
	int n = floor(double(v.size() / 2));
	nth_element(v.begin(), v.begin() + n, v.end());	//��v[n]����ź��棬С����ǰ�棬���Ҳ�һ������
	return v[n];
}

void Track::bbPredict(const vector<cv::Point2f>& pts1, const vector<cv::Point2f>& pts2, const BoundingBox &lastbox, BoundingBox &tbb) {
	
	int npoints = (int)pts1.size();
	vector<float> xoff(npoints);
	vector<float> yoff(npoints);
	printf("Track: bbPredict::tracked points : %d\n", npoints);
	for (int i = 0; i < npoints; i++) {
		xoff[i] = pts2[i].x - pts1[i].x;
		yoff[i] = pts2[i].y - pts1[i].y;
	}
	float dx = median(xoff);
	float dy = median(yoff);
	float s;
	if (npoints > 1) {
		vector<float> d;
		d.reserve(npoints*(npoints - 1) / 2);
		for (int i = 0; i < npoints; i++) {
			for (int j = i + 1; j < npoints; j++) {
				d.push_back(norm(pts2[i] - pts2[j]) / norm(pts1[i] - pts1[j]));
			}
		}
		s = median(d);
	}
	else {
		s = 1.0;
	}

	float s1 = 0.5*(s - 1)*lastbox.width;
	float s2 = 0.5*(s - 1)*lastbox.height;
	
	tbb.x = round(lastbox.x + dx - s1);
	tbb.y = round(lastbox.y + dy - s2);
	tbb.width = round(lastbox.width*s);
	tbb.height= round(lastbox.height*s);
	printf("Track: bbPredict::predicted tbb: %d %d %d %d\n", tbb.x, tbb.y, tbb.br().x, tbb.br().y);

}
