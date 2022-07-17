#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <windows.h> 
#include <math.h>
#include <GL/glew.h>
#include <gl/glut.h>
#include <cmath>
#include <vector>
#include <fstream>

#include "vector.h"
#include "textfile.h"

using namespace std;

const int screenWidth = 800;
const int screenHeight = 800;

Point P1, P2;
int num_clicks = 0;

double angleX = 0, angleY = 0, angleZ = 0;
double scaleF = 1.0;
double translateX = 0, translateY = 0, oldTx, oldTy;

float* depth;

// Shader variables

GLuint vShader_01, fShader_01, glslProgram_color, vShader_02, fShader_02, glslProgram_toon;
GLuint vShader_03, fShader_03, glslProgram_texture;

//Texture variables

unsigned char* image = 0;
GLuint		texture[4];
int image_w, image_h;

GLfloat aTime = 0.0f;
float lPos[3] = { -6, 4, -17 };
double triPos[3] = { -3.2, -1.2, -12.8 };
//double triPos[3] = { -2, 5.5, -16.6 };

bool showClouds = false, animateClouds = false, bumpN = false, highlight = false, diffuseLight = false;
bool textureEarth = false;
float cloudM = 0.0;

bool drawLight = false, drawRays = false;
bool drawTriangle = false;

// Triangle

struct Triangle {
	Point p[3];
	Vector normal;

	inline Triangle(const Point &_p1, const Point &_p2, const Point &_p3) {
		init(_p1, _p2, _p3);
	}

	inline Triangle() {}

	inline void init(const Point &_p1, const Point &_p2, const Point &_p3) {
		p[0] = _p1;
		p[1] = _p2;
		p[2] = _p3;

		Vector a = p[1] - p[0];
		Vector b = p[2] - p[0];

		normal = a * b;
		normal = normalize(normal);
	}
};

bool IntersectTriangle(const Point& RayStart, const Vector& RayDir, const Point& T1, const Point& T2, const Point& T3, Point& I, Vector &R)
{
	Vector a = T2 - T1;
	Vector b = T3 - T1;

	Vector norm = a * b;
	norm = normalize(norm);

	Point root = T1;

	double denom = fabs(norm % RayDir);

	if (fabs(denom) <= 1e-12)
	{
		return false;
	}

	double tHit = norm % (root - RayStart) / denom;

	I = RayStart + RayDir * tHit;

	R = RayDir - 2.0 * (RayDir % norm) * norm;
	R = normalize(R);

	Vector v0 = T2 - T1;
	Vector v1 = T3 - T1;
	Vector v2 = I - T1;

	double dot00 = v0 % v0;
	double dot01 = v0 % v1;
	double dot02 = v0 % v2;
	double dot11 = v1 % v1;
	double dot12 = v1 % v2;

	// Compute barycentric coordinates
	double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	// Check if point is in triangle
	if ((u >= 0.0) && (v >= 0.0) && (u + v <= 1.0))
		return true;

	return false;
}

bool IntersectSphere(const Point& RayStart, const Vector& RayDir, const Point &Center, const double &Radius, 
					 Point& I1, Point& I2, Vector& R)
{
	double a, b, c; //parameters of the quadratic equation
	double delta; //discriminant of the quadratic equation
	double x1, x2, minX, maxX;

	a =  RayDir % RayDir;
	b = 2 * RayDir % (RayStart - Center);
	c = (RayStart - Center) % (RayStart - Center) - Radius * Radius;

	delta = b * b - 4 * a * c;

	if (delta > 0)
	{
		x1 = (-b + sqrt(delta)) / 2 * a;
		x2 = (-b - sqrt(delta)) / 2 * a;

		if (x1 <= x2)
			minX = x1;
		else
			minX = x2;

		if (x1 >= x2)
			maxX = x1;
		else
			maxX = x2;

		I1 = RayStart + minX * RayDir;
		I2 = RayStart + maxX * RayDir;

		Vector n = I1 - Center;
		n = normalize(n);

		R = RayDir - 2.0 * (RayDir % n) * n;
		R = normalize(R);

		return true;
	}

	return false;
}

void drawSphere(double radius, double r, double g, double b)
{
	glColor3d(r, g, b);

	GLUquadricObj* sphere = NULL;
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluQuadricTexture(sphere, TRUE);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluSphere(sphere, radius, 80, 80);
	gluDeleteQuadric(sphere);
}

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line)
{
	//
	// Returns 1 if an OpenGL error occurred, 0 otherwise.
	//
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}


void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}



GLuint setShaders(string vFile, string fFile, GLuint &vShader, GLuint &fShader) {

	char *vs = NULL, *fs = NULL;

	vShader = glCreateShader(GL_VERTEX_SHADER);
	fShader = glCreateShader(GL_FRAGMENT_SHADER);

	//char vFile[80] = "color.vert";
	//char fFile[80] = "color.frag";

	/*string vFile = "color.vert";
	string fFile = "color.frag";*/

	vs = textFileRead(vFile.c_str());
	fs = textFileRead(fFile.c_str());

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(vShader, 1, &vv, NULL);
	glShaderSource(fShader, 1, &ff, NULL);

	free(vs); free(fs);

	glCompileShader(vShader);
	glCompileShader(fShader);

	printShaderInfoLog(vShader);
	printShaderInfoLog(fShader);

	GLuint pID = glCreateProgram();
	glAttachShader(pID, vShader);
	glAttachShader(pID, fShader);

	glLinkProgram(pID);
	printProgramInfoLog(pID);

	//glUseProgram(glslProgram_color);

	return pID;
}


//Read an 8 bit PPM file
unsigned char* readPPM(const char *filename, bool flag, int &dimx, int& dimy) {
	FILE          *ppmfile;
	char          line[256];
	int           i, pixels, x, y, r, g, b;
	unsigned char* p;
	unsigned char* f;

	if ((ppmfile = fopen(filename, "rb")) == NULL) {
		printf("can't open %s\n", filename);
		exit(1);
	}

	fgets(line, 255, ppmfile);
	fgets(line, 255, ppmfile);
	while (line[0] == '#' || line[0] == '\n') fgets(line, 255, ppmfile);
	sscanf(line, "%d %d", &dimx, &dimy);
	fgets(line, 255, ppmfile);

	pixels = dimx * dimy;
	p = (unsigned char *)calloc(3 * pixels, sizeof(unsigned char));
	f = (unsigned char *)calloc(3 * pixels, sizeof(unsigned char));
	// 3 * pixels because of R, G and B channels
	i = 0;
	for (y = 0; y < dimy; y++) {
		for (x = 0; x < dimx; x++) {
			i = 3 * x + y * (3 * dimx);
			r = getc(ppmfile);
			p[i] = r;
			g = getc(ppmfile);
			p[i + 1] = g;
			b = getc(ppmfile);
			p[i + 2] = b;
		}
	}
	fclose(ppmfile);

	unsigned char *ptr1, *ptr2;

	ptr1 = p;
	ptr2 = f + 3 * dimx * (dimy - 1);
	for (y = 0; y < dimy; y++) {
		for (x = 0; x < dimx * 3; x++) {
			*ptr2 = *ptr1;
			ptr1++;
			ptr2++;
		}
		ptr2 -= (2 * 3 * dimx);
	}

	if (!flag) {
		free(p);
		p = 0;
		return(f);
	}
	else {
		free(f);
		f = 0;
		return(p);
	}

	return 0;
}

Point get_3D_pos(int x, int y)
{
	GLdouble pMatrix[16], mMatrix[16];

	glGetDoublev(GL_PROJECTION_MATRIX, pMatrix);
	glGetDoublev(GL_MODELVIEW_MATRIX, mMatrix);

	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);
	y = viewport[3] - y;

	int winWidth = viewport[2];
	int winHeight = viewport[3];

	if (depth)
	{
		delete[] depth;
		depth = 0;
	}

	depth = new float[screenWidth*screenHeight];
	glReadPixels(0, 0, screenWidth, screenHeight, GL_DEPTH_COMPONENT, GL_FLOAT, depth);

	

	float z = depth[y * winWidth + x];

	cout << "Z = " << z << endl;

	if (z >= 1.0f - 1e-12 || z <= 1e-12)
		z = 0.5f;

	GLdouble wx = x;
	GLdouble wy = y;
	GLdouble wz = z;

	GLdouble px, py, pz;

	gluUnProject(wx, wy, wz, mMatrix, pMatrix, viewport, &px, &py, &pz);

	Point p = { (float)px, (float)py, (float)pz };

	cout << "Point = " << p << endl;

	return p;
}

//<<<<<<<<<<<<<<<<<<<<<<< myInit >>>>>>>>>>>>>>>>>>>>
void myInit(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);       // set white background color
	glColor3f(0.0f, 0.0f, 0.0f);          // set the drawing color 
	glPointSize(4.0);       // a ‘dot’ is 4 by 4 pixels

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	//glOrtho(-4.0, 4.0, -4.0 * (double)screenHeight / screenWidth, 4.0 * (double)screenHeight / screenWidth, -40.0, 40.0);

	gluPerspective(45.0, (double) screenHeight / screenWidth, 0.01, 1000);

	glGenTextures(4, &texture[0]);

	cout << "Loading Earth.ppm" << endl;

	// Load and setup the texture 01
	image = readPPM("Earth.ppm", false, image_w, image_h);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_w, image_h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	if (image)
		free(image);

	image = 0;

	cout << "Completed 25%" << endl;
	cout << "Loading Normals.ppm" << endl;

	// Load and setup the texture 02
	image = readPPM("Normals.ppm", false, image_w, image_h);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_w, image_h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	if (image)
		free(image);

	image = 0;

	cout << "Completed 50%" << endl;
	cout << "Loading Clouds.ppm" << endl;


	// Load and setup the texture 03
	image = readPPM("Clouds.ppm", false, image_w, image_h);

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_w, image_h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	if (image)
		free(image);

	image = 0;

	cout << "Completed 75%" << endl;
	cout << "Loading Water.ppm" << endl;

	// Load and setup the texture 04
	image = readPPM("Water.ppm", false, image_w, image_h);

	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_w, image_h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	if (image)
		free(image);

	image = 0;

	cout << "Completed 100%" << endl;
}
//<<<<<<<<<<<<<<<<<<<<<<<< myDisplay >>>>>>>>>>>>>>>>>
void myDisplay(void)
{
	//glClear(GL_COLOR_BUFFER_BIT);     // clear the screen

	glClearColor(1, 1, 1, 1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(glslProgram_color);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslated(lPos[0], lPos[1], lPos[2]);	
	if(drawLight)
		drawSphere(0.2, 1, 1, 0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(0, 0, 0);

	Point LightPos = Point(lPos[0], lPos[1], lPos[2]);
	Point Surface1 = Point(0, 2, -18);
	Point Surface2 = Point(0, -2, -17);

	Vector CL = Surface1 - LightPos;
	Vector CS = Surface2 - LightPos;

	CL = normalize(CL);
	CS = normalize(CS);

	Point EndCL = LightPos + 16 * CL;
	Point EndCS = LightPos + 16 * CS;

	if (drawRays)
	{
		glBegin(GL_LINES);
		glVertex3d(LightPos.x, LightPos.y, LightPos.z);
		glVertex3d(EndCL.x, EndCL.y, EndCL.z);
		glEnd();

		glBegin(GL_LINES);
		glVertex3d(LightPos.x, LightPos.y, LightPos.z);
		glVertex3d(EndCS.x, EndCS.y, EndCS.z);
		glEnd();
	}

	Point I1, I2;
	Vector R;

	bool f = IntersectSphere(LightPos, CL, Point(0, 0, -20), 4, I1, I2, R);

	if (drawRays && f == true)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslated(I1.x, I1.y, I1.z);
		drawSphere(0.1, 1, 0, 0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslated(I2.x, I2.y, I2.z);
		drawSphere(0.1, 1, 0, 0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		Point REnd = I1 + 10 * R;

		glColor3f(0, 1, 0);
		glBegin(GL_LINES);
		glVertex3d(I1.x, I1.y, I1.z);
		glVertex3d(REnd.x, REnd.y, REnd.z);
		glEnd();


		f = IntersectSphere(LightPos, CS, Point(0, 0, -20), 4, I1, I2, R);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslated(I1.x, I1.y, I1.z);
		drawSphere(0.1, 1, 0, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslated(I2.x, I2.y, I2.z);
		drawSphere(0.1, 1, 0, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		REnd = I1 + 10 * R;

		glColor3f(1, 0, 0);

		Point T1 = Point(triPos[0] - 2, triPos[1] + 2, triPos[2] - 2);
		Point T2 = Point(triPos[0], triPos[1] - 2, triPos[2]);
		Point T3 = Point(triPos[0] + 2, triPos[1] - 2, triPos[2] + 2);

		bool didTriangleIntersect;
		Point tI;
		Vector tR;

		if (drawTriangle)
		{
			glBegin(GL_TRIANGLES);
			glNormal3d(0, 1, 0);
			glVertex3d(T1.x, T1.y, T1.z);
			glVertex3d(T2.x, T2.y, T2.z);
			glVertex3d(T3.x, T3.y, T3.z);
			glEnd();

			didTriangleIntersect = IntersectTriangle(I1, R, T1, T2, T3, tI, tR);

			if (didTriangleIntersect)
			{
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				glTranslated(tI.x, tI.y, tI.z);
				drawSphere(0.1, 1, 0, 1);

				REnd = tI;
			}
		}
		

		glColor3f(0, 0, 0);
		glBegin(GL_LINES);
		glVertex3d(I1.x, I1.y, I1.z);
		glVertex3d(REnd.x, REnd.y, REnd.z);
		glEnd();

		if (drawTriangle && didTriangleIntersect)
		{
			Point tREnd = tI + 10 * tR;
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glColor3f(0, 0, 0);
			glBegin(GL_LINES);
			glVertex3d(tI.x, tI.y, tI.z);
			glVertex3d(tREnd.x, tREnd.y, tREnd.z);
			glEnd();
		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	
	glUseProgram(glslProgram_texture);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslated(0, 0, -20);

	glRotated(angleX, 1, 0, 0);
	glRotated(angleY, 0, 1, 0);
	glRotated(angleZ, 0, 0, 1);

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	GLuint texLoc = glGetUniformLocation(glslProgram_texture, "tex0");
	glUniform1i(texLoc, 0);

	texLoc = glGetUniformLocation(glslProgram_texture, "tex1");
	glUniform1i(texLoc, 1);

	texLoc = glGetUniformLocation(glslProgram_texture, "tex2");
	glUniform1i(texLoc, 2);

	texLoc = glGetUniformLocation(glslProgram_texture, "tex3");
	glUniform1i(texLoc, 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture[2]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture[3]);


	GLint loc = glGetUniformLocation(glslProgram_toon, "time");
	glUniform1f(loc, aTime);

	loc = glGetUniformLocation(glslProgram_texture, "lPos");
	glUniform3f(loc, lPos[0], lPos[1], lPos[2]);

	loc = glGetUniformLocation(glslProgram_texture, "showClouds");
	glUniform1i(loc, showClouds);

	loc = glGetUniformLocation(glslProgram_texture, "cloudM");
	glUniform1f(loc, cloudM);

	loc = glGetUniformLocation(glslProgram_texture, "bumpN");
	glUniform1f(loc, bumpN);

	loc = glGetUniformLocation(glslProgram_texture, "highlight");
	glUniform1f(loc, highlight);

	loc = glGetUniformLocation(glslProgram_texture, "diffuseLight");
	glUniform1f(loc, diffuseLight);

	loc = glGetUniformLocation(glslProgram_texture, "textureEarth");
	glUniform1f(loc, textureEarth);

	/*glUseProgram(glslProgram_toon);

	loc = glGetUniformLocation(glslProgram_toon, "lPos");
	glUniform3f(loc, lPos[0], lPos[1], lPos[2]);*/

	glColor3f(0, 0, 1);

	GLUquadricObj* sphere = NULL;
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluQuadricTexture(sphere, TRUE);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluSphere(sphere, 4.0, 80, 80);
	gluDeleteQuadric(sphere);

	glutSwapBuffers();
		
	glFlush();                 // send all output to display 
}

void myResize(int resizeWidth, int resizeHeight)
{
	glutPostRedisplay();
}

void myKeyboard(unsigned char key, int x, int y)
{
	if (key == 'w')
		angleX -= 5;
	
	if (key == 's')
		angleX += 5;

	if (key == 'a')
		angleY -= 5;

	if (key == 'd')
		angleY += 5;

	if (key == 'q')
		angleZ -= 5;

	if (key == 'e')
		angleZ += 5;

	if (key == 'n')
	{
		animateClouds = !animateClouds;
	}

	if (key == 'b')
	{
		bumpN = !bumpN;
	}

	if (key == 'h')
	{
		highlight = !highlight;
	}

	if (key == 'f')
	{
		diffuseLight = !diffuseLight;
	}

	if (key == 'l')
	{
		drawLight = !drawLight;

		if (!drawLight)
		{
			drawRays = false;	
			drawTriangle = false;
		}
	}

	if (key == 'r' && drawLight)
	{
		drawRays = !drawRays;
		if (!drawRays)
			drawTriangle = false;
	}

	if (key == 'g')
	{
		drawTriangle = !drawTriangle;

		if (!drawLight || !drawRays)
			drawTriangle = false;
	}



	if (key == 't')
	{
		textureEarth = !textureEarth;
	}

	if (key == 'c')
	{
		showClouds = !showClouds;
	}

	if (key == '1')
	{
		lPos[0] -= 0.1f;
	}
	
	if (key == '2')
	{
		lPos[0] += 0.1f;
	}

	if (key == '3')
	{
		lPos[1] -= 0.1f;
	}

	if (key == '4')
	{
		lPos[1] += 0.1f;
	}

	if (key == '5')
	{
		lPos[2] -= 0.1f;
	}

	if (key == '6')
	{
		lPos[2] += 0.1f;
	}

	if (key == '7')
	{
		triPos[0] -= 0.1f;
	}

	if (key == '8')
	{
		triPos[0] += 0.1f;
	}

	if (key == '9')
	{
		triPos[1] -= 0.1f;
	}

	if (key == '0')
	{
		triPos[1] += 0.1f;
	}

	if (key == '-')
	{
		triPos[2] -= 0.1f;
	}

	if (key == '=')
	{
		triPos[2] += 0.1f;
	}

	//cout << "Lpos: " << lPos[0] << " " << lPos[1] << " " << lPos[2] << endl;

	cout << "TriPos: " << triPos[0] << " " << triPos[1] << " " << triPos[2] << endl;

	glutPostRedisplay();
}

void myIdle()
{
	aTime += 0.01f;

	if (animateClouds)
	{
		cloudM += 0.0001f;
	}
	

	glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (num_clicks == 0)
		{
			P1 = get_3D_pos(x, y);
			num_clicks++;
		}
		else if (num_clicks == 1)
		{
			P2 = get_3D_pos(x, y);
			num_clicks++;
		}
	}

	if (button == 3 && state == GLUT_DOWN)
	{
		scaleF += 0.1;
	}

	if (button == 4 && state == GLUT_DOWN)
	{
		scaleF -= 0.1;
	}

	glutPostRedisplay();
}

//<<<<<<<<<<<<<<<<<<<<<<<< main >>>>>>>>>>>>>>>>>>>>>>
int main(int argc, char** argv)
{
	glutInit(&argc, argv);          // initialize the toolkit
	//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE); // set display mode

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);



	glutInitWindowSize(screenWidth, screenHeight);     // set window size
	glutInitWindowPosition(100, 100); // set window position on screen
	glutCreateWindow("My Graphics App"); // open the screen window

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(myDisplay);     // register redraw function


	glutReshapeFunc(myResize);
	glutKeyboardFunc(myKeyboard);
	glutIdleFunc(myIdle);
	glutMouseFunc(myMouse);

	myInit();

	glewInit();
	if (glewIsSupported("GL_VERSION_2_0"))
		printf("Ready for OpenGL 2.0\n");
	else {
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}

	glslProgram_color = setShaders("color.vert", "color.frag", vShader_01, fShader_01);
	glslProgram_toon = setShaders("toonf2.vert", "toonf2.frag", vShader_02, fShader_02);
	glslProgram_texture = setShaders("texture.vert", "texture.frag", vShader_03, fShader_03);
	glUseProgram(glslProgram_texture);

	glutMainLoop();      // go into a perpetual loop

	return 0;
}
