#include "Camera.h"

Camera::Camera()
{
	// initialise camera member variables
	mPosition = glm::vec3(0.0f, 0.0f, 1.0f);
	mLookAt = glm::vec3(0.0f, 0.0f, 0.0f);
	mUp = glm::vec3(0.0f, 1.0f, 0.0f);

	mYaw = 0.0f;
	mPitch = 0.0f;

	mViewMatrix = glm::lookAt(mPosition, mLookAt, mUp);
	mProjectionMatrix = glm::perspective(45.0f, 1.0f, 0.1f, 1000.0f);
}

Camera::~Camera()
{}

void Camera::topDownView(GLFWwindow* window)
{
	// variables to store forward/back and strafe movement
	float moveForward = 0;
	float strafeRight = 0;

	// update variables based on keyboard input
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveForward += MOVEMENT_SENSITIVITY;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveForward -= MOVEMENT_SENSITIVITY;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		strafeRight -= MOVEMENT_SENSITIVITY;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		strafeRight += MOVEMENT_SENSITIVITY;

	// rotate the respective unit vectors about the y-axis
	glm::vec3 rotatedForwardVec = glm::rotateY(glm::vec3(0.0f, 0.0f, -1.0f), mYaw);
	glm::vec3 rotatedRightVec = glm::rotateY(glm::vec3(1.0f, 0.0f, 0.0f), mYaw);
	// rotate the rotated forward vector about the rotated right vector
	rotatedForwardVec = glm::vec3(glm::rotate(mPitch, rotatedRightVec)*glm::vec4(rotatedForwardVec, 0.0f));

	// update position, look-at and up vectors
	mPosition += rotatedForwardVec * moveForward + rotatedRightVec * strafeRight;
	mLookAt = mPosition + rotatedForwardVec;
	mUp = glm::cross(rotatedRightVec, rotatedForwardVec);


	mViewMatrix = glm::lookAt(mPosition, mLookAt, mUp);
	return;
}
void Camera::overView(GLFWwindow* window)
{

}
void Camera::update(GLFWwindow* window)
{
	// variables to store forward/back and strafe movement
	float moveForward = 0;
	float strafeRight = 0;

	// update variables based on keyboard input
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveForward += MOVEMENT_SENSITIVITY;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveForward -= MOVEMENT_SENSITIVITY;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		strafeRight -= MOVEMENT_SENSITIVITY;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		strafeRight += MOVEMENT_SENSITIVITY;

	// rotate the respective unit vectors about the y-axis
	glm::vec3 rotatedForwardVec = glm::rotateY(glm::vec3(0.0f, 0.0f, -1.0f), mYaw);
	glm::vec3 rotatedRightVec = glm::rotateY(glm::vec3(1.0f, 0.0f, 0.0f), mYaw);
	// rotate the rotated forward vector about the rotated right vector
	rotatedForwardVec = glm::vec3(glm::rotate(mPitch, rotatedRightVec)*glm::vec4(rotatedForwardVec, 0.0f));

	// update position, look-at and up vectors
	mPosition += rotatedForwardVec * moveForward + rotatedRightVec * strafeRight;
	mLookAt = mPosition + rotatedForwardVec;
	mUp = glm::cross(rotatedRightVec, rotatedForwardVec);

	// compute the new view matrix
	mViewMatrix = glm::lookAt(mPosition, mLookAt, mUp);
}

void Camera::updateYaw(float yaw)
{
	mYaw -= yaw * ROTATION_SENSITIVITY;
}
void Camera::setYaw(float yaw)
{
	mYaw = yaw;
	return;
}
float Camera::getYaw()
{
	return mYaw;
}
void Camera::setPitch(float Pitch)
{
	mPitch = Pitch;
	return;
}
float Camera::getPitch()
{
	return mPitch;
}
void Camera::updatePitch(float pitch)
{
	mPitch -= pitch * ROTATION_SENSITIVITY;
}

void Camera::setViewMatrix(glm::vec3 position, glm::vec3 lookAt, glm::vec3 up)
{
	mPosition = position;
	mLookAt = lookAt;
	mUp = up;

	mViewMatrix = glm::lookAt(mPosition, mLookAt, mUp);
}
void Camera::setPosition(glm::vec3 position)
{
	mPosition = position;
	return;
}
void Camera::setProjectionMatrix(glm::mat4& matrix)
{
	mProjectionMatrix = matrix;
}

glm::mat4 Camera::getViewMatrix()
{
	return mViewMatrix;
}

glm::mat4 Camera::getProjectionMatrix()
{
	return mProjectionMatrix;
}