#include "polygon.h"
#include <glm/gtx/transform.hpp>

namespace
{
glm::vec3 PolygonNormal(const std::vector<Vertex>& vertices)
{
    glm::vec3 normal(0.f);
    for(unsigned int i = 0; i < vertices.size(); i++)
    {
        const glm::vec4& current = vertices[i].m_pos;
        const glm::vec4& next = vertices[(i + 1) % vertices.size()].m_pos;
        normal.x += (current.y - next.y) * (current.z + next.z);
        normal.y += (current.z - next.z) * (current.x + next.x);
        normal.z += (current.x - next.x) * (current.y + next.y);
    }
    return normal;
}
}

void Polygon::Triangulate()
{
    m_tris.clear();
    if(m_verts.size() < 3)
    {
        return;
    }

    glm::vec3 normal = PolygonNormal(m_verts);
    if(glm::length(normal) == 0.f)
    {
        return;
    }

    int dropAxis = 2;
    if(glm::abs(normal.x) > glm::abs(normal.y)
       && glm::abs(normal.x) > glm::abs(normal.z))
    {
        dropAxis = 0;
    }
    else if(glm::abs(normal.y) > glm::abs(normal.z))
    {
        dropAxis = 1;
    }

    std::vector<glm::vec2> projected;
    for(const Vertex& vertex : m_verts)
    {
        if(dropAxis == 0)
        {
            projected.push_back(glm::vec2(vertex.m_pos.y, vertex.m_pos.z));
        }
        else if(dropAxis == 1)
        {
            projected.push_back(glm::vec2(vertex.m_pos.x, vertex.m_pos.z));
        }
        else
        {
            projected.push_back(glm::vec2(vertex.m_pos.x, vertex.m_pos.y));
        }
    }

    float signedArea = 0.f;
    for(unsigned int i = 0; i < m_verts.size(); i++)
    {
        const glm::vec2& a = projected[i];
        const glm::vec2& b = projected[(i + 1) % projected.size()];
        signedArea += a.x * b.y - b.x * a.y;
    }
    if(signedArea == 0.f)
    {
        return;
    }
    float winding = signedArea > 0.f ? 1.f : -1.f;

    auto cross = [](const std::vector<glm::vec2>& points,
                    unsigned int a, unsigned int b, unsigned int c)
    {
        const glm::vec2& p = points[a];
        const glm::vec2& q = points[b];
        const glm::vec2& r = points[c];
        return (q.x - p.x) * (r.y - p.y)
             - (q.y - p.y) * (r.x - p.x);
    };

    bool concave = false;
    for(unsigned int i = 0; i < m_verts.size(); i++)
    {
        unsigned int previous = (i + m_verts.size() - 1) % m_verts.size();
        unsigned int next = (i + 1) % m_verts.size();
        if(winding * cross(projected, previous, i, next) < 0.f)
        {
            concave = true;
            break;
        }
    }

    if(!concave)
    {
        for(unsigned int i = 1; i + 1 < m_verts.size(); i++)
        {
            Triangle triangle = {{0, i, i + 1}};
            m_tris.push_back(triangle);
        }
        return;
    }

    std::vector<unsigned int> remaining;
    for(unsigned int i = 0; i < m_verts.size(); i++)
    {
        remaining.push_back(i);
    }

    while(remaining.size() > 3)
    {
        bool foundEar = false;
        for(unsigned int i = 0; i < remaining.size(); i++)
        {
            unsigned int previous = remaining[(i + remaining.size() - 1) % remaining.size()];
            unsigned int current = remaining[i];
            unsigned int next = remaining[(i + 1) % remaining.size()];
            if(winding * cross(projected, previous, current, next) <= 0.f)
            {
                continue;
            }

            bool containsVertex = false;
            for(unsigned int vertex : remaining)
            {
                if(vertex != previous && vertex != current && vertex != next
                   && winding * cross(projected, previous, current, vertex) >= 0.f
                   && winding * cross(projected, current, next, vertex) >= 0.f
                   && winding * cross(projected, next, previous, vertex) >= 0.f)
                {
                    containsVertex = true;
                    break;
                }
            }
            if(containsVertex)
            {
                continue;
            }

            Triangle triangle = {{previous, current, next}};
            m_tris.push_back(triangle);
            remaining.erase(remaining.begin() + i);
            foundEar = true;
            break;
        }
        if(!foundEar)
        {
            m_tris.clear();
            return;
        }
    }

    Triangle triangle = {{remaining[0], remaining[1], remaining[2]}};
    m_tris.push_back(triangle);
}

glm::vec3 GetImageColor(const glm::vec2 &uv_coord, const QImage* const image)
{
    if(image)
    {
        int X = glm::min(image->width() * uv_coord.x, image->width() - 1.0f);
        int Y = glm::min(image->height() * (1.0f - uv_coord.y), image->height() - 1.0f);
        QColor color = image->pixel(X, Y);
        return glm::vec3(color.red(), color.green(), color.blue());
    }
    return glm::vec3(255.f, 255.f, 255.f);
}


// Creates a polygon from the input list of vertex positions and colors
Polygon::Polygon(const QString& name, const std::vector<glm::vec4>& pos,
                 const std::vector<glm::vec3>& col, bool is3D)
    : m_tris(), m_verts(), m_name(name), m_is3D(is3D),
      mp_texture(nullptr), mp_normalMap(nullptr)
{
    for(unsigned int i = 0; i < pos.size(); i++)
    {
        m_verts.push_back(Vertex(pos[i], col[i], glm::vec4(), glm::vec2()));
    }
    if(m_is3D)
    {
        glm::vec3 normal = PolygonNormal(m_verts);
        if(glm::length(normal) > 0.f)
        {
            normal = glm::normalize(normal);
            for(Vertex& vertex : m_verts)
            {
                vertex.m_normal = glm::vec4(normal, 0.f);
            }
        }
    }
    Triangulate();
}

// Creates a regular polygon with a number of sides indicated by the "sides" input integer.
// All of its vertices are of color "color", and the polygon is centered at "pos".
// It is rotated about its center by "rot" degrees, and is scaled from its center by "scale" units
Polygon::Polygon(const QString& name, int sides, glm::vec3 color, glm::vec4 pos, float rot, glm::vec4 scale)
    : m_tris(), m_verts(), m_name(name), m_is3D(false),
      mp_texture(nullptr), mp_normalMap(nullptr)
{
    glm::vec4 v(0.f, 1.f, 0.f, 1.f);
    float angle = 360.f / sides;
    for(int i = 0; i < sides; i++)
    {
        glm::vec4 vert_pos = glm::translate(glm::vec3(pos.x, pos.y, pos.z))
                           * glm::rotate(rot, glm::vec3(0.f, 0.f, 1.f))
                           * glm::scale(glm::vec3(scale.x, scale.y, scale.z))
                           * glm::rotate(i * angle, glm::vec3(0.f, 0.f, 1.f))
                           * v;
        m_verts.push_back(Vertex(vert_pos, color, glm::vec4(), glm::vec2()));
    }

    Triangulate();
}

Polygon::Polygon(const QString &name)
    : m_tris(), m_verts(), m_name(name), m_is3D(true),
      mp_texture(nullptr), mp_normalMap(nullptr)
{}

Polygon::Polygon()
    : m_tris(), m_verts(), m_name("Polygon"), m_is3D(false),
      mp_texture(nullptr), mp_normalMap(nullptr)
{}

Polygon::Polygon(const Polygon& p)
    : m_tris(p.m_tris), m_verts(p.m_verts), m_name(p.m_name),
      m_is3D(p.m_is3D), mp_texture(nullptr), mp_normalMap(nullptr)
{
    if(p.mp_texture != nullptr)
    {
        mp_texture = new QImage(*p.mp_texture);
    }
    if(p.mp_normalMap != nullptr)
    {
        mp_normalMap = new QImage(*p.mp_normalMap);
    }
}

Polygon::~Polygon()
{
    delete mp_texture;
}

void Polygon::SetTexture(QImage* i)
{
    mp_texture = i;
}

void Polygon::SetNormalMap(QImage* i)
{
    mp_normalMap = i;
}

void Polygon::AddTriangle(const Triangle& t)
{
    m_tris.push_back(t);
}

void Polygon::AddVertex(const Vertex& v)
{
    m_verts.push_back(v);
}

void Polygon::ClearTriangles()
{
    m_tris.clear();
}

Triangle& Polygon::TriAt(unsigned int i)
{
    return m_tris[i];
}

Triangle Polygon::TriAt(unsigned int i) const
{
    return m_tris[i];
}

Vertex &Polygon::VertAt(unsigned int i)
{
    return m_verts[i];
}

Vertex Polygon::VertAt(unsigned int i) const
{
    return m_verts[i];
}
