#pragma once
#include "app.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>


namespace rack {


extern GLFWwindow *gWindow;
extern NVGcontext *gVg;
extern std::shared_ptr<Font> gGuiFont;
extern float gPixelRatio;


void guiInit();
void guiDestroy();
void guiRun();
void guiCursorLock();
void guiCursorUnlock();
bool guiIsModPressed();

} // namespace rack
