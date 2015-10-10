/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "ShaderProgram.h"

#include <utility/Logger.h>

gfx::ShaderProgram::ShaderProgram ( void )
{
    m_LoadedShaders = false;
	m_TextureCount = 0;
}

gfx::ShaderProgram::ShaderProgram ( rVector<Shader*> Shaders )
{
    this->m_Shaders = Shaders;
    Init ( m_Shaders,true );
}

gfx::ShaderProgram::~ShaderProgram ( void )
{
    if ( m_LoadedShaders )
    {
        for ( rVector<Shader*>::iterator it = m_Shaders.begin(); it != m_Shaders.end(); ++it )
        {
			tDelete( *it );
        }
        m_Shaders.clear();
    }

    glDeleteProgram ( m_Handle );
}

bool gfx::ShaderProgram::Init ( rVector<Shader*> Shaders, bool print )
{
    if ( m_Shaders.size() > 0 )
    {
        m_Shaders.clear();
    }
    //save shaders
    for ( auto it : Shaders )
    {
        m_Shaders.push_back ( it );
    }
    m_Handle = glCreateProgram();
    if ( m_Handle == 0 )
    {
        throw "Failed to create shader program";
        return false;
    }
    //attach shaders to program
    for ( int i = 0; i < static_cast<int> ( Shaders.size() ); i++ )
    {
        glAttachShader ( m_Handle,Shaders[i]->GetHandle() );
    }
    //link shaders
    glLinkProgram ( m_Handle );
    //error checking
    GLint status;
    glGetProgramiv ( m_Handle,GL_LINK_STATUS,&status );
    if ( status == GL_FALSE )
    {
        char log[1024];
        int len = 0;
		Logger::Log( "Can't link shaders", "ShaderProgram", LogSeverity::ERROR_MSG );
        glGetProgramInfoLog ( m_Handle, sizeof ( log ), &len, log );
		Logger::Log( "Errorlog: " + rString( log ), "ShaderProgram", LogSeverity::ERROR_MSG );
        return false;
    }
    if ( print )
    {
		Logger::Log( "Linked Shaderprogram", "ShaderProgram", LogSeverity::DEBUG_MSG );
    }
    // Not using "return Validate();" because stuff might get added after
    if ( !Validate() )
    {
        return false;
    }
    return true;
}

GLuint gfx::ShaderProgram::GetHandle()
{
    return m_Handle;
}

void gfx::ShaderProgram::Apply()
{
    // Lazy validation because of AMD driver behaviour.
    if ( !m_Validated )
    {
        if ( !Validate() )
        {
            GLint status;
            glGetProgramiv ( m_Handle, GL_VALIDATE_STATUS, &status );
            if ( status == GL_FALSE )
            {
                char log[1024];
                int len = 0;
                glGetProgramInfoLog ( m_Handle, sizeof ( log ), &len, log );
				
				if (len > 0)
				{
					Logger::Log("Validation of shader program: \"" + m_Filename + "\" failed: " + rString(log, len - 1), "ShaderProgram", LogSeverity::ERROR_MSG);
				}
                // TODO assert or fancy recovery when recompiling in run-time? assert for now.
               assert ( false );
            }
        }
    }

    m_TextureCount = 0;
    glUseProgram ( m_Handle );
}

bool gfx::ShaderProgram::Validate()
{
    glValidateProgram ( m_Handle );
    GLint status;
    glGetProgramiv ( m_Handle, GL_VALIDATE_STATUS, &status );
    m_Validated = ( status == GL_TRUE );
    return m_Validated;
}

GLuint gfx::ShaderProgram::GetUniformLocationByName ( const rString& name )
{
    return glGetUniformLocation ( m_Handle,name.c_str() );
}
//Create shaderprogram
bool gfx::ShaderProgram::LoadCompleteShaderProgramFromFile ( const rString& filename, bool print )
{
    //this function is used to load a set of shaders from a single file
    //seperate each shader with a #new_shader ******* and a #end_shader
    //where **** can be a vertex, geometry, control, evaluation or fragment
    //extra code can be loaded into the current shader by the #include filename macro
    m_Filename = rString ( filename.c_str() );
    m_LoadedShaders = true;
    ifstream fin;
    fin.open ( filename.c_str() );
    if ( !fin.is_open() )
    {
        if ( print )
        {
			Logger::Log( "Could not open file " + filename, "ShaderProgram", LogSeverity::ERROR_MSG );
			return false;
        }
        return false;
    }
    rString line;
    size_t found;
    while ( !fin.eof() )
    {
        getline ( fin, line );
		if ( (found = line.find( "#new_shader" )) != rString::npos )
        {
			if ( (found = line.find( "vertex" )) != rString::npos )
            {
                CreateShader ( &fin, GL_VERTEX_SHADER, filename,print );
            }
			if ( (found = line.find( "geometry" )) != rString::npos )
            {
                CreateShader ( &fin, GL_GEOMETRY_SHADER, filename, print );
            }
			if ( (found = line.find( "control" )) != rString::npos )
            {
                CreateShader ( &fin, GL_TESS_CONTROL_SHADER, filename, print );
            }
			if ( (found = line.find( "evaluation" )) != rString::npos )
            {
                CreateShader ( &fin, GL_TESS_EVALUATION_SHADER, filename, print );
            }
			if ( (found = line.find( "fragment" )) != rString::npos )
            {
                CreateShader ( &fin, GL_FRAGMENT_SHADER, filename, print );
            }
			if ( (found = line.find( "compute" )) != rString::npos )
            {
                CreateShader ( &fin, GL_COMPUTE_SHADER, filename, print );
            }
        }
    }

    m_Handle = glCreateProgram();
    if ( m_Handle == 0 )
    {
        if ( print )
        {
			Logger::Log( "Could not create shader program", "ShaderProgram", LogSeverity::ERROR_MSG );
			return false;
        }
        return false;
    }
    //attach shaders to program
    for ( int i = 0; i < static_cast<int> ( m_Shaders.size() ); i++ )
    {
        glAttachShader ( m_Handle, m_Shaders[i]->GetHandle() );
    }
    //link shaders
    glLinkProgram ( m_Handle );
    //error checking
    GLint status;
    glGetProgramiv ( m_Handle, GL_LINK_STATUS, &status );
    if ( status == GL_FALSE )
    {
        char log[1024];
        int len = 0;
		Logger::Log( "Could not link shaders to program", "ShaderProgram", LogSeverity::ERROR_MSG );
        glGetProgramInfoLog ( m_Handle, sizeof ( log ), &len, log );
		Logger::Log( "Error log: " + rString( log ), "ShaderProgram", LogSeverity::ERROR_MSG );
        return false;
    }
    if ( print )
    {
		Logger::Log( "Linked Shaders to program", "ShaderProgram", LogSeverity::DEBUG_MSG );
    }
    Validate();
	return true;
}
// Create shaderprogram with a define container
bool gfx::ShaderProgram::LoadCompleteShaderProgramFromFile ( const rString& filename, bool print, ShaderDefineContainer& container )
{
    //this function is used to load a set of shaders from a single file
    //seperate each shader with a #new_shader ******* and a #end_shader
    //where **** can be a vertex, geometry, control, evaluation or fragment
    //extra code can be loaded into the current shader by the #include filename macro
    m_Filename = rString ( filename.c_str() );
    m_LoadedShaders = true;
	m_Defines = container;
    ifstream fin;
    fin.open ( filename.c_str() );
    if ( !fin.is_open() )
    {
        if ( print )
        {
			Logger::Log( "Could not open file " + filename, "ShaderProgram", LogSeverity::ERROR_MSG );
			return false;
        }
        return false;
    }
    rString line;
    size_t found;
    while ( !fin.eof() )
    {
        getline ( fin, line );
		if ( (found = line.find( "#new_shader" )) != rString::npos )
        {
			if ( (found = line.find( "vertex" )) != rString::npos )
            {
                CreateShader ( &fin, GL_VERTEX_SHADER, filename,print,container );
            }
			if ( (found = line.find( "geometry" )) != rString::npos )
            {
                CreateShader ( &fin, GL_GEOMETRY_SHADER, filename, print,container );
            }
			if ( (found = line.find( "control" )) != rString::npos )
            {
                CreateShader ( &fin, GL_TESS_CONTROL_SHADER, filename, print,container );
            }
			if ( (found = line.find( "evaluation" )) != rString::npos )
            {
                CreateShader ( &fin, GL_TESS_EVALUATION_SHADER, filename, print,container );
            }
			if ( (found = line.find( "fragment" )) != rString::npos )
            {
                CreateShader ( &fin, GL_FRAGMENT_SHADER, filename, print,container );
            }
			if ( (found = line.find( "compute" )) != rString::npos )
            {
                CreateShader ( &fin, GL_COMPUTE_SHADER, filename, print,container );
            }
        }
    }
    m_Handle = glCreateProgram();
    if ( m_Handle == 0 )
    {
        if ( print )
        {
			Logger::Log( "Could not create shader program", "ShaderProgram", LogSeverity::ERROR_MSG );
			return false;
        }
        return false;
    }
    //attach shaders to program
    for ( int i = 0; i < static_cast<int> ( m_Shaders.size() ); i++ )
    {
        glAttachShader ( m_Handle, m_Shaders[i]->GetHandle() );
    }
    //link shaders
    glLinkProgram ( m_Handle );
    //error checking
    GLint status;
    glGetProgramiv ( m_Handle, GL_LINK_STATUS, &status );
    if ( status == GL_FALSE )
    {
        char log[1024];
        int len = 0;
		Logger::Log( "Could not link shaders to program", "ShaderProgram", LogSeverity::ERROR_MSG );
        glGetProgramInfoLog ( m_Handle, sizeof ( log ), &len, log );
		Logger::Log( "Error log: " + rString( log ), "ShaderProgram", LogSeverity::ERROR_MSG );
		return false;
    }
    if ( print )
    {
		Logger::Log( "Linked Shaders to program", "ShaderProgram", LogSeverity::DEBUG_MSG );
    }
    Validate();
	return true;
}

//Creates a shader
void gfx::ShaderProgram::CreateShader ( ifstream* FileStream, GLenum shaderType, const rString& filename,bool print )
{
    rString line;
    size_t found;

    rString shaderText;
    bool end = false;
    while ( !end )
    {
        getline ( *FileStream, line );
        //search for the end of the shader
        found = line.find ( "#include " );
        if ( found != rString::npos )
        {
            int i = static_cast<int> ( found ) + 9; //found + "#include "
            rString s;
            while ( i < static_cast<int> ( line.length() ) )
            {
                s += line[i];
                i++;
            }
            rString str = GetDir ( rString ( filename ) ) + s;
            shaderText += LoadText ( str.c_str() );
            shaderText += "\n";
        }
        else
        {
            found = line.find ( "#end_shader" );
			if ( found != rString::npos )
            {
                //break loop
                end = true;
                break;
            }
            else if ( FileStream->eof() )
            {
				Logger::Log( "Could not find end of file " + filename, "ShaderProgram", LogSeverity::ERROR_MSG );
                return;
            }
            else
            {
                shaderText += line;
                shaderText += '\n';
            }

        }

    }
    if ( shaderText.length() > 0 )
    {
		Shader* shader = tNew( Shader );
        shader->CreateFromString ( shaderText, shaderType, filename,print );
        m_Shaders.push_back ( shader );
        shaderText.clear();
    }
}
//Creates a shader with defines
void gfx::ShaderProgram::CreateShader ( ifstream* FileStream, GLenum shaderType, const rString& filename,bool print, ShaderDefineContainer& container )
{
    rString line;
    size_t found;
    rString shaderText;
	getline(*FileStream, line);
	shaderText += line;
	shaderText += "\n";
	shaderText += container.GetAllDefineLines();
    shaderText += container.GetDefinesShaderStage ( shaderType );
    bool end = false;
    while ( !end )
    {
        getline ( *FileStream, line );
        //search for the end of the shader
        found = line.find ( "#include " );
		if ( found != rString::npos )
        {
            int i = static_cast<int> ( found ) + 9; //found + "#include "
            rString s;
            while ( i < static_cast<int> ( line.length() ) )
            {
                s += line[i];
                i++;
            }
            rString str = GetDir ( rString ( filename.c_str() ) ) + s;
            shaderText += LoadText ( str.c_str() );
            shaderText += "\n";
        }
        else
        {
            found = line.find ( "#end_shader" );
			if ( found != rString::npos )
            {
                //break loop
                end = true;
                break;
            }
            else if ( FileStream->eof() )
            {
				Logger::Log( "Could not find end of file " + filename, "ShaderProgram", LogSeverity::ERROR_MSG );
                return;
            }
            else
            {
                shaderText += line;
                shaderText += '\n';
            }

        }

    }
    if ( shaderText.length() > 0 )
    {
		Shader* shader = tNew( Shader );
        shader->CreateFromString ( shaderText, shaderType, filename,print );
        m_Shaders.push_back ( shader );
        shaderText.clear();
    }
}
rString gfx::ShaderProgram::GetDir ( rString filename )
{
    bool found = false;
    for ( int i = static_cast<int> ( filename.size() ); i > 0; i-- )
    {
        if ( filename.c_str() [i] == '/' )
        {
            found = true;
        }
        if ( !found )
        {
            filename.erase ( i );
        }
    }
    return filename;
}

rString gfx::ShaderProgram::LoadText ( const char* filename )
{
    ifstream fin;
    fin.open ( filename );
    if ( !fin.is_open() )
    {
        return "";
    }
    rString text;
    rString line;
    while ( !fin.eof() )
    {
        getline ( fin, line );
        text += line;
		text += "\n";
    }
    return text;

}

void gfx::ShaderProgram::SetUniformFloat ( const rString& name, const float value )
{
    glUniform1f ( FetchUniform ( name ), value );
}

void gfx::ShaderProgram::SetUniformInt( const rString& name, const int value )
{
    glUniform1i ( FetchUniform ( name ), value );
}

void gfx::ShaderProgram::SetUniformUInt ( const rString& name, const unsigned int value )
{
    glUniform1ui ( FetchUniform ( name ), value );
}

void gfx::ShaderProgram::SetUniformBool ( const rString& name, const bool value )
{
    glUniform1i ( FetchUniform ( name ), value );
}

void gfx::ShaderProgram::SetUniformVec3 ( const rString& name, const glm::vec3& value )
{
    glUniform3f ( FetchUniform ( name ), value.x,value.y,value.z );
}

void gfx::ShaderProgram::SetUniformVec2 ( const rString& name, const glm::vec2& value )
{
    glUniform2f ( FetchUniform ( name ), value.x, value.y );
}

void gfx::ShaderProgram::SetUniformVec4 ( const rString& name, const glm::vec4& value )
{
    glUniform4f ( FetchUniform ( name ), value.x, value.y, value.z,value.w );
}

void gfx::ShaderProgram::SetUniformMat4 ( const rString& name, const glm::mat4x4& value )
{
    glUniformMatrix4fv ( FetchUniform ( name ), 1, GL_FALSE, &value[0][0] );
}

void gfx::ShaderProgram::SetUniformTexture(const rString& name, Texture& tex){
	tex.Apply(FetchUniform(name), m_TextureCount++);
}
void gfx::ShaderProgram::SetUniformTextureHandle(const rString& name, GLuint tex, int index){
	glUniform1i(FetchUniform(name), index);
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, tex);
}

GLint gfx::ShaderProgram::FetchUniform ( const rString& name )
{
	rMap<rString, GLint>::const_iterator it = m_UniformMap.find ( name );
	// Uniform wasn't found in cache
	if ( it == m_UniformMap.end() )
    {
		GLint loc = GetUniformLocationByName ( name );
        // Don't emplace in cache if uniform was invalid
        if ( loc != -1 )
        {
            m_UniformMap.emplace ( name, loc );
        }
        else
        {
            //printf("Failed to fetch unifrom : %s\n",name.c_str());
            //assert(false); //makes it annoying to debug shaders
        }
        return loc;// Uniform was found in cache
    }
    else
    {
        return it->second;
	}
}

bool gfx::ShaderProgram::Recompile()
{
    if ( !m_LoadedShaders )
    {
        for ( auto it : m_Shaders )
        {
            it->Recompile();
        }
        glDeleteProgram ( m_Handle );
        m_Validated = false;
        return Init ( m_Shaders, false );;
    }
    else
    {
        //clear up old shader
        for ( rVector<Shader*>::iterator i = m_Shaders.begin(); i != m_Shaders.end(); i++ )
        {
			tDelete( *i );
        }
        m_Shaders.clear();
        glDeleteProgram ( m_Handle );
        //reinit the program
		if(m_Defines.IsEmpty())
		{
			LoadCompleteShaderProgramFromFile ( m_Filename, false );
		}
		else 
		{
			LoadCompleteShaderProgramFromFile ( m_Filename, false, m_Defines );
		}
        
        m_Validated = false;
        return true;
    }

}

rString& gfx::ShaderProgram::GetFilename()
{
	return m_Filename;
}

void gfx::ShaderProgram::SaveProgramBinary(){
	if(m_Handle <= 0){
		return;
	}
	GLint formatCount;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS,&formatCount);

	GLint* formats = new GLint[formatCount];
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats);

	GLint length;
	glGetProgramiv(m_Handle,GL_PROGRAM_BINARY_LENGTH, &length);

	GLenum* binaryFormats = new GLenum();
	GLbyte* programBinary = new GLbyte[length];
	glGetProgramBinary(m_Handle,length,nullptr, binaryFormats,programBinary);

	ShaderProgramBinary header;
	header.Size = length;
	strcpy(header.Version,(char*)glGetString(GL_VERSION));

	std::stringstream ss;
	ss << m_Filename;
	ss << ".bin";
	FILE* file = fopen(ss.str().c_str(),"wb");
	fwrite(&header,sizeof(ShaderProgramBinary),1,file);

	fwrite(programBinary,length,1,file);
	fclose(file);
	Logger::Log("Saved program binary", "ShaderProgram", LogSeverity::INFO_MSG);
	delete [] programBinary;
	delete binaryFormats;
	delete formats;
}

void gfx::ShaderProgram::LoadProgramBinary(const rString& filename)
{
	std::stringstream ss;
	ss << filename;
	ss << ".bin";
	FILE* fp = fopen(ss.str().c_str(), "rb");
	//header
	ShaderProgramBinary header;
	fseek(fp, 0, SEEK_SET);
	fread(&header, sizeof(ShaderProgramBinary),1,fp);
	//shader prog
	GLbyte* binary = new GLbyte[header.Size];
	fseek(fp, sizeof(ShaderProgramBinary), SEEK_SET);
	fread(binary, header.Size, 1, fp);
	fclose(fp);

	GLint formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	GLint *binaryFormats = new GLint[formats];
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binaryFormats);

	m_Handle = glCreateProgram();

	glProgramBinary(m_Handle,(GLenum)(*binaryFormats),binary,header.Size);
	GLint succes;
	glGetProgramiv(m_Handle, GL_LINK_STATUS, &succes);

	if(!succes)
	{
		Logger::Log("Failed to load shader program binary", "ShaderProgram", LogSeverity::ERROR_MSG);
	}
	delete [] binary;
	delete [] binaryFormats;
}

bool gfx::ShaderProgram::LoadProgramBinaryHeader(const rString& filename, ShaderProgramBinary& outHeader)
{
	std::stringstream ss;
	ss << filename;
	ss << ".bin";
	FILE* fp = fopen(ss.str().c_str(), "rb");
	if(fp == nullptr)
		return false;
	//header
	fseek(fp, 0, SEEK_SET);
	fread(&outHeader, sizeof(ShaderProgramBinary),1,fp);
	fclose(fp);
	return true;
}
