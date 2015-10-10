/**************************************************
Copyright 2015 David Pejtersen
***************************************************/

#include "SSMemoryDebug.h"
#include <algorithm>

using namespace MemoryAllocator;

namespace MemoryAllocator
{
	size_t MemoryFree( )
	{
		GetLock( ).lock( );
		size_t freeMem = MEMORY_SIZE;

		Header* it = GetBottom( );
		while ( it )
		{
			if ( !it->Free )
				freeMem -= it->Size;
			freeMem -= sizeof( Header );
			it = it->Next;
		}

		it = GetTop( );
		while ( it )
		{
			if ( !it->Free )
				freeMem -= it->Size;
			freeMem -= sizeof( Header );
			it = it->Next;
		}

		GetLock( ).unlock( );
		return freeMem;
	}
}

SSMemoryDebug& SSMemoryDebug::GetInstance( )
{
	static SSMemoryDebug instance;
	return instance;
}

void SSMemoryDebug::Startup( )
{
	GUI::TextDefinition textDef( "", 5, 5, glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f) );
	
	m_Window = g_GUI.AddWindow( "MemoryWindow", GUI::Rectangle( 0, 0, 570, 25 ), "DebugWindow", true );
	m_Window->Open();
	m_Window->SetClickThrough( true );
	
	g_GUI.UseFont( FONT_ID_LEKTON_11 );
	
	m_MemoryUsageText = g_GUI.AddText( "MemoryUsageShit", textDef, "MemoryWindow" );
	
	m_MemoryAllocations = m_AvgMemoryAllocations = 0U;
	m_MemoryMisses = m_AvgMemoryMisses = 0U;

	m_LowestTop			= (size_t) GetTop( );
	m_HightestBottom	= (size_t) GetBottom( );

	Subsystem::Startup();
}

void SSMemoryDebug::UpdateUserLayer( const float deltaTime )
{
	if ( g_GUI.IsWindowOpen( "MemoryWindow" ) )
	{
		if( g_GUI.IsWindowOpen( "ConsoleWindow" ) )
			m_Window->SetPosition( m_Window->GetPosition().x, g_GUI.GetWindowSize( "ConsoleWindow" ).y );
		else
			m_Window->SetPosition( m_Window->GetPosition().x, 0 );
		
#if !defined(DISABLE_ALLOCATOR)
		size_t memorySize = MEMORY_SIZE;
		size_t memoryFree = MemoryFree( );
		size_t memoryUsed =	memorySize - memoryFree;
		size_t memoryPercentageUsed = (size_t) ((memoryUsed / (float) memorySize) * 100.0f);

		m_AvgMemoryMisses = m_MemoryMisses + m_AvgMemoryMisses * (MEMORY_AVG_FACTOR - 1);
		m_AvgMemoryMisses /= MEMORY_AVG_FACTOR;

		m_AvgMemoryAllocations = m_MemoryAllocations + m_AvgMemoryAllocations * (MEMORY_AVG_FACTOR - 1);
		m_AvgMemoryAllocations /= MEMORY_AVG_FACTOR;

		size_t lowPercent	= (size_t) ((m_HightestBottom - (size_t) GetBottom( )) / (float) MEMORY_SIZE * 100.0f);
		size_t highPercent	= (size_t) (((size_t) GetTop( ) - m_LowestTop) / (float) MEMORY_SIZE * 100.0f);

		m_MemoryUsageText->SetText(
			"[C=GREEN]"
			+ rToString( memoryUsed >> 20 ) + " MB"
			+ "[C=BLUE] / "
			+ rToString( memorySize >> 20 ) + " MB"
			+ "[C=PURPLE]   ( "
			+ rToString( memoryPercentageUsed ) + "% )"
			+ "[C=YELLOW]   Dist "
			+ rToString( lowPercent )
			+ "-"
			+ rToString( highPercent )
			+ "[C=GREEN]   Alloc "
			+ rToString( m_AvgMemoryAllocations )
			+ "[C=RED]   Miss "
			+ rToString( m_AvgMemoryMisses ) );
#else
		m_MemoryUsageText->SetText( "[C=RED]Allocator Disabled" );
#endif
		//g_GUI.EnqueueText( &m_MemoryUsageText );
	}
}

void SSMemoryDebug::Shutdown()
{
	g_GUI.DeleteObject( "MemoryWindow" );
}

void SSMemoryDebug::UpdateDistance( )
{
	m_LowestTop
		= (size_t) GetHigh( ) < m_LowestTop
		? (size_t) GetHigh( ) : m_LowestTop;
	m_HightestBottom
		= (size_t) GetLow( ) + GetLow( )->Size > m_HightestBottom
		? (size_t) GetLow( ) + GetLow( )->Size : m_HightestBottom;
}

std::string SSMemoryDebug::GetMemoryUsagePerFile( )
{
#if defined(DEBUG_MEMORY) && !defined(DISABLE_ALLOCATOR)
	std::vector<std::pair<std::string,size_t>> memoryUsage; // Use memory outside of memory allocator in order to be able to use the locks

	auto filename = [] (const std::string& fullPath ) -> std::string {
		size_t pos1 = fullPath.rfind( '/', fullPath.length( ) );
		pos1 = pos1 == std::string::npos ? 0 : pos1;
		size_t pos2 = fullPath.rfind( '\\', fullPath.length( ) );
		pos2 = pos2 == std::string::npos ? 0 : pos2;
		size_t pos = pos1 < pos2 ? pos2 : pos1;
		if ( pos != std::string::npos && pos != 0 )
			return fullPath.substr( pos + 1, std::string::npos );
		return fullPath;
	};

	memoryUsage.push_back( std::pair<std::string, size_t>( "_ALLOCATION_HEADER_", 0ULL ) );
	GetLock( ).lock( );
	Header* header = GetBottom( );
	while ( header )
	{
		if ( header->Next )
		{
			assert( header == header->Next->Prev ); // H <-> N     NN
			if ( header->Next->Next )
				assert( header->Next == header->Next->Next->Prev ); // H     N <-> NN
		}
		if ( header->Prev )
		{
			assert( header == header->Prev->Next ); // PP     P <-> H
			if ( header->Prev->Prev )
				assert( header->Prev == header->Prev->Prev->Next ); // PP <-> P     H
		}

		memoryUsage[0].second += sizeof( Header );

		if ( header->Free )
		{
			header = header->Next;
			continue;
		}

		bool found = false;
		for ( auto& file : memoryUsage )
		{
			if ( file.first == filename( header->File ) )
			{
				file.second += header->Size;
				found = true;
				break;
			}
		}

		if ( !found )
			memoryUsage.push_back( std::pair<std::string, size_t>( filename( header->File ), header->Size ) );

		header = header->Next;
	}

	header = GetTop( );
	while ( header )
	{
		if ( header->Next )
		{
			assert( header == header->Next->Prev ); // H <-> N     NN
			if ( header->Next->Next )
				assert( header->Next == header->Next->Next->Prev ); // H     N <-> NN
		}
		if ( header->Prev )
		{
			assert( header == header->Prev->Next ); // PP     P <-> H
			if ( header->Prev->Prev )
				assert( header->Prev == header->Prev->Prev->Next ); // PP <-> P     H
		}

		memoryUsage[0].second += sizeof( Header );

		if ( header->Free )
		{
			header = header->Next;
			continue;
		}

		bool found = false;
		for ( auto& file : memoryUsage )
		{
			if ( file.first == filename( header->File ) )
			{
				file.second += header->Size;
				found = true;
				break;
			}
		}

		if ( !found )
			memoryUsage.push_back( std::pair<std::string, size_t>( filename( header->File ), header->Size ) );

		header = header->Next;
	}
	GetLock( ).unlock( );

	std::sort( memoryUsage.begin( ), memoryUsage.end( ), [] ( const std::pair<std::string, size_t>& a, const std::pair<std::string, size_t>& b ) -> bool { return a.second > b.second; } );

	float f = 0;
	std::stringstream out;
	out.precision( 2 );
	for ( auto& file : memoryUsage )
		if ( file.second > (MEMORY_SIZE >> 10) )
			out << "[C=GREEN](" << 100.0f * file.second / MEMORY_SIZE << "%) [C=PURPLE]" << (file.second >> 10) << " kB [C=BLUE]" << file.first << "\n";
	return out.str( );
#else
	return "[C=RED]Memory Debug OFF";
#endif
}
