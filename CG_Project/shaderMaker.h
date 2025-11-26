#pragma once
#include <iostream>

#include <glew.h>
#include "filetobuf.h"

GLint width = 1500, height = 800;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

GLuint hpShaderProgramID;
GLuint hpVertexShader;
GLuint hpFragmentShader;

void make_vertexShaders()
{
	GLchar* vertexSource;
	vertexSource = filetobuf("vertex.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "Error: vertex shader            \n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	GLchar* fragmentSource;
	fragmentSource = filetobuf("fragment.glsl");
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader            \n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram()
{
	GLint result;
	GLchar* errorLog = NULL;
	GLuint shaderID;
	shaderID = glCreateProgram();

	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);

	glLinkProgram(shaderID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program          \n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID);
	return shaderID;
}

GLuint hpVAO, hpVBO;

void InitHPBar()
{
	float hpVertices[] = {
		// x,     y
		-0.9f,  0.90f,  // 왼쪽 위
		-0.9f,  0.85f,  // 왼쪽 아래
		-0.3f,  0.85f,  // 오른쪽 아래
		-0.3f,  0.90f   // 오른쪽 위
	};

	glGenVertexArrays(1, &hpVAO);
	glGenBuffers(1, &hpVBO);

	glBindVertexArray(hpVAO);
	glBindBuffer(GL_ARRAY_BUFFER, hpVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(hpVertices), hpVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void make_vertexShaders_hp()
{
	GLchar* vertexSource;
	vertexSource = filetobuf("vertex.glsl");
	hpVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(hpVertexShader, 1, &vertexSource, NULL);
	glCompileShader(hpVertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(hpVertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(hpVertexShader, 512, NULL, errorLog);
		std::cerr << "Error: vertex shader            \n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders_hp()
{
	GLchar* fragmentSource;
	fragmentSource = filetobuf("hp_fragment.glsl");
	hpFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(hpFragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(hpFragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(hpFragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(hpFragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader            \n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram_hp()
{
	GLint result;
	GLchar* errorLog = NULL;
	GLuint shaderID;
	shaderID = glCreateProgram();

	glAttachShader(shaderID, hpVertexShader);
	glAttachShader(shaderID, hpFragmentShader);

	glLinkProgram(shaderID);

	glDeleteShader(hpVertexShader);
	glDeleteShader(hpFragmentShader);

	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program          \n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID);
	return shaderID;
}