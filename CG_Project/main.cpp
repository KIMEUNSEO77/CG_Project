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
#include "texture_load.h"  // load texture

GLvoid drawScene();
GLvoid Reshape(int w, int h);

Mesh gSphere;  // sphere obj
Mesh gPlayer; // player obj

GLuint tex_bg;  // background texture
GLuint tex_boss;   // boss texture
// icon textures
GLuint tex_icon_boss;
GLuint tex_icon_player;
// game clear textures
GLuint tex_GAMECLEAR_1;
GLuint tex_GAMECLEAR_2;
GLuint tex_boss_died;
GLuint tex_player_smile;
// game over texture
GLuint tex_GAMEOVER_1;
GLuint tex_GAMEOVER_2;
GLuint tex_boss_smile;
// title texture
GLuint tex_TITLE_1;
GLuint tex_TITLE_2;
GLuint tex_chase;
// loading texture
GLuint tex_LOADING;
GLuint tex_floating;

Player player; // player object(temp)
int currentStage = 0;   // current stage 0: title, 1, 2, 3 -1: gameover, 4: gameclear, 5: loading
int nextStage = 0;    // next stage - before go to loading, set next stage here

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

// boss hp
float gBossHpMax = 60.0f;
float gBossHpStartTime = 0.0f;  // 초 단위
bool  gBossTimerStarted = false;
float gBossHpRatio = 1.0f;  // 0.0~1.0

// start time angle
float flightangle = 0.0f;
float flightangleradian = 0.0f;

// for 3 stage bullet timer
float bulletspawntimer = 0.0f;

void totheloading(int value);

void UpdateBossHpTimer()
{
	// 프로그램 시작 후 경과 시간
	float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	// 처음 호출될 때 시작 시각 설정
	if (!gBossTimerStarted)
	{
		gBossTimerStarted = true;
		gBossHpStartTime = now;
	}

	float elapsed = now - gBossHpStartTime;  // 얼마만큼 지났는지 (초)
	float remaining = gBossHpMax - elapsed;  // 1초에 1씩 줄어듦

	if (remaining < 0.0f) {
		remaining = 0.0f;

		// 보스 체력 0 도달 시 처리 (예: 게임 클리어)
		if (currentStage == 3) {
			nextStage = 4;  // 게임 클리어로 전환
			pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
			nextStage = currentStage + 1;
			currentStage = 4; // loading
			player.setScale(glm::vec3(0.0f));
			bullets.clear();
		}
		else {
			totheloading(0);
		}
	}

	gBossHpRatio = remaining / gBossHpMax;   // 0.0 ~ 1.0
}



void collidecheck() 
{

	for (auto it = bullets.begin(); it != bullets.end(); )
	{
		if (it->collide(vTransform, pTransform, player)) 
		{
			// 충돌 시 처리 
			player.damaged(10.0f); // 10 데미지 입힘
			// 충돌한 총알 제거
			//it = bullets.erase(it);
		}
		else {
			++it;
		}
	}
}

void startstageangle(float time)
{
	flightangle = sin(glm::radians(flightangleradian)) * 30.0f;
	flightangleradian += time * 100.0f;

}

void playersetting() {

	if (currentStage == 1) {
		pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 50.0f, 100.0f);
	}
	else {
		pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	}

	if (currentStage == 1 || currentStage == 2)
	{
		player.setPosition(glm::vec3(0.0f, 0.0f, -50.0f));
		player.setScale(glm::vec3(0.5f));
	}

	if (currentStage == 3)
	{
		player.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		player.setScale(glm::vec3(0.07f));
		player.setVelocity(glm::vec3(1.5f, 1.5f, 0.0f));
	}
	player.setColor(glm::vec3(0.2f, 0.8f, 1.0f));
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
	clock_t currentTime = clock();
	float deltaTime = float(currentTime - lastTime) / CLOCKS_PER_SEC;
	lastTime = currentTime;

	player.move(deltaTime, vTransform, pTransform);
	if (currentStage == 1)
	{
		
		// 여기에 1,2페이즈에 사용할 타이머 기능 구현
		for (auto& b : bullets)
		{
			b.update_first_paze(deltaTime);
			if (b.collide(vTransform, pTransform, player))
			{
				player.damaged(10.0f);   // HP 깎기
				// 필요하면 총알 지우기
				// b를 erase 해야 하면 구조 조금 바꿔야 함
			}
		}
	}
	else if (currentStage == 2)
	{
		glm::vec3 ppos = player.getPosition();
		// 여기에 1,2페이즈에 사용할 타이머 기능 구현
		for (auto& b : bullets)
		{
			b.update_second_paze(deltaTime);
			
			if (b.collide(vTransform, pTransform, player))
			{
				player.damaged(10.0f);   // HP 깎기
				// 필요하면 총알 지우기
				// b를 erase 해야 하면 구조 조금 바꿔야 함
			}
		}
	}
	else
	if (currentStage == 3)
	{
		float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

		t = t - bulletspawntimer;

		glm::vec3 ppos = player.getPosition();
		for (auto& b : bullets)
		{

			if (b.collide(vTransform, pTransform, player))
			{
				player.damaged(10.0f);   // HP 깎기
				// 필요하면 총알 지우기
				// b를 erase 해야 하면 구조 조금 바꿔야 함
			}
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
	
	startstageangle(deltaTime);

	glutPostRedisplay();
	glutTimerFunc(16, BulletTimer, 0); 
}

void UpdateBullets()
{
	if (currentStage == 3) 
	{
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
	gBossHpMax = 45.0f;
	bullets.clear();
	int wide = 24;
	for (int i = 0; i < wide * 3; ++i)
	{
		float xgap = static_cast <float>(i / 3) * ( 144 / wide );
		Bullet* b = new Bullet();
		b->setPosition(glm::vec3(-72.0f + xgap, bulletYDistribution(generator), bulletZDistribution(generator)));
		glm::vec3 color1(colorDistribution(generator), colorDistribution(generator), colorDistribution(generator));
		//b->setScale(glm::vec3(1.0f));
		b->setColor(color1);

		bullets.push_back(*b);
	}
}

void CreateBulletPaze_2()
{
	gBossHpMax = 15.0f;
	bullets.clear();
	float xangle = bulletAngleDistribution(generator);
	
	//39 / 2 = 19.5
	int vertexgap = 8;
	float bulletscale = 0.5f;
	for (int i = 0; i < 13 + 1; ++i)
	{
		float xgap = static_cast <float>(i) * 6 + 3;
		// 10 bullets per xgap y distribution is 20 ~ -20, z is -50
		for (int j = 0; j < vertexgap; ++j)
		{
			float ygap = static_cast <float>(j) * (40 / vertexgap) + 1;
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
			b->setScale(glm::vec3(bulletscale));
			bullets.push_back(*b);

			Bullet* b2 = new Bullet();
			initialPos = glm::vec3(-39.0f + xgap, 20.0f - ygap, -10.0f);
			rotatedPos = rotationMatrix * glm::vec4(initialPos, 1.0f);
			rotatedPos = translationMatrix * rotatedPos;
			b2->setPosition(glm::vec3(rotatedPos));
			glm::vec3 color2(colorDistribution(generator), colorDistribution(generator), colorDistribution(generator));
			b2->setColor(color2);
			b2->setScale(glm::vec3(bulletscale));
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
		player.setLeftKeyDown();
		break;
	case 'd':
		
		player.setRightKeyDown();
		break;
	case 'w':
		player.setUpKeyDown();
		break;
	case 's':
		player.setDownKeyDown();
		break;
	case 'y': if (angleCameraY == 0.0f) angleCameraY = 90.0f; else angleCameraY = 0.0f; break; // toggle camera rotation
	case 'q': exit(0); break;   // quit
	}

	if (currentStage == 0) {
		totheloading(0);
	}
}


GLvoid KeyboardUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		player.resetLeftKeyDown();
		break;
	case 'd':
		player.resetRightKeyDown();
		break;
	case 'w':
		player.resetUpKeyDown();
		break;
	case 's':
		player.resetDownKeyDown();
		break;
	}
}

// for stage setting
void loadingto(int value)
{
	gBossTimerStarted = false; // reset boss timer
	currentStage = nextStage;
	if (currentStage == 1) CreateBulletPaze_1();
	else if (currentStage == 2) CreateBulletPaze_2();
	else {
		gBossHpMax = 90.0f;
		bulletspawntimer = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	}
	playersetting();
}


void totheloading(int value)
{
	pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	nextStage = currentStage + 1;
	currentStage = 5; // loading
	player.setScale(glm::vec3(0.0f));
	bullets.clear();

	glutTimerFunc(5000, loadingto, 53); // 5초 후에 다음 스테이지로
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
	if (currentStage == 0) {
		cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	}
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);

	// Initialize projection transform
	pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  // using depth buffer
	glutInitWindowPosition(100, 50);
	glutInitWindowSize(width, height);
	glutCreateWindow("ComputerGraphics_Project");

	glewExperimental = GL_TRUE;
	glewInit();

	InitCube();  // boss cube init
	tex_bg = LoadTexture("CG_BackGround.png");  // background texture load
	tex_boss = LoadTexture("boss_image.png"); // boss texture load
	tex_icon_boss = LoadTexture("icon_boss.png"); // boss icon texture load
	tex_icon_player = LoadTexture("icon_airplane.png"); // player icon texture load
	tex_GAMECLEAR_1 = LoadTexture("CG_GameClear_1.png"); // game over texture load
	tex_GAMECLEAR_2 = LoadTexture("CG_GameClear_2.png");
	tex_boss_died = LoadTexture("boss_died.png");
	tex_player_smile = LoadTexture("game_clear_airplane.png");
	tex_GAMEOVER_1 = LoadTexture("CG_GameOver_1.png"); // game over texture load
	tex_GAMEOVER_2 = LoadTexture("CG_GameOver_2.png");
	tex_boss_smile = LoadTexture("boss_smile.png");
	tex_TITLE_1 = LoadTexture("title.png");
	tex_TITLE_2 = LoadTexture("pressanykey.png");
	tex_chase = LoadTexture("start_sprite.png");
	tex_LOADING = LoadTexture("loading_text.png");
	tex_floating = LoadTexture("loading.png");

	// callback 
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyboardUp);
	glutTimerFunc(16, BulletTimer, 0); // start bullet timer

	glEnable(GL_DEPTH_TEST); // depth buffer
	// use alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// bullet insert
	//if (currentStage == 1) CreateBulletPaze_1();
	//if (currentStage == 2) CreateBulletPaze_2();

	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	// hp bar shader program
	InitHPBar();
	make_vertexShaders_hp();
	make_fragmentShaders_hp();
	hpShaderProgramID = make_shaderProgram_hp();

	// background shader program
	make_vertexShaders_bg();
	make_fragmentShaders_bg();
	bgShaderProgramID = make_shaderProgram_bg();
	InitBackgroundQuad();
	
	// boss cube shader program
	make_vertexShaders_boss();
	make_vertexShaders_boss();
	make_fragmentShaders_boss();
	bossShaderProgramID = make_shaderProgram_boss();

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

	// player setting

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

// draw hp bar
void DrawSingleHPBar(float hpRatio, float posX, float posY, float sizeX, float sizeY, float r, float g, float b, float direction)
{
	// 유니폼 위치
	GLint locHP = glGetUniformLocation(hpShaderProgramID, "uHP");
	GLint locPos = glGetUniformLocation(hpShaderProgramID, "uPos");
	GLint locSize = glGetUniformLocation(hpShaderProgramID, "uSize");
	GLint locCol = glGetUniformLocation(hpShaderProgramID, "uColor");
	GLint locDir = glGetUniformLocation(hpShaderProgramID, "uDirection"); // decrease direction

	glUniform1f(locHP, hpRatio);
	glUniform2f(locPos, posX, posY);
	glUniform2f(locSize, sizeX, sizeY);
	glUniform3f(locCol, r, g, b);
	glUniform1f(locDir, direction);  // set direction

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
void DrawHPBars(Player& player)
{
	// 보스 HP 타이머 갱신
	UpdateBossHpTimer();

	glUseProgram(hpShaderProgramID);
	glBindVertexArray(hpVAO);
	glDisable(GL_DEPTH_TEST);  // 항상 앞에 출력

	// 플레이어 HP바
	float playerHpRatio =
		static_cast<float>(player.getCurrentHp()) /
		static_cast<float>(player.getMaxHp());

	DrawSingleHPBar(
		playerHpRatio,
		-0.75f, 0.8f,      // position
		0.6f, 0.05f,       // size
		0.0f, 1.0f, 0.0f,  // color
		+ 1.0f             // direction
	);

	// 보스 HP바
	DrawSingleHPBar(
		gBossHpRatio,      
		0.15f, 0.8f,     
		0.6f, 0.05f,
		1.0f, 0.0f, 0.0f,
		-1.0f
	);

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
	glUseProgram(0);
}

// draw boss
void DrawBossCube(
	GLuint shader, GLuint vao, GLuint texture, const glm::mat4& model, const glm::vec3& cameraPos, const glm::vec3& lightPos,
	const glm::mat4& view, const glm::mat4& projection
) {
	glUseProgram(shader);

	// ---- Uniform location ----
	GLint locModel = glGetUniformLocation(shader, "model");
	GLint locView = glGetUniformLocation(shader, "view");
	GLint locProj = glGetUniformLocation(shader, "projection");
	GLint locObjColor = glGetUniformLocation(shader, "objectColor");
	GLint locLightColor = glGetUniformLocation(shader, "lightColor");
	GLint locLightPos = glGetUniformLocation(shader, "lightPos");
	GLint locViewPos = glGetUniformLocation(shader, "viewPos");
	GLint locLightOn = glGetUniformLocation(shader, "lightOn");
	GLint locTex = glGetUniformLocation(shader, "outTexture");

	// ---- Set uniforms ----
	glUniformMatrix4fv(locModel, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(locView, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(locProj, 1, GL_FALSE, &projection[0][0]);
	
	// Boss는 텍스처 색 그대로 쓰므로 objectColor = white
	glUniform3f(locObjColor, 1.0f, 1.0f, 1.0f);
	glUniform3f(locLightColor, 1.0f, 1.0f, 1.0f);
	glUniform3f(locLightPos, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(locViewPos, cameraPos.x, cameraPos.y, cameraPos.z);

	// 조명 끄고(텍스처만 보이게)
	glUniform1i(locLightOn, 0);

	// Texture binding
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(locTex, 0);

	// ---- Draw cube ----
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 30, 6);
	glBindVertexArray(0);

	glUseProgram(0);
}

GLvoid drawScene()
{
	UpdateBullets();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST); // 배경을 먼저 그리기 위해 깊이 테스트 비활성화

	// draw background
	glUseProgram(bgShaderProgramID);
	glBindVertexArray(bgVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_bg);
	GLint bgTexLoc = glGetUniformLocation(bgShaderProgramID, "bgTexture");
	glUniform1i(bgTexLoc, 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);  // 깊이 테스트 다시 활성화

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

	// boss draw
	if (currentStage == 1 || currentStage == 2 || currentStage == 3) {
		// boss cube draw
		glm::mat4 model = glm::mat4(1.0f);
		if (currentStage == 3)
		{
			model = glm::translate(model, glm::vec3(0.0f, -1.0f, -10.0f));
			model = glm::scale(model, glm::vec3(22.0f, 22.0f, 0.01f));  // 납작하게
		}
		else
		{
			model = glm::translate(model, glm::vec3(0.0f, -5.0f, -89.0f));
			model = glm::scale(model, glm::vec3(80.0f, 80.0f, 0.01f));  // 납작하게
		}
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_boss, model, cameraPos, lightPos, vTransform, pTransform);
	}
	glUseProgram(shaderProgramID);
	glClear(GL_DEPTH_BUFFER_BIT);
	player.render(shaderProgramID, gPlayer.vao, gPlayer.vbo, bulletVertices);
	//glDrawArrays(GL_TRIANGLES, 0, 8448);

	// 깊이 버퍼만 클리어 - 비행기만의 깊이 공간 확보
	// 이렇게 하면 비행기 자체의 깊이 관계는 유지되면서(그림자 정상)
	// 총알보다 뒤에 그려지는 효과를 얻을 수 있음
	glClear(GL_DEPTH_BUFFER_BIT);

	// 플레이어 위치에 연한 주황색 구체 그리기 (디버그용)
	glm::mat4 scaleSphereModel = glm::scale(glm::mat4(1.0f), player.getScale()); // 작은 크기
	
	glm::vec3 playerPos = player.getPosition();
	glm::vec3 deltaPos = glm::vec3(0.0f, -0.6f, 0.1f);
	
	deltaPos = glm::vec3(scaleSphereModel * glm::vec4(deltaPos, 1.0f));

	playerPos += deltaPos;
	glm::mat4 debugSphereModel = glm::translate(glm::mat4(1.0f), playerPos);
	scaleSphereModel = glm::scale(scaleSphereModel, glm::vec3(3.0f)); // 구체 크기 조정
	debugSphereModel = debugSphereModel * scaleSphereModel;
	glm::vec3 lightOrangeColor = glm::vec3(1.0f, 0.85f, 0.7f); // 연한 주황색 (R=1.0, G=0.7, B=0.4)
	

	glBindVertexArray(gSphere.vao);
	glBindBuffer(GL_ARRAY_BUFFER, gSphere.vbo);

	// bullets
	for (Bullet& b : bullets) 
	{
		b.render(shaderProgramID, gSphere.vao, gSphere.vbo, bulletVertices);
	}

	glClear(GL_DEPTH_BUFFER_BIT);
	DrawSphere(gSphere, shaderProgramID, debugSphereModel, lightOrangeColor);

	glBindVertexArray(0);

	// hp bar draw
	if (currentStage == 1 || currentStage == 2 || currentStage == 3)
	{
		DrawHPBars(player);

		// boss icon
		glm::mat4 icon_boss = glm::mat4(1.0f);
		if (currentStage == 3)
		{
			icon_boss = glm::translate(icon_boss, glm::vec3(11.0f, 5.5f, -10.0f));
			icon_boss = glm::scale(icon_boss, glm::vec3(5.0f, 5.0f, 0.01f));
		}
		else
		{
			icon_boss = glm::translate(icon_boss, glm::vec3(41.0f, 21.0f, -60.0f));
			icon_boss = glm::scale(icon_boss, glm::vec3(20.0f, 20.0f, 0.01f));
		}
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_icon_boss, icon_boss, cameraPos, lightPos, vTransform, pTransform);

		// player icon
		glm::mat4 icon_player = glm::mat4(1.0f);
		if (currentStage == 3)
		{
			icon_player = glm::translate(icon_player, glm::vec3(-11.0f, 5.5f, -10.0f));
			icon_player = glm::scale(icon_player, glm::vec3(4.0f, 4.0f, 0.01f));
		}
		else
		{
			icon_player = glm::translate(icon_player, glm::vec3(-42.0f, 20.0f, -60.0f));
			icon_player = glm::scale(icon_player, glm::vec3(20.0f, 20.0f, 0.01f));
		}
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_icon_player, icon_player, cameraPos, lightPos, vTransform, pTransform);
	}

	// title screen
	if (currentStage == 0)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -10.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(38.0f * 2, 20.0f * 2, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_TITLE_1, model, cameraPos, lightPos, vTransform, pTransform);
		glm::mat4 model_2 = glm::mat4(1.0f);
		
		model_2 = glm::translate(model_2, glm::vec3(0.0f, -9.0f, -10.0f));
		model_2 = glm::rotate(model_2, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model_2 = glm::scale(model_2, glm::vec3(35.0f, 20.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_TITLE_2, model_2, cameraPos, lightPos, vTransform, pTransform);
		
		glm::mat4 model_3 = glm::mat4(1.0f);
		model_3 = glm::translate(model_3, glm::vec3(0.0f, 10.0f, 0.0f));
		model_3 = glm::rotate(model_3, glm::radians(flightangle), glm::vec3(0.0f, 0.0f, 1.0f));
		model_3 = glm::translate(model_3, glm::vec3(0.0f, -18.0f, -10.0f));
		model_3 = glm::rotate(model_3, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model_3 = glm::scale(model_3, glm::vec3(45.0f, 35.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_chase, model_3, cameraPos, lightPos, vTransform, pTransform);
	}

	// clear screen
	if (currentStage == 4)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-5.0f, 3.0f, -10.0f));
		model = glm::scale(model, glm::vec3(38.0f, 20.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_GAMECLEAR_1, model, cameraPos, lightPos, vTransform, pTransform);

		glm::mat4 model_2 = glm::mat4(1.0f);
		model_2 = glm::translate(model_2, glm::vec3(5.0f, -0.1f, -10.0f));
		model_2 = glm::scale(model_2, glm::vec3(35.0f, 20.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_GAMECLEAR_2, model_2, cameraPos, lightPos, vTransform, pTransform);

		glm::mat4 model_3 = glm::mat4(1.0f);
		model_3 = glm::translate(model_3, glm::vec3(0.0f, -8.0f, -10.0f));
		model_3 = glm::scale(model_3, glm::vec3(45.0f, 45.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_boss_died, model_3, cameraPos, lightPos, vTransform, pTransform);

		glm::mat4 model_4 = glm::mat4(1.0f);
		model_4 = glm::translate(model_4, glm::vec3(11.0f, -9.0f, -9.0f));
		model_4 = glm::scale(model_4, glm::vec3(50.0f, 30.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_player_smile, model_4, cameraPos, lightPos, vTransform, pTransform);
	}

	// load screen
	if (currentStage == 5)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 3.0f, -10.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(38.0f, 20.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_LOADING, model, cameraPos, lightPos, vTransform, pTransform);

		glm::mat4 model_2 = glm::mat4(1.0f);
		model_2 = glm::translate(model_2, glm::vec3(0.0f, sin(glm::radians(flightangleradian)) * 1.5f - 5.0f, -10.0f));
		model_2 = glm::rotate(model_2, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model_2 = glm::scale(model_2, glm::vec3(20.0f, 20.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_floating, model_2, cameraPos, lightPos, vTransform, pTransform);

		glm::mat4 model_3 = glm::mat4(1.0f);
		model_3 = glm::translate(model_3, glm::vec3(8.0f, sin(glm::radians(flightangleradian) - 1.57f) * 1.5f - 5.0f, -10.0f));
		model_3 = glm::rotate(model_3, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model_3 = glm::scale(model_3, glm::vec3(20.0f, 20.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_floating, model_3, cameraPos, lightPos, vTransform, pTransform);

		//sin(glm::radians(flightangleradian)) * 30.0f;

		model_3 = glm::mat4(1.0f);
		model_3 = glm::translate(model_3, glm::vec3(-8.0f, sin(glm::radians(flightangleradian) + 1.57f) * 1.5f - 5.0f, -10.0f));
		model_3 = glm::rotate(model_3, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model_3 = glm::scale(model_3, glm::vec3(20.0f, 20.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_floating, model_3, cameraPos, lightPos, vTransform, pTransform);
	}

	// game over screen
	if (currentStage == -1)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-4.0f, 2.5f, -10.0f));
		model = glm::scale(model, glm::vec3(23.0f, 15.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_GAMEOVER_1, model, cameraPos, lightPos, vTransform, pTransform);

		glm::mat4 model_2 = glm::mat4(1.0f);
		model_2 = glm::translate(model_2, glm::vec3(4.0f, 2.3f, -10.0f));
		model_2 = glm::scale(model_2, glm::vec3(23.0f, 15.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_GAMEOVER_2, model_2, cameraPos, lightPos, vTransform, pTransform);

		glm::mat4 model_3 = glm::mat4(1.0f);
		model_3 = glm::translate(model_3, glm::vec3(0.0f, -9.0f, -10.0f));
		model_3 = glm::scale(model_3, glm::vec3(45.0f, 45.0f, 0.01f));
		DrawBossCube(bossShaderProgramID, cubeVAO, tex_boss_smile, model_3, cameraPos, lightPos, vTransform, pTransform);
	}

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}

