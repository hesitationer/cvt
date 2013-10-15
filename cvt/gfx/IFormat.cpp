#include <cvt/gfx/IFormat.h>
#include <stdint.h>

namespace cvt {

#define FORMATDESC( c, t, id, type ) IFormat( c, sizeof( t ), c * sizeof( t ), id, type )

	const IFormat IFormat::GRAY_UINT8			= FORMATDESC( 1, uint8_t	, IFORMAT_GRAY_UINT8		, IFORMAT_TYPE_UINT8 );
	const IFormat IFormat::GRAY_UINT16			= FORMATDESC( 1, uint16_t	, IFORMAT_GRAY_UINT16		, IFORMAT_TYPE_UINT16 );
	const IFormat IFormat::GRAY_INT16			= FORMATDESC( 1, int16_t	, IFORMAT_GRAY_INT16		, IFORMAT_TYPE_INT16 );
	const IFormat IFormat::GRAY_FLOAT			= FORMATDESC( 1, float		, IFORMAT_GRAY_FLOAT		, IFORMAT_TYPE_FLOAT );
	const IFormat IFormat::GRAYALPHA_UINT8		= FORMATDESC( 2, uint8_t	, IFORMAT_GRAYALPHA_UINT8	, IFORMAT_TYPE_UINT8 );
	const IFormat IFormat::GRAYALPHA_UINT16		= FORMATDESC( 2, uint16_t	, IFORMAT_GRAYALPHA_UINT16	, IFORMAT_TYPE_UINT16 );
	const IFormat IFormat::GRAYALPHA_INT16		= FORMATDESC( 2, int16_t	, IFORMAT_GRAYALPHA_INT16	, IFORMAT_TYPE_INT16 );
	const IFormat IFormat::GRAYALPHA_FLOAT		= FORMATDESC( 2, float		, IFORMAT_GRAYALPHA_FLOAT	, IFORMAT_TYPE_FLOAT );
	const IFormat IFormat::RGBA_UINT8			= FORMATDESC( 4, uint8_t	, IFORMAT_RGBA_UINT8		, IFORMAT_TYPE_UINT8 );
	const IFormat IFormat::RGBA_UINT16			= FORMATDESC( 4, uint16_t	, IFORMAT_RGBA_UINT16		, IFORMAT_TYPE_UINT16 );
	const IFormat IFormat::RGBA_INT16			= FORMATDESC( 4, int16_t	, IFORMAT_RGBA_INT16		, IFORMAT_TYPE_INT16 );
	const IFormat IFormat::RGBA_FLOAT			= FORMATDESC( 4, float		, IFORMAT_RGBA_FLOAT		, IFORMAT_TYPE_FLOAT );
	const IFormat IFormat::BGRA_UINT8			= FORMATDESC( 4, uint8_t	, IFORMAT_BGRA_UINT8		, IFORMAT_TYPE_UINT8 );
	const IFormat IFormat::BGRA_UINT16			= FORMATDESC( 4, uint16_t	, IFORMAT_BGRA_UINT16		, IFORMAT_TYPE_UINT16 );
	const IFormat IFormat::BGRA_INT16			= FORMATDESC( 4, int16_t	, IFORMAT_BGRA_INT16		, IFORMAT_TYPE_INT16 );
	const IFormat IFormat::BGRA_FLOAT			= FORMATDESC( 4, float		, IFORMAT_BGRA_FLOAT		, IFORMAT_TYPE_FLOAT );
	const IFormat IFormat::BAYER_RGGB_UINT8		= FORMATDESC( 1, uint8_t	, IFORMAT_BAYER_RGGB_UINT8  , IFORMAT_TYPE_UINT8 );
    const IFormat IFormat::BAYER_GRBG_UINT8		= FORMATDESC( 1, uint8_t	, IFORMAT_BAYER_GRBG_UINT8  , IFORMAT_TYPE_UINT8 );
    const IFormat IFormat::BAYER_GBRG_UINT8		= FORMATDESC( 1, uint8_t	, IFORMAT_BAYER_GBRG_UINT8  , IFORMAT_TYPE_UINT8 );
	const IFormat IFormat::YUYV_UINT8			= FORMATDESC( 2, uint8_t	, IFORMAT_YUYV_UINT8		, IFORMAT_TYPE_UINT8 );
	const IFormat IFormat::UYVY_UINT8			= FORMATDESC( 2, uint8_t	, IFORMAT_UYVY_UINT8		, IFORMAT_TYPE_UINT8 );

#undef FORMATDESC

	IFormat::IFormat( const IFormat & f) :
		channels( f.channels ), bpc( f.bpc ),
		bpp( f.bpp ), formatID( f.formatID ), type( f.type )
	{}


	IFormat::IFormat( size_t c, size_t bpc, size_t bpp, IFormatID formatID, IFormatType type ) :
		channels( c ), bpc( bpc ), bpp( bpp ), formatID( formatID ), type( type )
	{}

	std::ostream& operator<<(std::ostream &out, const IFormat &f)
	{
		static const char * _iformatstring[] = {
			"GRAY_UINT8",
			"GRAY_UINT16",
			"GRAY_INT16",
			"GRAY_FLOAT",
			"GRAYALPHA_UINT8",
			"GRAYALPHA_UINT16",
			"GRAYALPHA_INT16",
			"GRAYALPHA_FLOAT",
			"RGBA_UINT8",
			"RGBA_UINT16",
			"RGBA_INT16",
			"RGBA_FLOAT",
			"BGRA_UINT8",
			"BGRA_UINT16",
			"BGRA_INT16",
			"BGRA_FLOAT",
			"BAYER_RGGB_UINT8",
            "BAYER_GRBG_UINT8",
            "BAYER_GBRG_UINT8",
			"YUYV_UINT8",
			"UYVY_UINT8"
		};

		out << "Format: " << _iformatstring[ f.formatID - 1 ];

		return out;
	}

}

