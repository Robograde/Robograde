/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "IntersectionTestLookupTable.h"

#include "RayOBB.h"
#include "RayPlane.h"
#include "RaySphere.h"
#include "RayHeightMap.h"
#include "PlaneOBB.h"
#include "PlaneSphere.h"
#include "FrustumOBB.h"
#include "FrustumSphere.h"

IntersectionTestLookupTable::IntersectionTestLookupTable()
{
	m_IntersectionTestFunctions[ VOLUME_TYPE_RAY		][ VOLUME_TYPE_RAY			] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_RAY		][ VOLUME_TYPE_OBB			] = &IntersectionTest::RayOBB;
	m_IntersectionTestFunctions[ VOLUME_TYPE_RAY		][ VOLUME_TYPE_PLANE		] = &IntersectionTest::RayPlane;
	m_IntersectionTestFunctions[ VOLUME_TYPE_RAY		][ VOLUME_TYPE_SPHERE		] = &IntersectionTest::RaySphere;
	m_IntersectionTestFunctions[ VOLUME_TYPE_RAY		][ VOLUME_TYPE_FRUSTUM		] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_RAY		][ VOLUME_TYPE_HEIGHT_MAP	] = &IntersectionTest::RayHeightMap;

	m_IntersectionTestFunctions[ VOLUME_TYPE_OBB		][ VOLUME_TYPE_RAY			] = &IntersectionTest::OBBRay;
	m_IntersectionTestFunctions[ VOLUME_TYPE_OBB		][ VOLUME_TYPE_OBB			] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_OBB		][ VOLUME_TYPE_PLANE		] = &IntersectionTest::OBBPlane;
	m_IntersectionTestFunctions[ VOLUME_TYPE_OBB		][ VOLUME_TYPE_SPHERE		] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_OBB		][ VOLUME_TYPE_FRUSTUM		] = &IntersectionTest::OBBFrustum;
	m_IntersectionTestFunctions[ VOLUME_TYPE_OBB		][ VOLUME_TYPE_HEIGHT_MAP	] = &NotImplemented;

	m_IntersectionTestFunctions[ VOLUME_TYPE_PLANE		][ VOLUME_TYPE_RAY			] = &IntersectionTest::PlaneRay;
	m_IntersectionTestFunctions[ VOLUME_TYPE_PLANE		][ VOLUME_TYPE_OBB			] = &IntersectionTest::PlaneOBB;
	m_IntersectionTestFunctions[ VOLUME_TYPE_PLANE		][ VOLUME_TYPE_PLANE		] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_PLANE		][ VOLUME_TYPE_SPHERE		] = &IntersectionTest::PlaneSphere;
	m_IntersectionTestFunctions[ VOLUME_TYPE_PLANE		][ VOLUME_TYPE_FRUSTUM		] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_PLANE		][ VOLUME_TYPE_HEIGHT_MAP	] = &NotImplemented;

	m_IntersectionTestFunctions[ VOLUME_TYPE_SPHERE		][ VOLUME_TYPE_RAY			] = &IntersectionTest::SphereRay;
	m_IntersectionTestFunctions[ VOLUME_TYPE_SPHERE		][ VOLUME_TYPE_OBB			] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_SPHERE		][ VOLUME_TYPE_PLANE		] = &IntersectionTest::SpherePlane;
	m_IntersectionTestFunctions[ VOLUME_TYPE_SPHERE		][ VOLUME_TYPE_SPHERE		] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_SPHERE		][ VOLUME_TYPE_FRUSTUM		] = &IntersectionTest::SphereFrustum;
	m_IntersectionTestFunctions[ VOLUME_TYPE_SPHERE		][ VOLUME_TYPE_HEIGHT_MAP	] = &NotImplemented;

	m_IntersectionTestFunctions[ VOLUME_TYPE_FRUSTUM	][ VOLUME_TYPE_RAY			] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_FRUSTUM	][ VOLUME_TYPE_OBB			] = &IntersectionTest::FrustumOBB;
	m_IntersectionTestFunctions[ VOLUME_TYPE_FRUSTUM	][ VOLUME_TYPE_PLANE		] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_FRUSTUM	][ VOLUME_TYPE_SPHERE		] = &IntersectionTest::FrustumSphere;
	m_IntersectionTestFunctions[ VOLUME_TYPE_FRUSTUM	][ VOLUME_TYPE_FRUSTUM		] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_FRUSTUM	][ VOLUME_TYPE_HEIGHT_MAP	] = &NotImplemented;

	m_IntersectionTestFunctions[ VOLUME_TYPE_HEIGHT_MAP	][ VOLUME_TYPE_RAY			] = &IntersectionTest::HeightMapRay;
	m_IntersectionTestFunctions[ VOLUME_TYPE_HEIGHT_MAP	][ VOLUME_TYPE_OBB			] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_HEIGHT_MAP	][ VOLUME_TYPE_PLANE		] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_HEIGHT_MAP	][ VOLUME_TYPE_SPHERE		] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_HEIGHT_MAP	][ VOLUME_TYPE_FRUSTUM		] = &NotImplemented;
	m_IntersectionTestFunctions[ VOLUME_TYPE_HEIGHT_MAP	][ VOLUME_TYPE_HEIGHT_MAP	] = &NotImplemented;
}

IntersectionTestFunction IntersectionTestLookupTable::Fetch( VOLUME_TYPE a, VOLUME_TYPE b )
{
	return m_IntersectionTestFunctions[a][b];
}

bool IntersectionTestLookupTable::NotImplemented( const Volume* aVolume, const Volume* bVolume, glm::vec3* outIntersectionPoint )
{
	return false;
}