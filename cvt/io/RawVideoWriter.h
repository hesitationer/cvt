/*
			CVT - Computer Vision Tools Library

 	 Copyright (c) 2012, Philipp Heise, Sebastian Klose

 	THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 	PARTICULAR PURPOSE.
 */
#ifndef CVT_RAWVIDEOWRITER_H
#define CVT_RAWVIDEOWRITER_H

#include <cvt/util/String.h>
#include <cvt/gfx/Image.h>

namespace cvt
{
	class RawVideoWriter
	{
		public:
			RawVideoWriter( const String & outname );
			~RawVideoWriter();

			void write( const Image & img );

		private:
			// file descriptor
			int		_fd;
			off_t	_currSize;
			off_t	_maxSize;

			// the total offset into the file
			off_t	_offsetInFile;

			/* current position in mapped region */
			void	* _map;
			size_t	  _mappedSize;
			uint8_t * _pos;
			size_t	  _pageSize;

			size_t	_width;
			size_t	_height;
			size_t	_stride;
			size_t	_formatID;
			size_t	_imgSize;

			/* change the filesize and update the mapping */
			void remapFile( size_t additionalBytes = 0 );

			/* write the header to the file */
			void writeHeader();

			/* resize to maxsize */
			void resizeFile();

	};
}

#endif
