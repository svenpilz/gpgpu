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

#ifndef GPGPU_OPENGL_FRAMEBUFFER_HPP
#define GPGPU_OPENGL_FRAMEBUFFER_HPP

#include <memory>
#include <vector>
#include <stdexcept>

#include "OpenGLObject.hpp"
#include "Texture.hpp"

namespace gpgpu {

    class FramebufferError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class Framebuffer : public OpenGLObject {
    public:
        Framebuffer(unsigned int width, unsigned int height, bool use_depth_test = false)
                : _width(width), _height(height), _use_depth_test(use_depth_test) {

            glGenFramebuffers(1, &_id);
            assertNoGLError("glGenFramebuffers");

            glBindFramebuffer(GL_FRAMEBUFFER, _id);
            assertNoGLError("glBindFramebuffer");

            if (_use_depth_test) {
                glGenRenderbuffers(1, &_depth_buffer);
                glBindRenderbuffer(GL_RENDERBUFFER, _depth_buffer);
                assertNoGLError("glBindRenderbuffer");

                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, _width, _height);
                assertNoGLError("glRenderbufferStorage");

                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_buffer);
                assertNoGLError("glFramebufferRenderbuffer");
            }
        }

        void bind() {
            if (_color_attachments.size() == 0) {
                throw FramebufferError("No color attachments, nothing to draw to.");
            }

            glBindFramebuffer(GL_FRAMEBUFFER, _id);
            assertNoGLError("glBindFramebuffer");

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                throw FramebufferError("Framebuffer is not complete.");
            }

            glViewport(0, 0, _width, _height);

            /*
             * Add color attachments as draw buffers.
             */
            const unsigned int size = _color_attachments.size();
            GLenum parameter[size];

            for (unsigned int i = 0; i < size; ++i) {
                parameter[i] = GL_COLOR_ATTACHMENT0 + i;
            }

            glDrawBuffers(size, parameter);
            assertNoGLError("glDrawBuffers");

            if (_use_depth_test) {
                glEnable(GL_DEPTH_TEST);
                glClear(GL_DEPTH_BUFFER_BIT);
            } else {
                glDisable(GL_DEPTH_TEST);
            }

            glClear(GL_COLOR_BUFFER_BIT);
        }

        void set_color_attachment(std::shared_ptr <Texture> texture, unsigned int id) {
            store_color_attachment(texture, id);
            glBindFramebuffer(GL_FRAMEBUFFER, _id);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + id, texture->target(), texture->id(), 0);
            assertNoGLError("glFramebufferTexture2D");
        }

        void set_color_attachment(std::shared_ptr <TextureArray2D> texture_array, unsigned int layer, unsigned int id) {
            store_color_attachment(texture_array, id);
            glBindFramebuffer(GL_FRAMEBUFFER, _id);
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + id, texture_array->id(), 0, layer);
            assertNoGLError("glFramebufferTextureLayer");
        }

    private:
        unsigned int _width;
        unsigned int _height;
        bool _use_depth_test;
        std::vector <std::shared_ptr<Texture>> _color_attachments;

        GLuint _id;
        GLuint _depth_buffer;

        void store_color_attachment(std::shared_ptr <Texture> texture, unsigned int id) {
            if (id < _color_attachments.size()) {
                _color_attachments.at(id) = texture;
            } else if (id == _color_attachments.size()) {
                _color_attachments.push_back(texture);
            } else {
                throw FramebufferError("Attachment id's must be sequential.");
            }
        }
    };
}

#endif /* GPGPU_OPENGL_FRAMEBUFFER_HPP */
