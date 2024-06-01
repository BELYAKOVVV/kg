#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"
using namespace std;
bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;


	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}


	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL* ogl, int key)
{

}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL* ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);


	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE* texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH);


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

void create_edges(double h)
{
	double A[] = { 0,0,h };
	double B[] = { 6,2,h };
	double C[] = { 8,-2,h };

	double D[] = { 15, -2,h };

	double E[] = { 10,-4,h };
	double F[] = { 11,-11,h };

	double G[] = { 3,-8,h };

	double K[] = { 5,-4,h };

	glBindTexture(GL_TEXTURE_2D, texId);

	glBegin(GL_TRIANGLES);

	glColor4d(1, 0, 0, 0.7);

	glNormal3f(0, 0, h > 0 ? 1 : -1);
	glTexCoord2d(8.0 / 15.0, 9.0 / 13.0);
	glVertex3dv(C);
	glTexCoord2d(5.0 / 15.0, 7.0 / 13.0);
	glVertex3dv(K);
	glTexCoord2d(10.0 / 15.0, 7.0 / 13.0);
	glVertex3dv(E);

	glNormal3f(0, 0, h > 0 ? 1 : -1);
	glTexCoord2d(1, 9.0 / 13.0);
	glVertex3dv(D);
	glTexCoord2d(8.0 / 15.0, 9.0 / 13.0);
	glVertex3dv(C);
	glTexCoord2d(10.0 / 15.0, 7.0 / 13.0);
	glVertex3dv(E);

	glEnd();

	glBegin(GL_QUADS);

	glNormal3f(0, 0, h > 0 ? 1 : -1);
	glTexCoord2d(10.0 / 15.0, 7.0 / 13.0);
	glVertex3dv(E);
	glTexCoord2d(5.0 / 15.0, 7.0 / 13.0);
	glVertex3dv(K);
	glTexCoord2d(3.0 / 15.0, 3.0 / 13.0);
	glVertex3dv(G);
	glTexCoord2d(11.0 / 15.0, 0);
	glVertex3dv(F);

	glNormal3f(0, 0, h > 0 ? 1 : -1);
	glTexCoord2d(0, 1.0 / 11.0);
	glVertex3dv(A);
	glTexCoord2d(6.0 / 15.0, 1);
	glVertex3dv(B);
	glTexCoord2d(8.0 / 15.0, 9.0 / 13.0);
	glVertex3dv(C);
	glTexCoord2d(5.0 / 15.0, 7.0 / 13.0);
	glVertex3dv(K);

	glEnd();

}

float normal[] = { 0, 0, 0 };

void calc_vector(vector <float> a, vector <float> b) {
	normal[0] = (a[1] * b[2] - b[1] * a[2]);
	normal[1] = (-a[0] * b[2] + b[0] * a[2]);
	normal[2] = a[0] * b[1] - b[0] * a[1];
}

void create_side(double h)
{
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);

	glColor3d(0, 1, 0);

	vector <float> vtr_1 = { -0.9486,-0.3162,0 };
	vector <float> vtr_2 = { 0,0,1 };
	calc_vector(vtr_1, vtr_2);
	glNormal3d(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3d(0, 0, 0);
	glTexCoord2d(0, 1);
	glVertex3d(0, 0, h);
	glTexCoord2d(1, 1);
	glVertex3d(6, 2, h);
	glTexCoord2d(1, 0);
	glVertex3d(6, 2, 0);

	vtr_1 = { -0.4472 ,0.8944, 0 };
	vtr_2 = { 0, 0, 1 };
	calc_vector(vtr_1, vtr_2);
	glNormal3f(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3d(6, 2, 0);
	glTexCoord2d(0, 1);
	glVertex3d(6, 2, h);
	glTexCoord2d(1, 1);
	glVertex3d(8, -2, h);
	glTexCoord2d(1, 0);
	glVertex3d(8, -2, 0);

	vtr_1 = { -1,0,0 };
	vtr_2 = { 0,0,1 };
	calc_vector(vtr_1, vtr_2);
	glNormal3f(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3d(8, -2, 0);
	glTexCoord2d(0, 1);
	glVertex3d(8, -2, h);
	glTexCoord2d(1, 1);
	glVertex3d(15, -2, h);
	glTexCoord2d(1, 0);
	glVertex3d(15, -2, 0);

	vtr_1 = { 0.9284,0.3713,0 };
	vtr_2 = { 0,0,1 };
	calc_vector(vtr_1, vtr_2);
	glNormal3f(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3d(15, -2, 0);
	glTexCoord2d(0, 1);
	glVertex3d(15, -2, h);
	glTexCoord2d(1, 1);
	glVertex3d(10, -4, h);
	glTexCoord2d(1, 0);
	glVertex3d(10, -4, 0);

	vtr_1 = { -0.1414,0.9899,0 };
	vtr_2 = { 0,0,1 };
	calc_vector(vtr_1, vtr_2);
	glNormal3f(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3d(10, -4, 0);
	glTexCoord2d(0, 1);
	glVertex3d(10, -4, h);
	glTexCoord2d(1, 1);
	glVertex3d(11, -11, h);
	glTexCoord2d(1, 0);
	glVertex3d(11, -11, 0);

	vtr_1 = { 0.9778,0.2095,0 };
	vtr_2 = { 0,0,1 };
	calc_vector(vtr_1, vtr_2);
	glNormal3f(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3d(11, -11, 0);
	glTexCoord2d(0, 1);
	glVertex3d(11, -11, h);
	glTexCoord2d(1, 1);
	glVertex3d(3, -8, h);
	glTexCoord2d(1, 0);
	glVertex3d(3, -8, 0);

	vtr_1 = { -0.4472,-0.8944,0 };
	vtr_2 = { 0,0,1 };
	calc_vector(vtr_1, vtr_2);
	glNormal3f(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3d(3, -8, 0);
	glTexCoord2d(0, 1);
	glVertex3d(3, -8, h);
	glTexCoord2d(1, 1);
	glVertex3d(5, -4, h);
	glTexCoord2d(1, 0);
	glVertex3d(5, -4, 0);

	vtr_1 = { 0.7808,-0.6246,0 };
	vtr_2 = { 0,0,1 };
	calc_vector(vtr_1, vtr_2);
	glNormal3f(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3d(5, -4, 0);
	glTexCoord2d(0, 1);
	glVertex3d(5, -4, h);
	glTexCoord2d(1, 1);
	glVertex3d(0, 0, h);
	glTexCoord2d(1, 0);
	glVertex3d(0, 0, 0);

	glEnd();
}

void texture_circle()
{
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_POLYGON);
	for (double i = 0; i <= 2; i += 0.01)
	{
		double x = 9 * cos(i * 3.141593);
		double y = 9 * sin(i * 3.141593);

		double tx = cos(i * 3.141593) * 0.5 + 0.5;
		double ty = sin(i * 3.141593) * 0.5 + 0.5;

		glColor3d(0.5f, 0.5f, 0.5f);
		glNormal3d(0, 0, 1);
		glTexCoord2d(tx, ty);
		glVertex3d(x, y, -5);
	}
	glEnd();

}

/*
* Функция, которая обрабатывает рисование круга с использованием метода треугольного веера
*. Это создаст заполненный круг.
*
* Параметры:
* x (GLfloat) - положение x центральной точки круга
* y (GLfloat) - положение центральной точки окружности по оси y
* radius (GLfloat) - радиус, который будет иметь нарисованный круг
*/
//void drawFilledCircle() {
//	float radius = 3.6;
//	float x = 0;
//	float y = 0;
//	int i;
//	int triangleAmount = 40; //# of triangles used to draw circle
//
//	//GLfloat radius = 0.8f; //radius
//	glColor3d(0.2, 0.6, 0.9);
//	glNormal3f(0, 0, -1);
//	GLfloat twicePi = 2.0f * 3.141592f;
//	glPushMatrix();
//	glTranslated(3, 2, 0);
//	glRotated(56.3, 0, 0, 1);
//	glBegin(GL_TRIANGLE_FAN);
//	glVertex3f(-1.11, -1.94, 0); // center of circle
//	for (i = 0; i <= triangleAmount; i++) {
//		double delta = i * twicePi / triangleAmount;
//		if (delta <= 1.7)
//		{
//			glVertex3f(
//				x + (radius * cos(i * twicePi / triangleAmount)),
//				y + (radius * sin(i * twicePi / triangleAmount)), 0
//			);
//		}
//	}
//	glEnd();
//
//
//	glPopMatrix();
//
//	glPushMatrix();
//
//	glTranslated(3, 2, 0);
//	glRotated(56.3, 0, 0, 1);
//
//	glBegin(GL_TRIANGLE_FAN);
//	glNormal3f(0, 0, 1);
//	glVertex3f(-1.11, -1.94, 1);  // center of circle
//	for (i = 0; i <= triangleAmount; i++) {
//		double delta = i * twicePi / triangleAmount;
//		if (delta <= 1.7)
//		{
//			glVertex3f(
//				x + (radius * cos(i * twicePi / triangleAmount)),
//				y + (radius * sin(i * twicePi / triangleAmount)), 1
//			);
//		}
//	}
//	glEnd();
//	glPopMatrix();
//}

//void circle_side() 
//{
//	float radius = 3.6;
//	float x = 0;
//	float z = 0;
//	int i;
//	int triangleAmount = 40; //# of triangles used to draw circle
//
//	//GLfloat radius = 0.8f; //radius
//	glColor3f(0.0f, 1.0f, 0.0f);
//	GLfloat twicePi = 2.0f * 3.141592f;
//	glPushMatrix();
//	glTranslated(3, 2, 1);
//	glRotated(56.3, 0, 0, 1);
//	glRotated(-90, 1, 0, 0);
//	glBegin(GL_TRIANGLE_STRIP);
//	for (i = 0; i <= triangleAmount; i++) {
//		double delta = i * twicePi / triangleAmount;
//		if (delta <= 1.7)
//		{
//			glNormal3f(x + (radius * cos(i * twicePi / triangleAmount)), 0, z + (radius * sin(i * twicePi / triangleAmount)));
//			glVertex3f(
//				x + (radius * cos(i * twicePi / triangleAmount)), 0,
//				z + (radius * sin(i * twicePi / triangleAmount))
//			);
//			glNormal3f(x + (radius * cos(i * twicePi / triangleAmount)), 0, z + (radius * sin(i * twicePi / triangleAmount)));
//			glVertex3f(
//				x + (radius * cos(i * twicePi / triangleAmount)), 1,
//				z + (radius * sin(i * twicePi / triangleAmount))
//			);
//		}
//	}
//	glEnd();
//	glPopMatrix();
//}

//void concavity() {
//	float radius = 10;
//	float x = 0;
//	float y = 0;
//	int i;
//	int triangleAmount = 1000; //# of triangles used to draw circle
//
//	//GLfloat radius = 0.8f; //radius
//	glColor3d(0.2, 0.6, 0.9);
//	glNormal3f(0, 0, -1);
//	GLfloat twicePi = 2.0f * 3.141592f;
//	glPushMatrix();
//	glTranslated(-8.4, -10.4, 0);
//	glBegin(GL_TRIANGLE_FAN);
//	glVertex3f(8.4f, 10.4, 0); // center of circle
//	for (i = 0; i <= triangleAmount / 5; i++) {
//		float delta = i * twicePi / triangleAmount;
//		x = radius * cos(i * twicePi / triangleAmount);
//		y = radius * sin(i * twicePi / triangleAmount);
//		if (x <= 9.41f && y <= 9.41f)
//		{
//			glVertex3f(x, y, 0);
//		}
//	}
//	glEnd();
//	glPopMatrix();
//
//	glBegin(GL_TRIANGLES);
//
//	glNormal3f(0, 0, -1);
//
//	glVertex3d(0, 0, 0);
//	glVertex3d(1, -7, 0);
//	glVertex3d(3, -3, 0);
//
//	glEnd();
//
//	glPushMatrix();
//	glTranslated(-8.4, -10.4, 1);
//	glBegin(GL_TRIANGLE_FAN);
//	glNormal3f(0, 0, 1);
//	glVertex3f(8.4f, 10.4, 0); // center of circle
//	for (i = 0; i <= triangleAmount / 5; i++) {
//		float delta = i * twicePi / triangleAmount;
//		x = radius * cos(i * twicePi / triangleAmount);
//		y = radius * sin(i * twicePi / triangleAmount);
//		if (x <= 9.41f && y <= 9.41f)
//		{
//			glVertex3f(x, y, 0);
//		}
//	}
//	glEnd();
//	glPopMatrix();
//
//	glBegin(GL_TRIANGLES);
//
//	glNormal3f(0, 0, 1);
//
//	glVertex3d(0, 0, 1);
//	glVertex3d(1, -7, 1);
//	glVertex3d(3, -3, 1);
//
//	glEnd();
//}
//void cancavity_side() {
//	float radius = 10;
//	float x = 0;
//	float z = 0;
//	int i;
//	int triangleAmount = 800; //# of triangles used to draw circle
//
//	//GLfloat radius = 0.8f; //radius
//	glColor3f(0.3f, 0.4f, 0.6f);
//	GLfloat twicePi = 2.0f * 3.141592f;
//	glPushMatrix();
//	//glRotated(-1, 0, 0, 1);
//	glTranslated(-8.41, -10.4, 1);
//	glRotated(-90, 1, 0, 0);
//	glBegin(GL_TRIANGLE_STRIP);
//	for (i = 0; i <= triangleAmount; i++) {
//		double delta = i * twicePi / triangleAmount;
//		if (delta <= 1.23 && delta >= 0.34)
//		{
//			glNormal3f(-(x + (radius * cos(i * twicePi / triangleAmount))), 0, -(z + (radius * sin(i * twicePi / triangleAmount))));
//			glVertex3f(
//				x + (radius * cos(i * twicePi / triangleAmount)), 0,
//				z + (radius * sin(i * twicePi / triangleAmount))
//			);
//			glNormal3f(-(x + (radius * cos(i * twicePi / triangleAmount))), 0, -(z + (radius * sin(i * twicePi / triangleAmount))));
//			glVertex3f(
//				x + (radius * cos(i * twicePi / triangleAmount)), 1,
//				z + (radius * sin(i * twicePi / triangleAmount))
//			);
//		}
//	}
//	glEnd();
//	glPopMatrix();
//}


void Render(OpenGL* ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина

	create_edges(0);

	create_side(5);

	texture_circle();

	//drawFilledCircle();

	//circle_side();

	//concavity();

	//cancavity_side();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	create_edges(5);
	glDisable(GL_BLEND);

	/*glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);*/





	glEnd();
	//конец рисования квадратика станкина


   //Сообщение вверху экрана


	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	//(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}