
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/SIMD.h>
#include <cvt/gfx/IThreshold.h>

namespace cvt
{
	template<typename DSTTYPE, typename SRCTYPE, typename THTYPE>
	static void thresholdTemplate( Image& dst, const Image& src, THTYPE t, void ( SIMD::*func )( DSTTYPE*, const SRCTYPE*, size_t, THTYPE ) const )
	{
		SIMD* simd = SIMD::instance();
		cvt::IMapScoped<DSTTYPE> mapdst( dst );
		cvt::IMapScoped<const SRCTYPE> mapsrc( src );
		size_t w = src.width();
		size_t h = src.height();

		while( h-- ) {
			( simd->*func )( mapdst.ptr(), mapsrc.ptr(), w, t );
			mapdst++;
			mapsrc++;
		}
	}

	template<typename DSTTYPE, typename SRCTYPE, typename THTYPE>
	static void thresholdAdaptiveTemplate( Image& dst, const Image& src, const Image& srcmean, THTYPE t, void ( SIMD::*func )( DSTTYPE*, const SRCTYPE*, const SRCTYPE*, size_t, THTYPE ) const )
	{
		SIMD* simd = SIMD::instance();
		cvt::IMapScoped<DSTTYPE> mapdst( dst );
		cvt::IMapScoped<const SRCTYPE> mapsrc( src );
		cvt::IMapScoped<const SRCTYPE> mapsrcmean( srcmean );
		size_t w = src.width();
		size_t h = src.height();

		while( h-- ) {
			( simd->*func )( mapdst.ptr(), mapsrc.ptr(), mapsrcmean.ptr(), w, t );
			mapdst++;
			mapsrc++;
			mapsrcmean++;
		}
	}


	void IThreshold::threshold( Image& dst, const Image& src, float t )
	{
		if( src.channels() != 1 )
			throw CVTException( "Not implemented IThreshold::threshold" );

		if( dst.width() != src.width() || dst.height() != src.height() || dst.channels() != 1 )
			dst.reallocate( src.width(), src.height(), src.format() );

		switch( src.format().formatID ) {
			case IFORMAT_GRAY_UINT8:
				{
					uint8_t tu8 = Math::round( t * 0xff );
					switch( dst.format().formatID ) {
						case IFORMAT_GRAY_UINT8: return thresholdTemplate<uint8_t, uint8_t, uint8_t>( dst, src, tu8, &SIMD::threshold1_u8_to_u8 );
						case IFORMAT_GRAY_FLOAT: return thresholdTemplate<float, uint8_t, uint8_t>( dst, src, tu8, &SIMD::threshold1_u8_to_f );
						default:				 throw CVTException( "Not implemented IThreshold::threshold" );
					}
				}
				break;
			case IFORMAT_GRAY_FLOAT:
				{
					switch( dst.format().formatID ) {
						case IFORMAT_GRAY_UINT8: return thresholdTemplate<uint8_t, float, float>( dst, src, t, &SIMD::threshold1_f_to_u8 );
						case IFORMAT_GRAY_FLOAT: return thresholdTemplate<float, float, float>( dst, src, t, &SIMD::threshold1_f_to_f );
						default:				  throw CVTException( "Not implemented IThreshold::threshold" );
					}
				}
				break;
			default:
				throw CVTException( "Not implemented IThreshold::threshold" );
		}
	}

	void IThreshold::thresholdAdaptive( Image& dst, const Image& src, const Image& boxfiltered, float t )
	{
		if( src.channels() != 1 || boxfiltered.channels() != 1 || src.format() != boxfiltered.format() ||
		    src.width() != boxfiltered.width() || src.height() != boxfiltered.height()  )
			throw CVTException( "Not implemented IThreshold::thresholdAdaptive" );

		if( dst.width() != src.width() || dst.height() != src.height() || dst.channels() != 1 )
			dst.reallocate( src.width(), src.height(), src.format() );

		switch( src.format().formatID ) {
			case IFORMAT_GRAY_FLOAT:
				{
					switch( dst.format().formatID ) {
						case IFORMAT_GRAY_UINT8: return thresholdAdaptiveTemplate<uint8_t, float, float>( dst, src, boxfiltered, t, &SIMD::adaptiveThreshold1_f_to_u8 );
						case IFORMAT_GRAY_FLOAT: return thresholdAdaptiveTemplate<float, float, float>( dst, src, boxfiltered, t, &SIMD::adaptiveThreshold1_f_to_f );
						default:				  throw CVTException( "Not implemented IThreshold::threshold" );
					}
				}
				break;
			default:
				throw CVTException( "Not implemented IThreshold::threshold" );
		}
	}

}

