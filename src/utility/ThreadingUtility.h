/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include <thread>
#include "../utility/Logger.h"
namespace ThreadingUtility
{
	/// <summary>
	/// Checks if the inputed thread is joinable and if so, joins it.
	/// </summary>
	/// <param name="thread">The thread to join.</param>
	static void JoinThread( std::thread& thread )
	{
		if ( thread.joinable( ) )
		{
			thread.join( );
		}
		else
		{
			Logger::Log( "Attempted to join unjoinable thread", "Threading", LogSeverity::WARNING_MSG );
		}
	}
}