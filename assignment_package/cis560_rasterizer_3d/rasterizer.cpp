#include "rasterizer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>

glm::vec3 BarycentricInterpolation(const std::array<glm::vec4, 3>& vertices,
                                   const glm::vec2& point)
{
    glm::vec3 a(vertices[0].x, vertices[0].y, 0.f);
    glm::vec3 b(vertices[1].x, vertices[1].y, 0.f);
    glm::vec3 c(vertices[2].x, vertices[2].y, 0.f);
    glm::vec3 p(point.x, point.y, 0.f);
    float area = glm::cross(b - a, c - a).z;

    if(area == 0.f)
    {
        return glm::vec3(0.f);
    }

    return glm::vec3(glm::cross(b - p, c - p).z / area,
                     glm::cross(c - p, a - p).z / area,
                     glm::cross(a - p, b - p).z / area);
}

float PerspectiveCorrectZ(const std::array<glm::vec4, 3>& vertices,
                          const glm::vec2& point)
{
    glm::vec3 weights = BarycentricInterpolation(vertices, point);
    return 1.f / (weights[0] / vertices[0].z
                + weights[1] / vertices[1].z
                + weights[2] / vertices[2].z);
}

glm::vec2 PerspectiveCorrectUV(const std::array<glm::vec4, 3>& vertices,
                               const std::array<glm::vec2, 3>& uvs,
                               const glm::vec2& point,
                               float fragmentZ)
{
    glm::vec3 weights = BarycentricInterpolation(vertices, point);
    return fragmentZ * (weights[0] * uvs[0] / vertices[0].z
                      + weights[1] * uvs[1] / vertices[1].z
                      + weights[2] * uvs[2] / vertices[2].z);
}

glm::vec3 PerspectiveCorrectColor(const std::array<glm::vec4, 3>& vertices,
                                  const std::array<glm::vec3, 3>& colors,
                                  const glm::vec2& point,
                                  float fragmentZ)
{
    glm::vec3 weights = BarycentricInterpolation(vertices, point);
    return fragmentZ * (weights[0] * colors[0] / vertices[0].z
                      + weights[1] * colors[1] / vertices[1].z
                      + weights[2] * colors[2] / vertices[2].z);
}

glm::vec4 PerspectiveCorrectNormal(const std::array<glm::vec4, 3>& vertices,
                                   const std::array<glm::vec4, 3>& normals,
                                   const glm::vec2& point,
                                   float fragmentZ)
{
    glm::vec3 weights = BarycentricInterpolation(vertices, point);
    glm::vec4 normal = fragmentZ
                     * (weights[0] * normals[0] / vertices[0].z
                      + weights[1] * normals[1] / vertices[1].z
                      + weights[2] * normals[2] / vertices[2].z);
    return glm::vec4(glm::normalize(glm::vec3(normal)), 0.f);
}

Rasterizer::Rasterizer(const std::vector<Polygon>& polygons)
    : m_polygons(polygons)
{}

QImage Rasterizer::RenderScene() const
{
    QImage result(512, 512, QImage::Format_RGB32);
    // Fill the image with black pixels.
    // Note that qRgb creates a QColor,
    // and takes in values [0, 255] rather than [0, 1].
    result.fill(qRgb(0.f, 0.f, 0.f));
    std::vector<float> depthBuffer(result.width() * result.height(),
                                   std::numeric_limits<float>::infinity());
    glm::mat4 viewMatrix = m_camera.GetViewMatrix();
    glm::mat4 projectionMatrix = m_camera.GetProjectionMatrix();
    glm::vec3 lightDirection = glm::normalize(-glm::vec3(m_camera.GetForward()));

    for(const Polygon& polygon : m_polygons)
    {
        bool is3D = polygon.m_is3D;
        for(const Triangle& triangle : polygon.m_tris)
        {
            std::array<glm::vec4, 3> vertices;
            std::array<glm::vec3, 3> colors;
            std::array<glm::vec2, 3> uvs;
            std::array<glm::vec4, 3> normals;
            bool behindCamera = false;
            for(unsigned int i = 0; i < vertices.size(); i++)
            {
                const Vertex& vertex = polygon.m_verts[triangle.m_indices[i]];
                colors[i] = vertex.m_color;
                uvs[i] = vertex.m_uv;
                normals[i] = vertex.m_normal;

                if(is3D)
                {
                    glm::vec4 cameraPosition = viewMatrix * vertex.m_pos;
                    if(cameraPosition.z <= 0.f)
                    {
                        behindCamera = true;
                        break;
                    }

                    glm::vec4 clipPosition = projectionMatrix * cameraPosition;
                    glm::vec4 screenPosition = clipPosition / clipPosition.w;
                    vertices[i] = glm::vec4(
                        (screenPosition.x + 1.f) * 0.5f * result.width(),
                        (1.f - screenPosition.y) * 0.5f * result.height(),
                        cameraPosition.z,
                        1.f);
                }
                else
                {
                    vertices[i] = vertex.m_pos;
                }
            }

            if(behindCamera)
            {
                continue;
            }

            std::array<Segment, 3> segments = {{
                Segment(glm::vec2(vertices[0]), glm::vec2(vertices[1])),
                Segment(glm::vec2(vertices[1]), glm::vec2(vertices[2])),
                Segment(glm::vec2(vertices[2]), glm::vec2(vertices[0]))
            }};

            float yMin = glm::min(vertices[0].y, glm::min(vertices[1].y, vertices[2].y));
            float yMax = glm::max(vertices[0].y, glm::max(vertices[1].y, vertices[2].y));
            int firstRow = glm::max(0, static_cast<int>(std::ceil(yMin)));
            int lastRow = glm::min(result.height() - 1, static_cast<int>(std::floor(yMax)));

            for(int y = firstRow; y <= lastRow; y++)
            {
                float xLeft = static_cast<float>(result.width());
                float xRight = 0.f;
                bool foundIntersection = false;

                for(const Segment& segment : segments)
                {
                    float xIntersection;
                    if(segment.getIntersection(y, &xIntersection))
                    {
                        if(!foundIntersection)
                        {
                            xLeft = xIntersection;
                            xRight = xIntersection;
                            foundIntersection = true;
                        }
                        else
                        {
                            xLeft = glm::min(xLeft, xIntersection);
                            xRight = glm::max(xRight, xIntersection);
                        }
                    }
                }

                if(!foundIntersection || xRight < 0.f || xLeft >= result.width())
                {
                    continue;
                }

                float firstColumn = glm::max(0.f, std::ceil(xLeft));
                float lastColumn = glm::min(static_cast<float>(result.width() - 1), std::floor(xRight));
                for(float x = firstColumn; x <= lastColumn; x += 1.f)
                {
                    glm::vec2 point(x, y);
                    glm::vec3 weights = BarycentricInterpolation(vertices, point);
                    float depth;
                    if(is3D)
                    {
                        depth = PerspectiveCorrectZ(vertices, point);
                    }
                    else
                    {
                        depth = weights[0] * vertices[0].z
                              + weights[1] * vertices[1].z
                              + weights[2] * vertices[2].z;
                    }
                    int pixelX = static_cast<int>(x);
                    int index = pixelX + result.width() * y;

                    if(depth < depthBuffer[index])
                    {
                        depthBuffer[index] = depth;
                        glm::vec3 color;
                        if(is3D)
                        {
                            glm::vec4 normal = PerspectiveCorrectNormal(vertices, normals, point, depth);
                            float diffuse = glm::max(glm::dot(glm::vec3(normal), lightDirection), 0.f);
                            float brightness = 0.3f + 0.7f * diffuse;
                            if(polygon.mp_texture != nullptr)
                            {
                                glm::vec2 uv = PerspectiveCorrectUV(vertices, uvs, point, depth);
                                color = brightness * GetImageColor(uv, polygon.mp_texture);
                            }
                            else
                            {
                                color = brightness
                                      * PerspectiveCorrectColor(vertices, colors, point, depth);
                            }
                        }
                        else
                        {
                            color = weights[0] * colors[0]
                                  + weights[1] * colors[1]
                                  + weights[2] * colors[2];
                        }
                        color = glm::clamp(color, glm::vec3(0.f), glm::vec3(255.f));
                        result.setPixel(pixelX, y, qRgb(color.r, color.g, color.b));
                    }
                }
            }
        }
    }
    return result;
}

void Rasterizer::ClearScene()
{
    m_polygons.clear();
}
