#ifndef CVT_TIME_H
#define CVT_TIME_H

#include <unistd.h>
#include <time.h>

#ifndef _POSIX_TIMERS
#error "Posix timers not supported"
#endif
#ifndef _POSIX_MONOTONIC_CLOCK
#error "Posix monotonic clock not supported"
#endif

namespace cvt {

	class Time {
		public:
			Time();
			Time( const Time& t );
			~Time();
			void reset();
			double elapsedSeconds() const;
			double elapsedMilliSeconds() const;
			double elapsedMicroSeconds() const;
			double ms() const;
			double operator-( const Time& t ) const;
			double operator+( const Time& t ) const;
			double operator-( double ms ) const;
			double operator+( double ms ) const;


		private:
			double timespecToMS( const struct timespec& ts ) const;
			double timespecToUS( const struct timespec& ts ) const;

			struct timespec _ts;
	};

	inline Time::Time()
	{
		reset();
	}

	inline Time::Time( const Time& t )
	{
		_ts.tv_sec = t._ts.tv_sec;
		_ts.tv_nsec = t._ts.tv_nsec;
	}

	inline Time::~Time() {}

	inline void Time::reset()
	{
		clock_gettime( CLOCK_MONOTONIC, &_ts );
	}

	double Time::timespecToMS( const struct timespec& ts ) const
	{
		return ( ( double ) ts.tv_sec ) * 1000.0 + ( ( double ) ts.tv_nsec ) * 0.000001;
	}

	double Time::timespecToUS( const struct timespec& ts ) const
	{
		return ( ( double ) ts.tv_sec ) * 1000000.0 + ( ( double ) ts.tv_nsec ) * 0.001;
	}

	inline double Time::elapsedSeconds() const
	{
		struct timespec ts2;
		clock_gettime( CLOCK_MONOTONIC, &ts2 );
		return ( double ) ts2.tv_sec - ( double ) _ts.tv_sec;
	}

	inline double Time::elapsedMilliSeconds() const
	{
		struct timespec ts2;
		clock_gettime( CLOCK_MONOTONIC, &ts2 );
		return timespecToMS( ts2 ) - timespecToMS( _ts );
	}

	inline double Time::elapsedMicroSeconds() const
	{
		struct timespec ts2;
		clock_gettime( CLOCK_MONOTONIC, &ts2 );
		return timespecToUS( ts2 ) - timespecToUS( _ts );
	}

	double Time::operator-( const Time& t ) const
	{
		return timespecToMS( _ts ) - timespecToMS( t._ts );
	}

	double Time::operator+( const Time& t ) const
	{
		return timespecToMS( _ts ) + timespecToMS( t._ts );
	}

	double Time::operator-( double ms ) const
	{
		return timespecToMS( _ts ) - ms;
	}

	double Time::operator+( double ms ) const
	{
		return timespecToMS( _ts ) + ms;
	}

	double Time::ms() const
	{
		return timespecToMS( _ts );
	}
}

#endif
