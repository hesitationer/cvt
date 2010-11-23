#ifndef CVT_CAMERAMODESET
#define CVT_CAMERAMODESET

#include <vector>

#include <cvt/io/CameraMode.h>

namespace cvt {
	class CameraModeSet
	{
		public:
			CameraModeSet();
			~CameraModeSet();

			void add( const CameraMode & mode );
			CameraModeSet filterFormat( const IFormat & format ) const;
			CameraModeSet filterSize( size_t minWidth, size_t minHeight ) const;
			CameraModeSet filterFPS( size_t minFPS ) const;
			CameraModeSet filter( const IFormat & format, size_t minWidth, size_t minHeight, size_t minFPS  ) const;
			CameraMode bestMatching( const IFormat & format, size_t width = 0, size_t height = 0, size_t fps = 0  ) const;

			size_t size() const;
			const CameraMode & mode( size_t idx ) const;

		private:
			std::vector<CameraMode> _modes;
	};

	std::ostream& operator<<( std::ostream &out, const CameraModeSet & info );
}

#endif