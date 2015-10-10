/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"

// Conveniency access function
#define g_SSMetaDataRecorder SSMetaDataRecorder::GetInstance( )

class SSMetaDataRecorder : public Subsystem
{
public:
	static SSMetaDataRecorder& GetInstance();

	void UpdateSimLayer( const float timeStep ) override;

private:
	// No external instancing allowed
	SSMetaDataRecorder() : Subsystem( "MetaDataRecorder" ) {}
	SSMetaDataRecorder( const SSMetaDataRecorder& rhs );
	~SSMetaDataRecorder() {};
	SSMetaDataRecorder& operator=( const SSMetaDataRecorder& rhs );
};