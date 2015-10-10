/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "UtilityLibraryDefine.h"
#include "memory/Alloc.h"

namespace FileUtility
{
	UTILITY_API bool	CreateFolder( const rString& directoryPath );
	UTILITY_API bool	DoesFileExist( const rString& filePath );
	UTILITY_API rString FindIncrementedPath( const rString& filePath, const rString& extension, unsigned int begin, unsigned int end );
	UTILITY_API rString FindLastIncrementationOfPath( const rString& filePath, const rString& extension, unsigned int begin, unsigned int end );
	UTILITY_API rString GetDirectoryFromFilePath( const rString& filePath );
	UTILITY_API bool	WriteToFile( const char* data, size_t dataLength, const rString& path );
	UTILITY_API char*	GetFileContent( const rString& filePath, std::ios::open_mode openMode = std::ios::in );
	UTILITY_API void	GetFileContent( const rString& filePath, char*& out, std::ios::open_mode openMode = std::ios::in );
	UTILITY_API rString	GetFileContentAsString( const rString& filePath, std::ios::open_mode openMode = std::ios::in );
	UTILITY_API size_t	GetFileContentSize( const rString& filePath, std::ios::openmode openMode = std::ios::in );
	UTILITY_API size_t	GetFileContentSize( std::ifstream& inStream );
	UTILITY_API void 	GetListOfContentInDirectory( const fString& directoryPath, rVector<rString>& outListWithEntries );
}