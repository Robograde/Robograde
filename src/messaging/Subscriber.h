/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include "Message.h"
#include <memory/Alloc.h>
#include "MessageLibraryDefine.h"

class Subscriber
{
public:
	MESSAGING_API const int			GetUserInterests() const;
	MESSAGING_API const int			GetSimInterests() const;
	MESSAGING_API const rString&	GetNameAsSubscriber() const;
	MESSAGING_API void				AddUserMessage( const Message* message );
	MESSAGING_API void				AddSimMessage( const Message* message );
	MESSAGING_API void				ClearUserMailbox();
	MESSAGING_API void				ClearSimMailbox();

	MESSAGING_API bool operator==( const Subscriber& rhs );

protected:
	// Only children may instance
	MESSAGING_API Subscriber( const rString& name );
	MESSAGING_API virtual ~Subscriber();

	int							m_SimInterests	= 0;
	int							m_UserInterests = 0;
	rVector<const Message*>		m_UserMailbox;
	rVector<const Message*>		m_SimMailbox;

private:
	// No copying allowed!
	Subscriber( const Subscriber& rhs );
	Subscriber& operator=( const Subscriber& rhs );

	rString m_Name;
};