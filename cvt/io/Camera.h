#ifndef CVT_CAMERA_H
#define CVT_CAMERA_H

#include <vector>

#include <cvt/io/VideoInput.h>
#include <cvt/io/CameraInfo.h>
#include <cvt/util/Exception.h>

namespace cvt {
	class Camera : public VideoInput
	{
		public:
			virtual ~Camera();
			virtual void startCapture() = 0;
			virtual void stopCapture() = 0;
			virtual const String& identifier() const = 0;

			virtual size_t frameIndex() const;
			virtual double stamp() const;

			/* number of available cameras */
			static size_t count();
			static const CameraInfo & info( size_t index );
			static void updateInfo();

			/* will create camera with index and closest possible parameters */
			static Camera* get( size_t index, size_t width = 640, size_t height = 480,
								size_t fps = 60, const IFormat & format = IFormat::BGRA_UINT8 );
			static Camera* get( size_t index, const CameraMode& mode );

		private:
			Camera( const Camera & other );
			static std::vector<CameraInfo> _camInfos;
		protected:
			Camera();
	};

	inline size_t Camera::count()
	{
		return _camInfos.size();
	}

	inline const CameraInfo& Camera::info( size_t index )
	{
		if( index >= _camInfos.size() ){
			throw CVTException( "Could not return CameraInfo: Index out of bounds!" );
		}

		return _camInfos[ index ];
	}

	inline size_t Camera::frameIndex() const
	{
		return 0;
	}

	inline double Camera::stamp() const
	{
		return 0.0;
	}
}

#endif
