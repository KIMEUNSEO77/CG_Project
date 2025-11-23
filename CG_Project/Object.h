#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <glew.h>

class Object
{
protected:
	float hp;
	float speed;
	glm::vec3 position;
	glm::vec3 cameraPosition;
	glm::vec3 cameraTarget;
	glm::vec3 cameraUp;
	glm::vec3 color;

	glm::vec3 velocity;  // not same speed
	glm::vec3 scale;
	
public:
	void update();
	virtual void render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) = 0;
	void setPosition(glm::vec3 pos)
	{
		position = pos;
	}
	glm::vec3 getPosition()
	{
		return position;
	}
	void setVelocity(glm::vec3 vel)
	{
		velocity = vel;        
		speed = glm::length(vel);
	}
	glm::vec3 getVelocity()
	{
		return velocity;
	}
	void setColor(glm::vec3 color)
	{
		// 색상 설정 (필요시 구현)
		this->color = color;
	}
	glm::vec3 getColor()
	{
		return color;
	}
	void setScale(glm::vec3 scale)
	{
		this->scale = scale;
	}
	glm::vec3 getScale()
	{
		return scale;
	}
};



class Player : public Object
{
private:
	float power;
public:
	Player()
	{
		// 기본값 설정
		hp = 100.0f;
		speed = 0.1f;
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		power = 10.0f;
	}
	void move(float dx, float dy);
	void render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) override;
	void damaged(float damage);  // 데미지 입음
};



class Bullet : public Object
{
private:
	float damage;
	float vy = 0.0f; // y축 속도 (중력 적용용)
public:
	void render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) override;
	void update_first_paze(float deltaTime);
};

