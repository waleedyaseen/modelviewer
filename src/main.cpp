#include "ModelViewer.h"
#include "RunetekColor.h"

void RunApp()
{
    using namespace imp;
    math::RunetekColor::InitColorTables();
    ModelViewer app;
    app.Start();
    math::RunetekColor::DestroyColorTables();
}

#ifdef IMP_PLATFORM_WINDOWS
#    include <Windows.h>
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    RunApp();
    return 0;
}
#else
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    RunApp();
    return 0;
}
#endif
