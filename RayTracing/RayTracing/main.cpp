#include <math.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#define M_PI 3.141592653589793 

struct Light {
	glm::vec3 position;
	float intensity;
	Light(const glm::vec3& set_position, const float& set_intensity) : position(set_position), intensity(set_intensity) {}
};

struct Material {
	glm::vec3 color;

	Material(const glm::vec3& set_color) : color(set_color) {
		color = set_color;
	}
};

class Object {
public:
	Material material;
	int obj_type; //1-сфера
				  //2-плоскость

	virtual bool intersect(const glm::vec3& v_origin, const glm::vec3& v_direction,
		float& t) const {
		std::cout << "Отработала вирутальная функция INTERSECT Object\n" << std::endl;
		return false;
	}

	virtual void get_color(glm::vec3& res, const glm::vec3& hit) const {

		std::cout << "Отработала вирутальная функция GET_COLOR Object\n" << std::endl;
	}

	virtual void set_N(glm::vec3& N_res, glm::vec3& hit) const {}

	Object(const glm::vec3& set_color, int object_type) :material(set_color), obj_type(object_type) {
	}
};

std::vector< Light* > lights;
std::vector <Object*> scene;



class Sphere : public Object {
public:
	glm::vec3 center;
	float r;

	Sphere(const glm::vec3& set_center, const glm::vec3& set_color, const float& radius) :Object(set_color, 1) {
		r = radius;
		center = set_center;
	}

	bool intersect(const glm::vec3& v_origin, const glm::vec3& v_direction,
		float& t) const override {

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
		if (t_RES <= 0)
			return false;
		//TODO:ПРОЧЕКАТЬ АЛГОРИТМ ЧТОБЫ ПОНЯТЬ КОСТЫЛЬ ВЕРХНИЕ ДВЕ СТРОЧКИ ИЛИ НЕТ 
		t = t_RES;
		return true;
	}

	void set_N(glm::vec3& N_res, glm::vec3& hit) const override {
		N_res = glm::normalize(hit - center);
	}
};

class Plane : public Object {
public:
	float r;
	glm::vec3 N;

	Plane(const glm::vec3& x1, const glm::vec3& x2,
		const glm::vec3& x3, const glm::vec3& set_color) : Object(set_color, 2) {
		N = glm::normalize(glm::cross(glm::vec3(x2 - x1), glm::vec3(x3 - x1)));
		r = N.x * x1.x + N.y * x1.y + N.z * x1.z;
		//std::cout << r << std::endl << N.x << ' ' << N.y << ' ' << N.z << std::endl;
	}

	Plane(const glm::vec3& set_N, const float& set_r, const glm::vec3 set_color) : Object(set_color, 2) {
		r = set_r;
		N = glm::normalize(set_N);
		//std::cout << r << std::endl << N.x << ' ' << N.y << ' ' << N.z << std::endl;
	}

	bool intersect(const glm::vec3& v_origin, const glm::vec3& v_direction,
		float& t) const override {
		float dn = glm::dot(v_direction, N);
		if (dn == 0) return false;
		float on = glm::dot(v_origin, N);
		t = (r - on) / dn;
		if (t < 0) return false;
		return true;
	}

	void set_N(glm::vec3& N_res, glm::vec3& hit) const override {
		N_res = N;
	}
};


void trace_ray(const glm::vec3& v_orig,const glm::vec3& v_dir,
	glm::vec3& color_res) {
	int nearest = -1;
	float t_solv = FLT_MAX;
	color_res.x = 0; color_res.y = 0; color_res.z = 0;

	for (size_t i = 0; i < scene.size(); i++) {
		float t_tmp; glm::vec3 N_tmp;
		if ((*scene[i]).intersect(v_orig, v_dir, t_tmp))
			if (t_tmp < t_solv) {
				t_solv = t_tmp;
				nearest = i;
			}
	}

	if (nearest < 0)
		return;

	glm::vec3 hit = v_orig + v_dir * t_solv;
	glm::vec3 N; scene[nearest]->set_N(N, hit);
	//ОСВЕЩЕНИЕ

	float DLI = 0; // Diffuse Light Intensity
	for (size_t i = 0; i < lights.size(); i++) {
		glm::vec3 light_dir = (lights[i]->position - hit);
		float light_distance = glm::length(light_dir);
		light_dir = glm::normalize(light_dir);


		//Тень
		bool light_saw = true;
		glm::vec3 shadow_orig = glm::dot(light_dir, N) < 0
			? hit - N * (float)(1e-3)
			: hit + N * (float)(1e-3);
		for (size_t j = 0; j < scene.size(); j++) {
			float t;
			if (scene[j]->intersect(shadow_orig, light_dir, t))
				if (t <= light_distance){
					light_saw = false;
					break;
				}
		}

		//если не в тени добавляем освещение
		if (light_saw) {
			if (scene[nearest]->obj_type == 1)
				DLI += (*lights[i]).intensity *
				std::max(0.f, glm::dot(light_dir, N));
			else //в плоскости возможно надо повернуть нормаль
				DLI += (*lights[i]).intensity *
				std::max(0.f, glm::abs(glm::dot(light_dir, N)));
		}
	}
	color_res = scene[nearest]->material.color*DLI;
}


int main() {

	const int width = 960, height = 540;
	const int fov_degree = 60;
	
	int right = 100;
	int left = -100;
	int top = 100;
	int bot = -20;
	int front = -200;
	int back = 40;
	//задаем сцену
		//ОБЪЕКТЫ
	glm::vec3 v1(10, 15, 23);
	Sphere s1 = Sphere(glm::vec3(-20, 20, -100), glm::vec3(1, 0.078, 0.57), 10);
	Sphere s2 = Sphere(glm::vec3(0, 40, -90), glm::vec3(1, 1, 1), 5);
	//Sphere s2 = Sphere(glm::vec3(-150, -50, -400), glm::vec3(1, 0.85, 0), 60);
	scene.push_back(&s1);
	scene.push_back(&s2);

	//пол
	Plane p1 = Plane(glm::vec3(0, bot, 0), glm::vec3(100, bot, 0), glm::vec3(0, bot, 100),
		glm::vec3((float)255 / 255, (float)239 / 255, (float)213 / 255));
	//потолок
	Plane p2 = Plane(glm::vec3(0, top, 0), glm::vec3(100, top, 0), glm::vec3(0, top, 100),
		glm::vec3((float)255 / 255, (float)239 / 255, (float)213 / 255));
	//задняя стена
	Plane p3 = Plane(glm::vec3(0, -100, back), glm::vec3(100, 0, back), glm::vec3(0, 0, back),
		glm::vec3((float)255 / 255, (float)239 / 255, (float)213 / 255));
	//передняя стена
	Plane p4 = Plane(glm::vec3(0, -100, front), glm::vec3(0, 0, front), glm::vec3(100, 0, front),
		glm::vec3((float)255 / 255, (float)239 / 255, (float)213 / 255));
	//правая стена
	Plane p5 = Plane(glm::vec3(right, 0, 0), glm::vec3(right, 0, 100), glm::vec3(right, 100, 0),
		glm::vec3((float)255 / 255, (float)100 / 255, (float)100 / 255));
	//левая стена
	Plane p6 = Plane(glm::vec3(left, 0, 0), glm::vec3(left, 0, 100), glm::vec3(left, 100, 0),
		glm::vec3((float)100 / 255, (float)100 / 255, (float)225 / 255));
	scene.push_back(&p1);
	scene.push_back(&p2);
	scene.push_back(&p3);
	scene.push_back(&p4);
	scene.push_back(&p5);
	scene.push_back(&p6);

	//СВЕТ
	Light l1 = Light(glm::vec3(0, 95, -120), 0.25);
	Light l2 = Light(glm::vec3(-20, 95, -80), 0.25);
	Light l3 = Light(glm::vec3(20, 95, -80), 0.25);
	Light l4 = Light(glm::vec3(0, 95, 0), 0.25);
	lights.push_back(&l1);
	lights.push_back(&l2);
	lights.push_back(&l3);
	lights.push_back(&l4);
	//TODO: нормировать интенсивность источников света, чтобы суммарно была 1
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
			trace_ray(glm::vec3(0), direction, *pixel);
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

	return 0;
}