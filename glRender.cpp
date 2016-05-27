


#include <stdio.h>    /* for printf and NULL */
#include <stdlib.h>   /* for exit */
#include <math.h>     /* for sqrt, sin, cos, and fabs */
#include <assert.h>   /* for assert */

#include "GL/glew.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif

#ifdef _WIN32
#include "GL/wglew.h"
#else
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/glxew.h>
#endif
#endif

#include "GL/glaux.h"
#include "glm.h"
static float eyeHeight = 3.0f;    /* Vertical height of light. */
static float eyeAngle  = 0.53f;   /* Angle in radians eye rotates around scene. */
static float lightAngle = -0.4;   /* Angle light rotates around scene. */
static float lightHeight = 2.0f;  /* Vertical height of light. */
static float lightTile = 0.0f;    /* Angle light Tile around scene*/
static float projectionMatrix[16];

static const float Kd[3] = { 1, 1, 1 };  /* White. */

static int window_width,window_height;
static GLuint texDepth = 667;

static const char *myProgramName = "render_test";

GLMmodel *  model;  

static int mode =0; //线框模式
static int mode1 =1; //平滑模式
static float m_xRotation = 0.0f;   
static float m_yRotation = 0.0f;  
static float m_zRotation = 0.0f;
static float m_xTranslation = 0.0f;   
static float m_yTranslation = 0.0f;   
static float m_zTranslation = -3.0f;   
static float m_xScaling = 0.5f;   
static float m_yScaling = 0.5f;   
static float m_zScaling = 0.5f; 
int m_ClearColorRed=0.0f;   
int m_ClearColorGreen=0.0f;   
int m_ClearColorBlue=0.0f;  

bool left_key_down=false;
bool right_key_down = false;
bool is_tex=true;
int mouse_x(0),mouse_y(0),mouse_x_add(0),mouse_y_add(0),
	mouse_x1(0),mouse_y1(0),mouse_x_add1(0),mouse_y_add1(0);
float spinx(0),spinx1(0);
float spiny(0),spiny1(0);

/* Forward declared GLUT callbacks registered by main. */
static void reshape(int width, int height);
static void display(void);
static void keyboard(unsigned char c, int x, int y);
static void menu(int item);
static void mouse(int button, int state, int x, int y);
static void motion(int x, int y);
static void setupDemonSampler(void);
static void requestSynchronizedSwapBuffers(void);
static GLuint LoadMapFromBMP();
static void drawCull();
static void init();

void init()
{
	model = glmReadOBJ("I:/download/113172236loadobj/glRender/Debug/BX-008.obj");   
	glmUnitize(model);   
	glmFacetNormals(model);   
	glmVertexNormals(model, 90);  

	glShadeModel(GL_SMOOTH);   
	glEnable(GL_NORMALIZE);   

	// Lights, material properties   
	GLfloat ambientProperties[]  = {0.5f, 0.5f, 0.5f, 1.0f};   
	GLfloat diffuseProperties[]  = {0.8f, 0.8f, 0.8f, 1.0f};   
	GLfloat specularProperties[] = {0.0f, 0.8f, 0.2f, 1.0f};   
	GLfloat lightposition[] = {0.0f, 0.8f, 0.2f, 1.0f}; 
	glClearDepth( 1.0f );   


	glLightfv(GL_LIGHT0,GL_POSITION,lightposition);
	glLightfv( GL_LIGHT0, GL_AMBIENT, ambientProperties);   
	glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseProperties);   
	glLightfv( GL_LIGHT0, GL_SPECULAR, specularProperties);   
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0f);   

	// Default : lighting   
	glEnable(GL_LIGHT0);   
	glEnable(GL_LIGHTING);   
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   
	glClearColor(m_ClearColorRed,m_ClearColorGreen,m_ClearColorBlue,1.0f);  
}


// 空闲处理
void idleFunc()
{
	// 请求重绘
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	window_width=600;window_height=400;
	glutInitWindowSize(window_width,window_height );
	//glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInit(&argc, argv);

	glutCreateWindow(myProgramName);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(idleFunc);
	/* Initialize OpenGL entry points. */
	if (glewInit()!=GLEW_OK || !GLEW_VERSION_1_1) {
		exit(1);
	}
	requestSynchronizedSwapBuffers();
	glClearColor(0.1, 0.1, 0.1, 0);  /* Gray background. */
	glEnable(GL_DEPTH_TEST);         /* Hidden surface removal. */

	glutCreateMenu(menu);
	glutAddMenuEntry("[ ] Animate", ' ');
	glutAttachMenu(GLUT_RIGHT_BUTTON);

   init();


	glutMainLoop();
	return 0;
}




static void reshape(int width, int height)
{
	window_width=width;window_height=height;
	double aspectRatio = (float) width / (float) height;
	double fieldOfView = 40.0; /* Degrees */

	/* Build eye's projection matrix once per window resize. */
	/* Light drawn with fixed-function transformation so also load same
	projection matrix in fixed-function projection matrix. */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fieldOfView, aspectRatio,
		1.0, 50.0);  /* Znear and Zfar */
	glMatrixMode(GL_MODELVIEW);
	/* Set viewport to window dimensions. */
	glViewport(0, 0, width, height);
	glutPostRedisplay();
}

static const double myPi = 3.14159265358979323846;


static void display(void)
{
	/* World-space positions for light and eye. */
	const float eyePosition[4] = { 10*sin(eyeAngle), 
		eyeHeight,
		10*cos(eyeAngle), 1 };
	const float lightPosition[4] = { 4.5*sin(lightAngle), 
		lightHeight,
		4.5*cos(lightAngle), 1 };

	const float lightup[3] =  { 0, cos(lightTile),sin(lightTile) };      /* up is positive Y direction */
	const float lightcenter[3] = {0, lightTile,0 };
	static const float center[3] = { 0, 0, 0 };  /* eye and light point at the origin */
	static const float up[3] = { 0, 1, 0 };      /* up is positive Y direction */


	static float pNear =0.25;//近裁剪面
	static float pFar = 25;//远裁剪面
	static float depthOffset =0.01;//深度偏移值，克服z-fighting
	static int pixelOffset = 0; //像素偏移值
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   
	//glClearColor(1.0,1.0,1.0,1.0f);     
	glClearColor(m_ClearColorRed,m_ClearColorGreen,m_ClearColorBlue,1.0f); 
	// scale / translation / rotation   
	glMatrixMode(GL_MODELVIEW);   
	glLoadIdentity();   
	glTranslatef((spinx1+mouse_x_add1)/window_width,(spiny1+mouse_y_add1)/window_height,-3.0);
	glRotatef((spinx+mouse_x_add), 1.0, 0.0, 0.0); 
	glRotatef((spiny+mouse_y_add),0.0,0.0,1.0);
	if (model!=NULL)
	{
		if(mode1)
			glmDraw(model, GLM_FLAT | GLM_COLOR);   
		else
			glmDraw(model,GLM_SMOOTH | GLM_COLOR);
	}
	glutSwapBuffers();
}

static void idle(void)
{
	lightAngle += 0.008;  /* Add a small angle (in radians). */
	if (lightAngle > 2*myPi) {
		lightAngle -= 2*myPi;
	}
	glutPostRedisplay();
}


static void keyboard(unsigned char c, int x, int y)
{
	static int animating = 0,
		repeat = 0;

	switch (c) {
	case ' ':
		animating = !animating; /* Toggle */
		if (animating) {
			glutIdleFunc(idle);
		} else {
			glutIdleFunc(NULL);
		}    
		break;
	case 't':
		repeat = !repeat;
		if (repeat) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		}
		glutPostRedisplay();
		break;
	case 'm':
	case 'M':if(mode==0){
		glPolygonMode(GL_FRONT,GL_LINE);// 设置正面为线框模式
		glPolygonMode(GL_BACK,GL_LINE); // 设置反面为线框模式
		mode=1;glutPostRedisplay();
		break;
			 }
			 else{
				 glPolygonMode(GL_FRONT,GL_FILL);// 设置正面为填充模式
				 glPolygonMode(GL_BACK,GL_FILL); // 设置反面为填充模式
				 mode=0;glutPostRedisplay();
				 break;
			 }
	case 'f':
	case 'F':if(mode1==0){
		mode1=1;glutPostRedisplay();
		break;
			 }
			 else{
				 mode1=0;glutPostRedisplay();
				 break;
			 }
	case 27:  /* Esc key */
		/* Demonstrate proper deallocation of Cg runtime data structures.
		Not strictly necessary if we are simply going to exit. */
		exit(0);
		break;
	}
}

static void menu(int item)
{
	/* Pass menu item character code to keyboard callback. */
	keyboard((unsigned char)item, 0, 0);
}

/* Use a motion and mouse GLUT callback to allow the viewer and light to
rotate around the monkey head and move the viewer up and down. */

static int beginx, beginy;
static int moving = 0;
static int movingLight = 0;
static int xLightBegin, yLightBegin;

void
	motion(int x, int y)
{
	static const float heightMax = 3,
		heightMin = -1.5;

	if (moving) {
		eyeAngle += 0.005*(beginx - x);
		eyeHeight += 0.03*(y - beginy);
		if (eyeHeight > heightMin&&eyeHeight < heightMax) {
			beginx = x;
			beginy = y;
			if(left_key_down)
			{

				mouse_x_add=y-mouse_y;
				mouse_y_add=x-mouse_x;
			}
		}
		glutPostRedisplay();
	}

}

void
	mouse(int button, int state, int x, int y)
{
	const int spinButton = GLUT_LEFT_BUTTON,
		lightButton = GLUT_MIDDLE_BUTTON,
		rightButton = GLUT_RIGHT_BUTTON;

	if (button == spinButton && state == GLUT_DOWN) {
		moving = 1;
		beginx = x;
		beginy = y;
		left_key_down=true;
		mouse_x=x;
		mouse_y=y;
	}
	if (button == spinButton && state == GLUT_UP) {
		moving = 0;
		spinx+=mouse_x_add;
		spiny+=mouse_y_add;
		mouse_x_add=0;
		mouse_y_add=0;
		left_key_down=false;
	}
	if (button == lightButton && state == GLUT_DOWN) { 
		movingLight = 1;
		xLightBegin = x;
		yLightBegin = y;
	}
	if (button == lightButton && state == GLUT_UP) {
		movingLight = 0;
	}

	if (button == lightButton && state == GLUT_DOWN) {

		right_key_down=true;
		mouse_x1=x;
		mouse_y1=y;
	}
	if (button == lightButton && state == GLUT_UP) {
		spinx1+=mouse_x_add1;
		spiny1+=mouse_y_add1;
		mouse_x_add1=0;
		mouse_y_add1=0;
		right_key_down=false;
	}
}

/* Platform-specific code to request synchronized buffer swaps. */

static void requestSynchronizedSwapBuffers(void)
{
#if defined(__APPLE__)
#ifdef CGL_VERSION_1_2
	const GLint sync = 1;
#else
	const long sync = 1;
#endif
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &sync);

#elif defined(_WIN32)
	if (wglSwapIntervalEXT) {
		wglSwapIntervalEXT(1);
	}
#else
	if (glXSwapIntervalSGI) {
		glXSwapIntervalSGI(1);
	}
#endif
}


void drawCull()
{
	glEnable(GL_CULL_FACE);

	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glVertex3f( 12, -2, -12);
	glVertex3f(-12, -2, -12);
	glVertex3f(-12, -2,  12);
	glVertex3f( 12, -2,  12);

	glNormal3f(0, 0, 1);
	glVertex3f(-12, -2, -12);
	glVertex3f( 12, -2, -12);
	glVertex3f( 12, 10, -12);
	glVertex3f(-12, 10, -12);

	glNormal3f(0, 0, -1);
	glVertex3f( 12, -2,  12);
	glVertex3f(-12, -2,  12);
	glVertex3f(-12, 10,  12);
	glVertex3f( 12, 10,  12);

	glNormal3f(0, -1, 0);
	glVertex3f(-12, 10, -12);
	glVertex3f( 12, 10, -12);
	glVertex3f( 12, 10,  12);
	glVertex3f(-12, 10,  12);

	glNormal3f(1, 0, 0);
	glVertex3f(-12, -2,  12);
	glVertex3f(-12, -2, -12);
	glVertex3f(-12, 10, -12);
	glVertex3f(-12, 10,  12);

	glNormal3f(-1, 0, 0);
	glVertex3f(12, -2, -12);
	glVertex3f(12, -2,  12);
	glVertex3f(12, 10,  12);
	glVertex3f(12, 10, -12);
	glEnd();

	glDisable(GL_CULL_FACE);
}


void creatDepthMapTexture(GLuint &tex,GLubyte *bits)
{
	GLuint texobj = tex;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); /* Tightly packed texture data. */

	glBindTexture(GL_TEXTURE_2D, texobj);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, window_width,   window_height, GL_RGB, GL_UNSIGNED_BYTE, bits);

	//gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8,
	//128, 128, GL_RGB, GL_UNSIGNED_BYTE, myDemonTextureImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);

	/* Assumes OpenGL 1.3 or ARB_texture_border_clamp */
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER                0x812D
#endif

	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Kd);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}


