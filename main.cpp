#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <fstream>

#include "GL\glew.h"
#include "GL\freeglut.h"

//narzedzie do ladowania i kompilowania shaderow z pliku
#include "shaderLoader.h" 
#include "tekstura.h"

//biblioteka do obs³ugi macierzy i wektorów
#include "glm/vec3.hpp" // trójwymiarowy wektor
#include "glm/vec4.hpp" // czterowymiarowy wektor
#include "glm/mat4x4.hpp" // czterowymiarowa macierz
#include "glm/gtc/matrix_transform.hpp" // macierze transformacji


//Wymiary okna
int screen_width = 640;
int screen_height = 480;


int pozycjaMyszyX;
int pozycjaMyszyY;
int mbutton;

double kameraX = -2.0;
double kameraZ = 1.0;
double kameraD = -100;
double kameraPredkosc;
double kameraKat = -20;
double kameraPredkoscObrotu;
double poprzednie_kameraX;
double poprzednie_kameraZ;
double poprzednie_kameraD;

double rotation = 0;

//macierze
glm::mat4 MV; //modelview - macierz modelu i œwiata
glm::mat4 P;  //projection - macierz projekcji (naszej perspektywy)
glm::vec3 lightPos(0.0f, -10.0f, 0.0f); //pozycja swiatla w przestrzeni
GLuint objectColor_id = 0; //kolor obiektu
GLuint lightColor_id = 0; //kolor œwiat³a
GLuint lightPos_id = 0; //pozycja œwiat³a
GLuint viewPos_id = 0; //pozycja widoku


//zmienne
float* vertices; //tablica wspolrzednych wierzcholkow
float* normals;  //tablica wektorow normalnych
float* colors; 
float* texture;
GLuint* elements; //tablica indeksow wierzcholkow definiujacych trojkaty

//identyfikatory shaderow
GLuint programID = 0;
GLuint programID2 = 0;
GLuint programID3 = 0;
GLuint programID4 = 0;

//lokalizacja uniform w shaderach
GLint uniformTex0, uniformTex1;

GLuint tex_id0, tex_id1;

unsigned int VBO, VBO_normals, VBO_colors, ebo, VBO_texture;

int numer = 0; //aktualna scena

//zdefiniowanie 5 scen
unsigned int VAO[5]; 

int n_v, n_el, n_t; //zmienne przechowujace ilosc wierzcholkow, ilosc elementow, ilosc koordynat wspolrzednych


/*******************************************/

void mysz(int button, int state, int x, int y)
{
	mbutton = button; //stan przycisku
	switch (state)
	{
	case GLUT_UP: //przycisk zwolniony
		break;
	case GLUT_DOWN: //zapis pozycji kursora i stanu kamery 
		pozycjaMyszyX = x;
		pozycjaMyszyY = y;
		poprzednie_kameraX = kameraX;
		poprzednie_kameraZ = kameraZ;
		poprzednie_kameraD = kameraD;
		break;

	}
}
/*******************************************/
void mysz_ruch(int x, int y)
{
	if (mbutton == GLUT_LEFT_BUTTON) //modyfikacja kameraX i kameraZ na podstawie roznicy poprzedniej pozycji, a obecnej + normalizacja
	{
		kameraX = poprzednie_kameraX - (pozycjaMyszyX - x) * 0.1;
		kameraZ = poprzednie_kameraZ - (pozycjaMyszyY - y) * 0.1;
	}
	if (mbutton == GLUT_RIGHT_BUTTON)
	{
		kameraD = poprzednie_kameraD + (pozycjaMyszyY - y) * 0.1;
	}

}
/******************************************/


void klawisz(GLubyte key, int x, int y)
{
	switch (key) {

	case 27:    // Esc - koniec 
		exit(1);
		break;
	case 'x':
		lightPos[1] += 10; //zmiana wysokosci oswietlenia
		break;
	case 'c':
		lightPos[1] += -10;
		break;
	case 'q':
		lightPos[0] += 5;
		break;
	case 'w':
		lightPos[0] -= 5;
		break;
	case 'a':
		lightPos[2] += 5;
		break;
	case 's':
		lightPos[2] -= 5;
		break;
	case 'r':				//reset oswietlenia
		lightPos[0] = 0;
		lightPos[1] = -10;
		lightPos[2] = 0;
		break;
	case '1':
		numer = 0;
		break;
	case '2':
		numer = 1;
		break;
	case '3':
		numer = 2;
		break;
	case '4':
		numer = 3;
		break;
	case '5':
		numer = 4;
		break;
	}
}


/******************************************/
void rysuj(void)
{

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);//biale t³o
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //czyszczenie bufora


	//SCENA 1////////////////////////

	if (numer == 0) {

		// aktywacja shadera
		glUseProgram(programID);

		//  lokalizacja zmiennej 'uniform' "MV" w programie
		GLuint MVP_id = glGetUniformLocation(programID, "MVP");


		MV = glm::mat4(1.0f);  //macierz mv -> macierz jednostkowa 4x4
		MV = glm::translate(MV, glm::vec3(0, -1, kameraD)); //przesuwanie modelu
		MV = glm::scale(MV, glm::vec3(10, 10, 10)); //skalowanie modelu
		MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0)); //obrót modelu
		MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));

		//koncowa macierz Model View Projection
		glm::mat4 MVP = P * MV;
		//przeslanie mvp do shadera
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));

		//funkcja rysujaca szkielet
		glBindVertexArray(VAO[0]);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(1);

		glUniform3f(objectColor_id, 0.5f, 0.5f, 0.5f); //kolor obiektu

		//renderowanie sceny
		glDrawElements(GL_QUADS, n_el, GL_UNSIGNED_INT, 0);
	}

	//SCENA 2////////////////////////

	else if (numer == 1) {
		// aktywacja shadera
		glUseProgram(programID);

		//  lokalizacja zmiennej 'uniform' "MV" w programie
		GLuint MVP_id = glGetUniformLocation(programID, "MVP");


		MV = glm::mat4(1.0f);  //macierz mv -> macierz jednostkowa
		MV = glm::translate(MV, glm::vec3(0, -1, kameraD)); //przesuwanie modelu
		MV = glm::scale(MV, glm::vec3(10, 10, 10)); //skalowanie modelu
		MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0)); //obrót modelu
		MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));

		//koncowa macierz Model View Projection
		glm::mat4 MVP = P * MV;
		//przeslanie mvp do shadera
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));

		glBindVertexArray(VAO[1]);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glUniform3f(objectColor_id, 0.5f, 0.5f, 0.5f); //kolor obiektu
		glUniform3f(lightColor_id, 1.0f, 1.0f, 1.0f);     //kolor œwiat³a
		glUniform3f(lightPos_id, lightPos[0], lightPos[1], lightPos[2]); //pozycja œwiat³a

		glDrawElements(GL_QUADS, n_el, GL_UNSIGNED_INT, 0);
	}
	//SCENA 3///////////////////////

	else if (numer == 2) {

		// aktywacja shadera
		glUseProgram(programID2);

		//  lokalizacja zmiennej 'uniform' "MV" w programie
		GLuint MVP_id = glGetUniformLocation(programID2, "MVP");

		MV = glm::mat4(1.0f);  //macierz mv -> macierz jednostkowa
		MV = glm::translate(MV, glm::vec3(0, -1, kameraD)); //przesuwanie modelu
		MV = glm::scale(MV, glm::vec3(10, 10, 10)); //skalowanie modelu
		MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0)); //obrót modelu
		MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));

		//koncowa macierz Model View Projection
		glm::mat4 MVP = P * MV;
		//przeslanie mvp do shadera
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));

		glBindVertexArray(VAO[2]);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glUniform3f(lightColor_id, 1.0f, 1.0f, 1.0f);     //kolor œwiat³a
		glUniform3f(lightPos_id, lightPos[0], lightPos[1], lightPos[2]); //pozycja œwiat³a

		//renderowanie sfery
		glDrawElements(GL_QUADS, n_el, GL_UNSIGNED_INT, 0);
	}

	//SCENA 4///////////////////

	else if (numer == 3) {

		// aktywacja shadera
		glUseProgram(programID3);

		//  lokalizacja zmiennej 'uniform' "MV" w programie
		GLuint MVP_id = glGetUniformLocation(programID3, "MVP");

		MV = glm::mat4(1.0f);  //macierz mv -> macierz jednostkowa
		MV = glm::translate(MV, glm::vec3(0, -1, kameraD)); //przesuwanie modelu
		MV = glm::scale(MV, glm::vec3(10, 10, 10)); //skalowanie modelu
		MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0)); //obrót modelu
		MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));

		//koncowa macierz Model View Projection
		glm::mat4 MVP = P * MV;
		//przeslanie mvp do shadera
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));

		glBindVertexArray(VAO[3]);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//tekstura
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_id0);
		uniformTex0 = glGetUniformLocation(programID3, "tex0");
		glUniform1i(uniformTex0, 0);

		glUniform3f(lightColor_id, 1.0f, 1.0f, 1.0f);     //kolor œwiat³a
		glUniform3f(lightPos_id, lightPos[0], lightPos[1], lightPos[2]); //pozycja œwiat³a

		//renderowanie strefy
		glDrawElements(GL_QUADS, n_el, GL_UNSIGNED_INT, 0);
	}

	//SCENA 5///////////////////

	else if (numer == 4) {

	// aktywacja shadera
	glUseProgram(programID3);

	//  lokalizacja zmiennej 'uniform' "MV" w programie
	GLuint MVP_id = glGetUniformLocation(programID3, "MVP");

	MV = glm::mat4(1.0f);  //macierz mv -> macierz jednostkowa
	MV = glm::translate(MV, glm::vec3(0, -1, kameraD)); //przesuwanie modelu
	MV = glm::scale(MV, glm::vec3(10, 10, 10)); //skalowanie modelu
	MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0)); //obrót modelu
	MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));

	//koncowa macierz Model View Projection
	glm::mat4 MVP = P * MV;
	//przeslanie mvp do shadera
	glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));

	glBindVertexArray(VAO[4]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//tekstura
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_id1);
	uniformTex0 = glGetUniformLocation(programID3, "tex0");
	glUniform1i(uniformTex0, 1);

	glUniform3f(lightColor_id, 1.0f, 1.0f, 1.0f);     //kolor œwiat³a
	glUniform3f(lightPos_id, lightPos[0], lightPos[1], lightPos[2]); //pozycja œwiat³a

	//renderowanie sfery
	glDrawElements(GL_QUADS, n_el, GL_UNSIGNED_INT, 0);
	}


	glFlush();//naatychmiastowe wykonanie operacji
	glutSwapBuffers();//zamiana bufora

}
/*****************************************************/
void rozmiar(int width, int height)
{
	screen_width = width;
	screen_height = height;

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screen_width, screen_height);

	P = glm::perspective(glm::radians(60.0f), (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 1000.0f);

	glutPostRedisplay(); // Przerysowanie sceny
}

/******************************************************/

//Funkcja bezczynnoœci
void idle()
{

	glutPostRedisplay(); //odœwie¿enie i przerysowanie obrazu
}

/*****************************************************/


void timer(int value) {


	glutTimerFunc(20, timer, 0);
}

/********************************************************/

int main(int argc, char** argv)
{
	//biblioteka GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); //podwojny bufor o w³asciwosciach rgb i g³êbi
	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Model terenu");

	//biblioteki GLEW i rozszerzenia OpenGL
	glewInit();  

	glutDisplayFunc(rysuj);//funkcja rysuj¹ca
	glutIdleFunc(idle);	// funkcja rysuj¹ca w czasie wolnym procesora 
	glutReshapeFunc(rozmiar); // obsluga zdarzenia resize 

	glutKeyboardFunc(klawisz);// obs³uga klawiatury
	glutMouseFunc(mysz); // obs³uga zdarzenia przycisku myszy 
	glutMotionFunc(mysz_ruch); //  obs³uga zdarzenia ruchu myszy 


	glEnable(GL_DEPTH_TEST); //test g³êbokoœci


	std::ifstream file("teren.txt");
	if (file.fail())
	{
		printf("Nie udalo siê otworzyc pliku. \n");
		system("pause");
		exit(-4);
	}

	if (file.is_open())
	{
		printf("Plik zostal otwarty. \n");
		file >> n_v;
		file >> n_t;
		file >> n_el;
		n_el = n_el * 4;
		n_v = n_v * 3;
		n_t = n_t * 2;

		printf("Ilosc wierzcholkow: %d \nIlosc elementow: %d \nIlosc wierzcholkow tekstury: %d\n", n_v, n_el, n_t);

		//alokacja pamiêci dla tablic na podstawie liczby wczytanych elementow
		vertices = (float*)calloc(n_v, sizeof(float));
		normals = (float*)calloc(n_v, sizeof(float));
		texture = (float*)calloc(n_t, sizeof(float));
		elements = (GLuint*)calloc(n_el, sizeof(int));


		int i = 0;
		for (i = 0; i < n_v; i++) {
			file >> vertices[i];
			//printf("%f \n", vertices[i]);
		}
		for (i = 0; i < n_t; i++) {
			file >> texture[i];
		}

		for (i = 0; i < n_el; i++)
		{

			file >> elements[i];
			elements[i] = elements[i] - 5;

		}

		for (i = 0; i < n_v; i++)
			normals[i] = -vertices[i];  //wektory normalne prostopadle do wierzcholka i skierowane w przeciwnym kierunku

	}

	//gradient 

	float min_y = -0.15609f;
	float max_y = 1.727043f;
	float range = max_y - min_y;
	float segment = range / 5; // dla 5 przejœæ kolorów

	colors = (float*)calloc(n_v, sizeof(int));

	for (int i = 0; i < (n_v); i = i + 3) {
		float y_normalized = (vertices[i + 1] - min_y) / range;

		float r, g, b;

		// Niebieski do jasnoniebieski
		if (y_normalized < 0.2f) {
			float t = y_normalized / 0.2f; // normalizacja do [0,1]
			r = 0.0f;
			g = t;
			b = 1.0f;
		}
		// Jasnoniebieski do zielonego
		else if (y_normalized < 0.4f) {
			float t = (y_normalized - 0.2f) / 0.2f; // normalizacja do [0,1]
			r = 0.0f;
			g = 1.0f;
			b = 1.0f - t;
		}
		// Zielony do ¿ó³tego
		else if (y_normalized < 0.6f) {
			float t = (y_normalized - 0.4f) / 0.2f; // normalizacja do [0,1]
			r = t;
			g = 1.0f;
			b = 0.0f;
		}
		// ¯ó³ty do pomarañczowego
		else if (y_normalized < 0.8f) {
			float t = (y_normalized - 0.6f) / 0.2f; // normalizacja do [0,1]
			r = 1.0f;
			g = 1.0f - 0.5f * t;
			b = 0.0f;
		}
		// Pomarañczowy do czerwonego
		else {
			float t = (y_normalized - 0.8f) / 0.2f; // normalizacja do [0,1]
			r = 1.0f;
			g = 0.5f - 0.5f * t;
			b = 0.0f;
		}

		colors[i] = r;
		colors[i + 1] = g;
		colors[i + 2] = b;
	}


	glGenVertexArrays(5, VAO);

	programID = loadShaders("vertex_shader.glsl", "fragment_shader.glsl");

	objectColor_id = glGetUniformLocation(programID, "objectColor");
	lightColor_id = glGetUniformLocation(programID, "lightColor");
	lightPos_id = glGetUniformLocation(programID, "lightPos");
	viewPos_id = glGetUniformLocation(programID, "viewPos");


	//generowanie buforow
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, n_v * sizeof(float), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO_normals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glBufferData(GL_ARRAY_BUFFER, n_v * sizeof(float), normals, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO_colors);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
	glBufferData(GL_ARRAY_BUFFER, n_v * sizeof(float), colors, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_el * sizeof(int), elements, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO_texture);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_texture);
	glBufferData(GL_ARRAY_BUFFER, n_t * sizeof(float), texture, GL_STATIC_DRAW);



	//SCENA 1///////////////

	glBindVertexArray(VAO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	//pozycja atrybutu
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	//SCENA 2/////////////////

	glBindVertexArray(VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	//SCENA 3/////////////////

	glBindVertexArray(VAO[2]);

	programID2 = loadShaders("vertex_shader.glsl", "fragment_shader2.glsl");

	lightColor_id = glGetUniformLocation(programID2, "lightColor");
	lightPos_id = glGetUniformLocation(programID2, "lightPos");
	viewPos_id = glGetUniformLocation(programID2, "viewPos");

	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	//SCENA 4///////////////////

	glBindVertexArray(VAO[3]);

	programID3 = loadShaders("vertex_shader.glsl", "fragment_shader3.glsl");

	//wczytanie tekstury

	tex_id0 = WczytajTeksture("tekstura.bmp");
	if (tex_id0 == -1)
	{
		MessageBox(NULL, "Nie znaleziono pliku z tekstur¹", "Problem", MB_OK | MB_ICONERROR);
		exit(0);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_id0);
	uniformTex0 = glGetUniformLocation(programID3, "tex0");
	glUniform1i(uniformTex0, 0);


	lightColor_id = glGetUniformLocation(programID3, "lightColor");
	lightPos_id = glGetUniformLocation(programID3, "lightPos");
	viewPos_id = glGetUniformLocation(programID3, "viewPos");

	glBindBuffer(GL_ARRAY_BUFFER, VBO_texture);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	
	//SCENA 5///////////////////

	glBindVertexArray(VAO[4]);

	programID4 = loadShaders("vertex_shader.glsl", "fragment_shader4.glsl");

	//wczytanie tekstury

	tex_id1 = WczytajTeksture("tekstura2.bmp");
	if (tex_id1 == -1)
	{
		MessageBox(NULL, "Nie znaleziono pliku z tekstur¹", "Problem", MB_OK | MB_ICONERROR);
		exit(0);
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_id1);
	uniformTex0 = glGetUniformLocation(programID3, "tex0");
	glUniform1i(uniformTex0, 1);


	lightColor_id = glGetUniformLocation(programID3, "lightColor");
	lightPos_id = glGetUniformLocation(programID3, "lightPos");
	viewPos_id = glGetUniformLocation(programID3, "viewPos");

	glBindBuffer(GL_ARRAY_BUFFER, VBO_texture);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);



	glutMainLoop();	// start

	glDeleteBuffers(1, &VBO);

	return(0);

}