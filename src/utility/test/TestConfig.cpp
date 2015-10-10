/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include <gtest/gtest.h>
#include <utility/Config.h>

TEST ( Config, ReadFile )
{
	Config cfg;
	ASSERT_TRUE ( cfg.ReadFile ( "../../../cfg/test/testconfig.cfg" ) ) << "Failed to read file";
}

TEST ( Config, ReadBasic )
{
	Config cfg;
	ASSERT_TRUE ( cfg.ReadFile ( "../../../cfg/test/testconfig.cfg" ) ) << "Failed to read file";

	ASSERT_EQ ( cfg.GetDouble 	( "aDouble" ), 	2.7 		)	<< "Failed to read double";
	ASSERT_EQ ( cfg.GetInt		( "anInt" 	), 	4 			)	<< "Failed to read int";
	ASSERT_EQ ( cfg.GetString	( "aString" ), 	"a string" 	) 	<< "Failed to read string";
}

TEST ( Config, ReadScopedBasic )
{
	Config cfg;
	ASSERT_TRUE ( cfg.ReadFile ( "../../../cfg/test/testconfig.cfg" ) ) << "Failed to read file";

	ASSERT_EQ ( cfg.GetDouble 	( "aScope.scopedDouble"	), 7.5 				)<< "Failed to read scoped double";
	ASSERT_EQ ( cfg.GetInt		( "aScope.scopedInt"	), 6   				)<< "Failed to read scoped int";
	ASSERT_EQ ( cfg.GetString	( "aScope.scopedString"	), "scoped string" 	)<< "Failed to read scoped string";
}

TEST ( Config, ReadScopeScope )
{
	Config cfg;
	ASSERT_TRUE ( cfg.ReadFile ( "../../../cfg/test/testconfig.cfg" ) ) << "Failed to read file";

	ASSERT_EQ ( cfg.GetInt( "aScope.scopeInScope.scopescopedInt" ), 50 ) << "Failed to read a scoped scoped int";
}

TEST ( Config, ReadPrimitiveArray )
{
	Config cfg;
	ASSERT_TRUE ( cfg.ReadFile ( "../../../cfg/test/testconfig.cfg" ) ) << "Failed to read file";
	
	ASSERT_EQ ( cfg.GetInt		( "anIntArray[0]" 	), 1 ) 	<< "Failed to read first int in array";
	ASSERT_EQ ( cfg.GetInt		( "anIntArray[1]" 	), 4 ) 	<< "Failed to read second int in array";
	ASSERT_EQ ( cfg.GetInt		( "anIntArray[2]" 	), 6 ) 	<< "Failed to read last int in array";
	ASSERT_EQ ( cfg.GetDouble	( "aDoubleArray[0]"	), 1.0 )<< "Failed to read first double in array";
	ASSERT_EQ ( cfg.GetDouble	( "aDoubleArray[1]"	), 2.0 )<< "Failed to read second double in array";
	ASSERT_EQ ( cfg.GetDouble	( "aDoubleArray[2]"	), 3.5 )<< "Failed to read last double in array";
}

TEST ( Config, ReadStringArray )
{
	Config cfg;
	ASSERT_TRUE ( cfg.ReadFile ( "../../../cfg/test/testconfig.cfg" ) ) << "Failed to read file";

	ASSERT_EQ ( cfg.GetString("aStringArray[0]"), "first" 			) << "Failed to read first string in array";
	ASSERT_EQ ( cfg.GetString("aStringArray[1]"), "second string" 	) << "Failed to read second string in array";
	ASSERT_EQ ( cfg.GetString("aStringArray[2]"), "third" 			) << "Failed to read first last in array";
}

TEST ( Config, ReadScopedPrimitiveArray)
{
	Config cfg;
	ASSERT_TRUE ( cfg.ReadFile ( "../../../cfg/test/testconfig.cfg" ) ) << "Failed to read file";

	ASSERT_EQ ( cfg.GetInt		( "aScope.scopedIntArray[0]" 	), 1 ) 	<< "Failed to read first int in scoped array";
	ASSERT_EQ ( cfg.GetInt		( "aScope.scopedIntArray[1]" 	), 2 ) 	<< "Failed to read second int in scoped array";
	ASSERT_EQ ( cfg.GetInt		( "aScope.scopedIntArray[2]" 	), 3 ) 	<< "Failed to read last int in scoped array";
	ASSERT_EQ ( cfg.GetDouble	( "aScope.scopedDoubleArray[0]"	), 1.1 )<< "Failed to read first double in scoped array";
	ASSERT_EQ ( cfg.GetDouble	( "aScope.scopedDoubleArray[1]"	), 2.2 )<< "Failed to read second double in scoped array";
	ASSERT_EQ ( cfg.GetDouble	( "aScope.scopedDoubleArray[2]"	), 3.3 )<< "Failed to read last double in scoped array";
}

TEST ( Config, ReadCombinedArray )
{
	Config cfg;
	ASSERT_TRUE ( cfg.ReadFile ( "../../../cfg/test/testconfig.cfg" ) ) << "Failed to read file";

	ASSERT_EQ ( cfg.GetString	( "aCombinedArray[0]" ), "a string" ) << "Failed to read string in combined array";
	ASSERT_EQ ( cfg.GetInt		( "aCombinedArray[1]" ), 17 		) << "Failed to read int in combined array";
	ASSERT_EQ ( cfg.GetDouble	( "aCombinedArray[2]" ), 20.7 		) << "Failed to read double in combined array";
}
