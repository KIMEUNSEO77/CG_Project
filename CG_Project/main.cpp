#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h> 

#include <iostream>
#include <vector>
#include <random>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "filetobuf.h"
#include "shaderMaker.h"
#include "Object.h"

#include "sphere_obj_load.h"

GLvoid drawScene();
GLvoid Reshape(int w, int h);

Mesh gSphere;  // sphere obj

Player player; // player object(temp)
int currentStage = 0;   // current stage 0: title, 1, 2, 3

std::vector<Bullet> bullets;  // bullet objects

// bullet 생성 함수 (랜덤 패턴)
void SpawnBullet()
{
	static std::mt19937 rng{ std::random_device{}() };
	std::uniform_int_distribution<int> patternDist(0, 2);

	int pattern = patternDist(rng);

	Bullet b;
	float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // 현재 시간(초)
	static int bulletCount = 0;
	bulletCount++;

	if (pattern == 0) // 원형 회전
	{
		int ringCount = 24;
		float angle = glm::radians(360.0f * (bulletCount % ringCount) / ringCount);
		float radius = 3.0f;
		glm::vec3 pos = glm::vec3(radius * cos(angle), radius * sin(angle), 0.0f);
		glm::vec3 vel = glm::normalize(pos) * 0.5f; // 원 밖으로 이동
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.2f, 0.8f, 1.0f));
		b.setScale(glm::vec3(0.6f, 0.6f, 0.6f));
	}
	else if (pattern == 1) // 나선형
	{
		float spiralAngle = glm::radians(30.0f * bulletCount);
		float spiralRadius = 1.0f + 0.1f * bulletCount;
		glm::vec3 pos = glm::vec3(spiralRadius * cos(spiralAngle), spiralRadius * sin(spiralAngle), 0.0f);
		glm::vec3 vel = glm::vec3(-sin(spiralAngle), cos(spiralAngle), 0.0f) * 0.5f;
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(1.0f, 0.5f, 0.2f));
		b.setScale(glm::vec3(0.6f, 0.6f, 0.6f));
	}
	else // 물결
	{
		float waveX = -4.0f + 8.0f * ((bulletCount % 40) / 40.0f); // -4 ~ 4
		float waveY = 0.0f;
		float waveZ = 0.0f;
		glm::vec3 pos = glm::vec3(waveX, waveY, waveZ);
		glm::vec3 vel = glm::vec3(0.0f, 0.5f * sin(t + waveX), 1.0f); // y방향 + 파동
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.9f, 0.3f, 0.8f));
		b.setScale(glm::vec3(0.3f, 0.3f, 0.3f));
	}
	bullets.push_back(b);
}

void BulletTimer(int value)
{
	SpawnBullet();
	glutPostRedisplay();
	glutTimerFunc(16, BulletTimer, 0); 
}

void UpdateBullets()
{
	for (Bullet& b : bullets)
	{
		glm::vec3 pos = b.getPosition();
		glm::vec3 vel = b.getVelocity();
		pos += vel * 0.05f; // 속도에 따라 이동 (프레임당 0.05배)
		b.setPosition(pos);
	}
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		player.move(-1.0f, 0.0f); // move left
		break;
	case 'd':
		player.move(1.0f, 0.0f); // move right
		break;
	case 'w':
		player.move(0.0f, -1.0f); // move front
		break;
	case 's':
		player.move(0.0f, 1.0f); // move back
		break;
	case 'q': exit(0); break;   // quit
	}
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  // using depth buffer
	glutInitWindowPosition(100, 50);
	glutInitWindowSize(width, height);
	glutCreateWindow("ComputerGraphics_Prject");

	glewExperimental = GL_TRUE;
	glewInit();

	// callback 
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(16, BulletTimer, 0); // start bullet timer

	glEnable(GL_DEPTH_TEST); // depth buffer


	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	// obj load
	if (!LoadOBJ_PosNorm_Interleaved("sphere.obj", gSphere))
	{
		std::cerr << "Failed to load sphere.obj\n";
		return 1;
	}

	glutMainLoop();

	return 0;
}

// draw sphere
void DrawSphere(const Mesh& mesh, GLuint shaderProgram, const glm::mat4& model, const glm::vec3& color)
{
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

	GLint objLoc = glGetUniformLocation(shaderProgram, "objectColor");
	glUniform3fv(objLoc, 1, &color[0]);

	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, mesh.count);
	glBindVertexArray(0);
}

GLvoid drawScene()
{
	UpdateBullets();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// using shader program
	glUseProgram(shaderProgramID);

	GLint lightOnLoc = glGetUniformLocation(shaderProgramID, "lightOn");
	glUniform1i(lightOnLoc, 1); // 1 light on / 0 light off

	GLint lightLoc = glGetUniformLocation(shaderProgramID, "lightColor");
	GLint objLoc = glGetUniformLocation(shaderProgramID, "objectColor");

	glm::vec3 lightBasePos(3.0f, 0.0f, 2.5f);
	glm::mat4 lightRotate = glm::rotate(glm::mat4(1.0f), glm::radians(-40.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 lightPos = glm::vec3(glm::vec4(lightBasePos, 1.0f));

	GLint uLightPos = glGetUniformLocation(shaderProgramID, "lightPos");     // light position
	GLuint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");    // camera position
	// transfer to shader
	glUniform3f(lightLoc, 1.0f, 1.0f, 1.0f);      // light color
	glUniform3f(objLoc, 1.0f, 0.7f, 0.7f);        // object color
	glUniform3f(uLightPos, lightPos.x, lightPos.y, lightPos.z); // light position

	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 12.0f);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);  // camera position to shader

	// x축 기준 -40도 회전 ( 위에서 아래로 보는 각도 )
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-40.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	cameraPos = glm::vec3(rotation * glm::vec4(cameraPos - cameraDirection, 1.0f)) + cameraDirection;

	glm::mat4 vTransform = glm::mat4(1.0f);
	vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

	glm::mat4 mTransform = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &mTransform[0][0]);

	glm::mat4 pTransform = glm::mat4(1.0f);
	pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);

	// temp player
	glm::mat4 tmpPlayer = glm::translate(glm::mat4(1.0f), player.getPosition());
	tmpPlayer = glm::scale(tmpPlayer, glm::vec3(1.5f, 1.5f, 1.5f));
	DrawSphere(gSphere, shaderProgramID, tmpPlayer, glm::vec3(0.8f, 0.0f, 0.0f));

	// bullets (temp)
	for (Bullet& b : bullets) 
	{
		glm::mat4 bulletModel = glm::translate(glm::mat4(1.0f), b.getPosition());
		bulletModel = glm::scale(bulletModel, b.getScale()); // 탄환 크기
		DrawSphere(gSphere, shaderProgramID, bulletModel, b.getColor());
	}
	
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}
