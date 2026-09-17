#include "segment.h"

Segment::Segment(const glm::vec2& p1, const glm::vec2& p2)
    : m_p1(p1), m_p2(p2), m_dx(p2.x - p1.x), m_dy(p2.y - p1.y)
{}

bool Segment::getIntersection(int y, float* x) const
{
    float row = static_cast<float>(y);
    if(x == nullptr || row < glm::min(m_p1.y, m_p2.y) || row > glm::max(m_p1.y, m_p2.y))
    {
        return false;
    }

    if(m_dy == 0.f)
    {
        *x = glm::min(m_p1.x, m_p2.x);
    }
    else if(m_dx == 0.f)
    {
        *x = m_p1.x;
    }
    else
    {
        *x = m_p1.x + (row - m_p1.y) * m_dx / m_dy;
    }
    return true;
}
