#include <cvt/vision/ORB2.h>

#include <cvt/vision/FAST.h>

#include <algorithm>

namespace cvt {
	struct FeatureInserter {
		FeatureInserter( std::vector<ORB2Feature>& container, int octave ) : _container( container ), _octave( octave )
		{
		}

		inline void operator()( float x, float y )
		{
			_container.push_back( ORB2Feature( x, y, 0.0f, _octave ) );
		}

		std::vector<ORB2Feature>& _container;
		int _octave;
	};

	ORB2::ORB2( const Image& img, size_t octaves, float scalefactor, uint8_t cornerThreshold, size_t numFeatures , bool nms ) :
		_currentOctave( 0 ),
		_threshold( cornerThreshold ),
		_numFeatures( numFeatures ),
        _nms( nms )
	{
		float scale = 1.0f;
		IScaleFilterBilinear scaleFilter;
        //IScaleFilterGauss scaleFilter;

		_scaleFactors = new float[ octaves ];
		_iimages = new IntegralImage[ octaves ];

		_features.reserve( 512 );
		_scaleFactors[ 0 ] = scale;

		detect( img, 0 );
		for( _currentOctave = 1; _currentOctave < octaves; _currentOctave++ ) {
			Image pyrimg;
			scale *=  scalefactor;
			img.scale( pyrimg, ( size_t )( img.width() * scale ), ( size_t )( img.height() * scale ), scaleFilter );
			_scaleFactors[ _currentOctave ] = scale;
			detect( pyrimg, _currentOctave );
		}
		if( _numFeatures )
			selectBestFeatures( _numFeatures );
		
		extract( octaves );
	}

	ORB2::~ORB2()
	{
		delete[] _scaleFactors;
		delete[] _iimages;
	}

	void ORB2::detect( const Image& img, size_t octave )
	{
		// detect the features for this level
		std::vector<ORB2Feature> octaveFeatures;
		FeatureInserter ftins( octaveFeatures, octave );
		FAST::detect9( img, _threshold, ftins, _border );

		size_t stride;
		const uint8_t * ptr = img.map( &stride );

		SIMD * simd = SIMD::instance();

		ContainerType::iterator it = octaveFeatures.begin();
		ContainerType::iterator itEnd = octaveFeatures.end();

		float xx, xy, yy, mx, my, n;
		Matrix2f cov, u, d, vt;

		while( it != itEnd ){
			const uint8_t * p = ptr + ( int )it->pt.y * stride + ( int )it->pt.x;
			it->score = simd->harrisResponseCircular1u8( xx, xy, yy, mx, my, p , stride, 0.04f );
			it->angle = Math::atan2( my, mx );
			if( it->angle < 0 )
				it->angle += Math::TWO_PI;
			//it->angle = Math::TWO_PI - it->angle + Math::HALF_PI;

			while( it->angle > Math::TWO_PI )
				it->angle -= Math::TWO_PI;

			cov = Matrix2f( xx, xy, xy, yy );
			cov.svd( u, d, vt );
			n = Math::sqrt( Math::sqr( d[ 0 ][ 0 ] ) + Math::sqr( d[ 1 ][ 1 ] ) );
			it->sx =  ( d[ 0 ][ 0 ] / n );
			it->sy =  ( d[ 1 ][ 1 ] / n );
			
			it->sx =  1.0f;
			it->sy =  1.0f;

			// find out the type of the feature:
			it->brighter = isBrighterFeature( p, stride );
			++it;
		}

		img.unmap( ptr );

		_iimages[ octave ].update( img );
		if( !octaveFeatures.empty() ){
            if( _nms )
                nonmaxSuppression( octaveFeatures );
            else
                _features.insert( _features.end(), octaveFeatures.begin(), octaveFeatures.end() );
        }
	}

	void ORB2::nonmaxSuppression( const std::vector<ORB2Feature> & features )
	{
		int numCorners = (int)features.size();
		int endRow = (int)features.back().pt.y;


		int firstFeatureIdxInRow[ endRow + 1 ];
		memset( firstFeatureIdxInRow, -1, (endRow + 1) * sizeof(int) );

		// initialize the indizes
		{
			int prev_row = -1;
			for( int i = 0; i < numCorners; i++ ){
				int currRow = ( int )features[ i ].pt.y;
				if(  currRow != prev_row ) {
					firstFeatureIdxInRow[ currRow ] = i;
					prev_row = currRow;
				}
			}
		}

		int idx, row, col;
		for( int i = 0; i < numCorners; i++ ) {
			float score = features[ i ].score;
			col = ( int )features[ i ].pt.x;
			row = ( int )features[ i ].pt.y;

			/* Check left */
			if( i > 0 ){
				idx = i-1;
				if( ( int )features[ idx ].pt.x == ( col - 1 ) && (int)features[ idx ].pt.y == row ){
					if( score <= features[ idx ].score  ){
						// left has better or same score:
						continue;
					}
				}
			}

			if( i < numCorners-1 ){
				idx = i+1;
				if( ( int )features[ idx ].pt.x == ( col + 1 ) && (int)features[ idx ].pt.y == row ){
					if( score < features[ idx ].score  ){
						// right has better score:
						continue;
					}
				}
			}

			bool thereIsABetterPoint = false;
			/* Check above (if there is a valid row above) */
			int pointIdxAbove;
			if( row > 0 && ( pointIdxAbove = firstFeatureIdxInRow[ row - 1 ] ) != -1 ){
				/* Make sure that current point_above is one row above. */
				while( (int)features[ pointIdxAbove ].pt.y == row - 1 &&
					  (int)features[ pointIdxAbove ].pt.x < col - 1 )
					pointIdxAbove++;

				while( (int)features[ pointIdxAbove ].pt.y == row - 1 &&
					  (int)features[ pointIdxAbove ].pt.x < col + 1 ){
					// check the three pixels above
					if( score <= features[ pointIdxAbove ].score ){
						thereIsABetterPoint = true;
						break;
					}
					pointIdxAbove++;
				}

				if( thereIsABetterPoint )
					continue;
			}

			if( row < endRow ){
				/* Make sure that current point_above is one row above. */
				int pointIdx = firstFeatureIdxInRow[ row + 1 ];

				if( pointIdx == -1 )
					continue;

				while( pointIdx < numCorners && (int)features[ pointIdx ].pt.x < col - 1 )
					pointIdx++;

				while( pointIdx < numCorners && (int)features[ pointIdx ].pt.x < col + 1 ){
					// check the three pixels above
					if( score < features[ pointIdx ].score ){
						thereIsABetterPoint = true;
						break;
					}
					pointIdx++;
				}

				if( thereIsABetterPoint )
					continue;
			}

			_features.push_back( features[ i ] );
		}
	}

	void ORB2::extract( size_t octaves )
	{
		const float* iimgptr[ octaves ];
		size_t strides[ octaves ];

		for( size_t i = 0; i < octaves; i++ ){
			iimgptr[ i ] = _iimages[ i ].sumImage().map<float>( &strides[ i ] );
		}

		for( size_t i = 0, iend = _features.size(); i < iend; i++ ){
			size_t octave = _features[ i ].octave;
			//centroidAngle( _features[ i ], iimgptr[ octave ], strides[ octave ] );
			descriptor( _features[ i ], iimgptr[ octave ], strides[ octave ] );

			std::cout << "Feature scales: " << _features[ i ].sx << ", " << _features[ i ].sy << std::endl;

			if( _features[ i ].brighter ){
				_brighterFeatures.push_back( &_features[ i ] );
			} else {
				_darkerFeatures.push_back( &_features[ i ] );
			}
		}

		for( size_t i = 0; i < octaves; i++ ){
			_iimages[ i ].sumImage().unmap<float>( iimgptr[ i ] );
		}
	}

	void ORB2::centroidAngle( ORB2Feature& feature, const float* iimgptr, size_t widthstep )
	{
		float mx = 0;
		float my = 0;

		int cury = ( int ) feature.pt.y - 15;
		int curx = ( int ) feature.pt.x;

		for( int i = 0; i < 15; i++ ) {
			mx +=( ( float ) i - 15.0f ) * ( IntegralImage::area( iimgptr, curx - _circularoffset[ i ], cury + i, 2 * _circularoffset[ i ] + 1, 1, widthstep )
											- IntegralImage::area( iimgptr, curx - _circularoffset[ i ], cury + 30 - i, 2 * _circularoffset[ i ] + 1, 1, widthstep ) );
		}

		cury = ( int ) feature.pt.y;
		curx = ( int ) feature.pt.x - 15;
		for( int i = 0; i < 15; i++ ) {
			my += ( ( float ) i - 15.0f ) * ( IntegralImage::area( iimgptr, curx + i, cury - _circularoffset[ i ], 1, 2 * _circularoffset[ i ] + 1, widthstep )
											 - IntegralImage::area( iimgptr, curx + 30 - i, cury - _circularoffset[ i ], 1, 2 * _circularoffset[ i ] + 1, widthstep ) );
		}

		feature.angle = Math::atan2( my, mx );


		if( feature.angle < 0 )
			feature.angle += Math::TWO_PI;
		feature.angle = Math::TWO_PI - feature.angle + Math::HALF_PI;

        while( feature.angle > Math::TWO_PI )
            feature.angle -= Math::TWO_PI;
	}

	void ORB2::descriptor( ORB2Feature& feature, const float* iimgptr, size_t widthstep )
	{
		size_t index = ( size_t ) ( feature.angle * 30.0f / Math::TWO_PI );
		if( index >= 30 )
			index = 0;
		int x = ( int ) feature.pt.x;
		int y = ( int ) feature.pt.y;


#define ORB2TEST( n ) ( IntegralImage::area( iimgptr, x + (int)( feature.sx * _patterns[ index ][ ( n ) * 2 ][ 0 ] ) - 2,\
													  y + (int)( feature.sy * _patterns[ index ][ ( n ) * 2 ][ 1 ] )- 2, 5, 5, widthstep ) < \
					    IntegralImage::area( iimgptr, x + (int)( feature.sx * _patterns[ index ][ ( n ) * 2 + 1 ][ 0 ] ) - 2,\
													  y + (int)( feature.sy * _patterns[ index ][ ( n ) * 2 + 1 ][ 1 ] ) - 2, 5, 5, widthstep ) )

		for( int i = 0; i < 32; i++ ) {
			feature.desc[ i ] = 0;
			for( int k = 0; k < 8; k++ ) {
				feature.desc[ i ] |= ( ORB2TEST( i * 8 + k ) ) << k;
			}
		}
		feature.pt /= _scaleFactors[ feature.octave ];
	}

	static bool compareOrbFeature( const ORB2Feature & a, const ORB2Feature & b )
	{
		return a.score > b.score;
	}

	void ORB2::selectBestFeatures( size_t num )
	{
		std::sort( _features.begin(), _features.end(), compareOrbFeature );

		if( _features.size() > num )
			_features.erase( _features.begin() + num, _features.end() );
	}

	bool ORB2::isBrighterFeature( const uint8_t * ptr, size_t stride ) const 
	{
		size_t numB = 0; 
		size_t numD = 0;

		int highT = *ptr + _threshold;
		int lowT  = *ptr - _threshold;

		const uint8_t * p = ptr - 3 * stride - 1;

		numB += ( p[ 0 ] > highT ) ? 1 : 0;
		numB += ( p[ 1 ] > highT ) ? 1 : 0;
		numB += ( p[ 2 ] > highT ) ? 1 : 0;
		numD += ( p[ 0 ] < lowT )  ? 1 : 0;
		numD += ( p[ 1 ] < lowT )  ? 1 : 0;
		numD += ( p[ 2 ] < lowT )  ? 1 : 0;

		p = ptr - 2 * stride - 2;
		numB += ( p[ 0 ] > highT ) ? 1 : 0;
		numD += ( p[ 0 ] < lowT )  ? 1 : 0;
		numB += ( p[ 4 ] > highT ) ? 1 : 0;
		numD += ( p[ 4 ] < lowT )  ? 1 : 0;

		p = ptr - stride - 3;
		numB += ( p[ 0 ] > highT ) ? 1 : 0;
		numD += ( p[ 0 ] < lowT )  ? 1 : 0;
		numB += ( p[ 6 ] > highT ) ? 1 : 0;
		numD += ( p[ 6 ] < lowT )  ? 1 : 0;
		p += stride;
		numB += ( p[ 0 ] > highT ) ? 1 : 0;
		numD += ( p[ 0 ] < lowT )  ? 1 : 0;
		numB += ( p[ 6 ] > highT ) ? 1 : 0;
		numD += ( p[ 6 ] < lowT )  ? 1 : 0;
		p += stride;
		numB += ( p[ 0 ] > highT ) ? 1 : 0;
		numD += ( p[ 0 ] < lowT )  ? 1 : 0;
		numB += ( p[ 6 ] > highT ) ? 1 : 0;
		numD += ( p[ 6 ] < lowT )  ? 1 : 0;

		p = p + stride + 1;
		numB += ( p[ 0 ] > highT ) ? 1 : 0;
		numD += ( p[ 0 ] < lowT )  ? 1 : 0;
		numB += ( p[ 4 ] > highT ) ? 1 : 0;
		numD += ( p[ 4 ] < lowT )  ? 1 : 0;
		
		p = p + stride + 1;
		numB += ( p[ 0 ] > highT ) ? 1 : 0;
		numB += ( p[ 1 ] > highT ) ? 1 : 0;
		numB += ( p[ 2 ] > highT ) ? 1 : 0;
		numD += ( p[ 0 ] < lowT )  ? 1 : 0;
		numD += ( p[ 1 ] < lowT )  ? 1 : 0;
		numD += ( p[ 2 ] < lowT )  ? 1 : 0;

		return numB > numD;
	}

	const int ORB2::_circularoffset[ 31 ] = {  3,  6,  8,  9, 10, 11, 12, 13, 13, 14, 14, 14, 15, 15, 15, 15,
		15, 15, 15, 14, 14, 14, 13, 13, 12, 11, 10,  9,  8,  6,  3 };
#include "ORB2Patterns.h"
}