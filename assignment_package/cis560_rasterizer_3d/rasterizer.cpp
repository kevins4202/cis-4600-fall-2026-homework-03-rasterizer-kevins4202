#include "rasterizer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

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
    // TODO: Complete the various components of code that make up this function.
    // It should return the rasterized image of the current scene.

    // Make liberal use of helper functions; writing your rasterizer as one
    // long RenderScene function will make it (a) hard to debug and
    // (b) hard to write without copy-pasting. Also, Adam will be sad when
    // he reads your code.

    // Also! As per the style requirements for this assignment, make sure you
    // use std::arrays to store things like your line segments, Triangles, and
    // vertex coordinates. This lets you easily use loops to perform operations
    // on your scene components, rather than copy-pasting operations three times
    // each!
    return result;
}

void Rasterizer::ClearScene()
{
    m_polygons.clear();
}
