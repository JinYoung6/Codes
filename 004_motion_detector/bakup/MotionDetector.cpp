/**
 * @file MotionDetector.cpp
 * @author guoqing (1337841346@qq.com)
 * @brief 各种运动检测算法的实现
 * @details 包括：背景减法
 * @version 0.1
 * @date 2019-01-07
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "common.h"
#include "MotionDetector.h"

using namespace std;
using namespace cv;

//背景减法
cv::Mat MotionDetector::back_sub(
    cv::Mat background,     //背景图像
    cv::Mat frame)          //当前帧图像
{
	//结果图像是在当前帧图像上进行处理得到的
	Mat result = frame.clone();

	//1.将background和frame转为灰度图
	Mat gray1, gray2;
	/* 	
		brief 			cvtColor 是opencv提供的颜色转换函数：
		 
		param[in] src 		原始图像
		param[out] dst 		目标图像
		param[in] code 		色彩空间的转换模式, CV_BGR2GRAY 表示将RGB图像转换成为灰度图像
		void cv::CvtColor( const CvArr* src, CvArr* dst, int code );
	*/

	cvtColor(background, gray1, CV_BGR2GRAY);
	cvtColor(frame, gray2, CV_BGR2GRAY);

	//2.将background和frame作差
	Mat diff;
	/*
		brief 			计算两个数组差的绝对值的函数	
		 
		param[in] src1 		第一个原数组
		param[in] src2 		第二个原数组
		param[out] dst 		计算结果，输出数组
		void cv::absdiff( const CvArr* src1, const CvArr* src2, CvArr* dst );
	*/
	absdiff(gray1, gray2, diff);
	imshow("diff", diff);


	//3.对差值图diff_thresh进行阈值化处理
	Mat diff_thresh;
	/*
		brief 	对单通道图像进行固定阈值操作来得到二值图像
		 
		param[in] 	src 			原始数组
		param[out] 	dst 			输出数组
		param[in] 	threshold 		阈值
		param[in] 	max_value 		当下个参数使用 CV_THRESH_BINARY 或 CV_THRESH_BINARY_INV 的最大值
		param[in] 	threshold_type 	阈值类型。
			为 CV_THRESH_BINARY 时， 	src(x,y) > threshold , dst(x,y) = max_value; 否则 , dst(x,y) = 0
			为 CV_THRESH_BINARY_INV 时，src(x,y) > threshold , dst(x,y) = 0; 否则 , dst(x,y) = max_value
			此外还有一些其他的参数：
			- CV_THRESH_TRUNC: dst(x,y) = threshold, if src(x,y)>threshold; dst(x,y) = src(x,y), otherwise.
			- CV_THRESH_TOZERO: dst(x,y) = src(x,y), if src(x,y)>threshold; dst(x,y) = 0, otherwise.
			- CV_THRESH_TOZERO_INV: dst(x,y) = 0,    if src(x,y)>threshold; dst(x,y) = src(x,y), otherwise.

		void cvThreshold( const CvArr* src, CvArr* dst, double threshold, double max_value, int threshold_type );
	*/
	//这里是对于超过50的像素，直接设置为白，反之直接设置为黑了
	threshold(diff, diff_thresh, 50, 255, CV_THRESH_BINARY);
	imshow("diff_thresh", diff_thresh);

	//4.腐蚀
	/*
		brief 返回指定形状和尺寸的结构元素 
		param shape 	内核的形状，有三种形状可以选择：
						- MORPH_RECT 	矩形
						- MORPH_CORSS	交叉形
						- MORPH_ELLIPSE	椭圆形
		param esize 	内核的大小
		param anchor 	锚点位置
		return cv::Mat 	内核矩阵
		
		cv::Mat cv::getStructuringElement(int shape, Size esize, Point anchor = Point(-1, -1));
	*/

	//Mat kernel_erode = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat kernel_erode = getStructuringElement(MORPH_RECT, Size(1, 1));
	//Mat kernel_dilate = getStructuringElement(MORPH_RECT, Size(15, 15));
    Mat kernel_dilate = getStructuringElement(MORPH_RECT, Size(18, 18));
	//看来腐蚀操作需要用到上面的这个东西
	erode(diff_thresh, diff_thresh, kernel_erode);
	imshow("erode", diff_thresh);

	//5.膨胀
	//膨胀操作也是一样
    dilate(diff_thresh, diff_thresh, kernel_dilate);
	imshow("dilate", diff_thresh);

	//6.查找轮廓并绘制轮廓
	//这里的轮廓其实 是一个点集
	vector<vector<Point>> contours;
	/*
		brief 在二值图像中找到轮廓的函数
		 
		param[in,out] 	image 输入图像，8bit单通道，其中非零的像素点将会被视作为1.
						NOTICE 函数在提取轮廓的过程中会改变图像
		param[out] 	contours 	找到的轮廓，其中的每个轮廓都会被存储为vector<cv::Point>	，所以输出的轮廓的类型就是vector<vector<cv::Point>>
		param[in] 	mode 		检测轮廓的模式。有这样几种：
								- CV_RETR_EXTERNAL 	只提取最外层轮廓
								- CV_RETR_LIST		提取所有轮廓，但是不会建立任何的层次关系
								- CV_RETR_CCOMP		检测所有轮廓，并且组织成为两个层次
								- CV_RETR_TREE 		检测所有轮廓，并且生成完整的嵌套的轮廓层次结构
		param[in] 	method 		轮廓估计的方法。有这样几个可选项：
								- CV_CHAIN_APPROX_NONE 		存储所有的轮廓点。其中两个相邻的轮廓点是水平、竖直，或者对角相邻的关系
								- CV_CHAIN_APPROX_SIMPLE 	压缩那些水平、垂直和呈现对角关系的连续相邻的点，只保留关键点。例如矩形就只存储四个点。
								- CV_CHAIN_APPROX_TC89_L1	应用了Teh-Chin chain 近似算法
								- CV_CHAIN_APPROX_TC89_KCOS 应用了Teh-Chin chain 近似算法
		param[in] 	offset 		可选的偏移，其实就是平移。如果是从ROI区域中提取出图像，然后又需要将它在整体的图像上分析的话会非常有帮助。
		 
		void findContours(InputOutputArray image, OutputArrayOfArrays contours, int mode, int method, Point offset=Point());
	*/
	findContours(diff_thresh,			//输入的二值图像
				 contours,				//轮廓
				 CV_RETR_EXTERNAL,		//仅仅提取最外层的轮廓
				 CV_CHAIN_APPROX_NONE);	//并且存储所有的轮廓点
	/*
	
	brief 绘制图像的轮廓
	 
	param[in,out] 	image 		表示目标图像
	param[in] 		contours 	表示输入的轮廓组，类型是vector<vector<cv::Point>>
	param[in] 		contourIdx 	指出要绘制哪个轮廓。如果这个参数是负值，那么就绘制出所有的轮廓
	param[in]		color 		绘制的轮廓的颜色
	param[in] 		thickness 	轮廓的线宽。如果参数为负值，或者为 CV_FILLED 则表示填充林廓内部
	param[in]		lineType 	线型
	param[in] 		hierarchy 	轮廓结构信息
	param[in]		maxLevel 	TODO ????
	param[in]		offset 		轮廓的水平平移
	 
	void drawContours(InputOutputArray image, InputArrayOfArrays contours, int contourIdx, const Scalar& color,  int thickness=1, 
					  int lineType=8, InputArray hierarchy=noArray(), int maxLevel=INT_MAX,  Point offset=Point());

	*/
	//在result上绘制轮廓
	drawContours(result,				//输出图像
				 contours,				//要绘制的轮廓
				 -1,					//绘制所有的轮廓
				 Scalar(0, 0, 255),		//颜色为红色
				 2);					//线宽为2个像素，其他的参数均保持默认值

	//7.查找正外接矩形
	//有几个轮廓，就有几个正外接矩形
	vector<Rect> boundRect(contours.size());
	//遍历所有的轮廓
	for (int i = 0; i < contours.size(); i++)
	{
		//对当前所遍历到的轮廓计算外接矩形
		/*
			brief 根据传入的轮廓点，计算外接矩形
			 
			param[in] points	轮廓点
			return cv::Rect 	计算出的外接矩形结果

			cv::Rect cv::boundingRect(InputArray points);
		*/
		boundRect[i] = boundingRect(contours[i]);
		//然后在最终的结果图像上绘制这个轮廓的外接矩形
		rectangle(result, 				//输出图像为最终的结果图像
				  boundRect[i], 		//当前遍历到的轮廓的外接矩形
				  Scalar(0, 255, 0), 	//颜色为红色？
				  2);					//线宽为2
	}
	
	return result;//返回result图像
}

//帧差法
static cv::Mat frame_sub(cv::Mat lastframe, cv::Mat frame)
{
    //其实和上面的背景减除法是完全相同的
    return MotionDetector::back_sub(lastframe,frame);
}
