#pragma once
#include <glm/glm.hpp>

class Segment
{
private:
    glm::vec2 m_p1;
    glm::vec2 m_p2;
    float m_dx;
    float m_dy;
public:
    Segment(const glm::vec2& p1, const glm::vec2& p2);
    bool getIntersection(int y, float* x) const;
};
