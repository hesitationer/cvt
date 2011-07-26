
#include <cvt/gui/Application.h>
#include <cvt/vision/ORB.h>

#include "VideoInputGui.h"
#include "ORBTemplateTracker.h"

#include <cvt/io/Camera.h>

using namespace cvt;

Camera* selectCam()
{
    Camera::updateInfo();
	size_t numCams = Camera::count();

	std::cout << "Overall number of Cameras: " << numCams << std::endl;
	if( numCams == 0 ){
		std::cout << "Please connect a camera!" << std::endl;
		return 0;
	}

	for( size_t i = 0; i < numCams; i++ ){
		const CameraInfo & info = Camera::info( i );
		std::cout << "Camera " << i << ": " << info << std::endl;
	}

	size_t selection = numCams;
	if( numCams == 1 )
		selection = 0;
	else {
		std::cout << "Select camera: ";
		std::cin >> selection;
		while ( selection >= numCams ){
			std::cout << "Index out of bounds -> select camera in Range:";
			std::cin >> selection;
		}
	}

    Camera * cam = 0;

    try {
		cam = Camera::get( selection, 640, 480, 60, IFormat::UYVY_UINT8 );
	} catch( cvt::Exception e ) {
		std::cout << e.what() << std::endl;
        return 0;
	}

    return cam;
}


int main( int argc, char* argv[] )
{
    if( argc < 2 ){
        std::cout << "USAGE: " << argv[ 0 ] << " <templateimage>" << std::endl;
        return 0;
    }

    Image reference( argv[ 1 ] );
    Image ref_gray;

    reference.convert( ref_gray, IFormat::GRAY_UINT8 );

    Camera * cam = selectCam();

    if( !cam )
        return 1;

    cam->startCapture();

    VideoInputGui gui( cam, ref_gray );

    Application::run();

    cam->stopCapture();
    delete cam;

    return 0;
}