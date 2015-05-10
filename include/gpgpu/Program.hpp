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

#ifndef GPGPU_OPENGL_PROGRAM_HPP
#define GPGPU_OPENGL_PROGRAM_HPP

#include <list>
#include <vector>
#include <memory>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <typeinfo>

#include <Eigen/Dense>

#include "OpenGLObject.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"

namespace gpgpu {

    class ShaderError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class ProgramError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;

        static ProgramError uniform(size_t rows, size_t cols, size_t arrayLength) {
            std::stringstream s;
            s << "Failed to set uniform with dimension " << rows << "x" << cols << " and array length " <<
            arrayLength;
            return ProgramError(s.str());
        }
    };

    class Shader : public OpenGLObject {
    public:
        enum Type {
            Vertex = GL_VERTEX_SHADER,
            Fragment = GL_FRAGMENT_SHADER,
            Geometry = GL_GEOMETRY_SHADER,
            TessControl = GL_TESS_CONTROL_SHADER,
            TessEvaluation = GL_TESS_EVALUATION_SHADER,
            Compute = GL_COMPUTE_SHADER
        };

        Shader(Type type, std::string source);

        ~Shader();

        GLuint id();

    protected:
        GLuint _shaderID;
        Type _type;
    };

    inline std::shared_ptr<Shader> create_shader(Shader::Type type, const std::string &source) {
        return std::make_shared<gpgpu::Shader>(type, source);
    }

    class Program : public OpenGLObject {
    public:
        Program();

        ~Program();

        GLuint id();

        void append(std::shared_ptr<Shader> shader);

        void append(std::initializer_list<std::shared_ptr<Shader>> shader);

        void link();

        void use();

        unsigned int uniformLocation(const std::string &name);

        void attribute(const std::string &name, std::shared_ptr<ArrayBuffer> buffer);

        void uniform(size_t location, std::initializer_list<int> value);

        void uniform(size_t location, std::initializer_list<unsigned int> value);

        void uniform(const std::string &location, int value);

        void uniform(const std::string &location, unsigned int value) {
            uniform(location, (int) value);
        }

        void uniform(const std::string &location, float value);

        void uniform(const std::string &location, std::shared_ptr<Texture> texture);

        template<int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
        void setUniformLocation(size_t location,
                                const Eigen::Matrix<float, _Rows, _Cols, _Options, _MaxRows, _MaxCols> &value,
                                size_t arrayLength = 1) {
            const auto cols = value.cols();
            const auto rows = value.rows() / arrayLength;

            if (_Options == Eigen::RowMajor) {
                Eigen::Matrix<float, _Rows, _Cols, _Options, _MaxRows, _MaxCols> m = value.transpose();
                setUniformLocation(location, m, arrayLength);
                return;
            }

            if (cols == 1) {
                if (rows == 1) {
                    glUniform1fv(location, arrayLength, value.data());
                } else if (rows == 2) {
                    glUniform2fv(location, arrayLength, value.data());
                } else if (rows == 3) {
                    glUniform3fv(location, arrayLength, value.data());
                } else if (rows == 4) {
                    glUniform4fv(location, arrayLength, value.data());
                } else {
                    throw ProgramError::uniform(rows, cols, arrayLength);
                }
            } else if (cols == 2) {
                if (rows == 2) {
                    glUniformMatrix2fv(location, arrayLength,
                                       GL_FALSE, value.data());
                } else if (rows == 3) {
                    glUniformMatrix2x3fv(location, arrayLength,
                                         GL_FALSE, value.data());
                } else if (rows == 4) {
                    glUniformMatrix2x4fv(location, arrayLength,
                                         GL_FALSE, value.data());
                } else {
                    throw ProgramError::uniform(rows, cols, arrayLength);
                }
            } else if (cols == 3) {
                if (rows == 2) {
                    glUniformMatrix3x2fv(location, arrayLength,
                                         GL_FALSE, value.data());
                } else if (rows == 3) {
                    glUniformMatrix3fv(location, arrayLength,
                                       GL_FALSE, value.data());
                } else if (rows == 4) {
                    glUniformMatrix3x4fv(location, arrayLength,
                                         GL_FALSE, value.data());
                } else {
                    throw ProgramError::uniform(rows, cols, arrayLength);
                }
            } else if (cols == 4) {
                if (rows == 2) {
                    glUniformMatrix4x2fv(location, arrayLength,
                                         GL_FALSE, value.data());
                } else if (rows == 3) {
                    glUniformMatrix4x3fv(location, arrayLength,
                                         GL_FALSE, value.data());
                } else if (rows == 4) {
                    glUniformMatrix4fv(location, arrayLength,
                                       GL_FALSE, value.data());
                } else {
                    throw ProgramError::uniform(rows, cols, arrayLength);
                }
            } else {
                throw ProgramError::uniform(rows, cols, arrayLength);
            }

            assertNoGLError("setUniform(Eigen::Matrix)");
        }

        template<int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
        void uniform(const std::string &location,
                     const Eigen::Matrix<float, _Rows, _Cols, _Options, _MaxRows, _MaxCols> &value,
                     size_t arrayLength = 1) {
            setUniformLocation(uniformLocation(location), value, arrayLength);
        }

        void enableAttributes() const;

        void disableAttributesAndClear();

        void render(const ElementArrayBuffer &faces);

        void render(std::shared_ptr<ArrayBuffer> vertices, const std::string &location, GLenum mode);

    protected:
        GLuint _programID;
        std::vector<std::shared_ptr<Shader>> _shaders;
        std::list<std::pair<GLuint, std::shared_ptr<ArrayBuffer>>>
                _activeAttributes;
        std::list<std::shared_ptr<Texture>> _activeTextures;
    };

    inline Shader::Shader(Shader::Type type, std::string source) : _type(type) {
        _shaderID = glCreateShader(static_cast<GLenum> (type));
        assertNoGLError("glCreateShader");

        const GLchar *sources[] = {
                static_cast<const GLchar *> (
                        source.c_str()
                )
        };

        const GLint lengths[] = {static_cast<GLint> (source.length())};

        glShaderSource(_shaderID, 1, sources, lengths);
        assertNoGLError("glShaderSource");

        glCompileShader(_shaderID);
        assertNoGLError("glCompileShader");

        GLint status;
        glGetShaderiv(_shaderID, GL_COMPILE_STATUS, &status);
        assertNoGLError("glGetShaderiv");

        if (status != GL_TRUE) {
            GLint length, actualLength;
            glGetShaderiv(_shaderID, GL_INFO_LOG_LENGTH, &length);
            assertNoGLError("glGetShaderiv");

            GLchar *error = new GLchar[length];
            glGetShaderInfoLog(_shaderID, length, &actualLength, error);
            assertNoGLError("glGetShaderInfoLog");

            std::string msg(static_cast<const char *> (error),
                            static_cast<size_t> (actualLength) - 1);
            delete[] error;

            throw ShaderError(msg);
        }
    }

    inline Shader::~Shader() {
        glDeleteShader(_shaderID);
        assertNoGLError("glDeleteShader");
    }

    inline GLuint Shader::id() {
        return _shaderID;
    }

    inline Program::Program() {
        _programID = glCreateProgram();
        assertNoGLError("glCreateProgram");
    }

    inline Program::~Program() {
        for (auto s : _shaders) {
            glDetachShader(_programID, s->id());
            assertNoGLError("glDetachShader");
        }

        glDeleteProgram(_programID);
        assertNoGLError("glCreateProgram");
    }

    inline unsigned int Program::uniformLocation(const std::string &name) {
        auto loc = glGetUniformLocation(_programID, name.c_str());
        assertNoGLError("glGetUniformLocation");

        if (loc < 0) {
            std::stringstream s;
            s << "Uniform “" << name << "” seems unknown!";
            throw ProgramError(s.str());
        }

        return loc;
    }

    inline void Program::attribute(const std::string &name, std::shared_ptr<ArrayBuffer> buffer) {
        auto loc = glGetAttribLocation(_programID, name.c_str());
        assertNoGLError("glGetAttribLocation");

        if (loc < 0) {
            std::stringstream s;
            s << "Attribute “" << name << "” seems unknown!";
            throw ProgramError(s.str());
        }

        buffer->bind(loc);
        _activeAttributes.push_back(make_pair(loc, buffer));
    }

    inline void Program::uniform(const std::string &location, std::shared_ptr<Texture> texture) {
        auto num = _activeTextures.size();
        uniform(location, (unsigned int) num);

        glActiveTexture(GL_TEXTURE0 + num);
        assertNoGLError("glActiveTexture");

        texture->bind();

        _activeTextures.push_back(texture);
    }

    inline void Program::enableAttributes() const {
        for (auto loc : _activeAttributes) {
            glEnableVertexAttribArray(loc.first);
            assertNoGLError("glEnableVertexAttribArray");
        }
    }

    inline void Program::disableAttributesAndClear() {
        for (auto attr : _activeAttributes) {
            glDisableVertexAttribArray(attr.first);
            assertNoGLError("glDisableVertexAttribArray");
        }

        _activeAttributes.clear();

        if (!_activeTextures.empty()) {
            _activeTextures.clear();
            glActiveTexture(GL_TEXTURE0);
            assertNoGLError("glActiveTexture");
        }
    }

    inline GLuint Program::id() {
        return _programID;
    }

    inline void Program::append(std::shared_ptr<Shader> shader) {
        _shaders.push_back(shader);
        glAttachShader(_programID, shader->id());
        assertNoGLError("glAttachShader");
    }

    inline void Program::append(std::initializer_list<std::shared_ptr<Shader> > shader) {
        for (auto s : shader) {
            append(s);
        }
    }

    inline void Program::link() {
        glLinkProgram(_programID);
        assertNoGLError("glLinkProgram");

        GLint status;
        glGetProgramiv(_programID, GL_LINK_STATUS, &status);
        assertNoGLError("glGetProgramiv");

        if (status != GL_TRUE) {
            GLint length, actualLength;
            glGetProgramiv(_programID, GL_INFO_LOG_LENGTH, &length);
            assertNoGLError("glGetProgramiv");

            GLchar *error = new GLchar[length];
            glGetProgramInfoLog(_programID, length, &actualLength, error);
            assertNoGLError("glGetProgramInfoLog");

            std::string msg(static_cast<const char *> (error),
                            static_cast<size_t> (actualLength) - 1);
            delete[] error;

            throw ProgramError(msg);
        }
    }

    inline void Program::use() {
        glUseProgram(_programID);
        assertNoGLError("glUseProgram");
    }

    inline void Program::uniform(const std::string &location, int value) {
        glUniform1i(uniformLocation(location), value);
        assertNoGLError("glUniform1i");
    }

    inline void Program::uniform(const std::string &location, float value) {
        glUniform1f(uniformLocation(location), value);
        assertNoGLError("glUniform1f");
    }

    inline void Program::render(const ElementArrayBuffer &faces) {
        enableAttributes();
        faces.bind();
        glDrawElements(faces.mode(), faces.elements() * faces.dimension(), faces.valueType(), nullptr);
        assertNoGLError("glDrawElements");
        disableAttributesAndClear();
    }

    inline void Program::render(std::shared_ptr<ArrayBuffer> vertices, const std::string &location, GLenum mode) {
        attribute(location, vertices);
        enableAttributes();

        glDrawArrays(mode, 0, vertices->elements() * vertices->dimension());
        assertNoGLError("glDrawArrays");

        disableAttributesAndClear();
    }
}

#endif /* GPGPU_OPENGL_PROGRAM_HPP */
