#include "stdafx.h"
#include "Vector.h"
#include "Matrix.h"


// ������
Vector::Vector() {
	// �հ��� ����Ʈ ���� �ʱ�ȭ
	handPointDefects = vector<int>(7);

	// �����̱� �� ȭ�� �հ��� ũ����� �� �ʱ�ȭ
	beforeHandTip = vector<vector<Point>>(2, vector<Point>(5));

	// ������ �� �հ��� ũ��� �� �ʱ�ȭ
	handTip = vector<vector<Point>>(2, vector<Point>(5));
}

void Vector::initHandTip() {
	for (int size = 0; size < 5; size++) {
		beforeHandTip[0][size] = Point(0, 0);
	}
	for (int size = 0; size < 5; size++) {
		beforeHandTip[1][size] = Point(0, 0);
	}

	for (int size = 0; size < 5; size++) {
		handTip[0][size] = Point(0, 0);
	}
	for (int size = 0; size < 5; size++) {
		handTip[1][size] = Point(0, 0);
	}
}

void Vector::findDrawContours() {
	// �ܰ��� ã��(Contours Detection)
	findContours(MATRIX->getContoursFrame(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	// �ܰ��� ��Ʈ���� ũ�� ����
	contSize = (int)contours.size();
	
	// �ܰ��� �׸���(Draw Contours)
	for (int i = 0; i < contSize; i++) {
		drawContours(MATRIX->getMainFrame(), contours, i, Scalar(0, 255, 0), 2, 8, hierarchy, 0, Point());
	}
}

void Vector::handRecognization() {
	intVecDefects = vector<vector<Vec4i>>(contSize); // ���� ���ϰ��Լ�
	pointDefects = vector<vector<Point>>(contSize);  // �� ���ϰ��Լ�
	pointHull = vector<vector<Point>>(contSize);	  // �ܰ���
	hullIndexes = vector<vector<int>>(contSize);   // �ܰ��� �ε���
														  // �� �ܰ� ����(hand contours vector)
	handContours = vector<vector<Point>>(2);
	for (int i = 0; i < contSize; i++) {
		//					std::cout << "Size " << endl;
		if (contourArea(contours[i]) > 4000) {
			if (handContours[0].empty()) {
				handContours[0].resize(contours[i].size());
				handContours[0] = contours[i];	//
			}
			else {
				handContours[1].resize(contours[i].size());
				handContours[1] = contours[i];	//
			}
		}
	}
}

void Vector::handTipDetection(KeyBoard& keyboard, int i) {
	//					std::cout << "for " << endl;
	if (!handContours[i].empty()) {
		vector<Point> pointCenter(contSize);
		// ������ �ܰ��� ã�� �޼ҵ�
		// convexHull(���� ����) : �� �� ���� ��� ���� ���� ��� �����ϴ� �ֿܰ� ������ ã��
		convexHull(handContours[i], hullIndexes[i], false);
		
		// convexityDefects : ������ ���� ����, �� �հ��� ������ �� �κ��� ã��
		convexityDefects(handContours[i], hullIndexes[i], intVecDefects[i]);
		
		for (int j = 0; j < hullIndexes[i].size(); j++) {
			int index = hullIndexes[i][j];
			pointHull[i].push_back(handContours[i][index]);
		}

		// ���� ���̸� ã�� ��ƾ?
		int count = 0;
		for (int k = 0; k < intVecDefects[i].size(); k++) {
			if (intVecDefects[i][k][3] > 30 * 256) {

				int index0 = intVecDefects[i][k][0];	//start_index
				int index1 = intVecDefects[i][k][1];	//end_index
				int index2 = intVecDefects[i][k][2];	//farthest_pt_index

				handPointDefects[count] = index2;
				count++;

				pointDefects[i].push_back(handContours[i][index2]);
				//drw depth_point				circle(Main_frame, contours[i][index2], 3, Scalar(0, 0, 255), 3);//red
				pointCenter[i] += handContours[i][index2];
			}
		}

		// ���� �߾��� ã��(���)
		pointCenter[i].x /= count;
		pointCenter[i].y /= count;
		// �� �߾� ũ�⸦ ��ȯ��
		pointCenter[i].x += 10;
		pointCenter[i].y -= 40;

		// ���� �����ӿ� ���� �׸�
		circle(MATRIX->getMainFrame(), pointCenter[i], 3, Scalar(0, 255, 0), 3);				// circle(Main_frame, pointCenter[i], 8*8, Scalar(0, 255, 0), 3);
		
		// ���� �����ӿ� ���� �ܰ����� �׸�
		//drawContours(MATRIX->getMainFrame(), pointHull, i, Scalar(0, 255, 0), 2, 8, hierarchy, 0, Point());
		
		// ���� �߻��� �׸����¸� �׸�
		Rect handSquare((pointCenter[i].x - 100), pointCenter[i].y, 200, 120);
		//rectangle(MATRIX->getMainFrame(), handSquare, Scalar(255, 255, 0));
		if (!handTip[i].empty() && i < 2) { //before value.hjhj
			for (int k = 0; k < 5; k++) {
				beforeHandTip[i][k] = handTip[i][k];
			}
		}
		// �հ��� ���̸� ����
		int preDist = 0, nextDist = 0, currDist = 0;
		int tipCount = 0;

		for (int a = 1; a < handContours[i].size() - 1; a++) {
			if (handContours[i][a].inside(handSquare)) {
				preDist = (int)(pow(pointCenter[i].x - handContours[i][a - 1].x, 2) + pow(pointCenter[i].y - handContours[i][a - 1].y, 2));
				currDist = (int)(pow(pointCenter[i].x - handContours[i][a].x, 2) + pow(pointCenter[i].y - handContours[i][a].y, 2));
				nextDist = (int)(pow(pointCenter[i].x - handContours[i][a + 1].x, 2) + pow(pointCenter[i].y - handContours[i][a + 1].y, 2));

				if (currDist > preDist && currDist > nextDist && currDist > 64 * 64) { //handTip detection.

					handTip[i][tipCount] = handContours[i][a];
					tipCount++;

					if (tipCount >= 5) break;

					a = handPointDefects[tipCount];
				}
			}
		}
		// �հ��� ���� ����
		for (int t = 0; t < tipCount; t++) {
			circle(MATRIX->getMainFrame(), handTip[i][t], 5, Scalar(255, 0, 0), 5);
		}
		// �հ������� Ű�� ������ keyBoardInput ����
		if (!handTip[i].empty()) {
			for (int k = 0; k < 5; k++) {
				if ((handTip[i][k].y - beforeHandTip[i][k].y) > 10 && (handTip[i][k].y - beforeHandTip[i][k].y) < 30
					&& (handTip[i][k].x - beforeHandTip[i][k].x < 20) && (handTip[i][k].x - beforeHandTip[i][k].x >(-20))) {

					/* keyboard input function.*/
					keyboard.keyBoardInput(beforeHandTip[i][k].x, beforeHandTip[i][k].y);
					break;
				}
			}
		}
	}
}

float Vector::innerAngle(float px1, float py1, float px2, float py2, float cx1, float cy1)
{
	float dist1 = sqrt((px1 - cx1)*(px1 - cx1) + (py1 - cy1)*(py1 - cy1));
	float dist2 = sqrt((px2 - cx1)*(px2 - cx1) + (py2 - cy1)*(py2 - cy1));

	float Ax, Ay;
	float Bx, By;
	float Cx, Cy;

	//find closest point to C  
	//printf("dist = %lf %lf\n", dist1, dist2);  

	Cx = cx1;
	Cy = cy1;
	if (dist1 < dist2)
	{
		Bx = px1;
		By = py1;
		Ax = px2;
		Ay = py2;
	}
	else {
		Bx = px2;
		By = py2;
		Ax = px1;
		Ay = py1;
	}
	float Q1 = Cx - Ax;
	float Q2 = Cy - Ay;
	float P1 = Bx - Ax;
	float P2 = By - Ay;

	float A = acos((P1*Q1 + P2*Q2) / (sqrt(P1*P1 + P2*P2) * sqrt(Q1*Q1 + Q2*Q2)));
	A = A * 180 / (float)CV_PI;

	return A;
}


void Vector::handTipDetectWithAngle() {
	
}


// �Ҹ���
Vector::~Vector() { }