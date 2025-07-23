#pragma once

namespace MobileGL {
	namespace MG_Impl::GLImpl {
		/* @INSERTION_POINT:FUNCTION_DECLARATION@ */
		const GLubyte* GetString(GLenum name);
		const GLubyte* GetStringi(GLenum name, GLuint index);
		void GetIntegerv(GLenum pname, GLint* params);
		GLenum GetError();
	}
}