// Copyright 2018 Slightech Co., Ltd. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <iostream>

#include <opencv2/highgui/highgui.hpp>

// 小觅相机的驱动
#include "mynteyed/camera.h"
#include "mynteyed/utils.h"

// 提供一些基本小工具，本例中用到的字符串的转换
#include "util/cam_utils.h"
// 提供字符串工具
#include "util/counter.h"
// 提供绘图工具
#include "util/cv_painter.h"

// 使用小觅相机的命名空间。其实就是一个宏
MYNTEYE_USE_NAMESPACE

// 主函数
int main(int argc, char const* argv[]) {

    // step 0 数据准备和设备枚举
    Camera cam;                 // 生成相机对象
    DeviceInfo dev_info;        // 生成设备信息对象,怀疑和USB设备有关系

    // 目测是自动选择设备, 如果找不到就返回
    if (!util::select(cam, &dev_info)) {
        return 1;
    }
    // 将相机本身的固件版本\硬件版本等信息输出
    util::print_stream_infos(cam, dev_info.index);

    // 我们程序自己输出要打开的相机的详细信息
    std::cout << "Open device: " << 
        dev_info.index << ", "<<                        // 实际运行的时候, 输出的结果为2 
        dev_info.name << std::endl << std::endl;        // 实际运行的时候, 输出的结果为"/dev/video0""

    // 准备参数
    OpenParams params(dev_info.index);
    {
        // Framerate: 30(default), [0,60], [30](STREAM_2560x720)
        params.framerate = 30;

        // Device mode, default DEVICE_ALL
        //   DEVICE_COLOR: IMAGE_LEFT_COLOR ✓ IMAGE_RIGHT_COLOR ? IMAGE_DEPTH x
        //   DEVICE_DEPTH: IMAGE_LEFT_COLOR x IMAGE_RIGHT_COLOR x IMAGE_DEPTH ✓
        //   DEVICE_ALL:   IMAGE_LEFT_COLOR ✓ IMAGE_RIGHT_COLOR ? IMAGE_DEPTH ✓
        // Note: ✓: available, x: unavailable, ?: depends on #stream_mode
        params.dev_mode = DeviceMode::DEVICE_ALL;

        // Color mode: COLOR_RAW(default), rectified
        // params.color_mode = ColorMode::COLOR_RECTIFIED;
        // params.color_mode = ColorMode::COLOR_RAW;

        // params.color_stream_format = StreamFormat::STREAM_MJPG;


        // Depth mode: colorful(default), gray, raw
        // params.depth_mode = DepthMode::DEPTH_GRAY;

        // Stream mode: left color only
        // params.stream_mode = StreamMode::STREAM_640x480;  // vga
        // params.stream_mode = StreamMode::STREAM_1280x720;  // hd
        // Stream mode: left+right color
        // params.stream_mode = StreamMode::STREAM_1280x480;  // vga
        // 注意只有在特别的分辨率模式下才能够获取相机中的左右双目信息，这里的分辨率相当于左右两目图像拼接起来的分辨率
        params.stream_mode = StreamMode::STREAM_1280x480;  // hd


        // Auto-exposure: true(default), false
        params.state_ae = false;
        

        // Auto-white balance: true(default), false
        params.state_awb = false;

        // IR Depth Only: true, false(default)
        // Note: IR Depth Only mode support frame rate between 15fps and 30fps.
        //     When dev_mode != DeviceMode::DEVICE_ALL,
        //       IR Depth Only mode not be supported.
        //     When stream_mode == StreamMode::STREAM_2560x720,
        //       frame rate only be 15fps in this mode.
        //     When frame rate less than 15fps or greater than 30fps,
        //       IR Depth Only mode will be not available.
        // params.ir_depth_only = true;

        // Infrared intensity: 0(default), [0,10]
        params.ir_intensity = 0;

        // Colour depth image, default 5000. [0, 16384]
        params.colour_depth_value = 5000;
    }

    // Enable what process logics
    cam.EnableProcessMode(ProcessMode::PROC_IMU_ALL);

    // DEBUG
    cam.AutoExposureControl(false);
    cam.AutoWhiteBalanceControl(false);


    // Enable image infos
    // cam.EnableImageInfo(true);
    cam.EnableImageInfo(false);
    
    // step 2 打开相机并且进行检查
    cam.Open(params);

    std::cout << std::endl;
    if (!cam.IsOpened()) {
        std::cerr << "Error: Open camera failed" << std::endl;
        return 1;
    }
    std::cout << "Open device success" << std::endl << std::endl;

    std::cout << "Press ESC/Q on Windows to terminate" << std::endl;

    // 这里需要确认各个流的状态都是正常的
    bool is_left_ok  = cam.IsStreamDataEnabled(ImageType::IMAGE_LEFT_COLOR);
    bool is_right_ok = cam.IsStreamDataEnabled(ImageType::IMAGE_RIGHT_COLOR);
    bool is_depth_ok = cam.IsStreamDataEnabled(ImageType::IMAGE_DEPTH);

    if (is_left_ok) cv::namedWindow("left color");
    if (is_right_ok) cv::namedWindow("right color");
    if (is_depth_ok) cv::namedWindow("depth");

    // step 3 准备绘制器
    CVPainter painter;
    util::Counter counter;          // 用于在下文中计算实际帧率

    
        

    // step 4 Main loop
    for (;;) {

        // step 4.1 等待. 这意味着这个线程就阻塞了
        cam.WaitForStream();
        // DEBUG 放在这里似乎是能够间歇性地起到作用
        // cam.SetExposureTime(1.0f);

        counter.Update();

        if (is_left_ok) {
            // step 4.2 获取左目图像
            auto left_color = cam.GetStreamData(ImageType::IMAGE_LEFT_COLOR);
            // 准备绘制
            if (left_color.img) {
                cv::Mat left = left_color.img->To(ImageFormat::COLOR_BGR)->ToMat();
                // painter.DrawSize(left, CVPainter::TOP_LEFT);
                // painter.DrawStreamData(left, left_color, CVPainter::TOP_RIGHT);
                // painter.DrawInformation(left, util::to_string(counter.fps()),
                    // CVPainter::BOTTOM_RIGHT);
                cv::imshow("left color", left);

                std::cout<<"Color: ("<<left.cols<<", "<<left.rows<<")"<<std::endl;
            }
        }

        if (is_right_ok) {
            auto right_color = cam.GetStreamData(ImageType::IMAGE_RIGHT_COLOR);
            if (right_color.img) {
                cv::Mat right = right_color.img->To(ImageFormat::COLOR_BGR)->ToMat();
                // painter.DrawSize(right, CVPainter::TOP_LEFT);
                // painter.DrawStreamData(right, right_color, CVPainter::TOP_RIGHT);
                cv::imshow("right color", right);
            }
        }

        if (is_depth_ok) {
            auto image_depth = cam.GetStreamData(ImageType::IMAGE_DEPTH);
            if (image_depth.img) {
                cv::Mat depth;
            if (params.depth_mode == DepthMode::DEPTH_COLORFUL) {
                depth = image_depth.img->To(ImageFormat::DEPTH_BGR)->ToMat();
            } else {
                depth = image_depth.img->ToMat();
            }
                // painter.DrawSize(depth, CVPainter::TOP_LEFT);
                // painter.DrawStreamData(depth, image_depth, CVPainter::TOP_RIGHT);
                cv::imshow("depth", depth);
                std::cout<<"depth: ("<<depth.cols<<", "<<depth.rows<<")"<<std::endl;
            }
        }

        char key = static_cast<char>(cv::waitKey(1));
        if (key == 27 || key == 'q' || key == 'Q') {  // ESC/Q
            break;
        }
    }

    // step 5 Stop
    cam.Close();
    cv::destroyAllWindows();
    return 0;
}
