/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <GL/glew.h>
#include <stdio.h>
class OGLTimer{
public:
	OGLTimer(){
		glGenQueries(1,&m_StartQ);
		glGenQueries(1,&m_EndQ);
	}
	~OGLTimer(){

	}
	void Start(){
		glQueryCounter(m_StartQ,GL_TIMESTAMP);
	}
	double Stop(){
		glQueryCounter(m_EndQ,GL_TIMESTAMP);

		GLint timerAvailable = 0;
		while (!timerAvailable){
			glGetQueryObjectiv(m_EndQ,GL_QUERY_RESULT_AVAILABLE,&timerAvailable);
		}

		glGetQueryObjectui64v(m_StartQ,GL_QUERY_RESULT,&m_StartTime);
		glGetQueryObjectui64v(m_EndQ, GL_QUERY_RESULT, &m_EndTime);
		double elapsedTime = (m_EndTime - m_StartTime) / 1000000.0;
		return elapsedTime;
	}
private:
	GLuint m_StartQ, m_EndQ;
	GLuint64 m_StartTime,m_EndTime;
};