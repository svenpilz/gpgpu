// The MIT License (MIT)
//
// Copyright (c) 2014-2015 Sven-Kristofer Pilz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef GPGPU_OPENGL_OPENGLOBJECT_HPP
#define GPGPU_OPENGL_OPENGLOBJECT_HPP

#include <sstream>
#include <stdexcept>

#include <GL/glew.h>

namespace gpgpu {

    class OpenGLError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class OpenGLObject {
    public:
        virtual ~OpenGLObject() = default;

    protected:
        void assertNoGLError(const std::string &component) const {
            GLenum error = glGetError();

            if (error != GL_NO_ERROR) {
                std::stringstream s;
                s << "OpenGL error 0x" << std::hex << error << " at " << component;
                throw OpenGLError(s.str());
            }
        }
    };
}

#endif /* GPGPU_OPENGL_OPENGLOBJECT_HPP */
