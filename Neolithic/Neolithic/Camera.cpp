#include "Camera.h"

Camera::Camera() : orientation(1.0f,0.0f,0.0f), 
	UP(0.0f,1.0f,0.0f),orbit(-6.0f,0.0f,0.0f),
	MOUSE_SENSITIVITY(0.25f),accelaration(.1f),inOrbit(false){}

void Camera::mouseUpdate(const vec2& newMousePosition)
{
	vec2 mouseDelta = newMousePosition - oldMousePosition;
	if(GetAsyncKeyState(VK_RBUTTON))
	{
	if(glm::length(mouseDelta) < 200.0f)
	{
		if(orientation.y < -0.95f && mouseDelta.y > 0) mouseDelta.y=0;
		if(orientation.y > 0.95f && mouseDelta.y < 0) mouseDelta.y=0;
		vec3 verticalRotationAxis = glm::cross(orientation,UP);
		mat4 horizontalRotation = glm::rotate(-mouseDelta.x*MOUSE_SENSITIVITY, UP);
		mat4 verticalRotation = glm::rotate(-mouseDelta.y*MOUSE_SENSITIVITY,verticalRotationAxis);
		mat3 netRotation = mat3(horizontalRotation*verticalRotation);
		orientation = netRotation * orientation;
		orbit = netRotation * orbit;
	}
	}
	oldMousePosition = newMousePosition;
}

mat4 Camera::getWorldtoVeiwMatrix() const
{
	return glm::lookAt(position, position+orientation, UP);
}

void Camera::update()
{
	float accel;
	if(GetAsyncKeyState(VK_LBUTTON))accel = accelaration*0.1f;
	else accel = accelaration;
	if (GetAsyncKeyState(Qt::Key::Key_R)) velocity += UP*accel;
	if (GetAsyncKeyState(Qt::Key::Key_F)) velocity -= UP*accel;
	if (GetAsyncKeyState(Qt::Key::Key_W)) velocity += orientation*accel;
	if (GetAsyncKeyState(Qt::Key::Key_S)) velocity -= orientation*accel;
	if (GetAsyncKeyState(Qt::Key::Key_A)) velocity -= glm::normalize(glm::cross(orientation,UP))*accel;
	if (GetAsyncKeyState(Qt::Key::Key_D)) velocity += glm::normalize(glm::cross(orientation,UP))*accel;

	velocity *= .9f; 
	position += velocity;
}