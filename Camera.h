#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

class Camera
{
public:
	Camera() : yaw(90.0f), pitch(0.0f), movementSpeed(10.0f), turnSpeed(0.05f) { update(); }
	Camera(const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp, 
		float cameraYaw, float cameraPitch, float cameraMovementSpeed, float cameraTurnSpeed);
	~Camera() = default;

	void update();
	void moveCamera(bool* keyStates, float deltaTime);
	void mouseMovement(float deltaX, float deltaY);
	void setPosition(float x, float y, float z);
	
	inline glm::mat4 calculateViewMatrix() { return glm::lookAt(pos, pos + front, up); }
	inline glm::vec3 getCameraPosition() { return pos; }

private:
	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 up;

	float yaw;
	float pitch;
	float movementSpeed;
	float turnSpeed;
};

