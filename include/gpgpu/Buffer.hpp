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

#ifndef GPGPU_OPENGL_BUFFER_HPP
#define GPGPU_OPENGL_BUFFER_HPP

#include "OpenGLObject.hpp"

namespace gpgpu {
    class Buffer : public OpenGLObject {
    public:
        enum BufferType {
            Array = GL_ARRAY_BUFFER,
            ElementArray = GL_ELEMENT_ARRAY_BUFFER
        };

        enum ValueType {
            Byte = GL_BYTE,
            UnsignedByte = GL_UNSIGNED_BYTE,
            Short = GL_SHORT,
            UnsignedShort = GL_UNSIGNED_SHORT,
            Integer = GL_INT,
            UnsignedInteger = GL_UNSIGNED_INT,
            HalfFloat = GL_HALF_FLOAT,
            Float = GL_FLOAT,
            Double = GL_DOUBLE,
            Fixed = GL_FIXED
        };

        Buffer(BufferType type) : _bufferType(type) {
            glGenBuffers(1, &_id);
            assertNoGLError("glGenBuffers");
        }

        virtual ~Buffer() {
            glDeleteBuffers(1, &_id);
            assertNoGLError("glDeleteBuffers");
        }

        template<typename T>
        void data(size_t elements, unsigned char dimension, ValueType valueType, const T *ptr) {
            _elements = elements;
            _dimension = dimension;
            _valueType = valueType;

            data(_elements * _dimension * sizeof(T), ptr);
        }

        void data(size_t elements, unsigned char dimension, const double *ptr) {
            data(elements, dimension, Buffer::Double, ptr);
        }

        void data(size_t elements, unsigned char dimension, const float *ptr) {
            data(elements, dimension, Buffer::Float, ptr);
        }

        void data(size_t elements, unsigned char dimension, const unsigned int *ptr) {
            data(elements, dimension, Buffer::UnsignedInteger, ptr);
        }

        GLuint id() const {
            return _id;
        }

        size_t elements() const {
            return _elements;
        }

        unsigned char dimension() const {
            return _dimension;
        }

        ValueType valueType() const {
            return _valueType;
        }

    protected:
        BufferType _bufferType;
        ValueType _valueType;
        size_t _elements;
        unsigned char _dimension;
        GLuint _id;

        void data(size_t size, const void *ptr) {
            glBindBuffer(_bufferType, _id);
            glBufferData(_bufferType, size, ptr, GL_STATIC_DRAW);
            assertNoGLError("glBufferData");
        }
    };

    class ArrayBuffer : public Buffer {
    public:
        ArrayBuffer() : Buffer(Buffer::Array) {

        }

        void bind(GLuint index) const {
            glBindBuffer(_bufferType, _id);
            glVertexAttribPointer(index, _dimension, _valueType, GL_FALSE, 0, nullptr);
            assertNoGLError("glVertexAttribPointer");
        }
    };

    class ElementArrayBuffer : public Buffer {
    public:
        ElementArrayBuffer(GLenum mode = GL_TRIANGLES)
                : Buffer(Buffer::ElementArray), _mode(mode) {

        }

        void bind() const {
            glBindBuffer(_bufferType, _id);
            assertNoGLError("glBindBuffer");
        }

        GLenum mode() const {
            return _mode;
        }

    protected:
        GLenum _mode;
    };
}

#endif /* GPGPU_OPENGL_BUFFER_HPP */
