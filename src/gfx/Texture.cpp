/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "Texture.h"
#include <utility/Logger.h>

using namespace gfx;

Texture::Texture()
{
	m_Loaded = false;
	m_Width = 0;
	m_Height = 0;
	m_Filename = "";
	m_Handle = 0;
}

Texture::~Texture()
{
	//check up on how to release textures later
	glDeleteTextures(1, &m_Handle);
}

bool Texture::Init(const char* Filename,TextureType Type)
{
	m_Type = Type;
	m_Filename = rString(Filename);
	if(Type == TEXTURE_2D)
	{
		m_Handle = SOIL_load_OGL_texture(Filename,SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,  SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT);
		if(m_Handle == 0)
		{
			Logger::Log( "Can't load texture " + rString(Filename), "Texture", LogSeverity::WARNING_MSG );
			return false;
		}
		glBindTexture(GL_TEXTURE_2D, m_Handle);
		glGenerateMipmap(GL_TEXTURE_2D);
		GLfloat fLargest;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		m_Loaded = true;
	}
	else if (Type == CUBE_TEXTURE)
	{
		m_Handle = SOIL_load_OGL_single_cubemap(Filename, SOIL_DDS_CUBEMAP_FACE_ORDER, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_COMPRESS_TO_DXT);
		if (m_Handle == 0)
		{
			Logger::Log( "Can't load cube texture " + rString(Filename), "Texture", LogSeverity::WARNING_MSG );
			return false;
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_Handle);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		m_Loaded = true;
	}
	return glIsTexture(m_Handle) == GL_TRUE;
}

bool Texture::Init(char* data, int channels,int width,int height, TextureType Type)
{
	m_Type = Type;
	m_Width = width;
	m_Height = height;
	glGenTextures(1, &m_Handle);
	glBindTexture(GL_TEXTURE_2D, m_Handle);
	switch (channels){
	case 1:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_Width, m_Height, 0, GL_RED, GL_FLOAT, data);
		break;
	case 2:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, m_Width, m_Height, 0, GL_RG, GL_FLOAT, data);
		break;
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, data);
		break;
	case 4:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, data);
		break;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_Loaded = true;
	return true;
}

bool Texture::InitLinear(const char* filename,int width,int height){

	int channels = 3;
	glGenTextures(1, &m_Handle);
	glBindTexture(GL_TEXTURE_2D, m_Handle);
	if (m_Handle == 0)
	{
		printf("Cant create texture");
		return false;
	}
	unsigned char* data = SOIL_load_image(filename, &width, &height, &channels, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, m_Width, m_Height, 0, GL_SRGB, GL_UNSIGNED_BYTE, data);
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_Loaded = true;
	SOIL_free_image_data(data);
	return true;
}

GLuint Texture::GetHandle()
{
	return m_Handle;
}

void Texture::Apply(GLuint location,int index)
{
	glUniform1i(location,index);
	glActiveTexture(GL_TEXTURE0 + index);
	if (m_Type == TEXTURE_2D){
		glBindTexture(GL_TEXTURE_2D, m_Handle);
	}
	else{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_Handle);
	}
}

TextureType Texture::GetType(){
	return m_Type;
}

bool Texture::GetLoaded(){
	return m_Loaded;
}

void Texture::SetFilename(const char* filename){
	m_Filename = rString(filename);
}

rString Texture::GetFilename(){
	return m_Filename;
}
