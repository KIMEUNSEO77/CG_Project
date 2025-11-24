#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h>
#include <vector>

#include "Object.h"

void Object::update()
{

}

void Player::move(float dx, float dz)
{
	position.x += dx * speed;
	position.z += dz * speed;
}

void Player::damaged(float damage)
{
	if (damage >= 0)
		hp -= damage;
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