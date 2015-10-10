/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include <atomic>
#include <memory/Alloc.h>

template <typename T>
class LocklessQueue
{
private:
	struct Node
	{
		Node( T val ) : value( val ), next( nullptr ) {}
		T value;
		Node* next;
	};

	Node* first;
	std::atomic<Node*> divider, last;

public:
	LocklessQueue( )
	{
		first = divider = last = tNew( Node, T( ) ); // Dummy divider
	}

	~LocklessQueue( )
	{
		while ( first != nullptr )	// Loop trough the queue and delete all remaining items
		{
			Node* temp = first;
			first = temp->next;
			tDelete( temp );
		}
	}

	void Produce( const T& t )
	{
		last.load( )->next = tNew( Node, t );	// Add the new node
		last = last.load( )->next;				// Last takes a step forward and thereby publishes the new node so that the consumers can use it
		while ( first != divider )				// Trim away unused nodes
		{
			Node* temp = first;
			first = first->next;
			tDelete( temp );
		}
	}

	bool Consume( T& result )
	{
		if ( divider.load( ) != last.load( ) )		// Check so that the queue isn't empty
		{
			result = divider.load( )->next->value;	// Get the value of the next item in queue
			divider = divider.load( )->next;		// Signal that we took it by moving the divider one step forward
			return true;
		}
		return false;	// Report list empty
	}

	void Clear() // May not run alongside a thread that uses Consume()
	{
		T val;
		while ( Consume( val ) );
	}
};