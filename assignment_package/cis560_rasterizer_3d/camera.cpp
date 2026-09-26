#include "camera.h"
#include <cmath>

namespace
{
glm::vec4 RotateVector(const glm::vec4& vector,
                       const glm::vec4& axis,
                       float degrees)
{
    // Rodrigues's formula
    const float pi = 3.14159265358979323846f;
    float radians = degrees * pi / 180.f;
    glm::vec3 v(vector);
    glm::vec3 a = glm::normalize(glm::vec3(axis));
    glm::vec3 rotated = v * std::cos(radians)
                      + glm::cross(a, v) * std::sin(radians)
                      + a * glm::dot(a, v) * (1.f - std::cos(radians));
    return glm::vec4(glm::normalize(rotated), 0.f);
}
}

Camera::Camera()
    : m_forward(0.f, 0.f, -1.f, 0.f),
      m_right(1.f, 0.f, 0.f, 0.f),
      m_up(0.f, 1.f, 0.f, 0.f),
      m_fovy(45.f),
      m_position(0.f, 0.f, 10.f, 1.f),
      m_nearClip(0.01f),
      m_farClip(100.f),
      m_aspectRatio(1.f)
{}

glm::mat4 Camera::GetViewMatrix() const
{
    glm::vec3 position(m_position);
    return glm::mat4(
        glm::vec4(m_right.x, m_up.x, m_forward.x, 0.f),
        glm::vec4(m_right.y, m_up.y, m_forward.y, 0.f),
        glm::vec4(m_right.z, m_up.z, m_forward.z, 0.f),
        glm::vec4(-glm::dot(glm::vec3(m_right), position),
                  -glm::dot(glm::vec3(m_up), position),
                  -glm::dot(glm::vec3(m_forward), position),
                  1.f));
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    float scale = 1.f / std::tan(m_fovy / 2.f);
    glm::mat4 projection(0.f);
    projection[0][0] = scale / m_aspectRatio;
    projection[1][1] = scale;
    projection[2][2] = m_farClip / (m_farClip - m_nearClip);
    projection[2][3] = 1.f;
    projection[3][2] = -(m_farClip * m_nearClip)
                     / (m_farClip - m_nearClip);
    return projection;
}

const glm::vec4& Camera::GetForward() const
{
    return m_forward;
}

void Camera::TranslateForward(float amount)
{
    m_position += amount * m_forward;
    m_position.w = 1.f;
}

void Camera::TranslateRight(float amount)
{
    m_position += amount * m_right;
    m_position.w = 1.f;
}

void Camera::TranslateUp(float amount)
{
    m_position += amount * m_up;
    m_position.w = 1.f;
}

void Camera::RotateForward(float degrees)
{
    m_right = RotateVector(m_right, m_forward, degrees);
    m_up = RotateVector(m_up, m_forward, degrees);
}

void Camera::RotateRight(float degrees)
{
    m_forward = RotateVector(m_forward, m_right, degrees);
    m_up = RotateVector(m_up, m_right, degrees);
}

void Camera::RotateUp(float degrees)
{
    m_forward = RotateVector(m_forward, m_up, degrees);
    m_right = RotateVector(m_right, m_up, degrees);
}
