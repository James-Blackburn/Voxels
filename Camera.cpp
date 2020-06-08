#include "Camera.h"

Camera::Camera(const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
			float cameraYaw, float cameraPitch, float cameraMovementSpeed, float cameraTurnSpeed)
{
	pos = cameraPos;
	front = cameraFront;
	up = cameraUp;

	yaw = cameraYaw;
	pitch = cameraPitch;
	movementSpeed = cameraMovementSpeed;
	turnSpeed = cameraTurnSpeed;

	update();
}

void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);
}

void Camera::moveCamera(bool* keyStates, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;
	if (keyStates[GLFW_KEY_W])
		pos += front * velocity;
	if (keyStates[GLFW_KEY_S])
		pos -= front * velocity;
	if (keyStates[GLFW_KEY_D])
		pos += glm::normalize(glm::cross(front, up)) * velocity;
	if (keyStates[GLFW_KEY_A])
		pos -= glm::normalize(glm::cross(front, up)) * velocity;
	if (keyStates[GLFW_KEY_SPACE])
		pos.y += velocity;
	if (keyStates[GLFW_KEY_LEFT_CONTROL])
		pos.y -= velocity;
}

void Camera::mouseMovement(float deltaX, float deltaY)
{
	yaw += deltaX * turnSpeed;
	pitch += deltaY * turnSpeed;

	if (pitch > 89.0f)
		pitch = 89.0f;
	else if (pitch < -89.0f)
		pitch = -89.0f;

	update();
}

void Camera::setPosition(float x, float y, float z)
{
	pos = glm::vec3(x, y, z);
}
