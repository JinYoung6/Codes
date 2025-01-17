/**
 * @file main.h
 * @author guoqing (1337841346@qq.com)
 * @brief 主函数所用到的一些辅助函数的声明都放在这里
 * @version 0.1
 * @date 2019-01-07
 * 
 * @copyright Copyright (c) 2019
 * 
 */


#ifndef __MAIN_H__
#define __MAIN_H__


#include "common.h"
#include "DataReader.h"
#include "MotionDetector_backsub.h"
#include "MotionDetector_framesub.h"
#include "MotionDetector_3framesub.h"
#include "MotionDetector_GMM.h"
#include "MotionDetector_GMM2.h"

/**
 * @brief 显示程序的命令行帮助信息
 * 
 * @param[in] name argv[0]
 */
void dispUsage(char* name);

/**
 * @brief 初始化窗口的位置
 * 
 * @param frameSize 帧大小
 */
void initWindowsPostion(cv::Size frameSize);

/**
 * @brief 
 * 
 * @param detector 
 */
void updateImgs(MotionDetector_DiffBase &detector);

#endif //__MAIN_H__