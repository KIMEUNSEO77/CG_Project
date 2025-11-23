#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h> 

#include <iostream>
#include <vector>
#include <random>
#include <ctime>

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
Mesh gPlayer; // player obj

Player player; // player object(temp)
int currentStage = 1;   // current stage 0: title, 1, 2, 3

std::vector<Bullet> bullets;  // bullet objects

// bullet 색상을 위한 random 엔진 - 색상은 밝은 색상 위주로
std::default_random_engine generator(static_cast<unsigned int>(time(0)));
std::uniform_real_distribution<float> colorDistribution(0.6f, 1.0f);

// bullet 생성 y좌표 범위
std::uniform_real_distribution<float> bulletYDistribution(-19.50f, 25.0f);
// bullet 생성 z좌표 범위
std::uniform_real_distribution<float> bulletZDistribution(-90.0f, -45.0f);

float angleCameraY = 0.0f; // 카메라 Y축 각도(디버깅 위해)
bool rotatingCamera = false; // 카메라 회전 여부

clock_t lastTime;  // 이전 프레임 시간

// bullet 생성 함수 - 3페이즈에 실행
void SpawnBullet(int pattern)
{
	Bullet b;
	float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // 현재 시간(초)
	static int bulletCount = 0;
	bulletCount++;

	// 원형 회전
	if (pattern == 0)
	{
		int ringCount = 24;
		float angle = glm::radians(360.0f * (bulletCount % ringCount) / ringCount);
		float radius = 10.0f;
		glm::vec3 pos = glm::vec3(radius * cos(angle), radius * sin(angle), -5.0f);
		glm::vec3 vel = glm::normalize(pos) * -1.0f;
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(1.0f, 0.7f, 0.7f));   // 연분홍
		b.setScale(glm::vec3(0.8f, 0.8f, 0.8f));
	}
	// 나선형
	else if (pattern == 1)
	{
		float spiralAngle = glm::radians(15.0f * bulletCount);
		float spiralRadius = 0.01f * bulletCount;
		glm::vec3 pos = glm::vec3(spiralRadius * cos(spiralAngle), spiralRadius * sin(spiralAngle), -5.0f);
		glm::vec3 vel = glm::vec3(-sin(spiralAngle), cos(spiralAngle), 1.0f) * 0.5f;
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(1.0f, 0.5f, 0.2f));
		b.setScale(glm::vec3(0.6f, 0.6f, 0.6f));
	}

	// 물결 패턴
	else if (pattern == 2)
	{
		float waveX = -4.0f + 16.0f * ((bulletCount % 10) / 10.0f); // -4 ~ 4
		float waveY = 0.0f;
		float waveZ = -5.0f;
		glm::vec3 pos = glm::vec3(waveX, waveY, waveZ);
		glm::vec3 vel = glm::vec3(0.0f, 0.5f * sin(t + waveX), 2.0f); // y방향 + 파동
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.2f, 0.2f, 0.8f));
		b.setScale(glm::vec3(0.5f, 0.5f, 0.8f));
	}
	// 한 점에 있다가 퍼지는 패턴
	else if (pattern == 3)
	{
		int bulletNum = 36; // 밀도
		for (int i = 0; i < bulletNum; ++i) 
		{
			float angle = glm::radians(360.0f * i / bulletNum);
			glm::vec3 vel = glm::vec3(cos(angle), sin(angle), 3.0f) * 0.7f;
			Bullet b;
			b.setPosition(glm::vec3(0.0f, 0.0f, -5.0f));
			b.setVelocity(vel);
			b.setColor(glm::vec3(0.5f, 1.0f, 1.0f));
			b.setScale(glm::vec3(0.1f, 0.1f, 0.1f));
			bullets.push_back(b);
		}
		return; // 이미 여러 개 생성했으므로 리턴
	}
	// 0번째 왼쪽 버전
	else if (pattern == 4)
	{
		int ringCount = 24;
		float angle = glm::radians(360.0f * (bulletCount % ringCount) / ringCount);
		float radius = 10.0f;
		glm::vec3 pos = glm::vec3(-5.0f + radius * cos(angle), radius * sin(angle), -5.0f);
		glm::vec3 vel = glm::normalize(pos) * -1.0f;
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(1.0f, 0.0f, 0.7f));
		b.setScale(glm::vec3(0.8f, 0.8f, 0.8f));
	}
	// 0번째 오른쪽 버전
	else if (pattern == 5)
	{
		int ringCount = 24;
		float angle = glm::radians(360.0f * (bulletCount % ringCount) / ringCount);
		float radius = 10.0f;
		glm::vec3 pos = glm::vec3(5.0f + radius * cos(angle), radius * sin(angle), -5.0f);
		glm::vec3 vel = glm::normalize(pos) * -1.0f;
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.0f, 1.0f, 0.7f));
		b.setScale(glm::vec3(0.8f, 0.8f, 0.8f));
	}
	// 위에서 대각선 방향으로 떨어지는 패턴
	else if (pattern == 6)
	{
		glm::vec3 pos = glm::vec3(-2.5f + (bulletCount % 16), 5.0f, -4.0f);
		glm::vec3 vel = glm::vec3(1.0f, -2.0f, 2.0f);
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.8f, 0.8f, 1.0f));
		b.setScale(glm::vec3(0.5f, 0.5f, 1.5f));
	}
	// 반대쪽 대각선
	else if (pattern == 7)
	{
		glm::vec3 pos = glm::vec3(2.5f - (bulletCount % 16), 5.0f, -4.0f);
		glm::vec3 vel = glm::vec3(-1.0f, -2.0f, 2.0f);
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.8f, 0.8f, 1.0f));
		b.setScale(glm::vec3(0.5f, 0.5f, 1.5f));
	}

	// 0번째 왼쪽 버전
	else if (pattern == 8)
	{
		int ringCount = 24;
		float angle = glm::radians(360.0f * (bulletCount % ringCount) / ringCount);
		float radius = 10.0f;
		glm::vec3 pos = glm::vec3(-5.0f + radius * cos(angle), radius * sin(angle), -5.0f);
		glm::vec3 vel = glm::normalize(pos) * -1.0f;
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(1.0f, 0.3f, 0.3f));
		b.setScale(glm::vec3(0.8f, 0.8f, 0.8f));
	}
		// 0번째 오른쪽 버전
	else if (pattern == 9)
	{
		int ringCount = 24;
		float angle = glm::radians(360.0f * (bulletCount % ringCount) / ringCount);
		float radius = 10.0f;
		glm::vec3 pos = glm::vec3(5.0f + radius * cos(angle), radius * sin(angle), -5.0f);
		glm::vec3 vel = glm::normalize(pos) * -1.0f;
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.6f, 1.0f, 0.0f));
		b.setScale(glm::vec3(0.8f, 0.8f, 0.8f));
	}
	// 기본 (랜덤한 x좌표에서 일자로 날라옴)
	else if (pattern == 10)
	{
		float randX = -8.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(16.0f)));
		glm::vec3 pos = glm::vec3(randX, 0.0f, -20.0f);
		glm::vec3 vel = glm::vec3(0.0f, 0.0f, 3.0f);
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.8f, 1.0f, 0.8f));
		b.setScale(glm::vec3(1.0f, 1.0f, 1.0f));
		}

	// 파동 없는 물결
	else if (pattern == 11)
	{
		float waveX = -4.0f + 25.0f * ((bulletCount % 10) / 10.0f); // -4 ~ 4
		float waveY = 0.0f;
		float waveZ = -5.0f;
		glm::vec3 pos = glm::vec3(waveX, waveY, waveZ);
		glm::vec3 vel = glm::vec3(0.0f, 0.0f, 2.0f); // y방향 파동 없음
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(1.0f, 0.8f, 0.8));
		b.setScale(glm::vec3(0.4f, 0.4f, 0.4f));
	}

	bullets.push_back(b);
}

void BulletTimer(int value)
{
	if (currentStage == 1)
	{
		clock_t currentTime = clock();
		float deltaTime = float(currentTime - lastTime) / CLOCKS_PER_SEC;
		lastTime = currentTime;

		// 여기에 1,2페이즈에 사용할 타이머 기능 구현
		for (auto& b : bullets)
		{
			b.update_first_paze(deltaTime);
		}
	}
	if (currentStage == 3)
	{
		float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
		// 시간에 따른 패턴 변경
		if (t > 0.0f && t < 0.2f) SpawnBullet(10);
		if (t > 2.0f && t < 3.0f) SpawnBullet(0); // 2~3초 사이에 0번 패턴
		if (t > 5.0f && t < 6.0f) SpawnBullet(1); // 5~6초 사이에 1번 패턴
		if (t > 6.0f && t < 9.0f) SpawnBullet(2); // 8~9초 사이에 2번 패턴
		if (t > 10.0f && t < 13.0f) SpawnBullet(3); // 10~11초 사이에 3번 패턴
		if (t > 13.0f && t < 13.2f) SpawnBullet(10);
		if (t > 14.0f && t < 15.0f) SpawnBullet(4); // 14~15초 사이에 4번 패턴
		if (t > 17.0f && t < 18.0f) SpawnBullet(0); // 0번 패턴
		if (t > 20.0f && t < 21.0f) SpawnBullet(5); // 5번 패턴
		if (t > 26.0f && t < 33.0f) SpawnBullet(2); // 2번 패턴
		if (t > 30.0f && t < 31.0f) SpawnBullet(3); // 3번 패턴
		if (t > 33.0f && t < 34.0f) SpawnBullet(3); // 3번 패턴
		if (t > 33.0f && t < 36.0f) SpawnBullet(6); // 6번 패턴
		if (t > 35.0f && t < 38.0f) SpawnBullet(7); // 7번 패턴
		if (t > 36.0f && t < 36.2f) SpawnBullet(10);
		if (t > 38.0f && t < 45.0f) SpawnBullet(9); // 5번 패턴
		if (t > 46.0f && t < 53.0f) SpawnBullet(8); // 4번 패턴
		if (t > 58.0f && t < 63.0f) SpawnBullet(11); // 11번 패턴
		if (t > 61.0f && t < 66.0f) SpawnBullet(2); // 2번 패턴
		if (t > 65.0f && t < 68.0f) SpawnBullet(3); // 3번 패턴
		if (t > 68.0f && t < 68.2f) SpawnBullet(10);
		if (t > 70.0f && t < 75.0f) SpawnBullet(0); // 0번 패턴
		if (t > 73.0f && t < 78.0f) SpawnBullet(7); // 7번 패턴
		if (t > 76.0f && t < 81.0f) SpawnBullet(6); // 6번 패턴
		if (t > 80.0f && t < 83.0f) SpawnBullet(5); // 5번 패턴
		if (t > 85.0f && t < 88.0f) SpawnBullet(4); // 4번 패턴
		if (t > 87.0f && t < 87.2f) SpawnBullet(10);
		if (t > 88.0f && t < 90.0f) SpawnBullet(11); // 11번 패턴
		if (t > 87.0f && t < 90.0f) SpawnBullet(3); // 3번 패턴
	}
 
	glutPostRedisplay();
	glutTimerFunc(16, BulletTimer, 0); 
}

// 1,2페이즈에 사용할 타이머
/*
void firstTimer(int value)
{
	clock_t currentTime = clock();
	float deltaTime = float(currentTime - lastTime) / CLOCKS_PER_SEC;	
	lastTime = currentTime;

	// 여기에 1,2페이즈에 사용할 타이머 기능 구현
	for ( auto& b : bullets)
	{
		b.update_first_paze(deltaTime); 
	}

	glutPostRedisplay();
	glutTimerFunc(16, firstTimer, 0);
}
*/

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

void CreateBulletPaze_1()
{
	for (int i = 0; i < 144 * 3; ++i)
	{
		float xgap = static_cast <float>(i / 3) * 2;
		Bullet* b = new Bullet();
		b->setPosition(glm::vec3(-72.0f + xgap, bulletYDistribution(generator), bulletZDistribution(generator)));
		glm::vec3 color1(colorDistribution(generator), colorDistribution(generator), colorDistribution(generator));
		b->setColor(color1);
		bullets.push_back(*b);
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
	case 'y': if (angleCameraY == 0.0f) angleCameraY = 90.0f; else angleCameraY = 0.0f; break; // toggle camera rotation
	case 'q': exit(0); break;   // quit
	}
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
	//glutTimerFunc(16, firstTimer, 0); // start bullet timer

	glEnable(GL_DEPTH_TEST); // depth buffer

	// bullet insert
	if (currentStage == 1) CreateBulletPaze_1();
	

	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	// obj load
	if (!LoadOBJ_PosNorm_Interleaved("sphere.obj", gSphere))
	{
		std::cerr << "Failed to load sphere.obj\n";
		return 1;
	}

	// 일반 OBJ 파일 로드 (airplane 등)
	if (!LoadOBJ_General("airplane.obj", gPlayer))
	{
		std::cerr << "Failed to load airplane.obj\n";
		return 1;
	}

	player.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	if (currentStage == 1) {
		player.setPosition(glm::vec3(0.0f, 0.0f, -50.0f));
	}
	player.setScale(glm::vec3(0.05f, 0.05f, 0.05f));
	if (currentStage == 1) {
		player.setScale(glm::vec3(0.2f));
	}
	player.setColor(glm::vec3(0.2f, 0.8f, 1.0f));

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

	
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 8.0f);
	if (currentStage == 1) {
		cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	}
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	//glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angleCameraY), glm::vec3(0.0f, 1.0f, 0.0f));
	//cameraPos = glm::vec3(rotation * glm::vec4(cameraPos - cameraDirection, 1.0f)) + cameraDirection;
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);  // camera position to shader

	// x축 기준 -40도 회전 ( 위에서 아래로 보는 각도 )
	//glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-40.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//cameraPos = glm::vec3(rotation * glm::vec4(cameraPos - cameraDirection, 1.0f)) + cameraDirection;

	glm::mat4 vTransform = glm::mat4(1.0f);
	vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

	glm::mat4 mTransform = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &mTransform[0][0]);

	glm::mat4 pTransform = glm::mat4(1.0f);
	if (currentStage == 1) {
		pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 50.0f, 100.0f);
	}
	else {
		pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	}
	
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);

	// temp player
	glm::mat4 tmpPlayer = glm::translate(glm::mat4(1.0f), player.getPosition());
	tmpPlayer = glm::scale(tmpPlayer, glm::vec3(1.5f, 1.5f, 1.5f));
	//DrawSphere(gSphere, shaderProgramID, tmpPlayer, glm::vec3(0.8f, 0.0f, 0.0f));

	std::vector<float> bulletVertices; // temp

	player.render(shaderProgramID, gPlayer.vao, gPlayer.vbo, bulletVertices);
	//glDrawArrays(GL_TRIANGLES, 0, 8448);

	glBindVertexArray(gSphere.vao);
	glBindBuffer(GL_ARRAY_BUFFER, gSphere.vbo);

	

	// bullets (temp)
	for (Bullet& b : bullets) 
	{
		//glm::mat4 bulletModel = glm::translate(glm::mat4(1.0f), b.getPosition());
		//bulletModel = glm::scale(bulletModel, b.getScale()); // 탄환 크기
		//DrawSphere(gSphere, shaderProgramID, bulletModel, b.getColor());
		b.render(shaderProgramID, gSphere.vao, gSphere.vbo, bulletVertices);
	}
	
	glBindVertexArray(0);

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}
