#pragma once
#include <iostream>

#include <glew.h>
#include "filetobuf.h"

GLint width = 1500, height = 800;
// general shader
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

// hp shader
GLuint hpShaderProgramID;
GLuint hpVertexShader;
GLuint hpFragmentShader;

// bg shader
GLuint bgShaderProgramID;
GLuint bgVertexShader;
GLuint bgFragmentShader;

// boss shader
GLuint bossShaderProgramID;
GLuint bossVertexShader;
GLuint bossFragmentShader;

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
		// x(0~1), y(0~1)
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
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
	vertexSource = filetobuf("hp_vertex.glsl");
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

void make_vertexShaders_bg()
{
	GLchar* vertexSource;
	vertexSource = filetobuf("bg_vertex.glsl");
	bgVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(bgVertexShader, 1, &vertexSource, NULL);
	glCompileShader(bgVertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(bgVertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(bgVertexShader, 512, NULL, errorLog);
		std::cerr << "Error: vertex shader            \n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders_bg()
{
	GLchar* fragmentSource;
	fragmentSource = filetobuf("bg_fragment.glsl");
	bgFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(bgFragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(bgFragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(bgFragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(bgFragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader            \n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram_bg()
{
	GLint result;
	GLchar* errorLog = NULL;
	GLuint shaderID;
	shaderID = glCreateProgram();

	glAttachShader(shaderID, bgVertexShader);
	glAttachShader(shaderID, bgFragmentShader);

	glLinkProgram(shaderID);

	glDeleteShader(bgVertexShader);
	glDeleteShader(bgFragmentShader);

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



void make_vertexShaders_boss()
{
	GLchar* vertexSource;
	vertexSource = filetobuf("boss_vertex.glsl");
	bossVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(bossVertexShader, 1, &vertexSource, NULL);
	glCompileShader(bossVertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(bossVertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(bossVertexShader, 512, NULL, errorLog);
		std::cerr << "Error: vertex shader            \n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders_boss()
{
	GLchar* fragmentSource;
	fragmentSource = filetobuf("boss_fragment.glsl");
	bossFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(bossFragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(bossFragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(bossFragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(bossFragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader            \n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram_boss()
{
	GLint result;
	GLchar* errorLog = NULL;
	GLuint shaderID;
	shaderID = glCreateProgram();

	glAttachShader(shaderID, bossVertexShader);
	glAttachShader(shaderID, bossFragmentShader);

	glLinkProgram(shaderID);

	glDeleteShader(bossVertexShader);
	glDeleteShader(bossFragmentShader);

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