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

void Player::render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) // 렌더링 할 때 넘겨줘야 하는 값들 - shaderProgramID, VAO, VBO, vertices, 정점 개수
{
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform");
	glm::mat4 modelTransform = glm::mat4(1.0f);
	modelTransform = glm::translate(modelTransform, position);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);

	// Player 클래스의 렌더링 코드 작성
	if (!vertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
			vertices.data(), GL_STATIC_DRAW);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0);
	}

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


void Bullet::render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) // 렌더링 할 때 넘겨줘야 하는 값들 - shaderProgramID, VAO, VBO, vertices, 정점 개수
{
	// shpere's radius = 1.0f, scale = 1.5f -> actual radius = 1.5f
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "model");
	glm::mat4 modelTransform = glm::mat4(1.0f);
	modelTransform = glm::translate(modelTransform, position);
	//glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);


	// center sphere
	glm::mat4 centerM = glm::translate(glm::mat4(1.0f), position);
	centerM = glm::scale(centerM, glm::vec3(1.5f, 1.5f, 1.5f));

	modelTransform = centerM;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);


	glDrawArrays(GL_TRIANGLES, 0, 960);



	glBindVertexArray(0);

}