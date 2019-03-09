//将会在test5的基础上，单独创建一个线程来运行这个东西

//这里和test4不同的地方是，使用欧式变换矩阵来记录相机的位姿
#include <pangolin/pangolin.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include <thread>
#include <Eigen/Core>
#include <Eigen/Geometry>

#define PI (3.1415926535897932346f)


using namespace std;

static const std::string window_name = "Frame - muti-thread";

typedef struct __Pose
{
    __Pose(const vector<GLfloat> _Twc,const bool _keyFrame):
        Twc(_Twc),keyFrame(_keyFrame)
        {;}

    bool keyFrame;
    vector<GLfloat> Twc;   
}Pose;

void drawFrame(const float w=2.0f);
double deg2rad(const double deg);
Eigen::Matrix3d degEuler2matrix(double pitch,double roll,double yaw);
vector<GLfloat> eigen2glfloat(Eigen::Isometry3d T);
void drawAllFrames(vector<Pose> frames);

//线程配置相关
void run(void);
void setup(void);

//图像显示相关
void setImageData(unsigned char * imageArray, int size){
  for(int i = 0 ; i < size;i++) {
    imageArray[i] = (unsigned char)(rand()/(RAND_MAX/255.0));
  }
}

int main(int argc,char* argv[])
{

    setup();

    thread render_loop;
    render_loop=thread(run);
    render_loop.join();

    return 0;
}



void drawFrame(const float w)
{
    const float h=w*0.75;
    const float z=w*0.6;

    glLineWidth(2);
    

    glBegin(GL_LINES);

    glVertex3f(0,0,0);
    glVertex3f(w,h,z);
    glVertex3f(0,0,0);
    glVertex3f(w,-h,z);
    glVertex3f(0,0,0);
    glVertex3f(-w,-h,z);
    glVertex3f(0,0,0);
    glVertex3f(-w,h,z);
    glVertex3f(w,h,z);
    glVertex3f(w,-h,z);
    glVertex3f(-w,h,z);
    glVertex3f(-w,-h,z);
    glVertex3f(-w,h,z);
    glVertex3f(w,h,z);
    glVertex3f(-w,-h,z);
    glVertex3f(w,-h,z);

    glEnd();
}

double deg2rad(const double deg)
{
    return deg/180.0f*PI;
}


Eigen::Matrix3d degEuler2matrix(double pitch,double roll,double yaw)
{
    Eigen::Vector3d rotation_vector(
            deg2rad((double)yaw),
            deg2rad((double)pitch),
            deg2rad((double)roll));
    Eigen::Matrix3d rotation_matrix=Eigen::Matrix3d::Identity();
    rotation_matrix=Eigen::AngleAxisd(rotation_vector[0],Eigen::Vector3d::UnitZ())
        *Eigen::AngleAxisd(rotation_vector[1],Eigen::Vector3d::UnitY())
        *Eigen::AngleAxisd(rotation_vector[2],Eigen::Vector3d::UnitX());

    return rotation_matrix;

}

vector<GLfloat> eigen2glfloat(Eigen::Isometry3d T)
{
    //注意是列优先
    vector<GLfloat> res;
    for(int j=0;j<4;j++)
    {
        for(int i=0;i<4;i++)
        {
            res.push_back(T(i,j));
        }
    }
    return res;
}


void drawAllFrames(vector<Pose> frames)
{
    size_t n=frames.size();

    for(int i=0;i<n;i++)
    {
        if(frames[i].keyFrame)
        {
            glColor3f(1.0f,0.0f,0.0f);
        }
        else
        {
            glColor3f(0.0f,1.0f,0.0f);
        }

        glPushMatrix();
        glMultMatrixf(frames[i].Twc.data());
        drawFrame();
        glPopMatrix();            

    }
}

//====================== 线程相关 ======================
void setup(void)
{
     //========================= 窗口 ========================
    pangolin::CreateWindowAndBind(
        window_name,     //窗口标题
        640,        //窗口尺寸
        480);       //窗口尺寸
    glEnable(GL_DEPTH_TEST);

    // unset the current context from the main thread
    pangolin::GetBoundWindow()->RemoveCurrent();
}


void run(void)
{

    // fetch the context and bind it to this thread
    pangolin::BindToContext(window_name);

    vector<Pose> frames;


    //========================== 3D 交互器 =====================
    // Define Projection and initial ModelView matrix
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(
            640,480,            //相机图像的长和宽
            420,420,320,240,    //相机的内参,fu fv u0 v0
            0.2,500),           //相机所能够看到的最浅和最深的像素
        pangolin::ModelViewLookAt(
            -20,-20,-20,            //相机光心位置,NOTICE z轴不要设置为0
            0,0,0,              //相机要看的位置
            pangolin::AxisY)    //和观察的方向有关
    );

    //======================== 调节面板 ==============================
     const int UI_WIDTH=180;
     pangolin::View& d_cam = pangolin::CreateDisplay()
    .SetBounds(0.0, 1.0, pangolin::Attach::Pix(UI_WIDTH), 1.0, -640.0f/480.0f)
    .SetHandler(new pangolin::Handler3D(s_cam));

    pangolin::CreatePanel("ui")
      .SetBounds(
          0.0, 
          pangolin::Attach::Pix(UI_WIDTH), 
          0.0, 
          pangolin::Attach::Pix(UI_WIDTH));

    //接下来要开始准备添加控制选项了
    pangolin::Var<double> axisSize("ui.axis_size",5,1,20);

    pangolin::Var<double> frameRoll("ui.frame_roll",0,-90,90);
    pangolin::Var<double> framePitch("ui.frame_pitch",0,-90,90);
    pangolin::Var<double> frameYaw("ui.frame_yaw",0,-180,180);
    pangolin::Var<double> frameX("ui.frame_X",0,-100,100);
    pangolin::Var<double> frameY("ui.frame_Y",0,-100,100);
    pangolin::Var<double> frameZ("ui.frame_Z",0,-100,100);

    pangolin::Var<bool> addFrameBtn("ui.add_frame",false,false);
    pangolin::Var<bool> addKeyFrameBtn("ui.add_keyFrame",false,false);    

    pangolin::Var<bool> resetFrameBtn("ui.reset_frame",false,false);  
    pangolin::Var<bool> resetViewBtn("ui.reset_view",false,false);  

    // =====================  图像窗口 ====================
    
    pangolin::View& d_image = pangolin::Display("image")
      .SetBounds(
          pangolin::Attach::Pix(300),
          pangolin::Attach::Pix(200),
          0.0,
          1.0,
          640.0/480)
      .SetLock(pangolin::LockRight, pangolin::LockTop);

    d_cam.AddDisplay(d_image);
    
    const int width =  64;
    const int height = 48;

    unsigned char* imageArray = new unsigned char[3*width*height];
  pangolin::GlTexture imageTexture(width,height,GL_RGB,false,0,GL_RGB,GL_UNSIGNED_BYTE);


    while( !pangolin::ShouldQuit() )
    {
        // Clear screen and activate view to render into
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        d_cam.Activate(s_cam);

        //改变背景颜色
        glClearColor(0.3,0.3,0.3,0.0);

        //按钮的响应 - 复位视图
        if(pangolin::Pushed(resetViewBtn))
        {
            s_cam.SetModelViewMatrix(
                pangolin::ModelViewLookAt(
            -50,-50,-50,  0,0,0,  pangolin::AxisNegY));

            cout<<"Reset view."<<endl;
        }

        //按钮的响应 - 复位帧的位姿
        if(pangolin::Pushed(resetViewBtn))
        {
            //BUG 目前的问题是，就算是数值复位了，但是控制面板上的gui并不会复位
            framePitch=0.0f;
            frameRoll=0.0f;
            frameYaw=0.0f;
            frameX=0.0f;
            frameY=0.0f;
            frameZ=0.0f;

            cout<<"Reset frame pose."<<endl;          
        }

        //首先计算当前帧的位姿矩阵
        Eigen::Matrix3d  rotationMatrix=degEuler2matrix(
            (double)framePitch,
            (double)frameRoll,
            (double)frameYaw);
        Eigen::Isometry3d T=Eigen::Isometry3d::Identity();
        T.rotate(rotationMatrix);
        T.translate(Eigen::Vector3d(
            (double)frameX,
            (double)frameY,
            (double)frameZ));

        vector<GLfloat> Twc=eigen2glfloat(T);

        //按钮的相应- 添加帧
        if(pangolin::Pushed(addFrameBtn))
        {
            frames.push_back(Pose(Twc,false));
        }

        if(pangolin::Pushed(addKeyFrameBtn))
        {
            frames.push_back(Pose(Twc,true));
        }

           
        //尝试按照谢晓佳的视频中给出的代码绘制
        pangolin::glDrawAxis((double)axisSize);

        //绘制帧
        glPushMatrix();
        glMultMatrixf(Twc.data());
        glColor3f(1.0f,1.0f,1.0f);
        drawFrame();
        glPopMatrix();

        drawAllFrames(frames);
        
        //不要忘记了这个东西!!!
        glFlush();


        //Set some random image data and upload to GPU
        setImageData(imageArray,3*width*height);
        imageTexture.Upload(imageArray,GL_RGB,GL_UNSIGNED_BYTE);

        //display the image
        d_image.Activate();
        glColor3f(1.0,1.0,1.0);
        imageTexture.RenderToViewport();

        // Swap frames and Process Events
        pangolin::FinishFrame();
    }
    
    //return 0;
    // unset the current context from the main thread
    pangolin::GetBoundWindow()->RemoveCurrent();
}