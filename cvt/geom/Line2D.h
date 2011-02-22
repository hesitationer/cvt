#ifndef CVT_LINE2D_H
#define CVT_LINE2D_H

#include <cvt/math/Vector.h>

namespace cvt {
	template<typename T>
	class Line2D {
		public:
			Line2D();
			Line2D( const Vector2<T>& p1, const Vector2<T>& p2 );
			Line2D( T x1, T y1, T x2, T y2 );
			Line2D( const Line2D<T>& line );

			void set( const Vector2<T>& p1, const Vector2<T>& p2 );
			void set( T x1, T y1, T x2, T y2 );
			void setOrthogonal( const Line2D<T>& line, const Vector2<T>& pt );

			T operator[]( int index ) const;
			Vector2<T> normal() const;
			bool intersect( const Line2D<T>& line, Vector2<T>& pt ) const;
			T distance( const Vector2<T>& pt ) const;

		public:
			Vector3<T> _line;
	};

	template<typename T>
	inline Line2D<T>::Line2D()
	{
	}

	template<typename T>
	inline Line2D<T>::Line2D( const Vector2<T>& p1, const Vector2<T>& p2 )
	{
		_line.set( p1.x, p1.y, 1 );
		Vector3<T> tmp( p2.x, p2.y, 1 );
		_line = _line.cross( tmp );
		Vector2<T> tmp2( _line.x, _line.y );
		_line /= tmp2.length();
	}

	template<typename T>
	inline Line2D<T>::Line2D( T x1, T y1, T x2, T y2 )
	{
		_line.set( x1, y1, 1 );
		Vector3<T> tmp( x2, y2, 1 );
		_line = _line.cross( tmp );
		Vector2<T> tmp2( _line.x, _line.y );
		_line /= tmp2.length();
	}


	template<typename T>
	inline Line2D<T>::Line2D( const Line2D<T>& line ) : _line( line._line )
	{
	}

	template<typename T>
	inline T Line2D<T>::operator[]( int index ) const
	{
	    return _line[ index ];
	}

	template<typename T>
	inline Vector2<T> Line2D<T>::normal() const
	{
		return Vector2<T>( _line[ 0 ], _line[ 1 ] );
	}

	template<typename T>
	inline void Line2D<T>::set( const Vector2<T>& p1, const Vector2<T>& p2 )
	{
		_line.set( p1.x, p1.y, 1 );
		Vector3<T> tmp( p2.x, p2.y, 1 );
		_line = _line.cross( tmp );
		Vector2<T> tmp2( _line.x, _line.y );
		_line /= tmp2.length();
	}

	template<typename T>
	inline void Line2D<T>::set( T x1, T y1, T x2, T y2 )
	{
		_line.set( x1, y1, 1 );
		Vector3<T> tmp( x2, y2, 1 );
		_line = _line.cross( tmp );
		Vector2<T> tmp2( _line.x, _line.y );
		_line /= tmp2.length();
	}

	template<typename T>
	inline void Line2D<T>::setOrthogonal( const Line2D<T>& line, const Vector2<T>& pt )
	{
		_line[ 0 ] = - line._line[ 1 ];
		_line[ 1 ] =   line._line[ 0 ];
		_line[ 2 ] = - ( _line[ 0 ] * pt.x + _line[ 1 ] * pt.y );
	}

	template<typename T>
	bool Line2D<T>::intersect( const Line2D<T>& line, Vector2<T>& pt ) const
	{
		Vector3<T> tmp = _line.cross( line._line );
		if( !tmp.z )
			return false;
		pt.set( tmp.x / tmp.z, tmp.y / tmp.z );
		return true;
	}


	template<>
	bool Line2D<float>::intersect( const Line2D<float>& line, Vector2<float>& pt ) const
	{
		Vector3<float> tmp = _line.cross( line._line );
		if( Math::abs( tmp.z ) < Math::EPSILONF )
			return false;
		pt.set( tmp.x / tmp.z, tmp.y / tmp.z );
		return true;
	}

	template<>
	bool Line2D<double>::intersect( const Line2D<double>& line, Vector2<double>& pt ) const
	{
		Vector3<double> tmp = _line.cross( line._line );
		if( Math::abs( tmp.z ) < Math::EPSILOND )
			return false;
		pt.set( tmp.x / tmp.z, tmp.y / tmp.z );
		return true;
	}

	template<typename T>
	T Line2D<T>::distance( const Vector2<T>& pt ) const
	{
		Vector3<T> tmp( pt.x, pt.y, 1 );
		return _line * tmp;
	}


	template<typename T>
	inline std::ostream& operator<<( std::ostream& out, const Line2D<T>& l )
	{
		out << l._line[ 0 ] << std::endl;
		out << l._line[ 1 ] << std::endl;
		out << l._line[ 2 ];
		return out;
	}

	typedef Line2D<float> Line2Df;
	typedef Line2D<double> Line2Dd;
}

#endif