/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "Subscriber.h"

Subscriber::Subscriber( const rString& name )
{
	m_Name = name;
}

Subscriber::~Subscriber( )
{

}

bool Subscriber::operator==( const Subscriber& rhs )
{
	return this->m_Name == rhs.m_Name;
}

const int Subscriber::GetUserInterests( ) const
{
	return m_UserInterests;
}

const int Subscriber::GetSimInterests( ) const
{
	return m_SimInterests;
}

const rString& Subscriber::GetNameAsSubscriber() const
{
	return m_Name;
}

void Subscriber::AddUserMessage( const Message* message )
{
	m_UserMailbox.push_back( message );
}

void Subscriber::AddSimMessage( const Message* message )
{
	m_SimMailbox.push_back( message );
}

void Subscriber::ClearUserMailbox( )
{
	m_UserMailbox.clear( );
}

void Subscriber::ClearSimMailbox( )
{
	m_SimMailbox.clear( );
}