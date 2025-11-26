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
int currentStage = 3;   // current stage 0: title, 1, 2, 3

std::vector<Bullet> bullets;  // bullet objects

// bullet 색상을 위한 random 엔진 - 색상은 밝은 색상 위주로
std::default_random_engine generator(static_cast<unsigned int>(time(0)));
std::uniform_real_distribution<float> colorDistribution(0.6f, 1.0f);

// bullet 생성 y좌표 범위
std::uniform_real_distribution<float> bulletYDistribution(-19.50f, 25.0f);
// bullet 생성 z좌표 범위
std::uniform_real_distribution<float> bulletZDistribution(-90.0f, -45.0f);
// scond phase bullet angle 
std::uniform_real_distribution<float> bulletAngleDistribution(-30.0f, 30.0f);

float angleCameraY = 0.0f; // 카메라 Y축 각도(디버깅 위해)
bool rotatingCamera = false; // 카메라 회전 여부

clock_t lastTime;  // 이전 프레임 시간

// variables for shader uniforms
GLint lightOnLoc;
GLint lightColorLoc;
GLint lightPosLoc;
GLint viewPosLoc;
GLint viewLoc;
GLint projectionLoc;
GLint modelLoc;
GLint objectColorLoc;

// Global transformation matrices and lighting
glm::vec3 lightPos;
glm::mat4 vTransform;
glm::mat4 pTransform;
glm::vec3 cameraPos;

// hp bar vertices
float hpBarVertices[] = 
{
	// pos      // uv
	-0.9f, 0.9f,  0.0f, 1.0f,
	-0.9f, 0.85f, 0.0f, 0.0f,
	-0.3f, 0.85f, 1.0f, 0.0f,
	-0.3f, 0.9f,  1.0f, 1.0f
};

void collidecheck() {

	for (auto it = bullets.begin(); it != bullets.end(); )
	{
		glm::vec3 pos = player.getPosition();
		if (it->collide(vTransform, pTransform, pos)) {
			// 충돌 시 처리 (예: 플레이어 데미지)
			player.damaged(10.0f); // 예시로 10 데미지 입힘
			// 충돌한 총알 제거
			//it = bullets.erase(it);
		}
		else {
			++it;
		}
	}
}

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
		b.setScale(glm::vec3(0.2f, 0.2f, 0.2f));
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
		b.setScale(glm::vec3(0.1f, 0.1f, 0.1f));
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
		b.setScale(glm::vec3(0.1f, 0.1f, 0.1f));
	}
	// 한 점에 있다가 퍼지는 패턴
	else if (pattern == 3)
	{
		int bulletNum = 36; // 밀도
		for (int i = 0; i < bulletNum; ++i)
		{
			float angle = glm::radians(180.0f * i / bulletNum);
			glm::vec3 vel = glm::vec3(cos(angle), sin(angle), 3.0f) * 0.7f;
			Bullet b;
			b.setPosition(glm::vec3(0.0f, 0.0f, -5.0f));
			b.setVelocity(vel);
			b.setColor(glm::vec3(0.5f, 1.0f, 1.0f));
			b.setScale(glm::vec3(0.05f, 0.05f, 0.05f));
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
		b.setScale(glm::vec3(0.2f, 0.2f, 0.2f));
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
		b.setScale(glm::vec3(0.2f, 0.2f, 0.2f));
	}
	// 위에서 대각선 방향으로 떨어지는 패턴
	else if (pattern == 6)
	{
		glm::vec3 pos = glm::vec3(-2.5f + (bulletCount % 16), 5.0f, -4.0f);
		glm::vec3 vel = glm::vec3(1.0f, -2.0f, 2.0f);
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.8f, 0.8f, 1.0f));
		b.setScale(glm::vec3(0.1f, 0.1f, 0.3f));
	}
	// 반대쪽 대각선
	else if (pattern == 7)
	{
		glm::vec3 pos = glm::vec3(2.5f - (bulletCount % 16), 5.0f, -4.0f);
		glm::vec3 vel = glm::vec3(-1.0f, -2.0f, 2.0f);
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.8f, 0.8f, 1.0f));
		b.setScale(glm::vec3(0.1f, 0.1f, 0.3f));
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
		b.setScale(glm::vec3(0.2f, 0.2f, 0.2f));
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
		b.setScale(glm::vec3(0.2f, 0.2f, 0.2f));
	}
	// 기본 (랜덤한 x좌표에서 일자로 날라옴)
	else if (pattern == 10)
	{
		float randX = -8.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (16.0f)));
		glm::vec3 pos = glm::vec3(randX, 0.0f, -20.0f);
		glm::vec3 vel = glm::vec3(0.0f, 0.0f, 3.0f);
		b.setPosition(pos);
		b.setVelocity(vel);
		b.setColor(glm::vec3(0.8f, 1.0f, 0.8f));
		b.setScale(glm::vec3(0.2f, 0.2f, 0.2f));
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
		b.setScale(glm::vec3(0.1f, 0.1f, 0.1f));
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
		glm::vec3 ppos = player.getPosition();
		// 여기에 1,2페이즈에 사용할 타이머 기능 구현
		for (auto& b : bullets)
		{
			b.update_first_paze(deltaTime);
			b.collide(vTransform, pTransform, ppos);
		}
	}
	else if (currentStage == 2)
	{
		clock_t currentTime = clock();
		float deltaTime = float(currentTime - lastTime) / CLOCKS_PER_SEC;
		lastTime = currentTime;
		glm::vec3 ppos = player.getPosition();
		// 여기에 1,2페이즈에 사용할 타이머 기능 구현
		for (auto& b : bullets)
		{
			b.update_second_paze(deltaTime);
			
			b.collide(vTransform, pTransform, ppos);
		}
	}
	else
	if (currentStage == 3)
	{
		float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

		glm::vec3 ppos = player.getPosition();
		for (auto& b : bullets)
		{
			b.collide(vTransform, pTransform, ppos); // 충돌 체크 추가
		}

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

void UpdateBullets()
{
	if (currentStage == 3) {
		// z좌표가 3.0 이상인 bullet 삭제
		for (auto it = bullets.begin(); it != bullets.end(); )
		{
			glm::vec3 pos = it->getPosition();
			if (pos.z >= 6.0f)
			{
				it = bullets.erase(it);
			}
			else
			{
				// 이동 및 유지
				glm::vec3 vel = it->getVelocity();
				pos += vel * 0.05f;
				it->setPosition(pos);
				++it;
			}
		}
	}
	else {
		// 기존 방식 유지
		for (Bullet& b : bullets)
		{
			glm::vec3 pos = b.getPosition();
			glm::vec3 vel = b.getVelocity();
			pos += vel * 0.05f;
			b.setPosition(pos);
		}
	}
}

void CreateBulletPaze_1()
{
	bullets.clear();
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

void CreateBulletPaze_2()
{
	bullets.clear();
	float xangle = bulletAngleDistribution(generator);
	
	//39 / 2 = 19.5

	for (int i = 0; i < 19 + 1; ++i)
	{
		float xgap = static_cast <float>(i) * 6;
		// 10 bullets per xgap y distribution is 20 ~ -20, z is -50
		for (int j = 0; j < 10; ++j)
		{
			float ygap = static_cast <float>(j) * 4 + 1;
			Bullet* b = new Bullet();
			glm::vec3 initialPos(-39.0f + xgap, 20.0f - ygap, 10.0f);
			// rotate around Y axis by xangle
			glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(xangle), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::vec4 rotatedPos = rotationMatrix * glm::vec4(initialPos, 1.0f);
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -50.0f));
			rotatedPos = translationMatrix * rotatedPos;
			b->setPosition(glm::vec3(rotatedPos));
			//b->setPosition(glm::vec3(initialPos));
			glm::vec3 color1(colorDistribution(generator), colorDistribution(generator), colorDistribution(generator));
			b->setColor(color1);
			bullets.push_back(*b);

			Bullet* b2 = new Bullet();
			initialPos = glm::vec3(-39.0f + xgap, 20.0f - ygap, -10.0f);
			rotatedPos = rotationMatrix * glm::vec4(initialPos, 1.0f);
			rotatedPos = translationMatrix * rotatedPos;
			b2->setPosition(glm::vec3(rotatedPos));
			glm::vec3 color2(colorDistribution(generator), colorDistribution(generator), colorDistribution(generator));
			b2->setColor(color2);
			bullets.push_back(*b2);
		}
		xangle += 45.0f; // increase angle for next column
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
		player.move(0.0f, 1.0f); // move front
		break;
	case 's':
		player.move(0.0f, -1.0f); // move back
		break;
	case 'y': if (angleCameraY == 0.0f) angleCameraY = 90.0f; else angleCameraY = 0.0f; break; // toggle camera rotation
	case 'q': exit(0); break;   // quit
	}
}

// Initialize all shader uniform locations
void InitUniformLocations(GLuint shaderProgram)
{
	lightOnLoc = glGetUniformLocation(shaderProgram, "lightOn");
	lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
	lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
	viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
	viewLoc = glGetUniformLocation(shaderProgram, "view");
	projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	modelLoc = glGetUniformLocation(shaderProgram, "model");
	objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
}

// Initialize global transformation matrices and lighting
void InitTransformsAndLighting()
{
	// Initialize light position
	glm::vec3 lightBasePos(3.0f, 0.0f, 2.5f);
	lightPos = glm::vec3(glm::vec4(lightBasePos, 1.0f));

	// Initialize view transform
	cameraPos = glm::vec3(0.0f, 0.0f, 8.0f);
	if (currentStage == 1) {
		cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	}
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);

	// Initialize projection transform
	if (currentStage == 1) {
		pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 50.0f, 100.0f);
	}
	else {
		pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
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

	glEnable(GL_DEPTH_TEST); // depth buffer

	// bullet insert
	if (currentStage == 1) CreateBulletPaze_1();
	if (currentStage == 2) CreateBulletPaze_2();

	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	// hp bar shader program
	InitHPBar();
	make_vertexShaders_hp();
	make_fragmentShaders_hp();
	hpShaderProgramID = make_shaderProgram_hp();

	// Initialize uniform locations
	InitUniformLocations(shaderProgramID);
	InitTransformsAndLighting();

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
	if (currentStage == 1 || currentStage == 2) {
		player.setPosition(glm::vec3(0.0f, 0.0f, -50.0f));
	}
	player.setScale(glm::vec3(0.5f, 0.5f, 0.5f));
	if (currentStage == 1 || currentStage == 2) {
		player.setScale(glm::vec3(0.2f));
	}
	if (currentStage == 3)
	{
		player.setScale(glm::vec3(0.1f));
	}
	player.setColor(glm::vec3(0.2f, 0.8f, 1.0f));

	glutMainLoop();

	return 0;
}

// draw sphere
void DrawSphere(const Mesh& mesh, GLuint shaderProgram, const glm::mat4& model, const glm::vec3& color)
{
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
	glUniform3fv(objectColorLoc, 1, &color[0]);

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

	glUniform1i(lightOnLoc, 1); // 1 light on / 0 light off

	

	// transfer to shader
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);      // light color
	glUniform3f(objectColorLoc, 1.0f, 0.7f, 0.7f);        // object color
	glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z); // light position

	
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);  // camera position to shader

	// Update view transform
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);


	
	
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &pTransform[0][0]);


	std::vector<float> bulletVertices; // temp

	player.render(shaderProgramID, gPlayer.vao, gPlayer.vbo, bulletVertices);
	//glDrawArrays(GL_TRIANGLES, 0, 8448);

	glBindVertexArray(gSphere.vao);
	glBindBuffer(GL_ARRAY_BUFFER, gSphere.vbo);

	// bullets
	for (Bullet& b : bullets) 
	{
		b.render(shaderProgramID, gSphere.vao, gSphere.vbo, bulletVertices);
	}
	
	glBindVertexArray(0);

	// 2) HP바 그리기
	glUseProgram(hpShaderProgramID);

	float hpRatio = player.getCurrentHp() / player.getMaxHp();  // 0.0 ~ 1.0
	glUniform1f(glGetUniformLocation(hpShaderProgramID, "uHP"), hpRatio);

	glDisable(GL_DEPTH_TEST);  // UI가 3D보다 앞에 보이도록
	glBindVertexArray(hpVAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}
