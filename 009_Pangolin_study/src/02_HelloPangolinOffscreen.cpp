//这个demo有问题,现在就不尝试了


#include <pangolin/pangolin.h>

int main( int /*argc*/, char** /*argv*/ )
{
    pangolin::CreateWindowAndBind("Main",640,480,pangolin::Params({{"scheme", "headless"}}));
    glEnable(GL_DEPTH_TEST);

    
    // Define Projection and initial ModelView matrix
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(640,480,420,420,320,240,0.2,100),
        pangolin::ModelViewLookAt(-2,2,-2, 0,0,0, pangolin::AxisY)
    );

    // Create Interactive View in window
    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(0.0, 1.0, 0.0, 1.0, -640.0f/480.0f)
            .SetHandler(&handler);

    pangolin::SaveWindowOnRender("window");

    // Clear screen and activate view to render into
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    d_cam.Activate(s_cam);

    // Render OpenGL Cube
    pangolin::glDrawColouredCube();

    // Swap frames and Process Events
    pangolin::FinishFrame();
    
    pangolin::QuitAll();
    
    return 0;
}
