/* Lab 5 base code - transforms using local matrix functions 
  to be written by students - 
  CPE 471 Cal Poly Z. Wood + S. Sueda
*/
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <sys/time.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"

#define PI 3.14159265

/* to use glee */
#define GLEE_OVERWRITE_GL_FUNCTIONS
#include "glee.hpp"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> phong_prog;
shared_ptr<Shape> bunny;
shared_ptr<Shape> cube;
shared_ptr<Shape> sphere;

int g_width, g_height;

float timeOfDay = 0.f;

char keys[6] = { 0, 0, 0, 0, 0, 0 };

glm::vec2 eye_angle = glm::vec2(0.0, 0.0);
glm::vec3 eye_position = glm::vec3(0.0, 0.0, 0.0);

glm::vec3 forward_direction = glm::vec3(0.0, 0.0, 1.0);
glm::vec3 side_direction = glm::vec3(1.0, 0.0, 0.0);

glm::vec3 lightPosition =  glm::vec3(glm::cos(timeOfDay * PI) * 30.f, 
  10.0, glm::sin(timeOfDay * PI) * 30.f);
// Arbitrary light color
glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);

glm::vec3 materialAmbient = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 materialDiffuse = glm::vec3(0.0, 0.0,	0.0);
glm::vec3 materialSpecular = glm::vec3(0.0, 0.0, 0.0);
float materialShininess = 0.0;

void changeMaterialToGold() {
  materialAmbient = glm::vec3(0.24725, 0.1995, 0.0745);
  materialDiffuse = glm::vec3(0.75164, 0.60648,	0.22648);
  materialSpecular = glm::vec3(0.628281, 0.555802, 0.366065);
  materialShininess = 0.4;
}

void changeMaterialToCopper() {
  materialAmbient = glm::vec3(0.19125, 0.0735, 0.0225);
  materialDiffuse = glm::vec3(0.7038, 0.27048, 0.0828);
  materialSpecular = glm::vec3(0.256777, 0.137622, 0.086014);
  materialShininess = 0.1;
}

void changeMaterialToChrome() {
  materialAmbient = glm::vec3(0.25, 0.25, 0.25);
  materialDiffuse = glm::vec3(0.4, 0.4, 0.4);
  materialSpecular = glm::vec3(0.774597, 0.774597, 0.774597);
  materialShininess = 0.6;
}

void changeMaterialToGrass() {
  materialAmbient = glm::vec3(0.25, 1.0, 0.5);
  materialDiffuse = glm::vec3(0.0, 0.0, 0.0);
  materialSpecular = glm::vec3(0.0, 0.0, 0.0);
  materialShininess = 0.0;
}

void changeMaterialToBlackPlastic() {
  materialAmbient = glm::vec3(0.f, 0.f, 0.f);
  materialDiffuse = glm::vec3(0.01f, 0.01f, 0.01f);
  materialSpecular = glm::vec3(0.5f, 0.5f, 0.5f);
  materialShininess = 0.25;
}

float absVal(float a) {
  if(a < 0)
    return -a;
  return a;
}

static void error_callback(int error, const char *description) {
  cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, 
  int mods) {
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
	else if(key == GLFW_KEY_A) {
		if(action == GLFW_PRESS)
			keys[0] = 1;
		else if(action == GLFW_RELEASE)
			keys[0] = 0;
    eye_position = eye_position + vec3(side_direction.x * 1 * 0.05,
      0, side_direction.z * 1 * 0.05);
	}
	else if(key == GLFW_KEY_D) {
		if(action == GLFW_PRESS)
			keys[1] = 1;
		else if(action == GLFW_RELEASE)
			keys[1] = 0;
    eye_position = eye_position + vec3(side_direction.x * -1 * 0.05,
      0, side_direction.z * -1 * 0.05);
	}
	else if(key == GLFW_KEY_W) {
		if(action == GLFW_PRESS)
			keys[2] = 1;
		else if(action == GLFW_RELEASE)
			keys[2] = 0;
    eye_position = eye_position + vec3(forward_direction.x * -1 * 0.05,
      0, forward_direction.z * -1 * 0.05);
	}
	else if(key == GLFW_KEY_S) {
		if(action == GLFW_PRESS)
			keys[3] = 1;
		else if(action == GLFW_RELEASE)
			keys[3] = 0;
    eye_position = eye_position + vec3(forward_direction.x * 1 * 0.05,
      0, forward_direction.z * 1 * 0.05);
	}
}

static void mouse_callback(GLFWwindow *window, int button, int action,
  int mods) {
  double posX, posY;
  if (action == GLFW_PRESS) {
    glfwGetCursorPos(window, &posX, &posY);
    cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
  }
}

static void resize_callback(GLFWwindow *window, int width, int height) {
  g_width = width;
  g_height = height;
  glViewport(0, 0, width, height);
}

static void init() {
  GLSL::checkVersion();

  // Set background color.
	glClearColor(.25f, .75f, 1.f, 0.f);
  // Enable z-buffer test.
  glEnable(GL_DEPTH_TEST);

  // Something about blending.
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  // Initialize mesh.
  bunny = make_shared<Shape>();
  bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
  bunny->resize();
  bunny->init();

  cube = make_shared<Shape>();
  cube->loadMesh(RESOURCE_DIR + "cube.obj");
  cube->resize();
  cube->init();

  sphere = make_shared<Shape>();
  sphere->loadMesh(RESOURCE_DIR + "sphere.obj");
  sphere->resize();
  sphere->init();

  // Initialize phong program
  phong_prog = make_shared<Program>();
  phong_prog->setVerbose(true);
  phong_prog->setShaderNames(RESOURCE_DIR + "phong_vert.glsl",
    RESOURCE_DIR + "phong_frag.glsl");
  phong_prog->init();
  // Perspective matrix
  phong_prog->addUniform("perspective");
  // Rotates the objects in the world to accommodate eye direction
  phong_prog->addUniform("eye_direction");
  // Brings the eye location to location (0, 0, 0), and moves
  // everything relative to that
  phong_prog->addUniform("centering");
  // Puts the model from modelspace to worldspace
  phong_prog->addUniform("placement");
  // Uniforms for shading
  phong_prog->addUniform("light_color");
  phong_prog->addUniform("light_position");
  phong_prog->addUniform("view_position");
  phong_prog->addUniform("material_ambient");
  phong_prog->addUniform("material_diffuse");
  phong_prog->addUniform("material_specular");
  phong_prog->addUniform("material_shininess");
  // Add the attributes
  phong_prog->addAttribute("vertPos");
  phong_prog->addAttribute("vertNor");
}

void placeObject(glm::mat4 *in_mat, glm::vec3 obj_size, glm::vec3 obj_loc) {
  *in_mat = glm::mat4(1.f);
  // Do not know why the following is necessary...
  *in_mat = glm::scale(glm::mat4(1.f), glm::vec3(.5f, .5f, .5f));
  // Scale box to correct size
  *in_mat = glm::scale(glm::mat4(1.f), obj_size) * *in_mat;
  // Put box in correct location
  *in_mat = glm::translate(glm::mat4(1.f), obj_loc) * *in_mat;
}

double signPow(double base, double exponent) {
  if(base < 0) {
    return pow(-base, exponent) * -1;
  }
  return pow(base, exponent);
}

void updateForwardDirection() {
  forward_direction = glm::vec3(glm::sin(eye_angle.y * PI), 0, 
    glm::cos(eye_angle.y * PI));
}

void updateSideDirection() {
  side_direction = glm::vec3(glm::sin(eye_angle.y * PI - PI / 2.f), 0,
    glm::cos(eye_angle.y * PI - PI / 2.f));
}

glm::mat4 createPlacementMatrix(glm::vec3 s, glm::vec3 r, glm::vec3 t) {
  glm::mat4 mat_placement;

  mat_placement = glm::mat4(1.f);

  // Scale
  mat_placement = glm::scale(glm::mat4(1.f), s) * mat_placement;

  // Rotate
  mat_placement = glm::rotate(glm::mat4(1.f), r.y,
    glm::vec3(0.f, 1.f, 0.f)) * mat_placement; 
  mat_placement = glm::rotate(glm::mat4(1.f), r.x,
    glm::vec3(1.f, 0.f, 0.f)) * mat_placement;
  mat_placement = glm::rotate(glm::mat4(1.f), r.z,
    glm::vec3(0.f, 0.f, 1.f)) * mat_placement;

  // Translate
  mat_placement = glm::translate(glm::mat4(1.f), t) *
    mat_placement;

  return mat_placement;
}

void drawGrass(shared_ptr<Program> *use_prog) {
  glm::mat4 mat_placement;

  changeMaterialToGrass();

  glUniform3fv((*use_prog)->getUniform("material_ambient"), 1,
    glm::value_ptr(materialAmbient));
  glUniform3fv((*use_prog)->getUniform("material_diffuse"), 1,
    glm::value_ptr(materialDiffuse));
  glUniform3fv((*use_prog)->getUniform("material_specular"), 1,
    glm::value_ptr(materialSpecular));
  glUniform1f((*use_prog)->getUniform("material_shininess"),
    materialShininess);

  mat_placement = glm::mat4(1.f);
  mat_placement = glm::translate(glm::mat4(1.f), glm::vec3(0, -11, 0)) *
    mat_placement;
  mat_placement = glm::scale(glm::mat4(1.f), glm::vec3(30.f, 0.1f, 30.f)) *
    mat_placement;

  glUniformMatrix4fv((*use_prog)->getUniform("placement"), 1, GL_FALSE,
    glm::value_ptr(mat_placement));

  cube->draw(*use_prog);
}

void drawSnowman(shared_ptr<Program> *use_prog, glm::mat4 mat_placement) {
  glm::mat4 temp_placement;
  
  changeMaterialToChrome();

  glUniform3fv((*use_prog)->getUniform("material_ambient"), 1,
    glm::value_ptr(materialAmbient));
  glUniform3fv((*use_prog)->getUniform("material_diffuse"), 1,
    glm::value_ptr(materialDiffuse));
  glUniform3fv((*use_prog)->getUniform("material_specular"), 1,
    glm::value_ptr(materialSpecular));
  glUniform1f((*use_prog)->getUniform("material_shininess"),
    materialShininess);

  temp_placement = glm::mat4(1.f);
  temp_placement = glm::scale(glm::mat4(1.f), glm::vec3(2.f, 1.f, 1.f)) * 
    temp_placement;
  temp_placement = mat_placement * temp_placement;
  glUniformMatrix4fv((*use_prog)->getUniform("placement"), 1, GL_FALSE,
    glm::value_ptr(temp_placement));

  sphere->draw(*use_prog);

  temp_placement = glm::mat4(1.f);
  temp_placement = glm::scale(glm::mat4(1.f), glm::vec3(1.5f, 1.f, 1.f)) *
    temp_placement;
  temp_placement = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 1.f, 0.f)) *
    temp_placement;
  temp_placement = mat_placement * temp_placement;
  glUniformMatrix4fv((*use_prog)->getUniform("placement"), 1, GL_FALSE,
    glm::value_ptr(temp_placement));

  sphere->draw(*use_prog);

  temp_placement = glm::mat4(1.f);
  temp_placement = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 2.f, 0.f)) *
    temp_placement;
  temp_placement = mat_placement * temp_placement;
  glUniformMatrix4fv((*use_prog)->getUniform("placement"), 1, GL_FALSE,
    glm::value_ptr(temp_placement));

  sphere->draw(*use_prog);

  changeMaterialToBlackPlastic();
  temp_placement = glm::mat4(1.f);
  temp_placement = glm::scale(glm::mat4(1.f), glm::vec3(.1f, .1f, .1f)) *
    temp_placement;
  temp_placement = glm::translate(glm::mat4(1.f), glm::vec3(.5f, 2.f, 1.f)) *
    temp_placement;
  temp_placement = mat_placement * temp_placement;
  glUniformMatrix4fv((*use_prog)->getUniform("placement"), 1, GL_FALSE,
    glm::value_ptr(temp_placement));

  sphere->draw(*use_prog);

  temp_placement = glm::mat4(1.f);
  temp_placement = glm::scale(glm::mat4(1.f), glm::vec3(.1f, .1f, .1f)) *
    temp_placement;
  temp_placement = glm::translate(glm::mat4(1.f), glm::vec3(-.5f, 2.f, 1.f)) *
    temp_placement;
  temp_placement = mat_placement * temp_placement;
  glUniformMatrix4fv((*use_prog)->getUniform("placement"), 1, GL_FALSE,
    glm::value_ptr(temp_placement));

  sphere->draw(*use_prog);

  temp_placement = glm::mat4(1.f);
  temp_placement = glm::scale(glm::mat4(1.f), glm::vec3(1.5f, .1f, .1f)) *
    temp_placement;
  temp_placement = glm::rotate(glm::mat4(1.f), (float) PI / 3,
    glm::vec3(0.f, 0.f, 1.f)) * temp_placement;
  temp_placement = glm::translate(glm::mat4(1.f), glm::vec3(1.f, 1.f, 0.f)) *
    temp_placement;
  temp_placement = mat_placement * temp_placement;
  glUniformMatrix4fv((*use_prog)->getUniform("placement"), 1, GL_FALSE,
    glm::value_ptr(temp_placement));

  sphere->draw(*use_prog);

  temp_placement = glm::mat4(1.f);
  temp_placement = glm::scale(glm::mat4(1.f), glm::vec3(1.5f, .1f, .1f)) *
    temp_placement;
  temp_placement = glm::rotate(glm::mat4(1.f), (float) -PI / 3,
    glm::vec3(0.f, 0.f, 1.f)) * temp_placement;
  temp_placement = glm::translate(glm::mat4(1.f), glm::vec3(-1.f, 1.f, 0.f)) *
    temp_placement;
  temp_placement = mat_placement * temp_placement;
  glUniformMatrix4fv((*use_prog)->getUniform("placement"), 1, GL_FALSE,
    glm::value_ptr(temp_placement));

  sphere->draw(*use_prog);
}

void drawCopperBunny(shared_ptr<Program> *use_prog, glm::mat4 mat_placement) {
  changeMaterialToCopper();

  glUniform3fv((*use_prog)->getUniform("material_ambient"), 1,
    glm::value_ptr(materialAmbient));
  glUniform3fv((*use_prog)->getUniform("material_diffuse"), 1,
    glm::value_ptr(materialDiffuse));
  glUniform3fv((*use_prog)->getUniform("material_specular"), 1,
    glm::value_ptr(materialSpecular));
  glUniform1f((*use_prog)->getUniform("material_shininess"),
    materialShininess);

  glUniformMatrix4fv((*use_prog)->getUniform("placement"), 1, GL_FALSE,
    glm::value_ptr(mat_placement));

  bunny->draw(*use_prog);
}

void drawGoldCube(shared_ptr<Program> *use_prog, glm::mat4 mat_placement) {
  changeMaterialToGold();

  glUniform3fv((*use_prog)->getUniform("material_ambient"), 1,
    glm::value_ptr(materialAmbient));
  glUniform3fv((*use_prog)->getUniform("material_diffuse"), 1,
    glm::value_ptr(materialDiffuse));
  glUniform3fv((*use_prog)->getUniform("material_specular"), 1,
    glm::value_ptr(materialSpecular));
  glUniform1f((*use_prog)->getUniform("material_shininess"),
    materialShininess);

  glUniformMatrix4fv((*use_prog)->getUniform("placement"), 1, GL_FALSE,
    glm::value_ptr(mat_placement));

  cube->draw(*use_prog);
}

void drawBunnies(shared_ptr<Program> *use_prog) {
  glm::mat4 mat_placement;

  mat_placement = createPlacementMatrix(glm::vec3(1.f, 1.f, 1.f),
    glm::vec3(0.f, 0.f / 12.f * PI, 0.f), glm::vec3(0, 0, 10));
  drawCopperBunny(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(1.f, 1.f, 1.f),
    glm::vec3(0.f, 1.f / 12.f * PI, 0.f), glm::vec3(0, 0, -10));
  drawCopperBunny(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(1.f, 1.f, 1.f),
    glm::vec3(0.f, 2.f / 12.f * PI, 0.f), glm::vec3(10, 0, 0));
  drawCopperBunny(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(1.f, 1.f, 1.f),
    glm::vec3(0.f, 3.f / 12.f * PI, 0.f), glm::vec3(-10, 0, 0));
  drawCopperBunny(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(1.f, 1.f, 1.f),
    glm::vec3(0.f, 4.f / 12.f * PI, 0.f), glm::vec3(10, 0, 10));
  drawCopperBunny(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(1.f, 1.f, 1.f),
    glm::vec3(0.f, 5.f / 12.f * PI, 0.f), glm::vec3(-10, 0, 10));
  drawCopperBunny(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(1.f, 1.f, 1.f),
    glm::vec3(0.f, 6.f / 12.f * PI, 0.f), glm::vec3(10, 0, -10));
  drawCopperBunny(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(1.f, 1.f, 1.f),
    glm::vec3(0.f, 7.f / 12.f * PI, 0.f), glm::vec3(-10, 0, -10));
  drawCopperBunny(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(1.f, 1.f, 1.f),
    glm::vec3(0.f, 8.f / 12.f * PI, 0.f), glm::vec3(0, 0, 20));
  drawCopperBunny(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(1.f, 1.f, 1.f),
    glm::vec3(0.f, 9.f / 12.f * PI, 0.f), glm::vec3(0, 0, -20));
  drawCopperBunny(use_prog, mat_placement);
}

void drawSnowmen(shared_ptr<Program> *use_prog) {
  glm::mat4 mat_placement;

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 10.f / 12.f * PI, 0.f), glm::vec3(20, -.5, 0));
  drawSnowman(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 11.f / 12.f * PI, 0.f), glm::vec3(-20, -.5, 0));
  drawSnowman(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 12.f / 12.f * PI, 0.f), glm::vec3(20, -.5, 20));
  drawSnowman(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 13.f / 12.f * PI, 0.f), glm::vec3(-20, -.5, 20));
  drawSnowman(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 14.f / 12.f * PI, 0.f), glm::vec3(20, -.5, -20));
  drawSnowman(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 15.f / 12.f * PI, 0.f), glm::vec3(-20, -.5, -20));
  drawSnowman(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 16.f / 12.f * PI, 0.f), glm::vec3(10, -.5, 20));
  drawSnowman(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 17.f / 12.f * PI, 0.f), glm::vec3(10, -.5, -20));
  drawSnowman(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 18.f / 12.f * PI, 0.f), glm::vec3(-20, -.5, 10));
  drawSnowman(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 19.f / 12.f * PI, 0.f), glm::vec3(20, -.5, 10));
  drawSnowman(use_prog, mat_placement);
}

static void render() {
  glm::mat4 mat_perspective;
  glm::mat4 mat_eye_direction;
  glm::mat4 mat_centering;
  glm::mat4 mat_placement;

  int width, height;
  float aspect;
  double posX, posY;

  shared_ptr<Program> *use_prog;
  use_prog = &phong_prog;

  // Get current frame buffer size.
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  // Clear framebuffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Perspective matrix
  aspect = width / (float)height;
  mat_perspective = glm::perspective(70.0f, aspect, 0.1f, 100.0f);

  // Update eye angle
  glfwGetCursorPos(window, &posX, &posY);
  eye_angle.x = eye_angle.x + 
    signPow(((posY / (double) height) - .5), 2) * -1 * 0.01 *
    (posY >= 0 && posY < (double) height && posX >= 0 && posX < (double) width);
  if(eye_angle.x > 1)
    eye_angle.x = -1;
  if(eye_angle.x < -1)
    eye_angle.x = 1;
  eye_angle.y = eye_angle.y + 
    signPow(((posX / (double) width) - .5), 2) * -1 * 0.01 *
    (posY >= 0 && posY < (double) height && posX >= 0 && posX < (double) width);
  if(eye_angle.y > 1)
    eye_angle.y = -1;
  if(eye_angle.y < -1)
    eye_angle.y = 1;

  // Eye direction matrix
  mat_eye_direction = glm::mat4(1.f);
  // Rotate on the y-axis
  mat_eye_direction = glm::rotate(glm::mat4(1.f), (float) (-eye_angle.y * PI),
    glm::vec3(0.f, 1.f, 0.f)) * mat_eye_direction;
  // Rotate on the x-axis
  mat_eye_direction = glm::rotate(glm::mat4(1.f), (float) (-eye_angle.x * PI),
    glm::vec3(1.f, 0.f, 0.f)) * mat_eye_direction;

  // Update forward direction and side direction
  updateForwardDirection();
  updateSideDirection();

  // Act as if the camera position is (0, 0, 0)
  mat_centering = glm::mat4(1.f);
  mat_centering = glm::translate(glm::mat4(1.f),
    vec3(-eye_position.x, -eye_position.y, -eye_position.z)) *
    mat_centering;

  // use program
  (*use_prog)->bind();

  glUniform3fv((*use_prog)->getUniform("light_color"), 1,
    glm::value_ptr(lightColor));
  glUniform3fv((*use_prog)->getUniform("light_position"), 1,
    glm::value_ptr(lightPosition));
  glUniform3fv((*use_prog)->getUniform("view_position"), 1,
    glm::value_ptr(eye_position));

  glUniformMatrix4fv((*use_prog)->getUniform("perspective"), 1, GL_FALSE,
    glm::value_ptr(mat_perspective));
  glUniformMatrix4fv((*use_prog)->getUniform("eye_direction"), 1, GL_FALSE,
    glm::value_ptr(mat_eye_direction));
  glUniformMatrix4fv((*use_prog)->getUniform("centering"), 1, GL_FALSE,
    glm::value_ptr(mat_centering));

  drawBunnies(use_prog);

  drawSnowmen(use_prog);

  // Draw cubes
  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 20.f / 12.f * PI, 0.f), glm::vec3(-10, -.5, 20));
  drawGoldCube(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 21.f / 12.f * PI, 0.f), glm::vec3(-10, -.5, -20));
  drawGoldCube(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 22.f / 12.f * PI, 0.f), glm::vec3(-20, -.5, -10));
  drawGoldCube(use_prog, mat_placement);

  mat_placement = createPlacementMatrix(glm::vec3(.5f, .5f, .5f),
    glm::vec3(0.f, 23.f / 12.f * PI, 0.f), glm::vec3(20, -.5, -10));
  drawGoldCube(use_prog, mat_placement);

  drawGrass(use_prog);

  (*use_prog)->unbind();

  timeOfDay = timeOfDay + 0.005;
  if(timeOfDay >= 2.f)
    timeOfDay = 0;

  lightPosition = glm::vec3(glm::cos(timeOfDay * PI) * 30.f, 10.0, 
    glm::sin(timeOfDay * PI) * 30.f);
}

int main(int argc, char **argv) {
  if(argc < 2) {
    cout << "Please specify the resource directory." << endl;
    return 0;
  }
  RESOURCE_DIR = argv[1] + string("/");

  // Set error callback.
  glfwSetErrorCallback(error_callback);
  // Initialize the library.
  if(!glfwInit()) {
    return -1;
  }
  //request the highest possible version of OGL - important for mac
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

  // Create a windowed mode window and its OpenGL context.
  window = glfwCreateWindow(640, 480, "do you want to build a snowman", 
    NULL, NULL);
  if(!window) {
    glfwTerminate();
    return -1;
  }
  // Make the window's context current.
  glfwMakeContextCurrent(window);
  // Initialize GLEW.
  glewExperimental = true;
  if(glewInit() != GLEW_OK) {
    cerr << "Failed to initialize GLEW" << endl;
    return -1;
  }
  //weird bootstrap of glGetError
  glGetError();
  cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
  cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  // Set vsync.
  glfwSwapInterval(1);
  // Set keyboard callback.
  glfwSetKeyCallback(window, key_callback);
  //set the mouse call back
  glfwSetMouseButtonCallback(window, mouse_callback);
  //set the window resize call back
  glfwSetFramebufferSizeCallback(window, resize_callback);

  // Initialize scene. Note geometry initialized in init now
  init();

  // Loop until the user closes the window.
  while(!glfwWindowShouldClose(window)) {
    // Render scene.
    render();
    // Swap front and back buffers.
    glfwSwapBuffers(window);
    // Poll for and process events.
    glfwPollEvents();
  }
  // Quit program.
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
