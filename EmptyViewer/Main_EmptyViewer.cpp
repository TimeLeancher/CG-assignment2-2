#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace glm;

int Width = 512;  // �̹��� �ػ� x
int Height = 512; // �̹��� �ػ� y
std::vector<vec3> OutputImage;
// -------------------------------------------------

// Ray Ŭ����: ������ ǥ���մϴ�.
class Ray {
public:
    vec3 origin;     // ������ ������
    vec3 direction;  // ������ ���� ����

    Ray(const vec3& origin, const vec3& direction) : origin(origin), direction(direction) {}
};

// Camera Ŭ����: ī�޶� ǥ���մϴ�.
class Camera {
public:
    vec3 eye;       // ī�޶��� ��ġ
    vec3 u, v, w;   // ī�޶��� ���� (u, v, -w)
    float l, r, b, t, d; // �� ���� (left, right, bottom, top, distance)

    Camera(const vec3& eye, const vec3& u, const vec3& v, const vec3& w,
        float l, float r, float b, float t, float d)
        : eye(eye), u(u), v(v), w(w), l(l), r(r), b(b), t(t), d(d) {
    }

    // �ȼ� ��ǥ�� ���� ������ �����ϴ� �Լ�
    Ray getRay(float ix, float iy) const {
        float ndc_x = (ix + 0.5f) / Width;
        float ndc_y = (iy + 0.5f) / Height;
        float screen_x = l + (r - l) * ndc_x;
        float screen_y = b + (t - b) * ndc_y;

        vec3 ray_direction = normalize(-d * w + screen_x * u + screen_y * v);
        return Ray(eye, ray_direction);
    }
};

// Material Ŭ����: ǥ���� ���� �Ӽ��� ǥ���մϴ�.
class Material {
public:
    vec3 ka; // Ambient �ݻ� ��� (�ֺ���)
    vec3 kd; // Diffuse �ݻ� ��� (���ݻ�)
    vec3 ks; // Specular �ݻ� ��� (���ݻ�)
    float specular_power; // Specular power (���ݻ� ����)

    Material(const vec3& ka, const vec3& kd, const vec3& ks, float specular_power)
        : ka(ka), kd(kd), ks(ks), specular_power(specular_power) {
    }
};

// Surface Ŭ����: ��� ǥ���� Ŭ�����Դϴ�.
class Surface {
public:
    virtual bool intersect(const Ray& ray, float& t) const = 0;
    virtual vec3 getNormal(const vec3& point) const = 0;
    virtual Material getMaterial() const = 0; // ǥ���� ������ �������� �Լ�
};

// Plane Ŭ����: ����� ǥ���մϴ�.
class Plane : public Surface {
public:
    float y; // ����� y ��ǥ
    Material material; // ����� ����

    Plane(float y, const Material& material) : y(y), material(material) {}

    bool intersect(const Ray& ray, float& t) const override {
        if (abs(ray.direction.y) < 1e-6) { // ������ ���� ������ ���
            return false;
        }
        t = (this->y - ray.origin.y) / ray.direction.y;
        return t > 0; // �������� ���� ���⿡ �־�� ��
    }

    vec3 getNormal(const vec3& point) const override {
        return vec3(0, 1, 0); // ����� ���� ���ʹ� (0, 1, 0)
    }

    Material getMaterial() const override {
        return material;
    }
};

// Sphere Ŭ����: ���� ǥ���մϴ�.
class Sphere : public Surface {
public:
    vec3 center; // ���� �߽�
    float radius; // ���� ������
    Material material; // ���� ����

    Sphere(const vec3& center, float radius, const Material& material)
        : center(center), radius(radius), material(material) {
    } // ���� �߽� ��ǥ(center)�� ������(radius)�� ���ڷ� �޾� �ʱ�ȭ

    bool intersect(const Ray& ray, float& t) const override {
        vec3 oc = ray.origin - center;
        float a = dot(ray.direction, ray.direction);
        float b = 2.0f * dot(oc, ray.direction);
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c; // �Ǻ���

        if (discriminant < 0) {
            return false; // �������� ���� ��� false ��ȯ
        }
        // �������� �� ���� ��� �� ���� �� ����
        t = (-b - ::sqrt(discriminant)) / (2 * a);
        if (t < 0) {
            t = (-b + ::sqrt(discriminant)) / (2 * a);
        } // �������� ���� ���⿡ ������ true ��ȯ
        return t > 0;
    }

    vec3 getNormal(const vec3& point) const override {
        return normalize(point - center);
    }

    Material getMaterial() const override {
        return material;
    }
};

// Scene Ŭ����: ����� �����մϴ�.
class Scene {
public:
    std::vector<Surface*> objects;
    Camera camera;
    vec3 light_pos; // ���� ��ġ

    Scene(const Camera& camera, const vec3& light_pos) : camera(camera), light_pos(light_pos) {}

    void addObject(Surface* object) {
        objects.push_back(object);
    }

    // ���� ���� �Լ�
    vec3 trace(const Ray& ray) const {
        float closest_t = INFINITY;
        Surface* closest_surface = nullptr;
        // ��� ��ü�� ���� ���� ����� �������� ���� ��ü�� ã��
        for (Surface* object : objects) {
            float t;
            if (object->intersect(ray, t) && t < closest_t) {
                closest_t = t;
                closest_surface = object;
            }
        }
        // ���� ����� �������� ���� ��ü�� �ִ°�
        if (closest_surface) {
            vec3 intersection_point = ray.origin + ray.direction * closest_t; // ������ ���
            vec3 normal = closest_surface->getNormal(intersection_point);     // ������������ ���� ���� ���
            Material material = closest_surface->getMaterial();             // ������ ǥ���� ���� ��������
            return phongShading(intersection_point, normal, material);       // Phong ���� ó�� ���
        }
        else {
            return vec3(0.0f, 0.0f, 0.0f); // ������ ��ȯ (����)
        }
    }

    // Phong ���� ó�� ��� �Լ�
    vec3 phongShading(const vec3& point, const vec3& normal, const Material& material) const {
        vec3 light_dir = normalize(light_pos - point); // ���������� �������� ���ϴ� ���� ����
        vec3 view_dir = normalize(-point);             // ���������� �������� ���ϴ� ���� ���� (ī�޶� ������ �ִٰ� ����)
        vec3 half_vector = normalize(light_dir + view_dir); // Half-vector ��� (Blinn-Phong ��)

        // Ambient ���� ���
        vec3 ambient = material.ka;

        // Diffuse ���� ���
        float diff = max(dot(normal, light_dir), 0.0f); // ���� ���Ϳ� ���� ���� ������ ����
        vec3 diffuse = material.kd * diff;

        // Specular ���� ���
        float spec = pow(max(dot(normal, half_vector), 0.0f), material.specular_power); // ���� ���Ϳ� half-vector�� ����
        vec3 specular = material.ks * spec;

        // �׸��� ���
        Ray shadow_ray(point + normal * 0.001f, light_dir); // Offset to avoid self-intersection
        bool in_shadow = false;
        for (Surface* object : objects) {
            float t;
            if (object->intersect(shadow_ray, t) && t > 0.001f) { // �׸��� ������ �ٸ� ��ü�� �����ϴ��� Ȯ��
                in_shadow = true;
                break;
            }
        }

        if (in_shadow) {
            return ambient; // �׸��� ���������� ambient ���и� ���
        }
        else {
            return ambient + diffuse + specular; // �׸��ڰ� �ƴϸ� ambient, diffuse, specular ���� ��� ���
        }
    }
};

// -------------------------------------------------

void render(Scene& scene) {
    OutputImage.clear(); // OutputImage �ʱ�ȭ
    for (int j = 0; j < Height; ++j) {
        for (int i = 0; i < Width; ++i) {
            Ray ray = scene.camera.getRay(i, j); // ī�޶��� �ȼ� ��ǥ�� ���� ����
            vec3 color = scene.trace(ray);       // ���� ����

            // ���� ���� ����
            float gamma = 2.2f;
            color.r = pow(color.r, 1.0f / gamma);
            color.g = pow(color.g, 1.0f / gamma);
            color.b = pow(color.b, 1.0f / gamma);

            OutputImage.push_back(color);       // OutputImage�� ���� �߰�
        }
    }
}

void resize_callback(GLFWwindow*, int nw, int nh) {
    Width = nw;
    Height = nh;
    glViewport(0, 0, nw, nh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(Width),
        0.0, static_cast<double>(Height),
        1.0, -1.0);
    OutputImage.resize(Width * Height);
}

int main(int argc, char* argv) {
    // -------------------------------------------------
    // Initialize Window
    // -------------------------------------------------
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(Width, Height, "OpenGL Viewer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // We have an opengl context now. Everything from here on out 
    // is just managing our window or opengl directly.

    // Tell the opengl state machine we don't want it to make  
    // any assumptions about how pixels are aligned in memory  
    // during transfers between host and device (like glDrawPixels(...) )
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // We call our resize function once to set everything up initially
    // after registering it as a callback with glfw
    glfwSetFramebufferSizeCallback(window, resize_callback);
    resize_callback(NULL, Width, Height);

    // -------------------------------------------------
    // Scene Setup
    // -------------------------------------------------
    Camera camera(vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1),
        -0.1f, 0.1f, -0.1f, 0.1f, 0.1f);

    // Material properties from the prompt
    Material plane_mat(vec3(0.2f, 0.2f, 0.2f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), 0.0f);
    Material sphere1_mat(vec3(0.2f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0.0f);
    Material sphere2_mat(vec3(0.0f, 0.2f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.5f, 0.5f, 0.5f), 32.0f);
    Material sphere3_mat(vec3(0.0f, 0.0f, 0.2f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), 0.0f);

    Scene scene(camera, vec3(-4.0f, 4.0f, -3.0f)); // ���� ��ġ
    scene.addObject(new Plane(-2.0f, plane_mat));
    scene.addObject(new Sphere(vec3(-4, 0, -7), 1.0f, sphere1_mat));
    scene.addObject(new Sphere(vec3(0, 0, -7), 2.0f, sphere2_mat));
    scene.addObject(new Sphere(vec3(4, 0, -7), 1.0f, sphere3_mat));

    OutputImage.resize(Width * Height); // OutputImage ũ�� ����
    render(scene);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // -------------------------------------------------------------
        // Rendering begins!
        glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, &OutputImage[0]);
        // and ends.
        // -------------------------------------------------------------

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        // Close when the user hits 'q' or escape
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}