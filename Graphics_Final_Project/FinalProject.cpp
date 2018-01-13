#include <stdio.h>
#include "SOIL.h"// this is for the image inserted in the graphics window
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <pthread.h>
#include <Windows.h>
#include <MMSystem.h>
#include <GL/glut.h>    // Header File For The GLUT Library 
#include <GL/gl.h>    // Header File For The OpenGL32 Library
#include <GL/glu.h>    // Header File For The GLu32 Library
//#include <unistd.h>     // Header File For sleeping.
#define MAX_PARTICLES 1000
/* ASCII code for the escape key. */
#define ESCAPE 27
#define RAIN 0

using namespace std;
/* The number of our GLUT window */
int window; 
//The Light effect
GLfloat AmbientLight[]  = {0.3f, 0.3f, 0.3f, 1.0f};
GLfloat DiffuseLight[]  = {0.8f, 0.8f, 0.8f, 1.0f};
GLfloat SpecularLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat SpecRef[]       = {0.7f, 0.7f, 0.5f, 1.0f};
GLfloat LightPos[]      = {-50.0f,50.0f,100.0f,1.0f};
GLubyte Shine           = 128; 

//floor colors
float r = 0.0;
float g = 1.0;
float b = 0.0;
float ground_points[21][21][3];
float ground_colors[21][21][4];
float accum = -10.0;

float slowdown = 2.0;
float velocity = 0.0;
float zoom = -20.0;

int loop;
int fall;

GLfloat rotationSpeed = 1.0;
GLfloat translation = -11.0;
GLfloat carRotation = 10.0;
GLfloat CarRotationSpeed = 365 / 360.0 * 0.05 ;

int w1 = 0;
int h1 = 0;
/* rotation angle for the triangle. */
float rtri = -12.0f;//

/* rotation angle for the quadrilateral. */
float rquad = -5.0f;

pthread_t threads[5];
string sounds[4] = {"Rain-SoundBible.com-176235038.wav", "car_crash.wav", "car_start.wav", "vehicle_Driving.wav"};

float rdynamic = -5.0f;

GLuint texture[1];

typedef struct {
	// Life
	bool alive;	// is the particle alive?
	float life;	// particle lifespan
	float fade; // decay
	// color
	float red; 	
	float green;
	float blue;
	// Position/direction
	float xpos; 
	float ypos; 
	float zpos;
	// Velocity/Direction, only goes down in y dir
	float vel;
	// Gravity
	float gravity;
}particles;

// Paticle System
particles par_sys[MAX_PARTICLES]; 

// Initialize/Reset Particles - give them their attributes
void initParticles(int i) {
		par_sys[i].alive = true;
		par_sys[i].life = 1.0;
		par_sys[i].fade = float(rand()%100)/1000.0f+0.003f;
		
		par_sys[i].xpos = (float) (rand() % 21) - 10;
		par_sys[i].ypos = 10.0;
		par_sys[i].zpos = (float) (rand() % 21) - 10;
		
		par_sys[i].red = 0.5;
		par_sys[i].green = 0.5;
		par_sys[i].blue = 1.0;
		
		par_sys[i].vel = velocity;
		par_sys[i].gravity = -0.8;//-0.8;
}

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
// We call this right after our OpenGL window is created.
void InitGL(int Width, int Height)         
{
	int x, z;
  // This Will Clear The Background Color To Black
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);     
  glClearDepth(1.0);                 // Enables Clearing Of The Depth Buffer
  glDepthFunc(GL_LESS);              // The Type Of Depth Test To Do
  glEnable(GL_DEPTH_TEST);           // Enables Depth Testing
  glEnable(GL_LIGHTING);			 //Enabling the light effect
  glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientLight);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseLight);
  glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularLight);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);//this applys the effect of the light on the materials
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glMaterialfv(GL_FRONT, GL_SPECULAR, SpecRef);
  glMateriali(GL_FRONT, GL_SHININESS, Shine);
  glShadeModel(GL_SMOOTH);            // Enables Smooth Color Shading

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();                // Reset The Projection Matrix

  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);  

  glMatrixMode(GL_MODELVIEW);
  
  	// Ground Verticies
    // Ground Colors
    for (z = 0; z < 21; z++) {
    	for (x = 0; x < 21; x++) {
    		ground_points[x][z][0] = x - 10.0;
    		ground_points[x][z][1] = accum;
    		ground_points[x][z][2] = z - 10.0;
    	
    		ground_colors[z][x][0] = r; // red value
    		ground_colors[z][x][1] = g; // green value
    		ground_colors[z][x][2] = b; // blue value
    		ground_colors[z][x][3] = 0.0; // acummulation factor
    	}
    } 

   // Initialize particles
    for (loop = 0; loop < MAX_PARTICLES; loop++) {
        initParticles(loop);
    }
}

//image importor for image textual mapping purpose
int LoadGLTextures()
{
    texture[0] = SOIL_load_OGL_texture
        (
        "background.bmp",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y
        );

    if(texture[0] == 0)
        return false;


    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    return true;
}
//maps the image with the fat textual made
void textureDisplay(void){
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0f, -100.0f,  -150.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 100.0f, -100.0f,  -150.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 100.0f,  100.0f,  -150.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-100.0f,  100.0f,  -150.0f);

    glEnd();
}



/* The function called when our window is resized (which shouldn't happen, because we're fullscreen) */
void ReSizeGLScene(int Width, int Height)
{
  if (Height==0)              // Prevent A Divide By Zero If The Window Is Too Small
    Height=1;

  glViewport(0, 0, Width, Height);        // Reset The Current Viewport And Perspective Transformation

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (Height == 0 || Width==0) return;
  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);
  glMatrixMode(GL_MODELVIEW);
  glViewport(0,0,Width,Height);
}

float ballX = -0.5f;
float ballY = 0.0f;
float ballZ = 0.0f;

void drawBall(void) {
        glColor3f(0.0, 0.0, 1.0); //set ball colou
        glTranslatef(-1,0,-10); //moving it toward the screen a bit on creation
        //glRotatef(ballX,ballX,ballY,ballZ);
        glutSolidSphere (0.3, 20, 20); //create ball.
        glTranslatef(5,0,0); //moving it toward the screen a bit on creation
        glutSolidSphere (0.3, 20, 20); //
        }
        
void drawTire(void) {
        glColor3f(0.0, 0.0, 1.0); //set ball colou
        glTranslatef(-1,0,-15); //moving it toward the screen a bit on creation
        //glRotatef(ballX,ballX,ballY,ballZ);
        glutSolidSphere (0.3, 20, 20); //create ball.
        glTranslatef(5,0,0); //moving it toward the screen a bit on creation
        glutSolidSphere (0.3, 20, 20); //
        }
        
void drawCarFront(){

	glTranslatef(0.0f,0.0,-10.0f);
	glBegin(GL_POLYGON);
	glColor3f(0.4,0.6,0.3);
	glVertex3f(0,3,0);
	glVertex3f(0,0,0);
	glVertex3f(5,0,0);
	glVertex3f(5,3,0);
	glEnd();
	
	glBegin(GL_POLYGON);
	glColor3f(0.4,0.6,0.3);
	glVertex3f(0,3,-5);
	glVertex3f(0,0,-5);
	glVertex3f(5,0,-5);
	glVertex3f(5,3,-5);
	glEnd();
	
	//the roof of the car
	glBegin(GL_POLYGON);
	glColor3f(0.4,0.6,0.3);
	glVertex3f(0,3,0);
	glVertex3f(5,3,0);
	glVertex3f(5,3,rdynamic);
	glVertex3f(0,3,-5);
	glEnd();
	
	//the front glass
	glBegin(GL_POLYGON);
	glColor3f(0.1,0.1,0.1);
	glVertex3f(0,3,0);
	glVertex3f(0,0,0);
	glVertex3f(0,0,-5);
	glVertex3f(0,3,-5);
	glEnd();
	//the front flashlight1
	glBegin(GL_POLYGON);
	glColor3f(1,1,0);//yello light
	glVertex3f(-1.51,1.5,-3.5);
	glVertex3f(-1.51,0.75,-3.5);
	glVertex3f(-1.51,0.75,-5);
	glVertex3f(-1.51,1.5,-5);
	glEnd();
	//the front flashliht2
	glBegin(GL_POLYGON);
	glColor3f(1,1,0);//yellow light
	glVertex3f(-1.51,1.5,0);
	glVertex3f(-1.51,0.75,0);
	glVertex3f(-1.51,0.75,-1.5);
	glVertex3f(-1.51,1.5,-1.5);
	glEnd();
	
	glBegin(GL_POLYGON);
	glColor3f(0.4,0.6,0.3);
	glVertex3f(5,3,0);
	glVertex3f(5,0,0);
	glVertex3f(5,0,-5);
	glVertex3f(5,3,-5);
	glEnd();
	
	glBegin(GL_POLYGON);
	glColor3f(0.4,0.6,0.3);
	glVertex3f(-1.5,1.5,-5);
	glVertex3f(-1.5,1.5,0);
	glVertex3f(0,1.5,0);
	glVertex3f(0,1.5,-5);
	glEnd();
	
	glBegin(GL_POLYGON);
	glColor3f(0.4,0.6,0.3);
	glVertex3f(-1.5,0,-5);
	glVertex3f(-1.5,0,0);
	glVertex3f(0,0,0);
	glVertex3f(0,0,-5);
	glEnd();
	
	glBegin(GL_POLYGON);
	glColor3f(0.4,0.6,0.3);
	glVertex3f(-1.5,1.5,0);
	glVertex3f(-1.5,0,0);
	glVertex3f(0,0,0);
	glVertex3f(0,1.5,0);
	glEnd();
	
	glBegin(GL_POLYGON);
	glColor3f(0.4,0.6,0.3);
	glVertex3f(-1.5,1.5,-5);
	glVertex3f(-1.5,0,-5);
	glVertex3f(0,0,-5);
	glVertex3f(0,1.5,-5);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(0.4,0.6,0.3);
	glVertex3f(-1.5,1.5,-5);
	glVertex3f(-1.5,1.5,0);
	glVertex3f(-1.5,0,0);
	glVertex3f(-1.5,0,-5);
	glEnd();
}
void drawGroundTile(void){
	glBegin(GL_POLYGON);
	glColor3f(0.0,1.0,0.0);
	glVertex3f(20,0,-100);
	glVertex3f(200,0,80);
	glVertex3f(-150,0,80);
	glVertex3f(-20,0,-100);
	glEnd();
}

void drawCube(){
	glColor3f(3.0,0.5,0.0);
	glTranslatef(3.0,1.0,0.0);
	glutSolidCube(3);
}

void drawSphere_top(){
	glColor3f(3.0,0.0,0.0);
	glTranslatef(0.0f,3.5,0.0);
	glutSolidSphere(1,8,9);
}

void drawCone_top_top(){
	glColor3f(0.0,4.5,0.0);
	glTranslatef(0.0f,7.0,0.0);
	glutSolidCone(1,1,5,9);
}

void drawSphere(){
	glColor3f(3.0,0.0,0.0);
	glTranslatef(0.0f,1.0,0.0);
	glutSolidSphere(1,8,9);
}

void drawCone_top(){
	glColor3f(0.0,4.5,0.0);
	glTranslatef(0.0f,3.5,0.0);
	glutSolidCone(1,1,5,9);
}

void drawCube_top_top(){
	glColor3f(3.0,0.5,0.0);
	glTranslatef(0.0f,7.0,0.0);
	glutSolidCube(3);
}

void drawCone(){
	glColor3f(0.0,4.5,0.0);
	glTranslatef(0.0f,1.0,0.0);
	glutSolidCone(1,1,5,9);
}

void drawCube_top(){
	glColor3f(3.0,0.5,0.0);
	glTranslatef(0.0f,3.5,0.0);
	glutSolidCube(3);
}

void drawSphere_top_top(){
	glColor3f(3.0,0.0,0.0);
	glTranslatef(0.0f,7.0,0.0);
	glutSolidSphere(1,8,9);
}

//drawing circles as a track
void DrawCircle(float cx, float cy, float r, int num_segments) {
    
	glBegin(GL_LINE_LOOP);
    for (int ii = 0; ii < num_segments; ii++)   {
        float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 
        float x = r * cosf(theta);//calculate the x component 
        float y = r * sinf(theta);//calculate the y component 
        glVertex2f(x + cx, y + cy);//output vertex 
    }
    glEnd();
}
 
 float Obj_rotation = 0.0;
 float Max_rotation = 0.0;
 float rotation_increment = 0.0;
             
/* The main drawing function. */
void DrawGLScene(void)
{
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        // Clear The Screen And The Depth Buffer
  //glLoadIdentity();                // Reset The View	
  glTranslatef(0,-12.0f,-60.0f);        // Move Left 1.5 Units And Into The Screen 6.0
	//the body part of the car
  glPushMatrix();
  glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(-8,1.0,translation); 
  glColor3f(0.4,0.6,0.3);
  drawCarFront();
  glPopMatrix();
  
  //the car's tire
  glPushMatrix();
  glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(-8,1.0,translation); 
  glColor3f(0.4,0.6,0.3);
  drawBall();
  glPopMatrix();
  
  glPushMatrix();
  glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(-8,1.0,translation); 
  glColor3f(0.4,0.6,0.3);
  drawTire();
  glPopMatrix();
  
  //the ground tile
  glPushMatrix();
  //glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(-8,1.0,-10.0); 
  glColor3f(0.4,0.6,0.3);
  drawGroundTile();
  glPopMatrix();
  
  //circle tracks
  glPushMatrix();
  glRotatef(90,1.0,0.0,0.0);
  glTranslatef(0,1.0,-1.0); 
  glColor3f(1,0,0);
  glLineWidth(6.0);
  DrawCircle(0,0,40,200);
  DrawCircle(0,0,30,200);
  DrawCircle(0,0,20,200);
  DrawCircle(0,0,10,200);
  glLineWidth(2.0);
  glPopMatrix();
  
  //the cube on the track
  glPushMatrix();
  //glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(-10,1.0,-10);
  drawCube();
  glPopMatrix();
  
  glPushMatrix();
  //glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(-10,1.0,-10);
  drawCone_top();
  glPopMatrix();
  
  glPushMatrix();
  //glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(-10,1.0,-10);
  drawSphere_top_top();
  glPopMatrix();
  
  glPushMatrix();
  //glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(23,1.0,10);
  drawSphere();
  glPopMatrix();
  
  glPushMatrix();
  //glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(23,1.0,10);
  drawCone_top();
  glPopMatrix();
  
  glPushMatrix();
  //glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(23,1.0,10);
  drawCube_top_top();
  glPopMatrix();
  
  glPushMatrix();
  //glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(-33,1.0,10);
  drawCone();
  glPopMatrix();
  
  glPushMatrix();
  //glRotatef(carRotation,0.0,1.0,0.0);
  glTranslatef(-33,1.0,10);
  drawCube_top();
  glPopMatrix();
  
  glPushMatrix();
 	//glRotatef(carRotation,0.0,1.0,0.0);
 	glTranslatef(-33,1.0,10);
  drawSphere_top_top();
  glPopMatrix();
  
  /*if(translation+1==rstatic11){
     	CarRotationSpeed =0;
     }
     
     else if(translation+1==rstatic21){
     	CarRotationSpeed =0;
     }
     
     else if(translation+1==rstatic31){
     	CarRotationSpeed =0;
     }*/
   
  //drawPolygonTowers();  
  
 //DrawPolygonTowers();
  //rtri+=0.005f;                    // Increase The Rotation Variable For The Triangle
  /*if(rtri>5){
  	 rtri=-10.0f;
  rquad-=15.0f;                    // Decrease The Rotation Variable For The Quad
  }*/
    /*
  // swap the buffers to display, since double buffering is used.
  //glutSwapBuffers();  */
  }

/* The function called whenever a key is pressed. */
void keyPressed(unsigned char key, int x, int y) 
{
    /* sleep to avoid thrashing this procedure */
   // usleep(100);
    /* If escape is pressed, kill everything. */
    if (key == ESCAPE) 
    { 
    /* shut down our window */
    glutDestroyWindow(window);
    
    /* exit the program...normal termination. */
    exit(0);                  
    }

    if(key == 's'){
    	CarRotationSpeed =0;
    }
}
void keyPress(int key, int x, int y)
{
    if(key==GLUT_KEY_RIGHT){
    	translation -= 5;
    }
    if(key==GLUT_KEY_LEFT){
    	translation += 5;
	}if(key==GLUT_KEY_UP){
		CarRotationSpeed +=0.025;
    	if(CarRotationSpeed>=2){
    		CarRotationSpeed=2;}
	}if(key==GLUT_KEY_DOWN){
		CarRotationSpeed -=0.025;
    	if(CarRotationSpeed<=0){
    		CarRotationSpeed = 0;
		}   
	}
    glutPostRedisplay();
}

// For Rain
void drawRain() {
	float x, y, z;
	for (loop = 0; loop < MAX_PARTICLES; loop=loop+2) {
		if (par_sys[loop].alive == true) {			
			x = par_sys[loop].xpos;
			y = par_sys[loop].ypos;
			z = par_sys[loop].zpos + zoom;
			
			// Draw particles
			glColor3f(0.5, 0.5, 1.0);
			glBegin(GL_LINES);
				glVertex3f(x, y, z);
				glVertex3f(x, y+0.5, z);
			glEnd();
			
			// Update values
			//Move
			// Adjust slowdown for speed!
			par_sys[loop].ypos += par_sys[loop].vel / (slowdown*1000);
			par_sys[loop].vel += par_sys[loop].gravity;
			// Decay
			par_sys[loop].life -= par_sys[loop].fade;
			
			if (par_sys[loop].ypos <= -10) {
				par_sys[loop].life = -1.0;
			}
			//Revive 
			if (par_sys[loop].life < 0.0) {
				initParticles(loop);
			}
		}
	}
}
//void * playSound(void *threadid)
//{
//long tid =2;
//tid = (long)threadid;
//player(sounds[tid], 1);
//pthread_exit(NULL);
//}
///pthread_create(&threads[2], NULL, playSound, (void *)2);

void Runner(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        // Clear The Screen And The Depth Buffer
	//textureDisplay();
	glLoadIdentity();	
	
	drawRain();
	
	//drawPolygonTowers(); 
	
	//drawSphere();
	
	//drawCone();
	PlaySound(TEXT("Rain-SoundBible.com-176235038.wav"), NULL, SND_ASYNC|SND_FILENAME|SND_LOOP);
	//Obj_rotation += rotation_increment;
	
	carRotation += CarRotationSpeed;
	DrawGLScene();
	glutPostRedisplay();
	
	glutSwapBuffers();
}

int main(int argc, char **argv) 
{  
  glutInit(&argc, argv);  

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);  

  /* get a 640 x 480 window */
  glutInitWindowSize(1300, 600);  

  /* the window starts at the upper left corner of the screen */
  glutInitWindowPosition(0, 0);  

  /* Open a window */  
  window = glutCreateWindow("Car Crashing to a Tower of Polygons");  

  /* Register the function to do all our OpenGL drawing. */
  glutDisplayFunc(&Runner); 

  /* Go fullscreen.  This is as soon as possible. */
  //glutFullScreen();

  /* Even if there are no events, redraw our gl scene. */
  glutIdleFunc(&Runner);

  /* Register the function called when our window is resized. */
  glutReshapeFunc(&ReSizeGLScene);

  /* Register the function called when the keyboard is pressed. */
  glutKeyboardFunc(&keyPressed);
  
  /* Register the function called when the special keys(UP,DOWN,RIGHT,LEFT)*/
  glutSpecialFunc(&keyPress);
  
  glEnable(GL_DEPTH_TEST);
  
  LoadGLTextures();

  /* Initialize our window. */
  InitGL(1300, 600);
  
  /* Start Event Processing Engine */  
  glutMainLoop();  

  return 1;
}
