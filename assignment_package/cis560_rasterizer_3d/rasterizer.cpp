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

Rasterizer::Rasterizer(const std::vector<Polygon>& polygons)
    : m_polygons(polygons)
{}

QImage Rasterizer::RenderScene()
{
    QImage result(512, 512, QImage::Format_RGB32);
    // Fill the image with black pixels.
    // Note that qRgb creates a QColor,
    // and takes in values [0, 255] rather than [0, 1].
    result.fill(qRgb(0.f, 0.f, 0.f));
    std::vector<float> depthBuffer(result.width() * result.height(),
                                   std::numeric_limits<float>::infinity());

    for(const Polygon& polygon : m_polygons)
    {
        for(const Triangle& triangle : polygon.m_tris)
        {
            std::array<glm::vec4, 3> vertices;
            std::array<glm::vec3, 3> colors;
            for(unsigned int i = 0; i < vertices.size(); i++)
            {
                const Vertex& vertex = polygon.m_verts[triangle.m_indices[i]];
                vertices[i] = vertex.m_pos;
                colors[i] = vertex.m_color;
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
                    glm::vec3 weights = BarycentricInterpolation(vertices, glm::vec2(x, y));
                    float depth = weights[0] * vertices[0].z
                                + weights[1] * vertices[1].z
                                + weights[2] * vertices[2].z;
                    int pixelX = static_cast<int>(x);
                    int index = pixelX + result.width() * y;

                    if(depth < depthBuffer[index])
                    {
                        depthBuffer[index] = depth;
                        glm::vec3 color = weights[0] * colors[0]
                                        + weights[1] * colors[1]
                                        + weights[2] * colors[2];
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
