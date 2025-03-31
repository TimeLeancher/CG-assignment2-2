#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace glm;

int Width = 512;  // 이미지 해상도 x
int Height = 512; // 이미지 해상도 y
std::vector<vec3> OutputImage;
// -------------------------------------------------

// Ray 클래스: 광선을 표현합니다.
class Ray {
public:
    vec3 origin;     // 광선의 시작점
    vec3 direction;  // 광선의 방향 벡터

    Ray(const vec3& origin, const vec3& direction) : origin(origin), direction(direction) {}
};

// Camera 클래스: 카메라를 표현합니다.
class Camera {
public:
    vec3 eye;       // 카메라의 위치
    vec3 u, v, w;   // 카메라의 방향 (u, v, -w)
    float l, r, b, t, d; // 뷰 영역 (left, right, bottom, top, distance)

    Camera(const vec3& eye, const vec3& u, const vec3& v, const vec3& w,
        float l, float r, float b, float t, float d)
        : eye(eye), u(u), v(v), w(w), l(l), r(r), b(b), t(t), d(d) {
    }

    // 픽셀 좌표를 통해 광선을 생성하는 함수
    Ray getRay(float ix, float iy) const {
        float ndc_x = (ix + 0.5f) / Width;
        float ndc_y = (iy + 0.5f) / Height;
        float screen_x = l + (r - l) * ndc_x;
        float screen_y = b + (t - b) * ndc_y;

        vec3 ray_direction = normalize(-d * w + screen_x * u + screen_y * v);
        return Ray(eye, ray_direction);
    }
};

// Material 클래스: 표면의 재질 속성을 표현합니다.
class Material {
public:
    vec3 ka; // Ambient 반사 계수 (주변광)
    vec3 kd; // Diffuse 반사 계수 (난반사)
    vec3 ks; // Specular 반사 계수 (정반사)
    float specular_power; // Specular power (정반사 강도)

    Material(const vec3& ka, const vec3& kd, const vec3& ks, float specular_power)
        : ka(ka), kd(kd), ks(ks), specular_power(specular_power) {
    }
};

// Surface 클래스: 모든 표면의 클래스입니다.
class Surface {
public:
    virtual bool intersect(const Ray& ray, float& t) const = 0;
    virtual vec3 getNormal(const vec3& point) const = 0;
    virtual Material getMaterial() const = 0; // 표면의 재질을 가져오는 함수
};

// Plane 클래스: 평면을 표현합니다.
class Plane : public Surface {
public:
    float y; // 평면의 y 좌표
    Material material; // 평면의 재질

    Plane(float y, const Material& material) : y(y), material(material) {}

    bool intersect(const Ray& ray, float& t) const override {
        if (abs(ray.direction.y) < 1e-6) { // 광선이 평면과 평행한 경우
            return false;
        }
        t = (this->y - ray.origin.y) / ray.direction.y;
        return t > 0; // 교차점이 광선 방향에 있어야 함
    }

    vec3 getNormal(const vec3& point) const override {
        return vec3(0, 1, 0); // 평면의 법선 벡터는 (0, 1, 0)
    }

    Material getMaterial() const override {
        return material;
    }
};

// Sphere 클래스: 구를 표현합니다.
class Sphere : public Surface {
public:
    vec3 center; // 구의 중심
    float radius; // 구의 반지름
    Material material; // 구의 재질

    Sphere(const vec3& center, float radius, const Material& material)
        : center(center), radius(radius), material(material) {
    } // 구의 중심 좌표(center)와 반지름(radius)을 인자로 받아 초기화

    bool intersect(const Ray& ray, float& t) const override {
        vec3 oc = ray.origin - center;
        float a = dot(ray.direction, ray.direction);
        float b = 2.0f * dot(oc, ray.direction);
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c; // 판별식

        if (discriminant < 0) {
            return false; // 교차점이 없는 경우 false 반환
        }
        // 교차점이 두 개인 경우 더 작은 값 선택
        t = (-b - ::sqrt(discriminant)) / (2 * a);
        if (t < 0) {
            t = (-b + ::sqrt(discriminant)) / (2 * a);
        } // 교차점이 광선 방향에 있으면 true 반환
        return t > 0;
    }

    vec3 getNormal(const vec3& point) const override {
        return normalize(point - center);
    }

    Material getMaterial() const override {
        return material;
    }
};

// Scene 클래스: 장면을 관리합니다.
class Scene {
public:
    std::vector<Surface*> objects;
    Camera camera;
    vec3 light_pos; // 광원 위치

    Scene(const Camera& camera, const vec3& light_pos) : camera(camera), light_pos(light_pos) {}

    void addObject(Surface* object) {
        objects.push_back(object);
    }

    // 광선 추적 함수
    vec3 trace(const Ray& ray) const {
        float closest_t = INFINITY;
        Surface* closest_surface = nullptr;
        // 모든 객체에 대해 가장 가까운 교차점을 가진 객체를 찾음
        for (Surface* object : objects) {
            float t;
            if (object->intersect(ray, t) && t < closest_t) {
                closest_t = t;
                closest_surface = object;
            }
        }
        // 가장 가까운 교차점을 가진 객체가 있는가
        if (closest_surface) {
            vec3 intersection_point = ray.origin + ray.direction * closest_t; // 교차점 계산
            vec3 normal = closest_surface->getNormal(intersection_point);     // 교차점에서의 법선 벡터 계산
            Material material = closest_surface->getMaterial();             // 교차한 표면의 재질 가져오기
            return phongShading(intersection_point, normal, material);       // Phong 음영 처리 계산
        }
        else {
            return vec3(0.0f, 0.0f, 0.0f); // 검은색 반환 (배경색)
        }
    }

    // Phong 음영 처리 계산 함수
    vec3 phongShading(const vec3& point, const vec3& normal, const Material& material) const {
        vec3 light_dir = normalize(light_pos - point); // 교차점에서 광원으로 향하는 방향 벡터
        vec3 view_dir = normalize(-point);             // 교차점에서 시점으로 향하는 방향 벡터 (카메라가 원점에 있다고 가정)
        vec3 half_vector = normalize(light_dir + view_dir); // Half-vector 계산 (Blinn-Phong 모델)

        // Ambient 성분 계산
        vec3 ambient = material.ka;

        // Diffuse 성분 계산
        float diff = max(dot(normal, light_dir), 0.0f); // 법선 벡터와 광선 방향 벡터의 내적
        vec3 diffuse = material.kd * diff;

        // Specular 성분 계산
        float spec = pow(max(dot(normal, half_vector), 0.0f), material.specular_power); // 법선 벡터와 half-vector의 내적
        vec3 specular = material.ks * spec;

        // 그림자 계산
        Ray shadow_ray(point + normal * 0.001f, light_dir); // Offset to avoid self-intersection
        bool in_shadow = false;
        for (Surface* object : objects) {
            float t;
            if (object->intersect(shadow_ray, t) && t > 0.001f) { // 그림자 광선이 다른 물체와 교차하는지 확인
                in_shadow = true;
                break;
            }
        }

        if (in_shadow) {
            return ambient; // 그림자 영역에서는 ambient 성분만 사용
        }
        else {
            return ambient + diffuse + specular; // 그림자가 아니면 ambient, diffuse, specular 성분 모두 사용
        }
    }
};

// -------------------------------------------------

void render(Scene& scene) {
    OutputImage.clear(); // OutputImage 초기화
    for (int j = 0; j < Height; ++j) {
        for (int i = 0; i < Width; ++i) {
            Ray ray = scene.camera.getRay(i, j); // 카메라의 픽셀 좌표로 광선 생성
            vec3 color = scene.trace(ray);       // 광선 추적

            // 감마 보정 적용
            float gamma = 2.2f;
            color.r = pow(color.r, 1.0f / gamma);
            color.g = pow(color.g, 1.0f / gamma);
            color.b = pow(color.b, 1.0f / gamma);

            OutputImage.push_back(color);       // OutputImage에 색상 추가
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

    Scene scene(camera, vec3(-4.0f, 4.0f, -3.0f)); // 광원 위치
    scene.addObject(new Plane(-2.0f, plane_mat));
    scene.addObject(new Sphere(vec3(-4, 0, -7), 1.0f, sphere1_mat));
    scene.addObject(new Sphere(vec3(0, 0, -7), 2.0f, sphere2_mat));
    scene.addObject(new Sphere(vec3(4, 0, -7), 1.0f, sphere3_mat));

    OutputImage.resize(Width * Height); // OutputImage 크기 설정
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