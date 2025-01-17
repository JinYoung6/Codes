#include <pangolin/pangolin.h>

int main( int /*argc*/, char** /*argv*/ )
{
    pangolin::CreateWindowAndBind(
        "headless",     //窗口标题
        640,        //窗口尺寸
        480);       //窗口尺寸
    glEnable(GL_DEPTH_TEST);

    // Define Projection and initial ModelView matrix
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(
            640,480,            //相机图像的长和宽
            420,420,320,240,    //相机的内参,fu fv u0 v0
            0.2,100),           //相机所能够看到的最浅和最深的像素
        pangolin::ModelViewLookAt(
            -2,2,-2,            //相机光心位置,NOTICE z轴不要设置为0
            0,0,0,              //相机要看的位置
            pangolin::AxisY)    //和观察的方向有关
    );

    // Create Interactive View in window
    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(
                0.0, 1.0, 0.0, 1.0,     //表示整个窗口都可以观测到
                -640.0f/480.0f)         //窗口的比例
            .SetHandler(&handler);

    while( !pangolin::ShouldQuit() )
    {
        // Clear screen and activate view to render into
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);

        // Render OpenGL Cube
        //pangolin::glDrawColouredCube();

        //尝试按照谢晓佳的视频中给出的代码绘制
        pangolin::glDrawAxis(3);

        //不知道为什么下面的程序没有起到作用
        glPointSize(5.0f);
        glBegin(GL_POINTS);
        glColor3f(1.0,1.0,1.0);
        glVertex3f(0.0f,0.0f,0.0f);
        glVertex3f(1,0,0);
        glVertex3f(0,2,0);
        glEnd();
        //不要忘记了这个东西!!!
        glFlush();


        // Swap frames and Process Events
        pangolin::FinishFrame();
    }
    
    return 0;
}
