#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h>
#include <vector>

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