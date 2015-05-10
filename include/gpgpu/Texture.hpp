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

#ifndef GPGPU_OPENGL_TEXTURE_HPP
#define GPGPU_OPENGL_TEXTURE_HPP

#include "OpenGLObject.hpp"

#include <stdexcept>
#include <vector>
#include <memory>

#include <OpenImageIO/imagebuf.h>

namespace gpgpu {
    static constexpr int DEFAULT_TEXTURE_CHANNELS = 4;
    static constexpr GLint DEFAULT_TEXTURE_INTERNAL_FORMAT = GL_RGBA;
    static constexpr GLint DEFAULT_TEXTURE_FORMAT = GL_RGBA;
    static constexpr GLint DEFAULT_TEXTURE_TYPE = GL_UNSIGNED_BYTE;

    class TextureError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class Texture : public OpenGLObject {
    public:
        Texture(GLenum target) {
            glGenTextures(1, &_id);
            _target = target;
            set_filter(GL_LINEAR);
        }

        virtual ~Texture() {
            glDeleteTextures(1, &_id);
        }

        GLuint id() const {
            return _id;
        }

        void set_filter(GLenum filter) {
            glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, filter);
            glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, filter);
            glTexParameteri(_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

            assertNoGLError("glTexParameteri");
        }

        virtual OpenImageIO::ImageSpec size() = 0;

        GLenum target() const {
            return _target;
        }

        void bind() {
            glBindTexture(_target, _id);
            assertNoGLError("glBindTexture");
        }

    private:
        GLuint _id;
        GLenum _target;
    };

    class Texture2D : public Texture {
    public:
        Texture2D(unsigned int width, unsigned int height,
                  GLenum internalFormat = DEFAULT_TEXTURE_INTERNAL_FORMAT,
                  GLenum format = DEFAULT_TEXTURE_FORMAT,
                  GLenum type = DEFAULT_TEXTURE_TYPE) : Texture(GL_TEXTURE_2D) {

            glBindTexture(target(), id());
            glTexImage2D(target(), 0, internalFormat, width, height, 0, format, type, nullptr);
            assertNoGLError("glTexImage2D");
        }

        std::shared_ptr <OpenImageIO::ImageBuf> image() {
            glBindTexture(target(), id());

            GLint internalFormat;
            glGetTexLevelParameteriv(target(), 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

            if (internalFormat != GL_RGBA && internalFormat != GL_BGRA) {
                throw TextureError("Internal format must be GL_RGBA or GL_BGRA to extract image.");
            }

            auto buffer = std::make_shared<OpenImageIO::ImageBuf>("texture", size());

            glGetTexImage(target(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer->localpixels());
            assertNoGLError("glGetTexImage");

            return buffer;
        }

        OpenImageIO::ImageSpec size() {
            GLint width;
            GLint height;

            glGetTexLevelParameteriv(target(), 0, GL_TEXTURE_WIDTH, &width);
            glGetTexLevelParameteriv(target(), 0, GL_TEXTURE_HEIGHT, &height);
            assertNoGLError("glGetTexLevelParameteriv");

            return OpenImageIO::ImageSpec(width, height, DEFAULT_TEXTURE_CHANNELS);
        }

        virtual void clear() {

        };
    };

    class TextureArray2D : public Texture {
    public:
        TextureArray2D(unsigned int width, unsigned int height, unsigned int layers,
                       GLenum internalFormat = DEFAULT_TEXTURE_INTERNAL_FORMAT,
                       GLenum format = DEFAULT_TEXTURE_FORMAT,
                       GLenum type = DEFAULT_TEXTURE_TYPE)
                : Texture(GL_TEXTURE_2D_ARRAY) {

            glBindTexture(target(), id());
            assertNoGLError("glBindTexture");

            glTexImage3D(target(), 0, internalFormat, width, height, layers, 0, format, type, nullptr);
            assertNoGLError("glTexImage3D");
        }


        std::shared_ptr<OpenImageIO::ImageBuf> image(unsigned int layer) {
            glBindTexture(target(), id());

            GLint internalFormat;
            glGetTexLevelParameteriv(target(), 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

            if (internalFormat != GL_RGBA && internalFormat != GL_BGRA) {
                throw TextureError("Internal format must be GL_RGBA or GL_BGRA to extract image.");
            }

            auto s = size();
            s.depth = 0;
            auto buffer = std::make_shared<OpenImageIO::ImageBuf>("texture", s);

            /*
             * Dummy FB to bind the requested layer as attachment.
             */
            GLuint fb;
            glGenFramebuffers(1, &fb);
            glBindFramebuffer(GL_FRAMEBUFFER, fb);
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id(), 0, layer);
            assertNoGLError("glFramebufferTextureLayer");

            /*
             * Read the attachment.
             */
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glReadPixels(0, 0, s.width, s.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer->localpixels());
            assertNoGLError("glReadPixels");

            glDeleteFramebuffers(1, &fb);
            assertNoGLError("glDeleteFramebuffers");

            return buffer;
        }

        void set(unsigned int layer, const OpenImageIO::ImageBuf &image) {
            const auto s = size();
            const auto i = image.spec();

            if (layer >= s.depth) {
                std::stringstream msg;
                msg << "Layer (" << layer << ") exceeds layer count (" << s.depth << ").";
                throw TextureError(msg.str());
            }

            if (i.width != s.width || i.height != s.height) {
                std::stringstream msg;
                msg << "Image needs to have the same size as an array layer (image=" << i.width << "x" << i.height <<
                        ", array=" << s.width << "x" << s.height << ").";
                throw TextureError(msg.str());
            }

            // Alpha channel?
            GLenum type = i.nchannels == 4 ? GL_RGBA : GL_RGB;

            glBindTexture(target(), id());
            glTexSubImage3D(target(), 0, 0, 0, layer, s.width, s.height, 1, type, GL_UNSIGNED_BYTE,
                            image.localpixels());
            assertNoGLError("glTexSubImage3D");
        }

        OpenImageIO::ImageSpec size() {
            GLint width;
            GLint height;
            GLint depth;

            glGetTexLevelParameteriv(target(), 0, GL_TEXTURE_WIDTH, &width);
            glGetTexLevelParameteriv(target(), 0, GL_TEXTURE_HEIGHT, &height);
            glGetTexLevelParameteriv(target(), 0, GL_TEXTURE_DEPTH, &depth);
            assertNoGLError("glGetTexLevelParameteriv");

            auto spec = OpenImageIO::ImageSpec(width, height, DEFAULT_TEXTURE_CHANNELS);
            spec.depth = depth;
            return spec;
        }
    };
}

#endif /* GPGPU_OPENGL_TEXTURE_HPP */
