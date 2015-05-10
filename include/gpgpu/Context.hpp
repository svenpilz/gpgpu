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

#ifndef GPGPU_OPENGL_CONTEXT_HPP
#define GPGPU_OPENGL_CONTEXT_HPP

#include "OpenGLObject.hpp"

#include <ostream>
#include <GLFW/glfw3.h>

namespace gpgpu {
    /*
     * Declaration
     */
    class ContextError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class Context : public OpenGLObject {
    public:
        Context();

        ~Context() {
            glfwTerminate();
        }

        void make_current() {
            glfwMakeContextCurrent(_handle);
        }

        std::string gl_vendor() const {
            return gl_query(GL_VENDOR) + "/" + gl_query(GL_RENDERER);
        }

        std::string gl_version() const {
            return gl_query(GL_VERSION);
        }

    protected:
        GLFWwindow* _handle;
        std::string gl_query(GLenum name) const;
    };


    /*
     * Definition
     */
    inline Context::Context() {
        if (!glfwInit()) {
            throw ContextError("Failed to initialize GLFW (OpenGL context creation).");
        }

        glfwWindowHint(GLFW_VISIBLE, 0);
        _handle = glfwCreateWindow(1, 1, "render target", NULL, NULL);

        if (_handle == nullptr) {
            throw ContextError("Failed to create an OpenGL context with GLFW.");
        }

        make_current();

        auto status = glewInit();
        if (status != GLEW_OK) {
            std::stringstream s;
            s << "Failed to initialize OpenGL extensions (GLEW): " << glewGetErrorString(status) << " (" << status << ").";
            throw ContextError(s.str());
        }
    }

    inline std::string Context::gl_query(GLenum name) const {
        auto v = glGetString(name);
        assertNoGLError("glGetString");

        if (v == nullptr) {
            std::stringstream s;
            s << "glGetString returned NULL for 0x" << std::hex << name;
            throw ContextError(s.str());
        }

        return std::string(reinterpret_cast<const char*>(v));
    }

    inline std::ostream& operator<<(std::ostream& s, const Context& c) {
        s << "opengl context: vendor=“" << c.gl_vendor() << "”, version=“" << c.gl_version() << "”";
        return s;
    }
}

#endif /* GPGPU_OPENGL_CONTEXT_HPP */