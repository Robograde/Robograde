/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "TextureAtlas.h"
#include <utility/Logger.h>
#include <fstream>
#include <string>
gfx::TextureAtlas::TextureAtlas()
{

}

gfx::TextureAtlas::~TextureAtlas()
{
	pDelete(m_Texture);
}

bool gfx::TextureAtlas::LoadFile(const rString& filename)
{
	//load file
	std::ifstream file;
	file.open(filename.c_str());
	if(!file.is_open())
	{
		Logger::Log("Failed to open TextureAtlas " + filename, "Texture",LogSeverity::ERROR_MSG );
		return false;
	}
	rString line;
	std::string json; //picojson only wants a std::string
	m_Filename = filename;
	while(!file.eof()) 
	{
		std::getline(file,line);
		json += line.c_str();
	}
	file.close();
	//parse json
	picojson::value val;
	picojson::parse(val,json);
	//check if file was parsed
	if (! val.is<picojson::object>()) {
		Logger::Log("Failed to parse json file", "Texture", LogSeverity::ERROR_MSG );
		return false;
	}
	//Get root objects
	picojson::object obj = val.get<picojson::object>();
	for ( auto& it : obj ) 
	{
		if(it.first == "frames")
		{
			gfx::TextureFrame texFrame;

			picojson::object filenames = it.second.get<picojson::object>();
			for(auto& it2 : filenames)
			{
				texFrame.Filename = rString(it2.first.c_str());

				picojson::object framedata = it2.second.get<picojson::object>();
				for(auto& data : framedata)
				{
					if(data.first == "frame")
					{
						texFrame.X		= stoi(data.second.get("x").to_str());
						texFrame.Y		= stoi(data.second.get("y").to_str());
						texFrame.Width	= stoi(data.second.get("w").to_str());
						texFrame.Height = stoi(data.second.get("h").to_str());
					}
				}
				m_Frames.push_back(texFrame);
			}
		} //end textureframe data
		else if(it.first == "meta")
		{
			picojson::object atlassData = it.second.get<picojson::object>();
			for(auto& data : atlassData)
			{
				if(data.first == "image")
				{
					m_ImageFilename = rString(data.second.to_str().c_str());
				}
				if(data.first == "size")
				{
					m_Width		= stoi(data.second.get("w").to_str());
					m_Height	= stoi(data.second.get("h").to_str());
				}
			}
		}
	} //end outer for
	m_Texture = pNew(Texture);
	m_Texture->Init((GetDir(m_Filename) + m_ImageFilename).c_str(),TEXTURE_2D);
	//generate handles
	int numerator = 0;
	for(auto& it : m_Frames)
	{
		it.Y = it.Y + it.Height;
		m_Handles[it.Filename] = numerator++;
	}
	//generate gpu frames
	GPUTexture tex;
	for(auto& t : m_Frames)
	{
		tex.S = (t.X / (float) m_Width);
		tex.T = ((m_Height - t.Y) / (float) m_Height);
		tex.NormWidth = t.Width / (float) m_Width;
		tex.NormHeight = t.Height / (float) m_Height;
		m_GPUFrames.push_back(tex);
	}
	m_Frames.clear();
	return true;
}

gfx::Texture* gfx::TextureAtlas::GetTexture()
{
	return m_Texture;
}

int gfx::TextureAtlas::GetHandle(const rString& name)
{
	rMap<rString, int>::iterator it = m_Handles.find(name);
	if(it == m_Handles.end())
	{
		return -1;
	}
	return it->second;
}

rVector<gfx::GPUTexture>& gfx::TextureAtlas::GetGPUTextures()
{
	return m_GPUFrames;
}

rString gfx::TextureAtlas::GetDir ( rString str )
{
    bool found = false;
    for ( int i = static_cast<int> ( str.size() ); i > 0; i-- )
    {
        if ( str.c_str() [i] == '/' )
        {
            found = true;
        }
        if ( !found )
        {
            str.erase ( i );
        }

    }
    return str;
}

void gfx::TextureAtlas::Clear()
{
	
}