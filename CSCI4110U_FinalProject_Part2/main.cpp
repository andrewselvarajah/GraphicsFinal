#include <string>
#include <iostream>
#include <fstream>
#include <stack>
#include <cmath>
#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "apis/stb_image.h"

#include "ShaderProgram.h"
#include "trackball.hpp"
#include "ObjMesh.h"

#define NUM_SHADERS 4
#define SHADER_SWITCH_DELAY 5000

#define NUM_SKYBOXES 3
#define SKYBOX_SWITCH_DELAY 20000

int width = 800;
int height = 768;

GLuint programIds[NUM_SHADERS];
int programIndex = 1;
int lastShaderTime = 0;
bool animateShaders = false;
int windowId = 0;




GLuint skyboxProgramId;

GLuint vertexBuffer;
GLuint indexBuffer;
GLenum positionBufferId;
GLuint positions_vbo = 0;
GLuint textureCoords_vbo = 0;
GLuint normals_vbo = 0;
GLuint colours_vbo = 0;
GLuint skybox_vbo = 0;

unsigned int skyboxTextures[NUM_SKYBOXES];
unsigned int skyboxIndex;
int lastSkyboxTime = 0;
bool animateSkyboxes = false;

unsigned int numVertices;

/////////////////////
bool isAnimating = true;
int frame_delay = 500;
int keyFrame = 1;
int lastFrameMillis = 0;
float times = 0;
GLuint programId2;
// projection matrix - perspective projection
glm::mat4 projectionMatrix;

// view matrix - orient everything around our preferred view
glm::mat4 viewMatrix;
/////////////////////

float angle = 0.0f;
float lightOffsetY = 0.0f;
float yRotationSpeed = 0.1f;
float yRotation = -85.0f;
float eyex, eyey, eyez;

float theta = -0.4f;
float phi = -1.0f;
float cameraRadius = 100.0f;



bool animateLight = true;
bool rotateObject = true;


void drawCube(glm::mat4 modelMatrix, glm::vec4 colour);
static GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
static GLuint createShader(const GLenum shaderType, const std::string shaderSource);
static void createTexture(std::string filename);
static unsigned int createCubemap(std::vector<std::string> filenames);


static const GLfloat vertexPositionData[] = {
	-0.5f, -0.5f,  1.0f,  // front
	 0.5f, -0.5f,  1.0f,
	-0.5f,  0.5f,  1.0f,
	 0.5f,  0.5f,  1.0f,
	-0.5f, -0.5f, -1.0f,  // back
	 0.5f, -0.5f, -1.0f,
	-0.5f,  0.5f, -1.0f,
	0.5f,  0.5f, -1.0f
};
static const GLushort indexData[] = {
  0, 1, 2,   // front
  3, 2, 1,
  5, 6, 7,   // back
  5, 4, 6,
  1, 7, 3,   // right
  1, 5, 7,
  4, 0, 2,   // left
  4, 2, 6,
  2, 7, 6,   // top
  2, 3, 7,
  0, 4, 5,   // bottom
  0, 5, 1
};
static void createSkybox(void) {
  float skyboxPositions[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
  };

  unsigned int numVertices = 36;

  glGenBuffers(1, &skybox_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float) * 3, skyboxPositions, GL_STATIC_DRAW);
}

static void createGeometry(void) {
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositionData), vertexPositionData, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);
   
}

static void update(void) {
   int milliseconds = glutGet(GLUT_ELAPSED_TIME);

   if (animateShaders) {
      int duration = milliseconds - lastShaderTime;

      if (duration > SHADER_SWITCH_DELAY) {
         programIndex++;

         if (programIndex >= NUM_SHADERS) {
            programIndex = 0;
         }

         lastShaderTime = milliseconds;
      }
   }

   if (animateSkyboxes) {
      int duration = milliseconds - lastSkyboxTime;

      if (duration > SKYBOX_SWITCH_DELAY) {
         skyboxIndex++;

         if (skyboxIndex >= NUM_SKYBOXES) {
            skyboxIndex = 0;
         }

         lastSkyboxTime = milliseconds;
      }
   }

   // rotate the shape about the y-axis so that we can see the shading
   if (rotateObject) {
      float degrees = (float)milliseconds / 80.0f;
      angle = degrees;
   }

   // move the light position over time along the x-axis, so we can see how it affects the shading
   if (animateLight) {
      float t = milliseconds / 1000.0f;
      lightOffsetY = sinf(t) * 100;
   }

   glutPostRedisplay();
}

glm::mat4 model;

static void render(void) {

   int programId = programIds[programIndex];
   int skyboxTexture = skyboxTextures[skyboxIndex];

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // activate our shader program
   glUseProgram(programId);
	//glUseProgram(programId);

   // turn on depth buffering
   glEnable(GL_DEPTH_TEST);

   // projection matrix - perspective projection
   // FOV:           45°
   // Aspect ratio:  4:3 ratio
   // Z range:       between 0.1 and 100.0
   float aspectRatio = (float)width / (float)height;
   glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);

   // view matrix - orient everything around our preferred view
   glm::mat4 view = glm::lookAt(
      eyePosition,
      glm::vec3(0,0,0),    // where to look
      glm::vec3(0,1,0)     // up
   );

   model = glm::mat4(1.0f);
   // draw the cube map sky box

   // provide the vertex positions to the shaders
   GLint skyboxPositionAttribId = glGetAttribLocation(skyboxProgramId, "position");
   glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
   glEnableVertexAttribArray(skyboxPositionAttribId);
   glVertexAttribPointer(skyboxPositionAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

   // texture sampler - a reference to the texture we've previously created
   GLuint skyboxTextureId = glGetUniformLocation(skyboxProgramId, "u_TextureSampler");
   glActiveTexture(GL_TEXTURE0);  // texture unit 0
   glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
   glUniform1i(skyboxTextureId, 0);

   /**/
   glUseProgram(skyboxProgramId);

   glDepthMask(GL_FALSE);
   glDisable(GL_DEPTH_TEST);
   glFrontFace(GL_CCW);
   glDisable(GL_CULL_FACE);

   // set model-view matrix

   GLuint skyboxMVMatrixId = glGetUniformLocation(skyboxProgramId, "u_MVMatrix");
   glUniformMatrix4fv(skyboxMVMatrixId, 1, GL_FALSE, &view[0][0]);

   // set projection matrix
   GLuint skyboxProjMatrixId = glGetUniformLocation(skyboxProgramId, "u_PMatrix");
   glUniformMatrix4fv(skyboxProjMatrixId, 1, GL_FALSE, &projection[0][0]);

   glBindVertexArray(skyboxPositionAttribId);
   glDrawArrays(GL_TRIANGLES, 0, 36);

   // disable the attribute array
   glDisableVertexAttribArray(skyboxPositionAttribId);

   // draw the subject

   glUseProgram(programId);

   glDepthMask(GL_TRUE);
   glEnable(GL_DEPTH_TEST);

   
   // model matrix: translate, scale, and rotate the model
   model = glm::mat4(1.0f);
   model = glm::mat4_cast(rotation);
   model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));

   // model-view-projection matrix
   glm::mat4 mvp = projection * view * model;
   GLuint mvpMatrixId = glGetUniformLocation(programId, "u_MVPMatrix");
   glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

   // model matrix
   GLuint mMatrixId = glGetUniformLocation(programId, "u_ModelMatrix");
   glUniformMatrix4fv(mMatrixId, 1, GL_FALSE, &model[0][0]);

   GLuint textureId = glGetUniformLocation(programId, "u_TextureSampler");
   glActiveTexture(GL_TEXTURE0);  // texture unit 0
   glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
   glUniform1i(textureId, 0);

   // the position of our camera/eye
   GLuint eyePosId = glGetUniformLocation(programId, "u_EyePosition");
   glUniform3f(eyePosId, eyePosition.x, eyePosition.y, eyePosition.z);

   // the position of our light
   GLuint lightPosId = glGetUniformLocation(programId, "u_LightPos");
   glUniform3f(lightPosId, 1, 8 + lightOffsetY, -2);

   // the colour of our object
   GLuint diffuseColourId = glGetUniformLocation(programId, "u_DiffuseColour");
   glUniform4f(diffuseColourId, 0.3, 0.2, 0.8, 1.0);

   // the shininess of the object's surface
   GLuint shininessId = glGetUniformLocation(programId, "u_Shininess");
   glUniform1f(shininessId, 25);

   // find the names (ids) of each vertex attribute
   GLint positionAttribId = glGetAttribLocation(programId, "position");
   GLint textureCoordsAttribId = glGetAttribLocation(programId, "textureCoords");
   GLint normalAttribId = glGetAttribLocation(programId, "normal");

   // provide the vertex positions to the shaders
   glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
   glEnableVertexAttribArray(positionAttribId);
   glVertexAttribPointer(positionAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

   // provide the vertex texture coordinates to the shaders
   glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo);
   glEnableVertexAttribArray(textureCoordsAttribId);
   glVertexAttribPointer(textureCoordsAttribId, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

   // provide the vertex normals to the shaders
   glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
   glEnableVertexAttribArray(normalAttribId);
   glVertexAttribPointer(normalAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

   // draw the triangles
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
   glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, (void*)0);

   // disable the attribute arrays
   glDisableVertexAttribArray(positionAttribId);
   glDisableVertexAttribArray(textureCoordsAttribId);
   glDisableVertexAttribArray(normalAttribId);
   // activate our shader program
   
   glUseProgram(programId2);

   // colours
   glm::vec4 red(0.8, 0.0, 0.0, 1.0);
   glm::vec4 green(0.0, 0.8, 0.0, 1.0);
   glm::vec4 blue(0.0, 0.0, 0.8, 1.0);
   glm::vec4 yellow(1.0, 1.0, 0.0, 1.0);
   glm::vec4 pink(2.0, 0.0, 3.0, 1.0);
   glm::vec4 white(1.0, 1.0, 1.0, 1.0);

   glm::mat4 baseMatrix = glm::mat4(0.5f);
   baseMatrix = glm::rotate(baseMatrix, glm::radians(yRotation), glm::vec3(0.0f, 1.0f, 0.0f));

   // Reminder:  Use this order for transforms:  scale, rotation, translation

   glm::mat4 torso, upperRLeg, lowerRLeg, upperLLeg, lowerLLeg, leftArm,
	   rightArm, head, closeLArm, closeRArm, farRArm, farLArm, torch, torchHead;

   torso = baseMatrix;
   torso = glm::translate(torso, glm::vec3(0.0f, 4.0f * sin(times) * 0.25, 0.0f));
   drawCube(glm::scale(torso, glm::vec3(3.0f, 3.0f, 1.0f)), red);

   head = torso;
   head = glm::translate(head, glm::vec3(0.0f, 4.8f, 0.0f));
   drawCube(glm::scale(head, glm::vec3(2.0, 1.5f, 1.0f)), green);

   upperRLeg = torso; //No scale from the Torso here
   upperRLeg = glm::translate(upperRLeg, glm::vec3(2.0f, -4.5f, 0.0f));//the pivot
   upperRLeg = glm::rotate(upperRLeg, glm::radians(-30.0f) * sin(times), glm::vec3(1.0f, 0.0f, 0.0f));
   drawCube(glm::scale(upperRLeg, glm::vec3(0.95f)), yellow);
   upperRLeg = glm::translate(upperRLeg, glm::vec3(0.0f, -1.5f, 0.0f));
   drawCube(glm::scale(upperRLeg, glm::vec3(1.0f, 2.0f, 1.0f)), blue);

   lowerRLeg = upperRLeg;
   lowerRLeg = glm::translate(lowerRLeg, glm::vec3(0.0f, -2.0f, 0.0f));
   lowerRLeg = glm::rotate(lowerRLeg, glm::radians(-30.0f) * sin(times) + glm::radians(-35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
   drawCube(glm::scale(lowerRLeg, glm::vec3(0.75f)), yellow);
   lowerRLeg = glm::translate(lowerRLeg, glm::vec3(0.0f, -2.0f, 0.0f));
   drawCube(glm::scale(lowerRLeg, glm::vec3(0.8f, 2.0f, 0.8f)), green);

   upperLLeg = torso; //No scale from the Torso here
   upperLLeg = glm::translate(upperLLeg, glm::vec3(-2.0f, -4.5f, 0.0f));//the pivot
   upperLLeg = glm::rotate(upperLLeg, glm::radians(30.0f) * sin(times), glm::vec3(1.0f, 0.0f, 0.0f));
   drawCube(glm::scale(upperLLeg, glm::vec3(0.95f)), yellow);
   upperLLeg = glm::translate(upperLLeg, glm::vec3(0.0f, -1.5f, 0.0f));
   drawCube(glm::scale(upperLLeg, glm::vec3(1.0f, 2.0f, 1.0f)), blue);

   lowerLLeg = upperLLeg;
   lowerLLeg = glm::translate(lowerLLeg, glm::vec3(0.0f, -2.0f, 0.0f));
   lowerLLeg = glm::rotate(lowerLLeg, glm::radians(30.0f) * sin(times) + glm::radians(-35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
   drawCube(glm::scale(lowerLLeg, glm::vec3(0.75f)), yellow);
   lowerLLeg = glm::translate(lowerLLeg, glm::vec3(0.0f, -2.0f, 0.0f));
   drawCube(glm::scale(lowerLLeg, glm::vec3(0.8f, 2.0f, 0.8f)), green);

   //Right Brachium
   closeRArm = torso;//no scal from torso here
   closeRArm = glm::translate(closeRArm, glm::vec3(4.0f, 1.0f, 0.0f));//the Pivot
   closeRArm = glm::rotate(closeRArm, glm::radians(30.0f) * sin(times), glm::vec3(1.0f, 0.0f, 0.0f));
   drawCube(glm::scale(closeRArm, glm::vec3(0.95f)), yellow);
   closeRArm = glm::translate(closeRArm, glm::vec3(0.0, -1.5, 0.0f));
   drawCube(glm::scale(closeRArm, glm::vec3(1.0, 1.7f, 1.0f)), blue);

   farRArm = closeRArm;
   farRArm = glm::translate(farRArm, glm::vec3(0.0, -2.0, 0.0f));
   farRArm = glm::rotate(farRArm, glm::radians(50.0f) * sin(times) + glm::radians(50.0f), glm::vec3(1.0f, 0.0f, 0.0f));
   drawCube(glm::scale(farRArm, glm::vec3(0.75f)), yellow);
   farRArm = glm::translate(farRArm, glm::vec3(0.0f, -2.0f, 0.0f));
   drawCube(glm::scale(farRArm, glm::vec3(0.8f, 2.0f, 0.8f)), green);

   //Left Arm

   closeLArm = torso;//no scal from torso here
   closeLArm = glm::translate(closeLArm, glm::vec3(-4.0f, 1.0f, 0.0f));//the Pivot
   closeLArm = glm::rotate(closeLArm, glm::radians(-30.0f) * sin(times), glm::vec3(1.0f, 0.0f, 0.0f));
   drawCube(glm::scale(closeLArm, glm::vec3(0.95f)), yellow);
   closeLArm = glm::translate(closeLArm, glm::vec3(0.0, -1.5, 0.0f));
   drawCube(glm::scale(closeLArm, glm::vec3(1.0, 1.7f, 1.0f)), blue);


   farLArm = closeLArm;
   farLArm = glm::translate(farLArm, glm::vec3(0.0, -2.0, 0.0f));
   farLArm = glm::rotate(farLArm, glm::radians(-50.0f) * sin(times) + glm::radians(50.0f), glm::vec3(1.0f, 0.0f, 0.0f));
   drawCube(glm::scale(farLArm, glm::vec3(0.75f)), yellow);
   farLArm = glm::translate(farLArm, glm::vec3(0.0f, -2.0f, 0.0f));
   drawCube(glm::scale(farLArm, glm::vec3(0.8f, 2.0f, 0.8f)), green);

   //torch
   torch = farLArm;
   torch = glm::translate(torch, glm::vec3(0.0, -1.0, 0.0f));
   torch = glm::rotate(torch, glm::radians(0.0f) * sin(times) + glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
   drawCube(glm::scale(torch, glm::vec3(0.25f)), yellow);
   torch = glm::translate(torch, glm::vec3(0.0f, -1.0f, -1.25f));
   drawCube(glm::scale(torch, glm::vec3(0.4f, 0.4f, 3.0f)), pink);

   //torch head
   torchHead = torch;
   torchHead = glm::translate(torchHead, glm::vec3(0.0, -1.0, 0.0f));
   torchHead = glm::rotate(torchHead, glm::radians(0.0f) * sin(times) + glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
   torchHead = glm::translate(torchHead, glm::vec3(0.0f, 1.0f, -4.25f));
   drawCube(glm::scale(torchHead, glm::vec3(0.4f, 0.4f, 0.4f)), white);

   glm::mat4 vMatrix = glm::mat4(1.0f);
   vMatrix = glm::translate(vMatrix, glm::vec3(-eyePosition.x, -eyePosition.y, -eyePosition.z));
   vMatrix = glm::scale(vMatrix, glm::vec3(1.0, 1.0, 1.0));


   
  

	// make the draw buffer to display buffer (i.e. display what we have drawn)
	glutSwapBuffers();
}
void drawCube(glm::mat4 modelMatrix, glm::vec4 colour) {
	// model-view-projection matrix
	glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
	GLuint mvpMatrixId = glGetUniformLocation(programId2, "u_MVP");
	glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

	// cube colour
	GLuint colourId = glGetUniformLocation(programId2, "u_colour");
	glUniform4fv(colourId, 1, (GLfloat*)&colour[0]);

	// enable the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	// configure the attribute array (the layout of the vertex buffer)
	glVertexAttribPointer(positionBufferId, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void *)0);
	glEnableVertexAttribArray(positionBufferId);

	// draw the triangle strip
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_SHORT, (void*)0);

	// disable the attribute array
	glDisableVertexAttribArray(positionBufferId);
}

static void reshape(int w, int h) {
   glViewport(0, 0, w, h);

   width = w;
   height = h;
}
static void reshape2(int width, int height) {
	float aspectRatio = (float)width / (float)height;
	projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);

	// if using perpsective projection, update projection matrix
	glViewport(0, 0, width, height);
}

static void keyboard(unsigned char key, int x, int y) {
   switch (key) {
   case 'l':
      animateLight = !animateLight;
      break;
   case 'r':
      rotateObject = !rotateObject;
      break;
   case 's':
      animateShaders = !animateShaders;
      break;
   
   }
}

int main(int argc, char** argv) {
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize(width, height);
   glutCreateWindow("CSCI 4110U - Final Project");
   glutIdleFunc(&update);
   glutDisplayFunc(&render);
   glutReshapeFunc(&reshape);
   glutMotionFunc(&drag);
   glutMouseFunc(&mouse);
   glutKeyboardFunc(&keyboard);

   glewInit();
   if (!GLEW_VERSION_2_0) {
      std::cerr << "OpenGL 2.0 not available" << std::endl;
      return 1;
   }
   std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "Using OpenGL " << glGetString(GL_VERSION) << std::endl;

   // determine the initial camera position
   eyePosition.x = cameraRadius * sin(theta) * cos(phi);
   eyePosition.y = cameraRadius * sin(theta) * sin(phi);
   eyePosition.z = cameraRadius * cos(theta);



   std::vector<std::string> filenames3;
   filenames3.push_back("textures/right.jpg");
   filenames3.push_back("textures/left.jpg");
   filenames3.push_back("textures/top.jpg");
   filenames3.push_back("textures/bottom.jpg");
   filenames3.push_back("textures/front.jpg");
   filenames3.push_back("textures/back.jpg");


   skyboxTextures[0] = createCubemap(filenames3);
   skyboxIndex = 0;
   glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

   // create the scene geometry
   createGeometry();
   createSkybox();


   // test out quaternions
   glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
   float sqrtHalf = sqrt(0.5f);
   glm::quat rotx(sqrtHalf, sqrtHalf, 0.0f, 0.0f);  // 90 degrees about x
   glm::quat rotz(sqrtHalf, 0.0f, 0.0f, sqrtHalf);  // 90 degrees about z
   rot *= rotx;
   rot *= rotz;

   // load the GLSL shader programs

   ShaderProgram program[NUM_SHADERS];
   program[0].loadShaders("shaders/reflection_vertex.glsl", "shaders/reflection_fragment.glsl");
   program[1].loadShaders("shaders/copper_vertex.glsl", "shaders/copper_fragment.glsl");
   program[2].loadShaders("shaders/refraction_vertex.glsl", "shaders/refraction_fragment.glsl");
   program[3].loadShaders("shaders/combined_vertex.glsl", "shaders/combined_fragment.glsl");
   programIds[0] = program[0].getProgramId();
   programIds[1] = program[1].getProgramId();
   programIds[2] = program[2].getProgramId();
   programIds[3] = program[3].getProgramId();

   ShaderProgram skyboxProgram;
   skyboxProgram.loadShaders("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl");
   skyboxProgramId = skyboxProgram.getProgramId();
   programId2 = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

   // output some basic help
   std::cout << "Controls:" << std::endl;
   std::cout << "\tLeft click + drag - rotate camera" << std::endl;
   std::cout << "\tRight click + drag - zoom camera" << std::endl;
   std::cout << "\tr - Enable/disable object auto-rotation" << std::endl;
   std::cout << "\ts - Enable/disable shader auto-switching" << std::endl;
   std::cout << "\tb - Enable/disable environment auto-switching" << std::endl;

   glutMainLoop();

   return 0;
}

static unsigned int createCubemap(std::vector<std::string> filenames) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, numChannels;
    for (unsigned int i = 0; i < filenames.size(); i++) {
        unsigned char *data = stbi_load(filenames[i].c_str(), &width, &height, &numChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << filenames[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glActiveTexture(GL_TEXTURE0);

    return textureID;
}

static void createTexture(std::string filename) {
  int width, height, numChannels;
  unsigned char *bitmap = stbi_load(filename.c_str(), &width, &height, &numChannels, 4);

  GLuint textureId;
  glGenTextures(1, &textureId);

  // make this texture active
  glBindTexture(GL_TEXTURE_2D, textureId);

  // specify the functions to use when shrinking/enlarging the texture image
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // send the data to OpenGL
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);

  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0);

  // we don't need the bitmap data any longer
  stbi_image_free(bitmap);
}
static GLuint createShader(const GLenum shaderType, const std::string shaderFilename) {
	// load the shader source code
	std::ifstream fileIn(shaderFilename.c_str());

	if (!fileIn.is_open()) {
		return -1;
	}

	std::string shaderSource;
	std::string line;
	while (getline(fileIn, line)) {
		shaderSource.append(line);
		shaderSource.append("\n");
	}

	const char* sourceCode = shaderSource.c_str();

	// create a shader with the specified source code
	GLuint shaderId = glCreateShader(shaderType);
	glShaderSource(shaderId, 1, &sourceCode, nullptr);

	// compile the shader
	glCompileShader(shaderId);

	// check if there were any compilation errors
	int result;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int errorLength;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &errorLength);
		char *errorMessage = new char[errorLength];

		glGetShaderInfoLog(shaderId, errorLength, &errorLength, errorMessage);
		std::cout << "Shader compilation failed: " << errorMessage << std::endl;

		delete[] errorMessage;

		glDeleteShader(shaderId);

		return 0;
	}

	return shaderId;
}

static GLuint createShaderProgram(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename) {
	// create and compile a shader for each
	GLuint vShaderId = createShader(GL_VERTEX_SHADER, vertexShaderFilename);
	GLuint fShaderId = createShader(GL_FRAGMENT_SHADER, fragmentShaderFilename);

	// create and link the shaders into a program
	GLuint programId = glCreateProgram();
	glAttachShader(programId, vShaderId);
	glAttachShader(programId, fShaderId);
	glLinkProgram(programId);
	glValidateProgram(programId);

	// delete the shaders
	glDetachShader(programId, vShaderId);
	glDetachShader(programId, fShaderId);
	glDeleteShader(vShaderId);
	glDeleteShader(fShaderId);

	return programId;
}

