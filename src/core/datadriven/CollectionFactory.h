/**************************************************
2015 Johan Melin
***************************************************/

#pragma once

#include "BaseCollection.h"
#include "DenseComponentCollection.h"

// Get a list of all given components collections. Also assigns unique ID to the collections.
template <typename... Components>
static rVector<BaseCollection*>* GetCollections()
{
	rVector<BaseCollection*>* collections = pNew (rVector<BaseCollection*>, (sizeof...(Components)));
	*collections = { &DenseComponentCollection<Components>::GetInstance()... };
	for (short i = 0; i < static_cast<short>(collections->size()); ++i )
	{
		collections->at( i )->SetComponentTypeIndex( i );
	}
	
	return collections;
}