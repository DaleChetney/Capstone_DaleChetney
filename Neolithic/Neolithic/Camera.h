#pragma once
#include <QtOpenGL\qglwidget>
#include <glm\glm.hpp>
using glm::vec3;
using glm::mat4;
using glm::vec2;
using glm::mat3;
#include <glm\gtx\transform.hpp>

class Camera
{
	const vec3 UP;
	vec2 oldMousePosition;
	const float MOUSE_SENSITIVITY;
	vec3 velocity;
	const float accelaration;
public:
	vec3 position;
	vec3 orientation;
	vec3 orbit;
	bool inOrbit;
	Camera(void);
	void mouseUpdate(const vec2& newMousePosition);
	void update();
	mat4 getWorldtoVeiwMatrix() const;
};