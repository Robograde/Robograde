/**************************************************
2015 Ola Enberg
***************************************************/

#include "Camera.h"

#include <iostream>
#include <iomanip>

using std::cout;
using std::cin;
using std::endl;
using glm::vec3;

#define TAU								6.28318530718f
#define CAMERA_TEST_OUTPUT_PRECISION	3

void SaveCameraInfo( const Camera& camera, vec3& prevPosition, vec3& prevForward, vec3& prevUp, vec3& prevRight )
{
	prevPosition	= camera.GetPosition();
	prevForward		= camera.GetForward();
	prevUp			= camera.GetUp();
	prevRight		= camera.GetRight();
}

void PrintState( const Camera& camera, vec3& prevPosition, vec3& prevForward, vec3& prevUp, vec3& prevRight )
{
	cout	<< std::fixed << std::setprecision( CAMERA_TEST_OUTPUT_PRECISION )
			<< "Pos.x: " << camera.GetPosition().x		<< "\tPos.y: " << camera.GetPosition().y	<< "\tPos.z: " << camera.GetPosition().z
			<< (camera.GetPosition() != prevPosition ? "\tChanged" : "") << endl;

	cout	<< std::fixed << std::setprecision( CAMERA_TEST_OUTPUT_PRECISION )
			<< "For.x: " << camera.GetForward().x		<< "\tFor.y: " << camera.GetForward().y		<< "\tFor.z: " << camera.GetForward().z
			<< (camera.GetForward() != prevForward ? "\tChanged" : "") << endl;

	cout	<< std::fixed << std::setprecision( CAMERA_TEST_OUTPUT_PRECISION )
			<< "Up.x : " << camera.GetUp().x			<< "\tUp.y : " << camera.GetUp().y			<< "\tUp.z : " << camera.GetUp().z
			<< (camera.GetUp() != prevUp ? "\tChanged" : "") << endl;

	cout	<< std::fixed << std::setprecision( CAMERA_TEST_OUTPUT_PRECISION )
			<< "Rig.x: " << camera.GetRight().x		<< "\tRig.y: " << camera.GetRight().y		<< "\tRig.z: " << camera.GetRight().z
			<< (camera.GetRight() != prevRight ? "\tChanged" : "") << endl;

	SaveCameraInfo( camera, prevPosition, prevForward, prevUp, prevRight );

	// Pauses the test so user can confirm the data.
	getchar();
}

bool Camera::UnitTest()
{
	Camera	camera1;
	vec3	prevPosition;
	vec3	prevForward;
	vec3	prevUp;
	vec3	prevRight;
	SaveCameraInfo( camera1, prevPosition, prevForward, prevUp, prevRight );

	cout << endl << endl << "CAMERA UNIT TEST STARTING (Press enter to procceed after each stage):" << endl << endl;

	cout << "Initial State:" << endl;
	PrintState( camera1, prevPosition, prevForward, prevUp, prevRight );

	cout << "Walking Forward 1 unit:" << endl;
	camera1.MoveRelative( vec3( 0.0f, 0.0f, -1.0f ) );
	PrintState( camera1, prevPosition, prevForward, prevUp, prevRight );

	cout << "Turning Left 90 degrees:" << endl;
	camera1.YawRelative( 0.25f * TAU );
	PrintState( camera1, prevPosition, prevForward, prevUp, prevRight );

	cout << "Walking Left 4 units:" << endl;
	camera1.MoveRelative( vec3( -4.0f, 0.0f, 0.0f ) );
	PrintState( camera1, prevPosition, prevForward, prevUp, prevRight );

	cout << "Looking Down 90 degrees:" << endl;
	camera1.PitchRelative( -0.25f * TAU );
	PrintState( camera1, prevPosition, prevForward, prevUp, prevRight );

	cout << "Moving world position by ( 5, 3, -5 ):" << endl;
	camera1.MoveWorld( vec3( 5.0f, 3.0f, -5.0f ) );
	PrintState( camera1, prevPosition, prevForward, prevUp, prevRight );

	cout << "Rotating with unit quaternion (i.e. no rotation):" << endl;
	camera1.RotateWithQuaternion( glm::quat( 1.0f, 0.0f, 0.0f, 0.0f ) );
	PrintState( camera1, prevPosition, prevForward, prevUp, prevRight );

	cout << "Set camera position to ( 13, 3, 7 ):" << endl;
	camera1.SetPosition( vec3( 13.0f, 3.0f, 7.0f ) );
	PrintState( camera1, prevPosition, prevForward, prevUp, prevRight );


	// Camera number 2
	cout << "Creating a new camera:" << endl;
	Camera camera2;
	PrintState( camera2, prevPosition, prevForward, prevUp, prevRight );

	cout << "Looking up 45 degrees:" << endl;
	camera2.PitchRelative( 0.125f * TAU );
	PrintState( camera2, prevPosition, prevForward, prevUp, prevRight );

	cout << "Rotating right 90 degrees around world up vector (y-axis):" << endl;
	camera2.YawWorld( -0.25f * TAU );
	PrintState( camera2, prevPosition, prevForward, prevUp, prevRight );

	cout << "Move up 2 units relative to camera:" << endl;
	camera2.MoveRelative( vec3( 0.0f, 2.0f, 0.0f ) );
	PrintState( camera2, prevPosition, prevForward, prevUp, prevRight );

	cout << "Setting the cameras rotation to its own rotation:" << endl;
	camera2.SetOrientation( camera2.GetOrientation() );
	PrintState( camera2, prevPosition, prevForward, prevUp, prevRight );

	cout << "Setting camera rotation to unit quaternion (no rotation):" << endl;
	camera2.SetOrientation( glm::quat( 1.0f, 0.0f, 0.0f, 0.0f ) );
	PrintState( camera2, prevPosition, prevForward, prevUp, prevRight );

	cout << "Rolling camera 45 degrees clockwise:" << endl;
	camera2.RollRelative( 0.25f * TAU );
	PrintState( camera2, prevPosition, prevForward, prevUp, prevRight );
	
	return true;
}