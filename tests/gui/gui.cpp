#include <iostream>
#include <cvt/gui/Application.h>
#include <cvt/gui/Window.h>

using namespace cvt;

#include <cvt/gui/TimeoutHandler.h>
#include <cvt/util/Time.h>


class MyTimer : public TimeoutHandler
{
	public:
	MyTimer()
	{
	}

	void onTimeout()
	{
		std::cout << "Timer: " << _last.elapsedMilliSeconds() << std::endl;
		_last.reset();
	}

	Time _last;
};

int main(int argc, char* argv[])
{
	MyTimer mytimer;
    Window w( "Test" );
	w.setSize( 640, 480 );
	w.setVisible( true );

	Application::instance()->registerTimer( 33, &mytimer );

    Application::run();
	return 0;
}