#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h>
#include <vector>

#include <iostream>

#include "Object.h"

void Object::update()
{

}

void Player::move(float dx, float dy)
{
	position.x += dx * speed;
	position.y += dy * speed;
}

void Player::damaged(float damage)
{
	if (currentHp - damage < 0)
		currentHp = 0;

	if (currentHp >= 0)
		currentHp -= damage;
}

void Player::render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices)
{
	// Transform 설정 - scale 멤버 변수 사용
	glm::mat4 modelTransform = glm::mat4(1.0f);
	modelTransform = glm::translate(modelTransform, position);
	modelTransform = glm::scale(modelTransform, scale);
	modelTransform = glm::rotate(modelTransform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // x축 기준 -90도 회전
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelTransform[0][0]);

	// 색상 설정
	GLint colorLoc = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(colorLoc, color.x, color.y, color.z);

	// VAO와 VBO는 이미 main.cpp에서 바인드되어 있으므로
	// 바로 그리기만 하면 됨
	// gPlayer.count를 외부에서 받아야 하지만, 
	// Mesh 구조를 보면 airplane.obj는 대략 8448개 정점
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glDrawArrays(GL_TRIANGLES, 0, 8448);
	glBindVertexArray(0);
}

void Bullet::update_first_paze(float deltaTime)
{
	// z축 이동
	if (position.z < -40.0f)
		position.z += 5.0f * deltaTime;

	// 중력 적용
	vy += -9.8f * deltaTime; // 9.8f is gravity acceleration

	// y 위치 업데이트
	position.y += vy * deltaTime;

	// 바닥 충돌 체크 (완전탄성 충돌)
	if (position.y <= -20.0f) { // groundY = -20.0f
		position.y = -20.0f;  // 바닥 위치로 보정
		vy *= -1.0f;  // 속도 반전 (완전탄성)
		if (vy < 29.2f) {
			vy = 29.2f; // 최소 반발 속도 설정
		}
	}

	if (-40.0f <= position.z) {
		position.z = -90.0f; // reset position
	}
}

void Bullet::update_second_paze(float deltaTime)
{
	position.z += 50.0f;

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(20.0f * deltaTime), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec4 rotatedPos = rotationMatrix * glm::vec4(position, 1.0f);
	rotatedPos.z -= 50.0f;
	position = glm::vec3(rotatedPos);
}


void Bullet::render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) // 렌더링 할 때 넘겨줘야 하는 값들 - shaderProgramID, VAO, VBO, vertices, 정점 개수
{
	// shpere's radius = 1.0f, scale = 1.5f -> actual radius = 1.5f
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "model");
	glm::mat4 modelTransform = glm::mat4(1.0f);
	modelTransform = glm::translate(modelTransform, position);
	//glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);


	// bulletModel
	glm::mat4 bulletModel = glm::translate(glm::mat4(1.0f), position);  // sphere position
	bulletModel = glm::scale(bulletModel, scale);   // sphere scale

	modelTransform = bulletModel;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);

	unsigned int colorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(colorLocation, color.x, color.y, color.z);


	glDrawArrays(GL_TRIANGLES, 0, 960);

}

bool Bullet::collide(const glm::mat4& view, const glm::mat4& proj, glm::vec3& playerPosWorld)
{
	// -------------------------------------------------------
	// 1. 총알 (Bullet) 투영 -> 화면상 영역(타원) 계산
	// -------------------------------------------------------
	glm::vec4 bulletPos = glm::vec4(position, 1.0f);
	glm::vec4 bulletViewPos = view * bulletPos;

	// 카메라 뒤에 있거나 너무 가까우면 무시
	if (bulletViewPos.z >= -0.1f) return false;

	float bulletDepth = -bulletViewPos.z; // 양수 깊이

	// 총알 중심점 NDC 변환
	float bx_ndc = bulletViewPos.x * proj[0][0] / bulletDepth;
	float by_ndc = bulletViewPos.y * proj[1][1] / bulletDepth;

	// 총알의 화면상 반지름 (Radius) 계산
	// scale.x, scale.y가 월드 공간에서의 반지름이라고 가정
	float b_radius_x_ndc = scale.x * proj[0][0] / bulletDepth / 2;
	float b_radius_y_ndc = scale.y * proj[1][1] / bulletDepth / 2;


	// -------------------------------------------------------
	// 2. 플레이어 (Player) 투영 -> 화면상 점(Point) 계산
	// -------------------------------------------------------
	glm::vec4 playerPos = glm::vec4(playerPosWorld, 1.0f);
	glm::vec4 playerViewPos = view * playerPos;

	// 플레이어가 카메라 뒤에 있으면 무시
	if (playerViewPos.z >= -0.1f) return false;

	float playerDepth = -playerViewPos.z;

	// 플레이어 중심점 NDC 변환 (반지름 계산 불필요)
	float px_ndc = playerViewPos.x * proj[0][0] / playerDepth;
	float py_ndc = playerViewPos.y * proj[1][1] / playerDepth;


	// -------------------------------------------------------
	// 3. 충돌 검사: 점이 타원 안에 있는가?
	// -------------------------------------------------------

	float dx = px_ndc - bx_ndc; // 플레이어 점 - 총알 중심
	float dy = py_ndc - by_ndc;

	// 타원 방정식: (x / rx)^2 + (y / ry)^2 <= 1
	// 이 값이 1보다 작거나 같으면 점이 타원 내부에 있는 것입니다.

	float x_term = dx / b_radius_x_ndc;
	float y_term = dy / b_radius_y_ndc;

	if ((x_term * x_term) + (y_term * y_term) <= 1.0f) 
	{
		std::cout << "Collision detected (Point inside Bullet)!" << std::endl;
		return true;
	}

	return false;
}