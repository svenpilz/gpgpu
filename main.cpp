#include <iostream>

#include <gpgpu/Context.hpp>
#include <gpgpu/Framebuffer.hpp>
#include <gpgpu/Program.hpp>

#include <OpenImageIO/imagebufalgo.h>

using namespace std;

int main() {
    /*
     * Context
     */
    gpgpu::Context context;
    cout << context << endl;

    /*
     * Shader
     */
    gpgpu::Program fixed_color_shader;
    auto vs = gpgpu::create_shader(gpgpu::Shader::Vertex, R"(
        #version 130
        uniform mat4 camera;
        in vec4 vertex;

        void main() {
            gl_Position = camera * vertex;
        })"
    );

    auto fs = gpgpu::create_shader(gpgpu::Shader::Fragment, R"(
        #version 130
        out vec4 color;

        void main() {
            color = vec4(1.0, 0.0, 0.0, 1.0);
        })"
    );

    fixed_color_shader.append({vs, fs});
    fixed_color_shader.link();

    /*
     * Framebuffer
     */
    gpgpu::Framebuffer fb(800, 600, true);
    auto canvas = make_shared<gpgpu::Texture2D>(800, 600);
    fb.set_color_attachment(canvas, 0);

    /*
     * Geometry
     */
    float vertices[] = {
            -0.5, -0.5, 0.0,
            0.0, 0.5, 0.0,
            0.5, -0.5, 0.0
    };

    auto geometry = make_shared<gpgpu::ArrayBuffer>();
    geometry->data(3, 3, vertices);

    unsigned int face_vertices[] = {
            0, 1, 2
    };

    gpgpu::ElementArrayBuffer faces;
    faces.data(3, 3, face_vertices);

    /*
     * Camera
     */
    Eigen::Matrix4f camera = Eigen::Matrix4f::Identity();

    /*
     * Render
     */
    fb.bind();
    fixed_color_shader.use();
    fixed_color_shader.uniform("camera", camera);


    fixed_color_shader.attribute("vertex", geometry);
    fixed_color_shader.render(faces);

    // Or without indirection.
    //fixed_color_shader.render(geometry, "vertex", GL_TRIANGLES);

    auto image = canvas->image();
    OpenImageIO::ImageBuf output;
    OpenImageIO::ImageBufAlgo::flip(output, *image);
    output.write("canvas.png");

    return 0;
}