#pragma once
#include <glm/glm.hpp>

class Camera
{
private:
    glm::vec4 m_forward;
    glm::vec4 m_right;
    glm::vec4 m_up;
    float m_fovy;
    glm::vec4 m_position;
    float m_nearClip;
    float m_farClip;
    float m_aspectRatio;

public:
    Camera();

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    const glm::vec4& GetForward() const;

    void TranslateForward(float amount);
    void TranslateRight(float amount);
    void TranslateUp(float amount);

    void RotateForward(float degrees);
    void RotateRight(float degrees);
    void RotateUp(float degrees);
};
