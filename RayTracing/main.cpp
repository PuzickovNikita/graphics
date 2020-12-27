#include <math.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <time.h>

#define M_PI 3.141592653589793 

struct Light {
	glm::vec3 position;
	float intensity;
	float r;
	Light(const glm::vec3& set_position, const float& set_r, const float& set_intensity) :
		position(set_position), r(set_r), intensity(set_intensity) {}

	bool intersect(const glm::vec3& v_origin, const glm::vec3& v_direction,
		float& t) const {

		float t_RES; // значение t точки пересечения со сферой

		bool inside = false;
		float r2 = r * r;
		glm::vec3 v_OC = position - v_origin;
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

};

struct Material {
	glm::vec3 color;
	float difusal_factor;
	float specular_factor;
	float specular_exponent;
	float reflection_factor;


	Material(const glm::vec3& set_color, const float& set_difusal_factor, const float& set_specular_factor,
		const float& set_specular_exponent, const float& set_reflection_factor) :
		color(set_color), difusal_factor(set_difusal_factor), specular_factor(set_specular_factor),
		specular_exponent(set_specular_exponent), reflection_factor(set_reflection_factor) {
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

	Object(const glm::vec3& set_color, const float& set_difusal_factor, const float& set_specular_factor,
		const float& set_specular_exponent, const float& set_reflection_factor, const int& object_type)
		:material(set_color, set_difusal_factor, set_specular_factor, set_specular_exponent, set_reflection_factor),
		obj_type(object_type) {
	}
};

std::vector< Light* > lights;
std::vector <Object*> scene;

class Sphere : public Object {
public:
	glm::vec3 center;
	float r;

	Sphere(const glm::vec3& set_center, const float& radius,
		const glm::vec3& set_color, const float& set_difusal_factor, const float& set_specular_factor,
		const float& set_specular_exponent, const float& set_reflection_factor)
		:Object(set_color, set_difusal_factor, set_specular_factor,
			set_specular_exponent, set_reflection_factor, 1) {
		r = radius;
		center = set_center;
	}

	bool intersect(const glm::vec3& v_origin, const glm::vec3& v_direction,
		float& t) const override {

		float t_RES;  // значение t точки пересечения со сферой

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

	Plane(const glm::vec3& x1, const glm::vec3& x2, const glm::vec3& x3,
		const glm::vec3& set_color, const float& set_difusal_factor, const float& set_specular_factor,
		const float& set_specular_exponent, const float& set_reflection_factor)
		: Object(set_color, set_difusal_factor, set_specular_factor, set_specular_exponent,
			set_reflection_factor, 2) {
		N = glm::normalize(glm::cross(glm::vec3(x2 - x1), glm::vec3(x3 - x1)));
		r = N.x * x1.x + N.y * x1.y + N.z * x1.z;
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

/*
Что делает trace_ray
1)находит ближайший объект, который пересекает луч
	1.2)првоеряем, что ближайший объект это не источник света
2)считает отражение
3)считает отсвещение
	3.1) проверяем какие источники света видны из точки
	3.2) расчитываем для этих источников дифузную и отраженную компоненты освещения
*/

void trace_ray(const glm::vec3& v_orig, const glm::vec3& v_dir,
	glm::vec3& color_res, const int& ray_reflect_lef) {

	color_res.x = 0; color_res.y = 0; color_res.z = 0;

	//1)
	int nearest = -1;
	float t_solv = FLT_MAX;
	for (size_t i = 0; i < scene.size(); i++) {
		float t_tmp;
		if ((*scene[i]).intersect(v_orig, v_dir, t_tmp))
			if (t_tmp < t_solv) {
				t_solv = t_tmp;
				nearest = i;
			}
	}
	//1.2)
	int nearest_light = -1;
	float t_solv_light = FLT_MAX;
	for (size_t i = 0; i < lights.size(); i++) {
		float t_tmp;
		if (lights[i]->intersect(v_orig, v_dir, t_tmp))
			if (t_tmp < t_solv_light) {
				t_solv_light = t_tmp;
				nearest_light = i;
			}
	}
	if (t_solv_light < t_solv) {
		color_res = glm::vec3(1);
		return;
	}

	if (nearest < 0)
		return;
	glm::vec3 hit = v_orig + v_dir * t_solv;
	glm::vec3 N; scene[nearest]->set_N(N, hit);
	float difusal_factor = scene[nearest]->material.difusal_factor;
	float specular_factor = scene[nearest]->material.specular_factor;
	float specular_exponent = scene[nearest]->material.specular_exponent;

	//2)
	glm::vec3 reflect_color(0);
	if (ray_reflect_lef > 0) {
		glm::vec3 reflect_dir = v_dir - 2.f * glm::dot(v_dir, N) * N;
		glm::vec3 reflect_orig = glm::dot(reflect_dir, N) < 0
			? hit - N * (float)(1e-3)
			: hit + N * (float)(1e-3);
		trace_ray(reflect_orig, reflect_dir, reflect_color, ray_reflect_lef - 1);
	}

	//3)

	float DLI = 0; // Diffuse Light Intensity
	float SLI = 0; //Specular Light Intensity
	for (size_t i = 0; i < lights.size(); i++) {
		glm::vec3 light_dir = (lights[i]->position - hit);
		float light_distance = glm::length(light_dir);
		light_dir = glm::normalize(light_dir);
		//3.1)
		bool light_saw = true;
		glm::vec3 shadow_orig = glm::dot(light_dir, N) < 0
			? hit - N * (float)(1e-3)
			: hit + N * (float)(1e-3);
		for (size_t j = 0; j < scene.size(); j++) {
			float t;
			if (scene[j]->intersect(shadow_orig, light_dir, t))
				if (t <= light_distance) {
					light_saw = false;
					break;
				}
		}

		//если не в тени добавляем освещение
		if (light_saw) {//Блинн-Фонг
			if (scene[nearest]->obj_type == 1) {
				if (difusal_factor != 0)
					DLI += (*lights[i]).intensity *
					std::max(0.f, glm::dot(light_dir, N));
				if ((specular_factor != 0) && (specular_exponent != 0))
				{
					glm::vec3 H = (glm::normalize(-v_dir + light_dir));//Не очень понял почему знаки так, но работает же)
					SLI += (lights[i]->intensity) *
						pow(std::max(0.f, glm::dot(H, N)), specular_exponent);
				}
			}
			else { //в плоскости возможно надо повернуть нормаль
				if (difusal_factor != 0)
					DLI += (*lights[i]).intensity *
					std::max(0.f, glm::abs(glm::dot(light_dir, N)));
				if ((specular_factor != 0) && (specular_exponent != 0))
				{
					glm::vec3 H = (glm::normalize(-v_dir + light_dir));//Не очень понял почему знаки так, но работает же)
					SLI += (lights[i]->intensity) *
						pow(std::max(0.f, glm::abs(glm::dot(H, N))), specular_exponent);
				}
			}
		}
	}
	color_res = scene[nearest]->material.color * DLI * difusal_factor +
		glm::vec3(1) * SLI * specular_factor
		+ reflect_color * scene[nearest]->material.reflection_factor;

	//color_res = glm::vec3(1) * SLI * specular_factor;
}


int main() {

	const int width = 640, height = 360;
	//const int width = 960, height = 540;
	//const int width = 1600, height = 900;
	const int fov_degree = 60;

	//задаем сцену
		//ОБЪЕКТЫ
#define material1 glm::vec3(1, 0.85, 0), 0.6, 0.4 , 55.0
#define material2 glm::vec3(1, 0.078, 0.57), 0.9, 0.7, 70.0
	// Сфера( вектор-центр, радиус, вектор-RGB, диффузный множитель, зеркальный множитель, зеркальная экспонента)
	// зеркальный множитель - яркость бликов
	// зеркальная экспонента - обратный радиус блика

	Sphere s3(glm::vec3(-3, 0, -16), 2, material1, 0);
	scene.push_back(&s3);
	Sphere s4(glm::vec3(-1, -1.5, -12), 2, material2, 0.7);
	scene.push_back(&s4);
	Sphere s5(glm::vec3(1.5, -0.5, -18), 3, material2, 0);
	scene.push_back(&s5);
	Sphere s6(glm::vec3(7, 5, -18), 4, material1, 0.8);
	scene.push_back(&s6);

	int right = 40;
	int left = -40;
	int top = 60;
	int bot = -5;
	int front = -30;
	int back = 40;
	//TODO заменить плоскости прямоугольниками, убрать потолок и/или заднюю стенку
	//TODO и настроить получше сцену, но сначала посмотреть на сцену в динамике
	// Плоскость по трем точкам( Х1, Х2, Х2
	//	вектор-RGB, дифузный множитель, зеркальный множитель, зеркальная степень)
	//пол
	Plane p1 = Plane(glm::vec3(0, bot, 0), glm::vec3(100, bot, 0), glm::vec3(0, bot, 100),
		glm::vec3((float)255 / 255, (float)239 / 255, (float)213 / 255), 1.0, 0, 1.0, 0);
	//потолок
	Plane p2 = Plane(glm::vec3(0, top, 0), glm::vec3(100, top, 0), glm::vec3(0, top, 100),
		glm::vec3((float)255 / 255, (float)239 / 255, (float)213 / 255), 1.0, 0, 1.0, 0);
	//задняя стена
	Plane p3 = Plane(glm::vec3(0, -100, back), glm::vec3(100, 0, back), glm::vec3(0, 0, back),
		glm::vec3((float)255 / 255, (float)239 / 255, (float)213 / 255), 1.0, 0, 1.0, 0);
	//передняя стена
	Plane p4 = Plane(glm::vec3(0, -100, front), glm::vec3(0, 0, front), glm::vec3(100, 0, front),
		glm::vec3((float)255 / 255, (float)239 / 255, (float)213 / 255), 1.0, 0, 1.0, 0);
	//правая стена
	Plane p5 = Plane(glm::vec3(right, 0, 0), glm::vec3(right, 0, 100), glm::vec3(right, 100, 0),
		glm::vec3((float)255 / 255, (float)100 / 255, (float)100 / 255), 1.0, 0, 1.0, 0);
	//левая стена
	Plane p6 = Plane(glm::vec3(left, 0, 0), glm::vec3(left, 0, 100), glm::vec3(left, 100, 0),
		glm::vec3((float)100 / 255, (float)100 / 255, (float)225 / 255), 1.0, 0, 1.0, 0);
	scene.push_back(&p1);
	scene.push_back(&p2);
	scene.push_back(&p3);
	scene.push_back(&p4);
	scene.push_back(&p5);
	scene.push_back(&p6);

	//СВЕТ

	Light l1 = Light(glm::vec3(-20, 20, 20), 3, 0.25);
	lights.push_back(&l1);
	Light l2 = Light(glm::vec3(30, 50, -25), 3, 0.5);
	lights.push_back(&l2);
	Light l3 = Light(glm::vec3(30, 20, 30), 3, 0.25);
	lights.push_back(&l3);
	float max = 0;
	for (size_t i = 0; i < lights.size(); i++)
		max += lights[i]->intensity;
	for (size_t i = 0; i < lights.size(); i++)
		lights[i]->intensity *= (float)1 / max;
	//____________________________

	float fov = (float)(M_PI * fov_degree) / 180;
	float aspect_ratio = width / (float)height;
	float invW = 1 / (float)width;
	float invH = 1 / (float)height;
	glm::vec3* frame = new glm::vec3[width * height];
	glm::vec3* pixel = frame;

	int ray_reflect_max = 2;

#define one_second 1000
	clock_t time_passed = 0;
	clock_t time_next_sec = 0;
	std::cout.precision(3);

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++, pixel++) {
			float xx = (2 * (x + 0.5) * invW - 1) * tan(fov / 2.) * aspect_ratio;
			float yy = -(2 * (y + 0.5) * invH - 1) * tan(fov / 2.);
			glm::vec3 direction = glm::normalize(glm::vec3(xx, yy, -1));
			*pixel = glm::vec3(0);
			trace_ray(glm::vec3(0), direction, *pixel, ray_reflect_max);
			time_passed = clock();
			if (time_passed >= time_next_sec) {
				time_next_sec += 1 * one_second;
				std::cout << "frame " << (y * width + x) / (float)(height * width) * 100 << "%" << std::endl;
			}

		}
	std::cout << "frame is rendered in " << time_passed / one_second
		<< " seconds" << std::endl;

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