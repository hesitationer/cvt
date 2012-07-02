/*
			CVT - Computer Vision Tools Library

 	 Copyright (c) 2012, Philipp Heise, Sebastian Klose

 	THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 	PARTICULAR PURPOSE.
 */
#ifndef CVT_ELLIPSE_H
#define CVT_ELLIPSE_H

#include <cvt/math/Vector.h>

namespace cvt {
	template<typename T>
	class Ellipse {
		public:
			Ellipse();
			Ellipse( const Vector2<T>& center, T semimajor, T semiminor, T orientation );
			Ellipse( const Ellipse<T>& ellipse );
			~Ellipse();

			const Ellipse<T>&	operator=( const Ellipse& elipse );

			const Vector2<T>&	center() const;
				  Vector2<T>&	center();

			T					semiMajor() const;
			T&					semiMajor();

			T					semiMinor() const;
			T&					semiMinor();

			T					orientation() const;
			T&					orientation();

			T					area() const;

		private:
			Vector2<T>	_center;
			T			_smajor;
			T			_sminor;
			T			_orientation;
	};

	typedef Ellipse<float> Ellipsef;
	typedef Ellipse<double> Ellipsed;

	template<typename T>
	inline Ellipse<T>::Ellipse() :
		_center( 0, 0 ),
		_smajor( 0 ),
		_sminor( 0 ),
		_orientation( 0 )
	{
	}

	template<typename T>
	inline Ellipse<T>::Ellipse( const Vector2<T>& center, T semimajor, T semiminor, T orientation ) :
		_center( center ),
		_smajor( semimajor ),
		_sminor( semiminor ),
		_orientation( orientation )
	{
	}


	template<typename T>
	inline Ellipse<T>::Ellipse( const Ellipse<T>& ellipse )	:
		_center( ellipse._center ),
		_smajor( ellipse._smajor ),
		_sminor( ellipse._sminor ),
		_orientation( ellipse._orientation )
	{
	}

	template<typename T>
	inline Ellipse<T>::~Ellipse()
	{
	}


	template<typename T>
	const Ellipse<T>& Ellipse<T>::operator=( const Ellipse& other )
	{
		_center = other._center;
		_smajor = other._smajor;
		_sminor = other._sminor;
		_orientation = other._orientation;
		return *this;
	}

	template<typename T>
	inline const Vector2<T>& Ellipse<T>::center() const
	{
		return _center;
	}

	template<typename T>
	inline Vector2<T>&	Ellipse<T>::center()
	{
		return _center;
	}

	template<typename T>
	inline T Ellipse<T>::semiMajor() const
	{
		return _smajor;
	}

	template<typename T>
	inline T& Ellipse<T>::semiMajor()
	{
		return _smajor;
	}

	template<typename T>
	inline T Ellipse<T>::semiMinor() const
	{
		return _sminor;
	}

	template<typename T>
	inline T& Ellipse<T>::semiMinor()
	{
		return _sminor;
	}

	template<typename T>
	inline T Ellipse<T>::orientation() const
	{
		return _orientation;
	}

	template<typename T>
	inline T& Ellipse<T>::orientation()
	{
		return _orientation;
	}

	template<typename T>
	inline T Ellipse<T>::area() const
	{
		return Math::PI * _smajor * _sminor;
	}

	template<typename T>
	inline std::ostream& operator<<( std::ostream& out, const Ellipse<T>& e )
	{
		out << e.center() << " " << e.semiMajor() << " , " << e.semiMinor() << " " << Math::rad2Deg( e.orientation() ) << std::endl;
		return out;
	}
}

#endif