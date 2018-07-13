#include <GLFW/glfw3.h>

#include <iostream>
using namespace std;

int main(int argc, char**argv)
{
  if(!glfwInit())
  {
    cout << "Could not init glfw3" << endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL window", NULL, NULL);
  if(!window)
  {
    cout << "Could not create a glfw3 window" << endl;
    glfwTerminate();
    return -1;
  }
  glfwSwapInterval(1);
  glfwMakeContextCurrent(window);

  while(!glfwWindowShouldClose(window))
  {

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
