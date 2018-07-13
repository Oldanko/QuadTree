#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <map>
#include <algorithm>

using namespace std;

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

struct point
{
  static constexpr float radius = 0.01f;
  float x, y;
  float vx, vy;
};

std::vector<point> points;

struct QuadTree
{
  unique_ptr<QuadTree> kids[4];
  QuadTree* dad;
  static const int max = 2;
  static const int lvls = 5;
  int lvl;
  map<int, int> ids;

  float x, y;
  float size;
  bool subdivided = false;

  QuadTree(float x = 0, float y = 0, float size = 1, int lvl = 0, QuadTree* dad = nullptr) : x(x), y(y), size(size), lvl(lvl), dad(dad) {}

  void addPoint(int id)
  {
    ids[id] = -1;

    if(ids.size() >= max)
    {
      if (lvl >= lvls) return;
      auto insert = [this](int id)
      {
        int pos = 0;
        if(points[id].x > x) pos+=1;
        if(points[id].y < y) pos+=2;
        kids[pos]->addPoint(id);
        ids[id] = pos;
      };

      if(subdivided)
      {
       insert(id);
       return;
      }
      subdivide();
      for(auto it = ids.begin(); it != ids.end(); it++) insert(it->first);
    }
  }

  void subdivide()
  {
    if(subdivided) return;
    float _size = size/2;
    kids[0] = make_unique<QuadTree>(QuadTree(x - _size, y + _size, _size, lvl+1, this));
    kids[1] = make_unique<QuadTree>(QuadTree(x + _size, y + _size, _size, lvl+1, this));
    kids[2] = make_unique<QuadTree>(QuadTree(x - _size, y - _size, _size, lvl+1, this));
    kids[3] = make_unique<QuadTree>(QuadTree(x + _size, y - _size, _size, lvl+1, this));
    subdivided = true;
  }

  void render()
  {
    if(subdivided)
    {
      glUniform3f(0, x, y, size);
      glDrawArrays(GL_LINES, 0, 4);
      for(auto& kid : kids)
        kid->render();
    }
  }

  bool within(int id)
  {
    if(dad == nullptr) return true;
    return
    points[id].x < x + size &&
    points[id].x > x - size &&
    points[id].y < y + size &&
    points[id].y > y - size;

  }

  void merge()
  {
    subdivided = false;
    for(auto& k : kids)
      k = nullptr;
  }

  vector<int> reduce()
  {
    std::vector<int> v;
    if(!subdivided)
    {
      for(auto it = ids.begin(); it != ids.end(); it++)
        if(!within(it->first))
        {
          v.push_back(it->first);
          ids.erase(it->first);
        }
      return v;
    }

    for(auto& kid : kids){
        for(auto r : kid->reduce())
        {
          if(within(r)) addPoint(r);
          else
          {
            v.push_back(r);
            ids.erase(r);
          }
        }
    }

    if(ids.size() < max)
      merge();
    return v;
  }
};


QuadTree tree;

point cursor;

void update(point& p)
{
  p.x += p.vx;
  p.y += p.vy;
  if(p.x <= -1 || p.x >= 1) p.vx *= -1;
  if(p.y <= -1 || p.y >= 1) p.vy *= -1;
  p.x += p.vx;
  p.y += p.vy;
}

void render(point& p)
{
  glUniform3f(0, p.x, p.y, point::radius);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 14);
}

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
  //glfwSwapInterval(1);
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  if(glewInit() != GLEW_OK)
  {
    cout << "Could not init glew" << endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  GLuint program = LoadShaders(
    "res/shaders/def_vertex.c",
    "res/shaders/def_fragment.c"
  );
  GLuint program_grid = LoadShaders(
    "res/shaders/grid_vertex.c",
    "res/shaders/grid_fragment.c"
  );

  GLuint program_circle = LoadShaders(
    "res/shaders/circle_vertex.c",
    "res/shaders/circle_fragment.c"
  );

  glUseProgram(program_grid);
  glUniform3f(0, 0.0, 0.0, 1.0);
  glUseProgram(program_circle);
  glUniform3f(0, 0.0, 0.0, .05);

  GLuint vao[2], vbo;

  glGenVertexArrays(2, vao);
  glBindVertexArray(vao[0]);

  float triangle[] =
  {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    0.0f,  1.0f, 0.0f,
  };

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(
     0,
     3,
     GL_FLOAT,
     GL_FALSE,
     0,
     (void*)0
  );
  glClearColor(1,1,1,1);

  while(!glfwWindowShouldClose(window))
  {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    cursor.x = (float)xpos/640*2 - 1.0f;
    cursor.y = 1.0f - (float)ypos/480*2;

    static bool clicked = false;
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
      if(!clicked)
      {
        points.push_back(point{
          cursor.x,
          cursor.y,
          rand() % 101 / 10000.0f - 0.005f,
          rand() % 101 / 10000.0f - 0.005f
        });
        tree.addPoint(points.size()-1);
        clicked = true;
      }
    }
    else clicked = false;

    for(auto & p : points) update(p);
    if(0 != tree.reduce().size())
    cout << "Loose balls" << endl;

    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao[1]);

    glUseProgram(program_grid);

    tree.render();

    glUseProgram(program_circle);
    render(cursor);
    for(auto & p : points) render(p);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }


  glDeleteVertexArrays(2, vao);
  glDeleteProgram(program);
  glDeleteProgram(program_grid);
  glDeleteProgram(program_circle);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}


GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
