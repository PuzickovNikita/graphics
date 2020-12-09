#include "object.h"
#include <math.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#define M_PI 3.141592653589793 

//СВЕТ
struct Light {
	glm::vec3 position;
	float intensity;
	Light(const glm::vec3& set_position, const float& set_intensity) : position(set_position), intensity(set_intensity) {}
};



//ОБЪЕКТЫ

struct Material {
	glm::vec3 color;

	Material(const glm::vec3& set_color) : color(set_color) {}
};

class Object {
public:
	Material material;

	virtual bool intersect(const glm::vec3& v_origin, const glm::vec3& v_direction, float& t) const {
		std::cout << "Отработала вирутальная функция INTERSECT Object\n" << std::endl;
		return false;
	}

	virtual void get_color (glm::vec3& res, const glm::vec3& hit) const {

		std::cout << "Отработала вирутальная функция GET_COLOR Object\n" << std::endl;
	}

	Object(const glm::vec3& set_color):material(set_color){
	}
};

std::vector< Light* > lights;
std::vector <Object*> scene;

float get_light(const glm::vec3& N, const glm::vec3& hit, int obj_type = 1) {//1 - сфера
	float DLI = 0; // Diffuse Light Intensity								 //2 - плоскость
	for (size_t i = 0; i < lights.size(); i++) {
		glm::vec3 light_dir = glm::normalize((*lights[i]).position - hit);
		if(obj_type == 1)
			DLI += (*lights[i]).intensity * std::max(0.f, dot(light_dir, N));
		else
			DLI += (*lights[i]).intensity * std::max(0.f, abs(dot(light_dir, N)));
	}
	return DLI;
}

class Sphere : public Object {
public:

	glm::vec3 center;
	float r;

	Sphere(const glm::vec3& set_center, const glm::vec3& set_color, const float& radius) :Object( set_color) {
		r = radius;
		center = set_center;
	}

	bool intersect(const glm::vec3& v_origin, const glm::vec3& v_direction, float& t) const override {

		float t_RES; // значение t точки пересечения со сферой

		bool inside = false;
		float r2 = r * r;
		glm::vec3 v_OC = center - v_origin;
		float v_OC_mod = dot(v_OC, v_OC);
		if (v_OC_mod < r2)
			inside = true;
		float t_CTC = dot(v_OC, v_direction); //значение t точки ближней к центру сферы (Closest To Center)
		if (inside && (t_CTC < 0))
			return false;
		float d2 = v_OC_mod - t_CTC * t_CTC;
		if (d2 > r2)
			return false;
		if (inside)
			t_RES = t_CTC + sqrtf(r2 - d2);
		else
			t_RES = t_CTC - sqrtf(r2 - d2);

		t = t_RES;
		return true;
	}

	

	void get_color (glm::vec3& res, const glm::vec3& hit) const override {
		glm::vec3 N = glm::normalize(hit - center);
		res = material.color * get_light(N, hit);
	}
};

class Plane : public Object {
public:
	float r;
	glm::vec3 N;
	
	Plane(const glm::vec3& x1, const glm::vec3& x2,
		const glm::vec3& x3, const glm::vec3 &set_color) : Object(set_color) {
		N = glm::normalize(glm::cross(glm::vec3(x2 - x1),glm::vec3(x3 - x1)));
		r = N.x * x1.x + N.y * x1.y + N.z * x1.z;
		//std::cout << r << std::endl << N.x << ' ' << N.y << ' ' << N.z << std::endl;
	}

	Plane(const glm::vec3& set_N, const float& set_r, const glm::vec3 set_color) : Object(set_color) {
		r = set_r;
		N = glm::normalize(set_N);
		//std::cout << r << std::endl << N.x << ' ' << N.y << ' ' << N.z << std::endl;
	}

	bool intersect(const glm::vec3& v_origin, const glm::vec3& v_direction, float& t) const override {
		float dn = glm::dot(v_direction, N);
		if (dn == 0) return false;
		float on = glm::dot(v_origin, N);
		t = (r - on) / dn;
		if (t < 0) return false;
		return true;
	}

	void get_color(glm::vec3& res,const glm::vec3& hit) const override {
		res = material.color *get_light(N, hit, 2);
	}

};

//СЦЕНА
const int width = 1600, height = 900;
int fov_degree = 60;

size_t do_trace(const glm::vec3& v_origin, const glm::vec3& v_dir, float &t_solv) {
	int nearest = -1;
	t_solv = FLT_MAX; // луч задается как point = v_origin + t * v_dir
							// t_solv -- t, при котором происходит пересечение
	for (size_t i = 0; i < scene.size(); i++) {
		float t_tmp; \
			if ((*scene[i]).intersect(v_origin, v_dir, t_tmp))
				if (t_tmp < t_solv) {
					t_solv = t_tmp;
					nearest = i;
				}
	}

	if (nearest < 0)
		return -1;
	return nearest;
}

void do_light(const glm::vec3 &v_origin, const glm::vec3 &v_dir, const float &t_solv,
	Object* obj,
	glm::vec3 &color_res){

	glm::vec3 hit = v_origin + t_solv * v_dir; // координаты точки пересечения

	(*obj).get_color(color_res, hit);
}

int main() {
	//задаем сцену
		//ОБЪЕКТЫ
	Sphere s1 = Sphere(glm::vec3(0	, 50, -200), glm::vec3(1, 0.078, 0.57), 30);
	//Sphere s2 = Sphere(glm::vec3(-150, -50, -400), glm::vec3(1, 0.85, 0), 60);
	//пол
	Plane p1 = Plane(glm::vec3(0, -100, 0), glm::vec3(100, -100, 0), glm::vec3(0, -100, 100),
		glm::vec3((float)135 / 255, (float)206 / 255, (float)250 / 255));
	//потолок
	Plane p2 = Plane(glm::vec3(0, 200, 0), glm::vec3(100, 200, 0), glm::vec3(0, 200, 100),
		glm::vec3((float)135 / 255, (float)206 / 255, (float)250 / 255));
	//задняя стена
	Plane p3 = Plane(glm::vec3(0, -100, -700), glm::vec3(100, 0, -700), glm::vec3(0, 0, -700),
		glm::vec3((float)189 / 255, (float)183 / 255, (float)107 / 255));
	//передняя стена
	Plane p4 = Plane(glm::vec3(0, -10, 10),  glm::vec3(0, 0, 10),  glm::vec3(10, 0, 10), 
		glm::vec3(0, 1, 0));
	//правая стена
	Plane p5 = Plane(glm::vec3(350, 0, 0), glm::vec3(350, 0, 100), glm::vec3(350, 100, 0), 
		glm::vec3((float)238/255, (float)232/255, (float)170/255));
	//левая стена
	Plane p6 = Plane(glm::vec3(-450, 0, 0), glm::vec3(-450, 0, 100), glm::vec3(-450, 100, 0),
		glm::vec3((float)238 / 255, (float)232 / 255, (float)170 / 255));
	scene.push_back(&p1);
	scene.push_back(&p2);
	scene.push_back(&p3);
	scene.push_back(&p4);
	scene.push_back(&p5);
	scene.push_back(&p6);
	
	scene.push_back(&s1);
	//scene.push_back(&s2
	//СВЕТ
	Light l1 = Light(glm::vec3(100, 150, -50), 1.0);
	lights.push_back(&l1);
	//____________________________

	float fov = (float)(M_PI * fov_degree) / 180;
	float aspect_ratio = width / (float)height;
	float invW = 1 / (float)width;
	float invH = 1 / (float)height;
	glm::vec3* frame = new glm::vec3[width * height];
	glm::vec3* pixel = frame;

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++, pixel++) {
			float xx = (2 * (x + 0.5) * invW - 1) * tan(fov / 2.) * aspect_ratio;
			float yy = -(2 * (y + 0.5) * invH - 1) * tan(fov / 2.);
			glm::vec3 direction = glm::normalize(glm::vec3(xx, yy, -1));
			*pixel = glm::vec3(0);

			float t_solv;
			size_t i = do_trace(glm::vec3(0), direction, t_solv);
			do_light(glm::vec3(0), direction, t_solv, scene[i], *pixel);



		}

	//вывод кадра
	std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
	ofs << "P6\n" << width << " " << height << "\n255\n";
	for (unsigned i = 0; i < height * width; ++i) {
		ofs << (unsigned char)(255 * std::max(0.f, std::min(1.f, frame[i].x)));
		ofs << (unsigned char)(255 * std::max(0.f, std::min(1.f, frame[i].y)));
		ofs << (unsigned char)(255 * std::max(0.f, std::min(1.f, frame[i].z)));
	}
	ofs.close();
	//for (int i = 0; i < n; i++)
	//	delete scene[i];
	delete[] frame;

}