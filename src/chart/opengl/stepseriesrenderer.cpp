/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stepseriesrenderer.h"
#include "../stepplot.h"
#include "notifier.h"
using namespace chart;

StepSeriesRenderer::StepSeriesRenderer() : XYSeriesRenderer(), m_window(WindowFunction::Hann)
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/pos.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/line.geom");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/color.frag");
    if (!m_program.link()) {
        emit Notifier::getInstance()->newMessage("StepSeriesRenderer", m_program.log());
    }
    m_colorUniform = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
    m_widthUniform  = m_program.uniformLocation("width");
    m_screenUniform = m_program.uniformLocation("screen");
}

void StepSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->impulseSize())
        return;

    unsigned int maxBufferSize = m_source->impulseSize() * 4, verticiesCount = 0;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    float res = 0.f;
    float offsetValue = 0;
    float dcOffset = 0;
    for (unsigned int i = 1; i < m_source->impulseSize() / 4; ++i) {
        dcOffset += m_source->impulseValue(i);
    }
    dcOffset /= m_source->impulseSize() / 4;
    for (unsigned int i = 1, j = 0; i < m_source->impulseSize(); ++i, j += 2) {
        res += m_source->impulseValue(i) - dcOffset;
        m_vertices[j] = m_source->impulseTime(i);
        m_vertices[j + 1] = res;
        if (m_source->impulseTime(i) < m_zero) {
            offsetValue = res;
        }
        verticiesCount += 1;
    }

    updateMatrix();
    m_matrix.translate(QVector3D(0, -offsetValue, 0));
    m_program.setUniformValue(m_matrixUniform, m_matrix);
    m_program.setUniformValue(m_screenUniform, m_width, m_height);
    m_program.setUniformValue(m_widthUniform, m_weight * m_retinaScale);

    if (m_refreshBuffers) {
        m_openGLFunctions->glGenBuffers(1, &m_vertexBufferId);
        m_openGLFunctions->glGenVertexArrays(1, &m_vertexArrayId);
    }
    m_openGLFunctions->glBindVertexArray(m_vertexArrayId);
    m_openGLFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    if (m_refreshBuffers) {
        m_openGLFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * maxBufferSize, nullptr, GL_DYNAMIC_DRAW);
        m_openGLFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(0));
    }
    m_openGLFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(GLfloat) * verticiesCount, m_vertices.data());

    m_openGLFunctions->glEnableVertexAttribArray(0);
    m_openGLFunctions->glDrawArrays(GL_LINE_STRIP, 0, verticiesCount);
    m_openGLFunctions->glDisableVertexAttribArray(0);

    m_refreshBuffers = false;
}

void StepSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);
    if (auto *plot = qobject_cast<StepPlot *>(m_item->parent())) {
        m_zero = plot->zero();
    }
}

void StepSeriesRenderer::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(m_xMin, m_xMax, m_yMax, m_yMin, -1, 1);
}
