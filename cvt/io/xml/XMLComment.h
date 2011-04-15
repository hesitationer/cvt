#ifndef CVT_XMLCOMMENT_H
#define CVT_XMLCOMMENT_H

#include <cvt/io/xml/XMLLeaf.h>

namespace cvt {
	class XMLComment : public XMLLeaf {
		public:
			XMLComment( const String& value );
			XMLComment( const XMLComment& other );
			XMLComment& operator=( const XMLComment& other );
			~XMLComment();

	};

	inline XMLComment::XMLComment( const String& value ) : XMLLeaf( XML_NODE_COMMENT, "", value )
	{
	}

	inline XMLComment::XMLComment( const XMLComment& other ) : XMLLeaf( XML_NODE_COMMENT, "", other._value )
	{
	}

	inline XMLComment::~XMLComment()
	{
	}

	XMLComment& XMLComment::operator=( const XMLComment& other )
	{
		_name = other._name;
		_value = other._value;
		return *this;
	}
}

#endif