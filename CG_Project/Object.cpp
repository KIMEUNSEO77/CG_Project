#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h>
#include <vector>

#include <iostream>

#include "Object.h"

void Object::update()
{

}

void Player::move(float deltaTime, const glm::mat4& view, const glm::mat4& proj)
{
	float dirx = 0.0f;
	float diry = 0.0f;

	if (left_keydown) dirx -= 1.0f;
	if (right_keydown) dirx += 1.0f;
	if (up_keydown) diry += 1.0f;
	if (down_keydown) diry -= 1.0f;

	if (dirx != 0.0f || diry != 0.0f)
	{
		// 새로운 위치 계산
		glm::vec3 newPosition = position;

		if (sensitivity == 1) // 느린 이동
		{
			dirx *= 0.5f;
			diry *= 0.5f;
		}

		newPosition.x += dirx * speed * deltaTime;
		newPosition.y += diry * speed * deltaTime;

		// NDC 좌표로 변환하여 화면 경계 체크
		glm::vec4 worldPos = glm::vec4(newPosition, 1.0f);
		glm::vec4 viewPos = view * worldPos;
		
		// 카메라 앞에 있는지 확인
		if (viewPos.z < -0.1f)
		{
			float depth = -viewPos.z;
			
			// NDC 변환
			float ndc_x = viewPos.x * proj[0][0] / depth;
			float ndc_y = viewPos.y * proj[1][1] / depth;
			
			// NDC 범위: -1.0 ~ 1.0
			// 여유를 두기 위해 -0.95 ~ 0.95 범위로 제한
			const float NDC_LIMIT = 0.95f;
			
			// X축 경계 체크
			if (ndc_x < -NDC_LIMIT || ndc_x > NDC_LIMIT)
			{
				// X축 이동만 취소
				newPosition.x = position.x;
			}
			
			// Y축 경계 체크
			if (ndc_y < -NDC_LIMIT || ndc_y > NDC_LIMIT)
			{
				// Y축 이동만 취소
				newPosition.y = position.y;
			}
			
			// 최종 위치 적용
			position = newPosition;
		}
	}

	
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


	// bulletModel
	glm::mat4 bulletModel = glm::translate(glm::mat4(1.0f), position);  // sphere position
	bulletModel = glm::scale(bulletModel, scale);   // sphere scale

	modelTransform = bulletModel;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);

	unsigned int colorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(colorLocation, color.x, color.y, color.z);


	glDrawArrays(GL_TRIANGLES, 0, 960);

}

bool Bullet::collide(const glm::mat4& view, const glm::mat4& proj, Player& player)
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
	float b_radius_x_ndc = scale.x * proj[0][0] / bulletDepth / 2;
	float b_radius_y_ndc = scale.y * proj[1][1] / bulletDepth / 2;


	// -------------------------------------------------------
	// 2. 플레이어 (Player) 투영 -> 화면상 원(Circle) 계산
	// -------------------------------------------------------
	// 플레이어 충돌 판정 위치 계산 (debugSphereModel과 동일)
	glm::vec3 playerPosWorld = player.getPosition();
	glm::vec3 playerScale = player.getScale();
	
	// deltaPos 계산: player scale을 적용한 오프셋
	glm::vec3 deltaPos = glm::vec3(0.0f, -0.6f, 0.1f);
	glm::mat4 scaleSphereModel = glm::scale(glm::mat4(1.0f), playerScale);
	deltaPos = glm::vec3(scaleSphereModel * glm::vec4(deltaPos, 1.0f));
	
	glm::vec3 playerCollisionPos = playerPosWorld + deltaPos;
	
	glm::vec4 playerPos = glm::vec4(playerCollisionPos, 1.0f);
	glm::vec4 playerViewPos = view * playerPos;

	// 플레이어가 카메라 뒤에 있으면 무시
	if (playerViewPos.z >= -0.1f) return false;

	float playerDepth = -playerViewPos.z;

	// 플레이어 중심점 NDC 변환
	float px_ndc = playerViewPos.x * proj[0][0] / playerDepth;
	float py_ndc = playerViewPos.y * proj[1][1] / playerDepth;
	
	// 플레이어의 화면상 반지름 계산
	// debugSphereModel: scale = playerScale * 3.0f
	glm::vec3 sphereScale = playerScale * 1.0f;
	float p_radius_x_ndc = sphereScale.x * proj[0][0] / playerDepth / 2;
	float p_radius_y_ndc = sphereScale.y * proj[1][1] / playerDepth / 2;


	// -------------------------------------------------------
	// 3. 충돌 검사: 원 대 원 충돌 (타원 대 타원)
	// -------------------------------------------------------
	// 두 원의 중심 사이 거리 계산
	float dx = px_ndc - bx_ndc;
	float dy = py_ndc - by_ndc;
	
	// 타원 대 타원 충돌: 분리축 정리(SAT)의 간단한 근사
	// 각 타원을 정규화된 공간으로 변환하여 원으로 만든 후 거리 비교
	
	// 총알 타원의 평균 반지름
	float b_radius_avg = (b_radius_x_ndc + b_radius_y_ndc) / 2.0f;
	// 플레이어 타원의 평균 반지름
	float p_radius_avg = (p_radius_x_ndc + p_radius_y_ndc) / 2.0f;
	
	// 정규화된 거리 계산 (타원 공간에서의 거리)
	float normalized_dx = dx / ((b_radius_x_ndc + p_radius_x_ndc) / 2.0f);
	float normalized_dy = dy / ((b_radius_y_ndc + p_radius_y_ndc) / 2.0f);
	float normalized_distance_sq = normalized_dx * normalized_dx + normalized_dy * normalized_dy;
	
	// 두 원의 반지름 합 (정규화된 공간에서는 2.0)
	float sum_radius = 2.0f;
	
	// 충돌 판정: 거리가 반지름 합보다 작으면 충돌
	if (normalized_distance_sq <= sum_radius * sum_radius)
	{
		return true;
	}

	return false;
}