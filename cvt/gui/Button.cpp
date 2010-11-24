#include <cvt/gui/Button.h>

namespace cvt {
	Button::Button( const std::string& label ) : _label( label )
	{
	}

	Button::~Button()
	{

	}

	void Button::paintEvent( PaintEvent* event, GFX* g )
	{
		int w, h;
		g->color().set( 0.2f, 0.2f, 0.2f, 1.0f );
		size( w, h );
		g->fillRoundRect( 0, 0, w, h, 0.5f * ( float ) h );
		g->color().set( 0.8f, 0.8f, 0.8f, 1.0f );
		int tw = g->textWidth( _label.c_str() );
		g->drawText( ( ( float ) w - ( float ) tw ) * 0.5f , ( ( float ) h + 11.0f ) * 0.5f, _label.c_str() );
	}

	void Button::mousePressEvent( MousePressEvent* event )
	{
		std::cout << "PRESS: " << event->x << ", " << event->y  << std::endl;
	}

	void Button::mouseReleaseEvent( MouseReleaseEvent* event )
	{
		std::cout << "Release: " << event->x << ", " << event->y  << std::endl;
	}


}
