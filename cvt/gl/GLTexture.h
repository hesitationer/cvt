#ifndef CVTGLTEXTURE_H
#define CVTGLTEXTURE_H

#include <cvt/gl/OpenGL.h>

namespace cvt {
	class GLTexture {
		public:
			GLTexture( GLenum target = GL_TEXTURE_2D );
			~GLTexture();
			void bind() const;
			void unbind() const;
			GLenum target() const;
			void alloc( GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data = NULL, size_t stride = 0 );
			void alloc( const Image& img, bool copy = false );
			void setData( GLint	xoffset, GLint yoffset,	GLsizei	width, GLsizei height, GLenum format, GLenum type, const GLvoid* data, size_t stride = 0 );
			void setData( const GLBuffer& buffer, GLint	xoffset, GLint yoffset,	GLsizei	width, GLsizei height, GLenum format, GLenum type, const GLvoid* data, size_t stride = 0 );

		private:
			GLuint _tex;
			GLenum _target;
			GLint _internalFormat;
			GLsizei _width, _height;
	};

	inline GLTexture::target() const
	{
		return _target;
	}
}

#endif
