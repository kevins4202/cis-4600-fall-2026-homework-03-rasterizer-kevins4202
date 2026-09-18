#pragma once
#include <polygon.h>
#include "segment.h"
#include <QImage>
#include <array>

glm::vec3 BarycentricInterpolation(const std::array<glm::vec4, 3>& vertices,
                                   const glm::vec2& point);

class Rasterizer
{
private:
    //This is the set of Polygons loaded from a JSON scene file
    std::vector<Polygon> m_polygons;
public:
    Rasterizer(const std::vector<Polygon>& polygons);
    QImage RenderScene();
    void ClearScene();
};
