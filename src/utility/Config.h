/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <memory/Alloc.h>
#include "UtilityLibraryDefine.h"

class Config
{
public:
	UTILITY_API enum class Type		{ String, Int, Double, Bool, Array, Map };
	UTILITY_API enum class Stage	{ Key, Value };
	// TODO maybe move to other file?
	UTILITY_API struct ConfigEntry
	{
		Type Type = Type::Int;
		rString Comment = "";

		ConfigEntry() {}
		union ValueHolder
		{
			int 	IntVal;
			double 	DoubleVal;
			bool 	BoolVal;
			// Stupid vc++ compiler can only handle chars, not strings, in unions
			// THIS IS DEPRECATED FFS
			char* 	StringVal;
			rVector<ConfigEntry*>* Array;
			rMap<rString, ConfigEntry*>* Map;
		} Value;

		~ConfigEntry()
		{
			if ( Type == Type::String )
			{
				pDeleteArray( Value.StringVal );
			}
			else if ( Type == Type::Array )
			{
				for ( rVector<ConfigEntry*>::iterator it = Value.Array->begin();
					  it != Value.Array->end(); ++it )
				{
					pDelete( *it );
				}
				pDelete( Value.Array );
			}
			else if ( Type == Type::Map )
			{
				for ( rMap<rString, ConfigEntry*>::iterator it = Value.Map->begin();
					  it != Value.Map->end(); ++it )
				{
					if ( it->second != nullptr )
					{
						pDelete( it->second );
						it->second = nullptr;
					}
				}
				pDelete( Value.Map );
			}
		}
	};

	UTILITY_API Config();
	UTILITY_API ~Config();

	UTILITY_API virtual bool 	ReadFile	( const rString& path );
	UTILITY_API virtual bool 	SaveFile	( const rString& path );

	UTILITY_API rString 	GetString	( const rString& key, const rString& defaultValue 	= "",	const rString& comment = "" );
	UTILITY_API double 		GetDouble	( const rString& key, double 	defaultValue 		= 0.0, 	const rString& comment = "" );
	UTILITY_API float 		GetFloat	( const rString& key, float 	defaultValue 		= 0.0, 	const rString& comment = "" );
	UTILITY_API int 		GetInt		( const rString& key, int 		defaultValue		= 0, 	const rString& comment = "" );
	UTILITY_API bool 		GetBool		( const rString& key, bool 		defaultValue 		= false,const rString& comment = "" );

	UTILITY_API rMap<rString, ConfigEntry*>* 	GetScopeMap	( const rString& scopes ) const;
	UTILITY_API rVector<ConfigEntry*>* 			GetArray 	( const rString& key );

	UTILITY_API void 		SetString 	( const rString& key, const rString& value );
	UTILITY_API void 		SetDouble 	( const rString& key, double value );
	UTILITY_API void 		SetFloat 	( const rString& key, float value );
	UTILITY_API void 		SetInt	 	( const rString& key, int value );
	UTILITY_API void 		SetBool 	( const rString& key, bool value );

	UTILITY_API bool		IsDirty		( ) const;

private:
	bool SaveMap	( std::ostream& saveStream, const rMap<rString, ConfigEntry*>* mapToSave, unsigned int indention = 0 );
	void AppendEntry( std::ostream& saveStream, const rString& key, const ConfigEntry& entry, unsigned int indention = 0 );
	bool PutEntry	( const rString& key, ConfigEntry* entry );
	rMap<rString, ConfigEntry*>* CreateNeededMaps( const rString& keyMapPart );

	bool Parse			( const rString& strToParse, rMap<rString, ConfigEntry*>* mapToAddTo );
	bool ParseArray		( const rString& arrayStr, rVector<ConfigEntry*>* vec	);

	ConfigEntry* 	AssertFind ( const rString& key ) const;

	rMap<rString, ConfigEntry*> m_configs;
	bool m_Dirty = false;
};
