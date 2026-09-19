#pragma once
#include <polygon.h>
#include "camera.h"
#include "segment.h"
#include <QImage>
#include <array>

glm::vec3 BarycentricInterpolation(const std::array<glm::vec4, 3>& vertices,
                                   const glm::vec2& point);
float PerspectiveCorrectZ(const std::array<glm::vec4, 3>& vertices,
                          const glm::vec2& point);
glm::vec2 PerspectiveCorrectUV(const std::array<glm::vec4, 3>& vertices,
                               const std::array<glm::vec2, 3>& uvs,
                               const glm::vec2& point,
                               float fragmentZ);
glm::vec3 PerspectiveCorrectColor(const std::array<glm::vec4, 3>& vertices,
                                  const std::array<glm::vec3, 3>& colors,
                                  const glm::vec2& point,
                                  float fragmentZ);
glm::vec4 PerspectiveCorrectNormal(const std::array<glm::vec4, 3>& vertices,
                                   const std::array<glm::vec4, 3>& normals,
                                   const glm::vec2& point,
                                   float fragmentZ);

class Rasterizer
{
private:
    //This is the set of Polygons loaded from a JSON scene file
    std::vector<Polygon> m_polygons;
public:
    Camera m_camera;
    Rasterizer(const std::vector<Polygon>& polygons);
    QImage RenderScene() const;
    void ClearScene();
};
