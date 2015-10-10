/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "PlatformDefinitions.h"
#if PLATFORM == PLATFORM_WINDOWS
#include "RetardedWindowsIncludes.h"
#endif
#include "FileUtility.h"
#include <fstream>
#include "Logger.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

bool FileUtility::CreateFolder( const rString& directoryPath )
{ 
	int result;

#if PLATFORM == PLATFORM_WINDOWS
	result = _mkdir( directoryPath.c_str() );
#else
	result = mkdir( directoryPath.c_str(), 0777 );
#endif
	if ( result == 0 )
		return true;
	else
	{
		switch ( errno ) // TODODB: Check against more error values
		{
			case EEXIST:
				return true;

			case ENOENT:
				Logger::Log( "Unable to create diretory on path " + directoryPath + "since it is invalid", "FileUtility", LogSeverity::WARNING_MSG );
				return false;

			default:
				Logger::Log( "Unable to create directory - Unknown error", "FileUtility", LogSeverity::WARNING_MSG );
				return false;
				break;
		}
	}
}

bool FileUtility::DoesFileExist( const rString& filePath )
{
	std::ifstream file( filePath.c_str() );
	return file.good();
}

rString FileUtility::FindIncrementedPath( const rString& filePath, const rString& extension, unsigned int begin, unsigned int end )
{
	for ( unsigned int i = begin; i < end; ++i )
	{
		rString pathToTest = filePath + rToString( i ) + extension;
		if ( !DoesFileExist( pathToTest ) )
			return pathToTest;
	}
	
	return "";
}

rString FileUtility::FindLastIncrementationOfPath( const rString& filePath, const rString& extension, unsigned int begin, unsigned int end )
{
	rString lastTestedPath = "";
	for ( unsigned int i = begin; i < end; ++i )
	{
		rString pathToTest = filePath + rToString( i ) + extension;
		if ( !DoesFileExist( pathToTest ) )
			return lastTestedPath;

		lastTestedPath = pathToTest;
	}

	return filePath + rToString( end - 1 ) + extension;
}

rString FileUtility::GetDirectoryFromFilePath( const rString& filePath )
{
	int lastSlashPos = static_cast<int>( filePath.rfind( '/' ) );
	return filePath.substr( 0, lastSlashPos );
}

bool FileUtility::WriteToFile( const char* data, size_t dataLength, const rString& path ) // TODODB: Make ++C
{
	FILE* pFile;
	if ( ( pFile = fopen( path.c_str(), "w" ) ) == nullptr )
		return false;

	fwrite( data, sizeof( char ), dataLength, pFile );

	fclose( pFile );
	return true;
}

char* FileUtility::GetFileContent( const rString& filePath, std::ios::open_mode openMode )
{
	char* toReturn = nullptr;

	std::ifstream inStream;
	inStream.open( filePath.c_str(), openMode );
	if ( inStream.is_open() )
	{
		// Check the file to see how big a buffer we are going to need
		size_t bufferSize = GetFileContentSize( inStream );

		// Allocate the buffer and fill it with the data from the file
		if ( bufferSize > 0 )
		{
			toReturn = static_cast<char*>( fMalloc( bufferSize ) ); // TODODB: Take this as out parameter instead
			inStream.read( toReturn, bufferSize );
		}
		else
			Logger::Log( "Attempted to get content of empty file(Path = " + filePath + " )", "FileUtility", LogSeverity::WARNING_MSG );
		inStream.close(); // We now have all the data in memory so the file can be closed
	}
	else
		Logger::Log( "GetFileContent was unable to open file (Path = " + filePath + " )", "FileUtility", LogSeverity::WARNING_MSG );

	return toReturn;
}

void FileUtility::GetFileContent( const rString& filePath, char*& out, std::ios::open_mode openMode )
{
	out = GetFileContent( filePath, openMode );
}

rString FileUtility::GetFileContentAsString( const rString& filePath, std::ios::open_mode openMode )
{
	rString toReturn;

	size_t fileContentSize = GetFileContentSize( filePath, openMode );
	char* fileContent = static_cast<char*>( fMalloc( fileContentSize + 1) );
	GetFileContent( filePath, fileContent, openMode );
	fileContent[fileContentSize] = '\0';

	if ( fileContent != nullptr )
	{
		toReturn = rString( fileContent );
		fFree( fileContent );
	}

	return toReturn;
}

size_t FileUtility::GetFileContentSize( const rString& filePath, std::ios::openmode openMode )
{
	size_t size = 0;

	std::ifstream inStream;
	inStream.open( filePath.c_str(), openMode );

	if ( inStream.is_open() )
	{
		inStream.seekg( 0, std::ios::end );
		size = inStream.tellg();
		inStream.close();
	}
	else
		Logger::Log( "GetFileSize was unable to open file (Path = " + filePath + " )", "FileUtility", LogSeverity::WARNING_MSG );

	return size;
}

size_t FileUtility::GetFileContentSize( std::ifstream& inStream )
{
	size_t size = 0;

	if ( inStream.is_open() )
	{
		size_t startPos = inStream.tellg();
		inStream.seekg( 0, std::ios::end );
		size = inStream.tellg();
		inStream.clear();
		inStream.seekg( startPos );
	}
	else
		Logger::Log( "GetFileContentSize received an unopened ifstream", "FileUtility", LogSeverity::WARNING_MSG );

	return size;
}

void FileUtility::GetListOfContentInDirectory( const fString& directoryPath, rVector<rString>& outListWithEntries )
{
#if PLATFORM == PLATFORM_LINUX
	DIR *directory = opendir(directoryPath.c_str());
    if ( directory )
    {
        struct dirent *directoryEntry; 
        while((directoryEntry = readdir(directory)) != nullptr) 
        {
			fString inFolder( directoryEntry->d_name );
			if ( inFolder != "." && inFolder != ".." ) // Ignore irrelevant folders
			{
				outListWithEntries.push_back( inFolder.c_str() );
			}
        }
    } 
    else
    { 
		Logger::Log( "Failed to open directory: " + rString(directoryPath.c_str()), "FileUtility", LogSeverity::ERROR_MSG );
    }
#endif
#if PLATFORM == PLATFORM_WINDOWS
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;

	hFind = FindFirstFile((directoryPath + "*").c_str(), &findFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
	{
		Logger::Log( "Failed to open directory: " + rString(directoryPath.c_str()), "FileUtility", LogSeverity::ERROR_MSG );
		return;
	} 
	else 
	{
		do
		{
			fString inFolder( findFileData.cFileName );
			if ( inFolder != "." && inFolder != ".." ) // Ignore irrelevant folders
				outListWithEntries.push_back( inFolder.c_str() );
		}
		while ( FindNextFile(hFind, &findFileData) != 0 );
		FindClose(hFind);
	}
#endif
}
