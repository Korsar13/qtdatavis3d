/****************************************************************************
**
** Copyright (C) 2013 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://qt.digia.com
**
** This file is part of the QtDataVis3D module.
**
** Licensees holding valid Qt Enterprise licenses may use this file in
** accordance with the Qt Enterprise License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.
**
** If you have questions regarding the use of this file, please use
** contact form at http://qt.digia.com
**
****************************************************************************/

#include "maps3dcontroller_p.h"
#include "maps3drenderer_p.h"
#include "camerahelper_p.h"
#include "shaderhelper_p.h"
#include "objecthelper_p.h"
#include "texturehelper_p.h"
#include "theme_p.h"
#include "utils_p.h"
#include "drawer_p.h"
#include "maprenderitem_p.h"
#include "qmapdataproxy_p.h"

#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <qmath.h>

#include <QDebug>

//#define DISPLAY_RENDER_SPEED

// Commenting this draws the shadow map with perspective projection. Otherwise it's drawn in
// orthographic projection.
//#define USE_WIDER_SHADOWS

// You can verify that depth buffer drawing works correctly by uncommenting this.
// You should see the scene from  where the light is
//#define SHOW_DEPTH_TEXTURE_SCENE

#ifdef DISPLAY_RENDER_SPEED
#include <QTime>
#endif

QT_DATAVIS3D_BEGIN_NAMESPACE

//#define DISPLAY_FULL_DATA_ON_SELECTION // Append selection value text with row and column labels

const GLfloat gridLineWidth = 0.005f;
GLfloat distanceMod = 0.0f;
static QVector3D skipColor = QVector3D(255, 255, 255); // Selection texture's background color

Maps3DController::Maps3DController(const QRect &rect)
    : m_camera(new CameraHelper()),
      m_barShader(0),
      m_depthShader(0),
      m_selectionShader(0),
      m_backgroundShader(0),
      m_labelShader(0),
      m_barObj(0),
      m_backgroundObj(0),
      m_gridLineObj(0),
      m_labelObj(0),
      m_objFile(QStringLiteral(":/defaultMeshes/bar")),
      m_mousePressed(MouseNone),
      m_mousePos(QPoint(0, 0)),
      m_zoomLevel(100),
      m_autoScaleAdjustment(1.0f),
      m_horizontalRotation(0.0f),
      m_verticalRotation(45.0f),
      m_barThickness(QVector3D(1.0f, 1.0f, 1.0f)),
      m_heightNormalizer(0.0f),
      m_yAdjustment(0.0f),
      m_scaleFactor(1.0f),
      m_theme(new Theme()),
      m_isInitialized(false),
      m_selectionMode(QDataVis::ModeItem),
      m_selectedBar(0),
      m_previouslySelectedBar(0),
      m_axisLabelX(QStringLiteral("X")),
      m_axisLabelZ(QStringLiteral("Z")),
      m_axisLabelY(QStringLiteral("Y")),
      m_sceneViewPort(rect.x(), rect.y(), rect.width(), rect.height()),
      m_zoomViewPort(rect.x(), rect.y(), rect.width(), rect.height()),
      m_zoomActivated(false),
      m_textureHelper(new TextureHelper()),
      m_labelTransparency(QDataVis::TransparencyFromTheme),
      m_font(QFont(QStringLiteral("Arial"))),
      m_drawer(new Drawer(*m_theme, m_font, m_labelTransparency)),
      m_areaSize(QSizeF(1.0, 1.0)),
      m_bgrTexture(0),
      m_depthTexture(0),
      m_selectionTexture(0),
      m_depthFrameBuffer(0),
      m_selectionFrameBuffer(0),
      m_selectionDepthBuffer(0),
      m_updateLabels(true),
      m_adjustDirection(Q3DMaps::AdjustHeight),
      m_shadowQuality(QDataVis::ShadowLow),
      m_shadowQualityToShader(33.3f),
      m_bgrHasAlpha(false),
      m_boundingRect(rect.x(), rect.y(), rect.width(), rect.height()),
      m_data(0),
      m_valuesDirty(false)
{
    //m_data->setDrawer(m_drawer);
    //QObject::connect(m_drawer, &Drawer::drawerChanged, this, &Maps3DController::updateTextures);
}

Maps3DController::~Maps3DController()
{
    m_textureHelper->glDeleteFramebuffers(1, &m_depthFrameBuffer);
    m_textureHelper->glDeleteFramebuffers(1, &m_selectionFrameBuffer);
    m_textureHelper->glDeleteRenderbuffers(1, &m_selectionDepthBuffer);
    m_textureHelper->deleteTexture(&m_selectionTexture);
    m_textureHelper->deleteTexture(&m_bgrTexture);
    delete m_barShader;
    delete m_selectionShader;
    delete m_backgroundShader;
    delete m_barObj;
    delete m_backgroundObj;
    delete m_gridLineObj;
    delete m_textureHelper;
    delete m_drawer;
    delete m_camera;
}

void Maps3DController::initializeOpenGL()
{
    // Initialization is called multiple times when Qt Quick components are used
    if (m_isInitialized)
        return;

    m_renderer = new Maps3DRenderer(this);
    initializeOpenGLFunctions();

    m_textureHelper = new TextureHelper();
    m_drawer->initializeOpenGL();

    // Initialize shaders
#if !defined(QT_OPENGL_ES_2)
    if (m_shadowQuality > QDataVis::ShadowNone) {
        if (!m_theme->m_uniformColor) {
            initShaders(QStringLiteral(":/shaders/vertexShadow"),
                        QStringLiteral(":/shaders/fragmentShadowNoTexColorOnY"));
        } else {
            initShaders(QStringLiteral(":/shaders/vertexShadow"),
                        QStringLiteral(":/shaders/fragmentShadowNoTex"));
        }
        initBackgroundShaders(QStringLiteral(":/shaders/vertexShadow"),
                              QStringLiteral(":/shaders/fragmentShadow"));
    } else {
        if (!m_theme->m_uniformColor) {
            initShaders(QStringLiteral(":/shaders/vertex"),
                        QStringLiteral(":/shaders/fragmentColorOnY"));
        } else {
            initShaders(QStringLiteral(":/shaders/vertex"),
                        QStringLiteral(":/shaders/fragment"));
        }
        initBackgroundShaders(QStringLiteral(":/shaders/vertexTexture"),
                              QStringLiteral(":/shaders/fragmentTexture"));
    }
#else
    if (!m_theme->m_uniformColor) {
        initShaders(QStringLiteral(":/shaders/vertexES2"),
                    QStringLiteral(":/shaders/fragmentColorOnYES2"));
    } else {
        initShaders(QStringLiteral(":/shaders/vertexES2"),
                    QStringLiteral(":/shaders/fragmentES2"));
    }
    initBackgroundShaders(QStringLiteral(":/shaders/vertexTexture"), // Same vertex shader ok for ES2
                          QStringLiteral(":/shaders/fragmentTextureES2"));
#endif
    initLabelShaders(QStringLiteral(":/shaders/vertexLabel"),
                     QStringLiteral(":/shaders/fragmentLabel"));

#if !defined(QT_OPENGL_ES_2)
    // Init depth shader (for shadows). Init in any case, easier to handle shadow activation if done via api.
    initDepthShader();
#endif

    // Init selection shader
    initSelectionShader();

    // Load default mesh
    loadBarMesh();

    // Load grid line mesh
    loadGridLineMesh();

    // Load label mesh
    loadLabelMesh();

    // Set OpenGL features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

#if !defined(QT_OPENGL_ES_2)
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
#endif

    // Set initial camera position
    // X must be 0 for rotation to work - we can use "setCameraRotation" for setting it later
    m_camera->setDefaultCameraOrientation(QVector3D(0.0f, 0.0f, 1.0f + 2.9f * zComp),
                                          QVector3D(0.0f, 0.0f, zComp),
                                          QVector3D(0.0f, 1.0f, 0.0f));

    // Adjust to default rotation
    setCameraPosition(m_horizontalRotation, m_verticalRotation, m_zoomLevel);

    // Set view port
    glViewport(0, 0, width(), height());

    // Set initialized -flag
    m_isInitialized = true;

    // Resize in case we've missed resize events
    // Resize calls initSelectionBuffer and initDepthBuffer, so they don't need to be called here
    resizeNotify();

    // Load background mesh (we need to be initialized first)
    loadBackgroundMesh();

    // Update default light position
#ifndef USE_WIDER_SHADOWS
    distanceMod = 5.0f;
#endif
}

void Maps3DController::synchDataToRenderer()
{
    // TODO: Implement!
}


/*!
 * \internal
 */
void Maps3DController::render(const GLuint defaultFboHandle)
{
    if (!m_isInitialized)
        return;

#ifdef DISPLAY_RENDER_SPEED
    // For speed computation
    static bool firstRender = true;
    static QTime lastTime;
    static GLint nbFrames = 0;
    if (firstRender) {
        lastTime.start();
        firstRender = false;
    }

    // Measure speed (as milliseconds per frame)
    nbFrames++;
    if (lastTime.elapsed() >= 1000) { // print only if last measurement was more than 1s ago
        qDebug() << qreal(lastTime.elapsed()) / qreal(nbFrames) << "ms/frame (=" << qreal(nbFrames) << "fps)";
        nbFrames = 0;
        lastTime.restart();
    }
#endif

    // Update cached values
    if (m_valuesDirty) {
        const QMapDataArray &dataArray = *m_data->array();
        int dataSize = dataArray.size();
        m_renderItemArray.resize(dataSize);
        for (int i = 0; i < dataSize ; i++) {
            qreal value = dataArray.at(i).value();
            m_renderItemArray[i].setValue(value);
            m_renderItemArray[i].setMapPosition(dataArray.at(i).mapPosition());
            m_renderItemArray[i].setHeight(value / m_heightNormalizer);
            m_renderItemArray[i].setItemLabel(dataArray.at(i).label());
            calculateTranslation(m_renderItemArray[i]);
            m_renderItemArray[i].setRenderer(this);
        }
        m_valuesDirty = false;
    }

    if (defaultFboHandle) {
        glDepthMask(true);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    // Draw scene
    drawScene(defaultFboHandle);
}

/*!
 * \internal
 */
void Maps3DController::drawScene(const GLuint defaultFboHandle)
{
    // Set clear color
    QVector3D clearColor = Utils::vectorFromColor(m_theme->m_windowColor);
    glClearColor(clearColor.x(), clearColor.y(), clearColor.z(), 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static QVector3D selection = skipColor;

    // Specify viewport
    glViewport(m_sceneViewPort.x(), m_sceneViewPort.y(),
               m_sceneViewPort.width(), m_sceneViewPort.height());

    // Set up projection matrix
    QMatrix4x4 projectionMatrix;
    projectionMatrix.perspective(45.0f, (GLfloat)m_sceneViewPort.width()
                                 / (GLfloat)m_sceneViewPort.height(), 0.1f, 100.0f);

    // Calculate view matrix
    QMatrix4x4 viewMatrix = m_camera->calculateViewMatrix(m_mousePos,
                                                          m_zoomLevel * m_autoScaleAdjustment,
                                                          m_sceneViewPort.width(),
                                                          m_sceneViewPort.height());

    // Get light position (rotate light with camera, a bit above it (as set in defaultLightPos))
    QVector3D lightPos = m_camera->calculateLightPosition(defaultLightPos, 0.0f, distanceMod);

    // Map adjustment direction to model matrix scaling
    GLfloat heightMultiplier = 0.0f;
    GLfloat widthMultiplier = 0.0f;
    GLfloat depthMultiplier = 0.0f;
    GLfloat heightScaler = 0.0f;
    GLfloat widthScaler = 0.0f;
    GLfloat depthScaler = 0.0f;
    switch (m_adjustDirection) {
    case Q3DMaps::AdjustHeight:
        widthMultiplier = 0.0f;
        heightMultiplier = 1.0f;
        depthMultiplier = 0.0f;
        widthScaler = m_barThickness.x() / m_scaleFactor;
        heightScaler = 0.0f;
        depthScaler = m_barThickness.z() / m_scaleFactor;
        break;
    case Q3DMaps::AdjustWidth:
        widthMultiplier = 1.0f;
        heightMultiplier = 0.0f;
        depthMultiplier = 0.0f;
        widthScaler = 0.0f;
        heightScaler = m_barThickness.y() / m_scaleFactor;
        depthScaler = m_barThickness.z() / m_scaleFactor;
        break;
    case Q3DMaps::AdjustDepth:
        widthMultiplier = 0.0f;
        heightMultiplier = 0.0f;
        depthMultiplier = 1.0f;
        widthScaler = m_barThickness.x() / m_scaleFactor;
        heightScaler = m_barThickness.y() / m_scaleFactor;
        depthScaler = 0.0f;
        break;
    case Q3DMaps::AdjustRadius:
        widthMultiplier = 1.0f;
        heightMultiplier = 0.0f;
        depthMultiplier = 1.0f;
        widthScaler = 0.0f;
        heightScaler = m_barThickness.y() / m_scaleFactor;
        depthScaler = 0.0f;
        break;
    case Q3DMaps::AdjustAll:
        widthMultiplier = 1.0f;
        heightMultiplier = 1.0f;
        depthMultiplier = 1.0f;
        widthScaler = 0.0f;
        heightScaler = 0.0f;
        depthScaler = 0.0f;
        break;
    }

    // Introduce regardless of shadow quality to simplify logic
    QMatrix4x4 depthViewMatrix;
    QMatrix4x4 depthProjectionMatrix;

#if !defined(QT_OPENGL_ES_2)
    if (m_shadowQuality > QDataVis::ShadowNone) {
        // Render scene into a depth texture for using with shadow mapping
        // Bind depth shader
        m_depthShader->bind();

        // Set viewport for depth map rendering. Must match texture size. Larger values give smoother shadows.
        glViewport(m_sceneViewPort.x(), m_sceneViewPort.y(),
                   m_sceneViewPort.width() * m_shadowQuality,
                   m_sceneViewPort.height() * m_shadowQuality);

        // Enable drawing to framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_depthFrameBuffer);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Set front face culling to reduce self-shadowing issues
        glCullFace(GL_FRONT);

        // Get the depth view matrix
        // It may be possible to hack lightPos here if we want to make some tweaks to shadow
        QVector3D depthLightPos = m_camera->calculateLightPosition(
                    defaultLightPos, 0.0f, (distanceMod + 1.5f) / m_autoScaleAdjustment);
        depthViewMatrix.lookAt(depthLightPos, QVector3D(0.0f, -m_yAdjustment, zComp),
                               QVector3D(0.0f, 1.0f, 0.0f));
        // TODO: Why does depthViewMatrix.column(3).y() goes to zero when we're directly above? That causes the scene to be not drawn from above -> must be fixed
        //qDebug() << lightPos << depthViewMatrix << depthViewMatrix.column(3);
        // Set the depth projection matrix
#ifndef USE_WIDER_SHADOWS
        // Use this for perspective shadows
        depthProjectionMatrix.perspective(15.0f, (GLfloat)m_sceneViewPort.width()
                                          / (GLfloat)m_sceneViewPort.height(), 3.0f, 200.0f);
#else
        // Use these for orthographic shadows
        //qDebug() << m_areaSize.width() / m_scaleFactor << m_yAdjustment;
        GLfloat testAspectRatio = (GLfloat)m_sceneViewPort.width() / (GLfloat)m_sceneViewPort.height();
        depthProjectionMatrix.ortho(-(2.0f * m_areaSize.width()) / m_scaleFactor,
                                    (2.0f * m_areaSize.width()) / m_scaleFactor,
                                    -m_yAdjustment * 4.0f * testAspectRatio,
                                    m_yAdjustment * 4.0f * testAspectRatio,
                                    0.0f, 100.0f);
#endif
#if 0
        // Draw background to depth buffer (You don't want to do this, except maybe for debugging purposes)
        if (m_backgroundObj) {
            QMatrix4x4 modelMatrix;
            QMatrix4x4 MVPMatrix;

            modelMatrix.translate(0.0f, -m_yAdjustment, zComp);
            modelMatrix.scale(QVector3D(m_areaSize.width() / m_scaleFactor,
                                        1.0f,
                                        m_areaSize.height() / m_scaleFactor));
            modelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);

            MVPMatrix = depthProjectionMatrix * depthViewMatrix * modelMatrix;

            m_depthShader->setUniformValue(m_depthShader->MVP(), MVPMatrix);

            // 1st attribute buffer : vertices
            glEnableVertexAttribArray(m_depthShader->posAtt());
            glBindBuffer(GL_ARRAY_BUFFER, m_backgroundObj->vertexBuf());
            glVertexAttribPointer(m_depthShader->posAtt(), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_backgroundObj->elementBuf());

            // Draw the triangles
            glDrawElements(GL_TRIANGLES, m_backgroundObj->indexCount(), GL_UNSIGNED_SHORT,
                           (void *)0);

            // Free buffers
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDisableVertexAttribArray(m_depthShader->posAtt());
        }
#endif
        // Draw bars to depth buffer
        for (int bar = 0; bar < m_renderItemArray.size(); bar++) {
            const MapRenderItem &item = m_renderItemArray.at(bar);
            if (!item.value())
                continue;

            QMatrix4x4 modelMatrix;
            QMatrix4x4 MVPMatrix;

            modelMatrix.translate(item.translation().x(),
                                  heightMultiplier * item.height() + heightScaler - m_yAdjustment,
                                  item.translation().z());
            modelMatrix.scale(QVector3D(widthMultiplier * item.height() + widthScaler,
                                        heightMultiplier * item.height() + heightScaler,
                                        depthMultiplier * item.height() + depthScaler));

            MVPMatrix = depthProjectionMatrix * depthViewMatrix * modelMatrix;

            m_depthShader->setUniformValue(m_depthShader->MVP(), MVPMatrix);

            // 1st attribute buffer : vertices
            glEnableVertexAttribArray(m_depthShader->posAtt());
            glBindBuffer(GL_ARRAY_BUFFER, m_barObj->vertexBuf());
            glVertexAttribPointer(m_depthShader->posAtt(), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_barObj->elementBuf());

            // Draw the triangles
            glDrawElements(GL_TRIANGLES, m_barObj->indexCount(), GL_UNSIGNED_SHORT, (void *)0);

            // Free buffers
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDisableVertexAttribArray(m_depthShader->posAtt());
        }

        // Disable drawing to framebuffer (= enable drawing to screen)
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFboHandle);

        // Reset culling to normal
        glCullFace(GL_BACK);

        // Release depth shader
        m_depthShader->release();

        // Revert to original viewport
        glViewport(m_sceneViewPort.x(), m_sceneViewPort.y(),
                   m_sceneViewPort.width(), m_sceneViewPort.height());

#if 0 // Use this if you want to see what is being drawn to the framebuffer
        // You'll also have to comment out GL_COMPARE_R_TO_TEXTURE -line in texturehelper (if using it)
        m_labelShader->bind();
        glEnable(GL_TEXTURE_2D);
        QMatrix4x4 modelMatrix;
        QMatrix4x4 viewmatrix;
        viewmatrix.lookAt(QVector3D(0.0f, 0.0f, 2.5f + zComp),
                          QVector3D(0.0f, 0.0f, zComp),
                          QVector3D(0.0f, 1.0f, 0.0f));
        modelMatrix.translate(0.0, 0.0, zComp);
        QMatrix4x4 MVPMatrix = projectionMatrix * viewmatrix * modelMatrix;
        m_labelShader->setUniformValue(m_labelShader->MVP(), MVPMatrix);
        m_drawer->drawObject(m_labelShader, m_labelObj,
                             m_depthTexture);
        glDisable(GL_TEXTURE_2D);
        m_labelShader->release();
#endif
    }
#endif

#if 1
    // Skip selection mode drawing if we're zoomed or have no selection mode
    if (!m_zoomActivated && m_selectionMode > QDataVis::ModeNone) {
        // Bind selection shader
        m_selectionShader->bind();

        // Draw bars to selection buffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_selectionFrameBuffer);
        glEnable(GL_DEPTH_TEST); // Needed, otherwise the depth render buffer is not used
        glClearColor(skipColor.x() / 255, skipColor.y() / 255, skipColor.z() / 255, 1.0f); // Set clear color to white (= skipColor)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Needed for clearing the frame buffer
        glDisable(GL_DITHER); // disable dithering, it may affect colors if enabled
        GLint barIdxRed = 0;
        GLint barIdxGreen = 0;
        GLint barIdxBlue = 0;
        for (int bar = 0; bar < m_renderItemArray.size(); bar++, barIdxRed++) {
            const MapRenderItem &item = m_renderItemArray.at(bar);
            if (!item.value())
                continue;

            QMatrix4x4 modelMatrix;
            QMatrix4x4 MVPMatrix;

            modelMatrix.translate(item.translation().x(),
                                  heightMultiplier * item.height() + heightScaler - m_yAdjustment,
                                  item.translation().z());
            modelMatrix.scale(QVector3D(widthMultiplier * item.height() + widthScaler,
                                        heightMultiplier * item.height() + heightScaler,
                                        depthMultiplier * item.height() + depthScaler));

            MVPMatrix = projectionMatrix * viewMatrix * modelMatrix;

            if (barIdxRed > 0 && barIdxRed % 256 == 0) {
                barIdxRed = 0;
                barIdxGreen++;
            }
            if (barIdxGreen > 0 && barIdxGreen % 256 == 0) {
                barIdxGreen = 0;
                barIdxBlue++;
            }
            if (barIdxBlue > 255)
                qFatal("Too many objects");

            QVector3D barColor = QVector3D((GLfloat)barIdxRed / 255.0f,
                                           (GLfloat)barIdxGreen / 255.0f,
                                           (GLfloat)barIdxBlue / 255.0f);

            m_selectionShader->setUniformValue(m_selectionShader->MVP(), MVPMatrix);
            m_selectionShader->setUniformValue(m_selectionShader->color(), barColor);

            // 1st attribute buffer : vertices
            glEnableVertexAttribArray(m_selectionShader->posAtt());
            glBindBuffer(GL_ARRAY_BUFFER, m_barObj->vertexBuf());
            glVertexAttribPointer(m_selectionShader->posAtt(), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_barObj->elementBuf());

            // Draw the triangles
            glDrawElements(GL_TRIANGLES, m_barObj->indexCount(), GL_UNSIGNED_SHORT, (void *)0);

            // Free buffers
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDisableVertexAttribArray(m_selectionShader->posAtt());
        }
        glEnable(GL_DITHER);

        // Read color under cursor
        if (Maps3DController::MouseOnScene == m_mousePressed)
            selection = Utils::getSelection(m_mousePos, height());

        glBindFramebuffer(GL_FRAMEBUFFER, defaultFboHandle);

        // Release selection shader
        m_selectionShader->release();

#if 0 // Use this if you want to see what is being drawn to the framebuffer
        m_labelShader->bind();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        QMatrix4x4 modelMatrix;
        QMatrix4x4 viewmatrix;
        viewmatrix.lookAt(QVector3D(0.0f, 0.0f, 2.0f + zComp),
                          QVector3D(0.0f, 0.0f, zComp),
                          QVector3D(0.0f, 1.0f, 0.0f));
        modelMatrix.translate(0.0, 0.0, zComp);
        QMatrix4x4 MVPMatrix = projectionMatrix * viewmatrix * modelMatrix;
        m_labelShader->setUniformValue(m_labelShader->MVP(), MVPMatrix);
        m_drawer->drawObject(m_labelShader, m_labelObj,
                             m_selectionTexture);
        glDisable(GL_TEXTURE_2D);
        m_labelShader->release();
#endif
    }
#if 1
    // Bind bar shader
    m_barShader->bind();

    // Enable texture
    glEnable(GL_TEXTURE_2D);

    // Draw bars
    // TODO: Handle zoom by camera transformations
    //if (!m_zoomActivated)

    bool barSelectionFound = false;
    for (int bar = 0; bar < m_renderItemArray.size(); bar++) {
        MapRenderItem &item = m_renderItemArray[bar];
        if (!item.value())
            continue;

        QMatrix4x4 modelMatrix;
        QMatrix4x4 MVPMatrix;
        QMatrix4x4 depthMVPMatrix;
        QMatrix4x4 itModelMatrix;

        modelMatrix.translate(item.translation().x(),
                              heightMultiplier * item.height() + heightScaler - m_yAdjustment,
                              item.translation().z());
        modelMatrix.scale(QVector3D(widthMultiplier * item.height() + widthScaler,
                                    heightMultiplier * item.height() + heightScaler,
                                    depthMultiplier * item.height() + depthScaler));
        itModelMatrix.scale(QVector3D(widthMultiplier * item.height() + widthScaler,
                                      heightMultiplier * item.height() + heightScaler,
                                      depthMultiplier * item.height() + depthScaler));

#ifdef SHOW_DEPTH_TEXTURE_SCENE
        MVPMatrix = depthProjectionMatrix * depthViewMatrix * modelMatrix;
#else
        MVPMatrix = projectionMatrix * viewMatrix * modelMatrix;
#endif
        depthMVPMatrix = depthProjectionMatrix * depthViewMatrix * modelMatrix;

        QVector3D baseColor = Utils::vectorFromColor(m_theme->m_baseColor);
        QVector3D heightColor = Utils::vectorFromColor(m_theme->m_heightColor) * item.height();

        QVector3D barColor = baseColor + heightColor;

        GLfloat lightStrength = m_theme->m_lightStrength;
        if (m_selectionMode > QDataVis::ModeNone) {
            Maps3DController::SelectionType selectionType = isSelected(bar, selection);
            switch (selectionType) {
            case Maps3DController::SelectionBar: {
                barColor = Utils::vectorFromColor(m_theme->m_highlightBarColor);
                lightStrength = m_theme->m_highlightLightStrength;
                // Insert data to QDataItem. We have no ownership, don't delete the previous one
                if (!m_zoomActivated) {
                    m_selectedBar = &item;
                    barSelectionFound = true;
                }
                break;
            }
            case Maps3DController::SelectionNone: {
                // Current bar is not selected, nor on a row or column
                // do nothing
                break;
            }
            default: {
                // Unsupported selection mode
                // do nothing
                break;
            }
            }
        }

        if (item.height() != 0) {
            // Set shader bindings
            m_barShader->setUniformValue(m_barShader->lightP(), lightPos);
            m_barShader->setUniformValue(m_barShader->view(), viewMatrix);
            m_barShader->setUniformValue(m_barShader->model(), modelMatrix);
            m_barShader->setUniformValue(m_barShader->nModel(),
                                         itModelMatrix.inverted().transposed());
            m_barShader->setUniformValue(m_barShader->MVP(), MVPMatrix);
            m_barShader->setUniformValue(m_barShader->color(), barColor);
            m_barShader->setUniformValue(m_barShader->ambientS(), m_theme->m_ambientStrength);

#if !defined(QT_OPENGL_ES_2)
            if (m_shadowQuality > QDataVis::ShadowNone) {
                // Set shadow shader bindings
                m_barShader->setUniformValue(m_barShader->shadowQ(), m_shadowQualityToShader);
                m_barShader->setUniformValue(m_barShader->depth(), depthMVPMatrix);
                m_barShader->setUniformValue(m_barShader->lightS(), lightStrength / 10.0f);

                // Draw the object
                m_drawer->drawObject(m_barShader, m_barObj, 0, m_depthTexture);
            } else
#endif
            {
                // Set shadowless shader bindings
                m_barShader->setUniformValue(m_barShader->lightS(), lightStrength);

                // Draw the object
                m_drawer->drawObject(m_barShader, m_barObj);
            }
        }
    }

    // Release bar shader
    m_barShader->release();
#if 1
    // Bind background shader
    m_backgroundShader->bind();
    if (m_bgrHasAlpha) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // Draw background
    if (m_backgroundObj) {
        QMatrix4x4 modelMatrix;
        QMatrix4x4 MVPMatrix;
        QMatrix4x4 depthMVPMatrix;
        QMatrix4x4 itModelMatrix;

        modelMatrix.translate(0.0f, -m_yAdjustment, zComp);
        modelMatrix.scale(QVector3D(m_areaSize.width() / m_scaleFactor,
                                    1.0f,
                                    m_areaSize.height() / m_scaleFactor));
        modelMatrix.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
        itModelMatrix.scale(QVector3D(m_areaSize.width() / m_scaleFactor,
                                      1.0f,
                                      m_areaSize.height() / m_scaleFactor));

#ifdef SHOW_DEPTH_TEXTURE_SCENE
        MVPMatrix = depthProjectionMatrix * depthViewMatrix * modelMatrix;
#else
        MVPMatrix = projectionMatrix * viewMatrix * modelMatrix;
#endif
        depthMVPMatrix = depthProjectionMatrix * depthViewMatrix * modelMatrix;

        // Set shader bindings
        m_backgroundShader->setUniformValue(m_backgroundShader->lightP(), lightPos);
        m_backgroundShader->setUniformValue(m_backgroundShader->view(), viewMatrix);
        m_backgroundShader->setUniformValue(m_backgroundShader->model(), modelMatrix);
        m_backgroundShader->setUniformValue(m_backgroundShader->nModel(),
                                            itModelMatrix.inverted().transposed());
        m_backgroundShader->setUniformValue(m_backgroundShader->MVP(), MVPMatrix);
        m_backgroundShader->setUniformValue(m_backgroundShader->ambientS(),
                                            m_theme->m_ambientStrength * 3.0f);

#if !defined(QT_OPENGL_ES_2)
        if (m_shadowQuality > QDataVis::ShadowNone) {
            // Set shadow shader bindings
            m_backgroundShader->setUniformValue(m_backgroundShader->shadowQ(),
                                                m_shadowQualityToShader);
            m_backgroundShader->setUniformValue(m_backgroundShader->depth(),
                                                depthMVPMatrix);
            m_backgroundShader->setUniformValue(m_backgroundShader->lightS(),
                                                m_theme->m_lightStrength / 25.0f);

            // Draw the object
            m_drawer->drawObject(m_backgroundShader, m_backgroundObj, m_bgrTexture, m_depthTexture);
        } else
#endif
        {
            // Set shadowless shader bindings
            m_backgroundShader->setUniformValue(m_backgroundShader->lightS(),
                                                m_theme->m_lightStrength);

            // Draw the object
            m_drawer->drawObject(m_backgroundShader, m_backgroundObj, m_bgrTexture);
        }
    }

    // Disable textures
    glDisable(GL_TEXTURE_2D);
    if (m_bgrHasAlpha)
        glDisable(GL_BLEND);

    // Release background shader
    m_backgroundShader->release();
#endif

    // Handle zoom activation and label drawing
    if (!barSelectionFound) {
        // We have no ownership, don't delete. Just NULL the pointer.
        m_selectedBar = NULL;
        //if (m_zoomActivated && Maps3DController::MouseOnOverview == m_mousePressed) {
        //m_sceneViewPort = QRect(0, 0, width(), height());
        //m_zoomActivated = false;
        //}
    } /*else if (m_selectionMode >= ModeZoomRow
               && Maps3DController::MouseOnScene == m_mousePressed) {
        // Activate zoom mode
        m_zoomActivated = true;
        m_sceneViewPort = QRect(0, height() - height() / 5, width() / 5, height() / 5);

        // Create label textures
        for (int col = 0; col < m_zoomSelection->row().size(); col++) {
            QDataItem *item = m_zoomSelection->getItem(col);
            m_drawer->generateLabelTexture(item);
        }
    }*/ else {
        // Print value of selected bar
        m_labelShader->bind();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        if (m_labelTransparency > QDataVis::TransparencyNone) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
#ifndef DISPLAY_FULL_DATA_ON_SELECTION
        // Draw just the value string of the selected bar
        if (m_previouslySelectedBar != m_selectedBar || m_updateLabels) {
            m_drawer->generateLabelTexture(m_selectedBar);
            m_previouslySelectedBar = m_selectedBar;
        }

        m_drawer->drawLabel(*m_selectedBar, m_selectedBar->labelItem(),
                            viewMatrix, projectionMatrix,
                            QVector3D(0.0f, m_yAdjustment, zComp),
                            QVector3D(0.0f, 0.0f, 0.0f), m_selectedBar->height(),
                            m_selectionMode, m_labelShader,
                            m_labelObj, m_camera, true);
#else
        // Draw the value string followed by row label and column label
        LabelItem &labelItem = m_selectedBar->selectionLabel();
        if (m_previouslySelectedBar != m_selectedBar || m_updateLabels || !labelItem.textureId()) {
            QString labelText = m_selectedBar->label();
            // TODO More elaborate label?
            m_drawer->generateLabelItem(&labelItem, labelText);
            m_previouslySelectedBar = m_selectedBar;
        }

        m_drawer->drawLabel(*m_selectedBar, labelItem, viewMatrix, projectionMatrix,
                            QVector3D(0.0f, m_yAdjustment, zComp),
                            QVector3D(0.0f, 0.0f, 0.0f), m_selectedBar->height(),
                            m_selectionMode, m_labelShader,
                            m_labelObj, true, false);
#endif
        glDisable(GL_TEXTURE_2D);
        if (m_labelTransparency > QDataVis::TransparencyNone)
            glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        // Release label shader
        m_labelShader->release();

        // Reset label update flag; they should have been updated when we get here
        m_updateLabels = false;
    }
#if 0
    // TODO: Calculations done temporarily here. When optimizing, move to after data set addition? Keep drawing of the labels here.
    // Bind label shader
    m_labelShader->bind();

    glEnable(GL_TEXTURE_2D);
    if (m_labelTransparency > TransparencyNone) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // Calculate the positions for row and column labels and store them into QDataItems (and QDataRows?)
    for (int row = 0; row != m_sampleCount.second; row += 1) {
        // Go through all rows and get position of max+1 or min-1 column, depending on x flip
        // We need only positions for them, labels have already been generated at QDataSet. Just add LabelItems
        rowPos = (row + 1) * (m_barSpacing.height());
        barPos = 0;
        GLfloat rotLabelX = -90.0f;
        GLfloat rotLabelY = 0.0f;
        Qt::AlignmentFlag alignment = Qt::AlignRight;
        if (m_zFlipped)
            rotLabelY = 180.0f;
        if (m_xFlipped) {
            barPos = (m_sampleCount.first + 1) * (m_barSpacing.width());
            alignment = Qt::AlignLeft;
        }
        QVector3D labelPos = QVector3D((m_rowWidth - barPos) / m_scaleFactor,
                                       -m_yAdjustment + 0.005f, // raise a bit over background to avoid depth "glimmering"
                                       (m_columnDepth - rowPos) / m_scaleFactor + zComp);

        // TODO: Try it; draw the label here

        // Create a data item
        QDataItem *label = new QDataItem();
        label->setTranslation(labelPos);
        if (m_data->d_ptr->rowLabelItems().size() > row) {
            label->setLabel(m_data->d_ptr->rowLabelItems().at(m_data->d_ptr->rowLabelItems().size()
                                                              - row - 1));
        }

        //qDebug() << "labelPos, row" << row + 1 << ":" << labelPos << m_dataSet->rowLabels().at(row);

        m_drawer->drawLabel(*label, label->label(), viewMatrix, projectionMatrix,
                            QVector3D(0.0f, m_yAdjustment, zComp),
                            QVector3D(rotLabelX, rotLabelY, 0.0f), m_heightNormalizer,
                            m_selectionMode, m_labelShader,
                            m_labelObj, true, true, Drawer::LabelMid, alignment);

        delete label;
    }
    for (int bar = 0; bar != m_sampleCount.first; bar += 1) {
        // Go through all columns and get position of max+1 or min-1 row, depending on z flip
        // We need only positions for them, labels have already been generated at QDataSet. Just add LabelItems
        barPos = (bar + 1) * (m_barSpacing.width());
        rowPos = 0;
        GLfloat rotLabelX = -90.0f;
        GLfloat rotLabelY = 90.0f;
        Qt::AlignmentFlag alignment = Qt::AlignLeft;
        if (m_xFlipped)
            rotLabelY = -90.0f;
        if (m_zFlipped) {
            rowPos = (m_sampleCount.second + 1) * (m_barSpacing.height());
            alignment = Qt::AlignRight;
        }
        QVector3D labelPos = QVector3D((m_rowWidth - barPos) / m_scaleFactor,
                                       -m_yAdjustment + 0.005f, // raise a bit over background to avoid depth "glimmering"
                                       (m_columnDepth - rowPos) / m_scaleFactor + zComp);

        // TODO: Try it; draw the label here

        // Create a data item
        QDataItem *label = new QDataItem();
        label->setTranslation(labelPos);
        if (m_data->d_ptr->columnLabelItems().size() > bar) {
            label->setLabel(m_data->d_ptr->columnLabelItems().at(
                                m_data->d_ptr->columnLabelItems().size() - bar - 1));
        }

        //qDebug() << "labelPos, col" << bar + 1 << ":" << labelPos << m_dataSet->columnLabels().at(bar);

        m_drawer->drawLabel(*label, label->label(), viewMatrix, projectionMatrix,
                            QVector3D(0.0f, m_yAdjustment, zComp),
                            QVector3D(rotLabelX, rotLabelY, 0.0f), m_heightNormalizer,
                            m_selectionMode, m_labelShader,
                            m_labelObj, true, true, Drawer::LabelMid, alignment);

        delete label;
    }
    glDisable(GL_TEXTURE_2D);
    if (m_labelTransparency > TransparencyNone)
        glDisable(GL_BLEND);

    // Release label shader
    m_labelShader->release();
#endif
#endif
#endif
}

#if defined(Q_OS_ANDROID)
/*!
 * \internal
 */
void Maps3DController::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_zoomActivated) {
        m_mousePressed = Maps3DController::MouseOnScene;
        // update mouse positions to prevent jumping when releasing or repressing a button
        m_mousePos = event->pos();
    }
}

/*!
 * \internal
 */
void Maps3DController::touchEvent(QTouchEvent *event)
{
    static int prevDistance = 0;

    QList<QTouchEvent::TouchPoint> points;
    points = event->touchPoints();

    if (points.count() == 2) {
        m_mousePressed = Maps3DController::MouseOnPinch;

        QPointF distance = points.at(0).pos() - points.at(1).pos();
        int newDistance = distance.manhattanLength();
        int zoomRate = 1;
        if (m_zoomLevel > 100)
            zoomRate = 5;
        if (newDistance > prevDistance)
            m_zoomLevel += zoomRate;
        else
            m_zoomLevel -= zoomRate;
        if (m_zoomLevel > 500)
            m_zoomLevel = 500;
        else if (m_zoomLevel < 10)
            m_zoomLevel = 10;
        prevDistance = newDistance;
        //qDebug() << "distance" << distance.manhattanLength();
    }
}
#endif

/*!
 * \internal
 */
void Maps3DController::mousePressEvent(QMouseEvent *event, const QPoint &mousePos)
{
    if (Qt::LeftButton == event->button()) {
        if (m_zoomActivated) {
            //qDebug() << event->pos().x() << event->pos().y() << m_sceneViewPort << m_zoomViewPort;
            if (mousePos.x() <= m_sceneViewPort.width()
                    && mousePos.y() <= m_sceneViewPort.height()) {
                m_mousePressed = Maps3DController::MouseOnOverview;
                //qDebug() << "Mouse pressed on overview";
            } else {
                m_mousePressed = Maps3DController::MouseOnZoom;
                //qDebug() << "Mouse pressed on zoom";
            }
        } else {
#if !defined(Q_OS_ANDROID)
            m_mousePressed = Maps3DController::MouseOnScene;
#else
            m_mousePressed = Maps3DController::MouseRotating;
#endif
            // update mouse positions to prevent jumping when releasing or repressing a button
            m_mousePos = mousePos;
            //qDebug() << "Mouse pressed on scene";
        }
    } else if (Qt::MiddleButton == event->button()) {
        // reset rotations
        m_mousePos = QPoint(0, 0);
    } else if (Qt::RightButton == event->button()) {
#if !defined(Q_OS_ANDROID)
        m_mousePressed = Maps3DController::MouseRotating;
#else
        m_mousePressed = Maps3DController::MouseOnScene;
#endif
        // update mouse positions to prevent jumping when releasing or repressing a button
        m_mousePos = mousePos;
    }
    m_camera->updateMousePos(m_mousePos);
}

/*!
 * \internal
 */
void Maps3DController::mouseReleaseEvent(QMouseEvent *event, const QPoint &mousePos)
{
    Q_UNUSED(event);
    //qDebug() << "mouse button released" << event->button();
    if (Maps3DController::MouseRotating == m_mousePressed) {
        // update mouse positions to prevent jumping when releasing or repressing a button
        m_mousePos = mousePos;
        m_camera->updateMousePos(mousePos);
    }
    m_mousePressed = Maps3DController::MouseNone;
}

/*!
 * \internal
 */
void Maps3DController::mouseMoveEvent(QMouseEvent *event, const QPoint &mousePos)
{
    Q_UNUSED(event);
    if (Maps3DController::MouseRotating == m_mousePressed) {
        //qDebug() << "mouse moved while pressed" << event->pos();
        m_mousePos = mousePos;
    }
#if 0
    // TODO: Testi - laske kursorin sijainti scenessä
    QPointF mouse3D((2.0f * event->pos().x() - width()) / height(),
                    1.0f - (2.0f * event->pos().y()) / height());
    //qDebug() << "mouse position in scene" << mouse3D;

    // TODO: Testi laske focal point
    GLfloat focalPoint = tan(45.0f / 2.0f);

    // TODO: Testi - laske viewmatriisin kerroin
    QVector3D worldRay = QVector3D(0.0f, 0.0f, 0.0f) - QVector3D(mouse3D.x(), mouse3D.y(),
                                                                 -focalPoint);
    //qDebug() << "worldRay" << worldRay;
    // multiply viewmatrix with this to get something?
#endif
}

/*!
 * \internal
 */
void Maps3DController::wheelEvent(QWheelEvent *event)
{
    if (m_zoomLevel > 100)
        m_zoomLevel += event->angleDelta().y() / 12;
    else if (m_zoomLevel > 50)
        m_zoomLevel += event->angleDelta().y() / 60;
    else
        m_zoomLevel += event->angleDelta().y() / 120;
    if (m_zoomLevel > 500)
        m_zoomLevel = 500;
    else if (m_zoomLevel < 10)
        m_zoomLevel = 10;
}

/*!
 * \internal
 */
void Maps3DController::resizeNotify()
{
    if (!m_isInitialized)
        return;

    // Set view port
    if (m_zoomActivated)
        m_sceneViewPort = QRect(0, height() - height() / 5, width() / 5, height() / 5);
    else
        m_sceneViewPort = QRect(0, 0, width(), height());
    m_zoomViewPort = QRect(0, 0, width(), height());

    // Calculate zoom level based on aspect ratio
    GLfloat div;
    GLfloat zoomAdjustment;
    div = qMin(width(), height());
    zoomAdjustment = defaultRatio * ((width() / div) / (height() / div));
    //qDebug() << "zoom adjustment" << zoomAdjustment;
    m_autoScaleAdjustment = qMin(zoomAdjustment, 1.0f); // clamp to 1.0f

    // Re-init selection buffer
    initSelectionBuffer();

#if !defined(QT_OPENGL_ES_2)
    // Re-init depth buffer
    initDepthBuffer();
#endif
}

void Maps3DController::setBarSpecs(const QVector3D &thickness,
                                   Q3DMaps::AdjustmentDirection direction)
{
    m_barThickness = thickness;
    m_adjustDirection = direction;
}

void Maps3DController::setBarType(QDataVis::MeshStyle style, bool smooth)
{
    if (style == QDataVis::Bars) {
        if (smooth)
            m_objFile = QStringLiteral(":/defaultMeshes/barSmooth");
        else
            m_objFile = QStringLiteral(":/defaultMeshes/bar");
    } else if (style == QDataVis::Pyramids) {
        if (smooth)
            m_objFile = QStringLiteral(":/defaultMeshes/pyramidSmooth");
        else
            m_objFile = QStringLiteral(":/defaultMeshes/pyramid");
    } else if (style == QDataVis::Cones) {
        if (smooth)
            m_objFile = QStringLiteral(":/defaultMeshes/coneSmooth");
        else
            m_objFile = QStringLiteral(":/defaultMeshes/cone");
    } else if (style == QDataVis::Cylinders) {
        if (smooth)
            m_objFile = QStringLiteral(":/defaultMeshes/cylinderSmooth");
        else
            m_objFile = QStringLiteral(":/defaultMeshes/cylinder");
    } else if (style == QDataVis::BevelBars) {
        if (smooth)
            m_objFile = QStringLiteral(":/defaultMeshes/bevelbarSmooth");
        else
            m_objFile = QStringLiteral(":/defaultMeshes/bevelbar");
    } else if (style == QDataVis::Spheres) {
        if (smooth)
            m_objFile = QStringLiteral(":/defaultMeshes/sphereSmooth");
        else
            m_objFile = QStringLiteral(":/defaultMeshes/sphere");
    }
    // Reload mesh data
    if (m_isInitialized)
        loadBarMesh();
}

void Maps3DController::setMeshFileName(const QString &objFileName)
{
    m_objFile = objFileName;
}

void Maps3DController::setCameraPreset(QDataVis::CameraPreset preset)
{
    m_camera->setCameraPreset(preset);
}

void Maps3DController::setCameraPosition(GLfloat horizontal, GLfloat vertical, GLint distance)
{
    m_horizontalRotation = qBound(-180.0f, horizontal, 180.0f);
    m_verticalRotation = qBound(0.0f, vertical, 90.0f);
    m_zoomLevel = qBound(10, distance, 500);
    m_camera->setCameraRotation(QPointF(m_horizontalRotation, m_verticalRotation));
    //qDebug() << "camera rotation set to" << m_horizontalRotation << m_verticalRotation;
}

void Maps3DController::setTheme(QDataVis::ColorTheme colorTheme)
{
    m_theme->useColorTheme(colorTheme);
    m_drawer->setTheme(*m_theme);
#if !defined(QT_OPENGL_ES_2)
    if (m_shadowQuality > QDataVis::ShadowNone) {
        // Re-init shaders
        if (!m_theme->m_uniformColor) {
            initShaders(QStringLiteral(":/shaders/vertexShadow"),
                        QStringLiteral(":/shaders/fragmentShadowNoTexColorOnY"));
        } else {
            initShaders(QStringLiteral(":/shaders/vertexShadow"),
                        QStringLiteral(":/shaders/fragmentShadowNoTex"));
        }
    } else {
        // Re-init shaders
        if (!m_theme->m_uniformColor) {
            initShaders(QStringLiteral(":/shaders/vertex"),
                        QStringLiteral(":/shaders/fragmentColorOnY"));
        } else {
            initShaders(QStringLiteral(":/shaders/vertex"),
                        QStringLiteral(":/shaders/fragment"));
        }
    }
#else
    if (!m_theme->m_uniformColor) {
        initShaders(QStringLiteral(":/shaders/vertexES2"),
                    QStringLiteral(":/shaders/fragmentColorOnYES2"));
    } else {
        initShaders(QStringLiteral(":/shaders/vertexES2"),
                    QStringLiteral(":/shaders/fragmentES2"));
    }
#endif
    m_updateLabels = true;
}

void Maps3DController::setBarColor(QColor baseColor, QColor heightColor, bool uniform)
{
    m_theme->m_baseColor = baseColor;
    m_theme->m_heightColor = heightColor;
    if (m_theme->m_uniformColor != uniform) {
#if !defined(QT_OPENGL_ES_2)
        if (m_shadowQuality > QDataVis::ShadowNone) {
            // Re-init shaders
            if (!m_theme->m_uniformColor) {
                initShaders(QStringLiteral(":/shaders/vertexShadow"),
                            QStringLiteral(":/shaders/fragmentShadowNoTexColorOnY"));
            } else {
                initShaders(QStringLiteral(":/shaders/vertexShadow"),
                            QStringLiteral(":/shaders/fragmentShadowNoTex"));
            }
        } else {
            // Re-init shaders
            if (!m_theme->m_uniformColor) {
                initShaders(QStringLiteral(":/shaders/vertex"),
                            QStringLiteral(":/shaders/fragmentColorOnY"));
            } else {
                initShaders(QStringLiteral(":/shaders/vertex"),
                            QStringLiteral(":/shaders/fragment"));
            }
        }
#else
        if (!m_theme->m_uniformColor) {
            initShaders(QStringLiteral(":/shaders/vertexES2"),
                        QStringLiteral(":/shaders/fragmentColorOnYES2"));
        } else {
            initShaders(QStringLiteral(":/shaders/vertexES2"),
                        QStringLiteral(":/shaders/fragmentES2"));
        }
#endif
    }
    m_theme->m_uniformColor = uniform;
}

void Maps3DController::setAreaSpecs(const QRect &areaRect, const QImage &image)
{
    calculateSceneScalingFactors(areaRect);
    setImage(image);
}

void Maps3DController::setImage(const QImage &image)
{
    m_bgrHasAlpha = image.hasAlphaChannel();
    if (m_bgrTexture)
        glDeleteTextures(1, &m_bgrTexture);
    m_bgrTexture = m_textureHelper->create2DTexture(image, true, true);
}

void Maps3DController::setSelectionMode(QDataVis::SelectionMode mode)
{
    m_selectionMode = mode;
    // Disable zoom if mode changes
    //m_zoomActivated = false;
    //m_sceneViewPort = QRect(0, 0, width(), height());
}

QDataVis::SelectionMode Maps3DController::selectionMode()
{
    return m_selectionMode;
}

void Maps3DController::setFontSize(float fontsize)
{
    m_font.setPointSizeF(fontsize);
    m_drawer->setFont(m_font);
    m_updateLabels = true;
}

float Maps3DController::fontSize()
{
    return m_font.pointSizeF();
}

void Maps3DController::setFont(const QFont &font)
{
    m_font = font;
    m_drawer->setFont(font);
    m_updateLabels = true;
}

QFont Maps3DController::font()
{
    return m_font;
}

void Maps3DController::setLabelTransparency(QDataVis::LabelTransparency transparency)
{
    m_labelTransparency = transparency;
    m_drawer->setTransparency(transparency);
    m_updateLabels = true;
}

QDataVis::LabelTransparency Maps3DController::labelTransparency()
{
    return m_labelTransparency;
}

QDataVis::ShadowQuality Maps3DController::setShadowQuality(QDataVis::ShadowQuality quality)
{
    m_shadowQuality = quality;
    switch (quality) {
    case QDataVis::ShadowLow:
        //qDebug() << "ShadowLow";
        m_shadowQualityToShader = 33.3f;
        break;
    case QDataVis::ShadowMedium:
        //qDebug() << "ShadowMedium";
        m_shadowQualityToShader = 100.0f;
        break;
    case QDataVis::ShadowHigh:
        //qDebug() << "ShadowHigh";
        m_shadowQualityToShader = 200.0f;
        break;
    default:
        m_shadowQualityToShader = 0.0f;
        break;
    }
    if (m_isInitialized) {
#if !defined(QT_OPENGL_ES_2)
        if (m_shadowQuality > QDataVis::ShadowNone) {
            // Re-init shaders
            if (!m_theme->m_uniformColor) {
                initShaders(QStringLiteral(":/shaders/vertexShadow"),
                            QStringLiteral(":/shaders/fragmentShadowNoTexColorOnY"));
            } else {
                initShaders(QStringLiteral(":/shaders/vertexShadow"),
                            QStringLiteral(":/shaders/fragmentShadowNoTex"));
            }
            initBackgroundShaders(QStringLiteral(":/shaders/vertexShadow"),
                                  QStringLiteral(":/shaders/fragmentShadow"));
        } else {
            // Re-init shaders
            if (!m_theme->m_uniformColor) {
                initShaders(QStringLiteral(":/shaders/vertex"),
                            QStringLiteral(":/shaders/fragmentColorOnY"));
            } else {
                initShaders(QStringLiteral(":/shaders/vertex"),
                            QStringLiteral(":/shaders/fragment"));
            }
            initBackgroundShaders(QStringLiteral(":/shaders/vertexTexture"),
                                  QStringLiteral(":/shaders/fragmentTexture"));
        }
        // Re-init depth buffer
        initDepthBuffer();
#else
        if (!m_theme->m_uniformColor) {
            initShaders(QStringLiteral(":/shaders/vertexES2"),
                        QStringLiteral(":/shaders/fragmentColorOnYES2"));
        } else {
            initShaders(QStringLiteral(":/shaders/vertexES2"),
                        QStringLiteral(":/shaders/fragmentES2"));
        }
        initBackgroundShaders(QStringLiteral(":/shaders/vertexTexture"), // Same vertex shader ok for ES2
                              QStringLiteral(":/shaders/fragmentTextureES2"));
#endif
    }
    return m_shadowQuality;
}

QDataVis::ShadowQuality Maps3DController::shadowQuality()
{
    return m_shadowQuality;
}



const QSize Maps3DController::size()
{
    return m_boundingRect.size();
}

const QRect Maps3DController::boundingRect()
{
    return m_boundingRect;
}

void Maps3DController::setBoundingRect(const QRect boundingRect)
{
    m_boundingRect = boundingRect;
    resizeNotify();
}

void Maps3DController::setWidth(const int width)
{
    m_boundingRect.setWidth(width);
    resizeNotify();
}

int Maps3DController::width()
{
    return m_boundingRect.width();
}

void Maps3DController::setHeight(const int height)
{
    m_boundingRect.setHeight(height);
    resizeNotify();
}

int Maps3DController::height()
{
    return m_boundingRect.height();
}

void Maps3DController::setX(const int x)
{
    m_boundingRect.setX(x);
}

int Maps3DController::x()
{
    return m_boundingRect.x();
}

void Maps3DController::setY(const int y)
{
    m_boundingRect.setY(y);
}

int Maps3DController::y()
{
    return m_boundingRect.y();
}

void Maps3DController::loadBarMesh()
{
    if (m_barObj)
        delete m_barObj;
    m_barObj = new ObjectHelper(m_objFile);
    m_barObj->load();
}

void Maps3DController::loadBackgroundMesh()
{
    if (m_backgroundObj)
        delete m_backgroundObj;
    m_backgroundObj = new ObjectHelper(QStringLiteral(":/defaultMeshes/label"));
    m_backgroundObj->load();
}

void Maps3DController::loadGridLineMesh()
{
    if (m_gridLineObj)
        delete m_gridLineObj;
    m_gridLineObj = new ObjectHelper(QStringLiteral(":/defaultMeshes/bar"));
    m_gridLineObj->load();
}

void Maps3DController::loadLabelMesh()
{
    if (m_labelObj)
        delete m_labelObj;
    m_labelObj = new ObjectHelper(QStringLiteral(":/defaultMeshes/label"));
    m_labelObj->load();
}

void Maps3DController::initShaders(const QString &vertexShader, const QString &fragmentShader)
{
    if (m_barShader)
        delete m_barShader;
    m_barShader = new ShaderHelper(this, vertexShader, fragmentShader);
    m_barShader->initialize();
}

void Maps3DController::initSelectionShader()
{
    if (m_selectionShader)
        delete m_selectionShader;
    m_selectionShader = new ShaderHelper(this, QStringLiteral(":/shaders/vertexSelection"),
                                         QStringLiteral(":/shaders/fragmentSelection"));
    m_selectionShader->initialize();
}

void Maps3DController::initSelectionBuffer()
{
    if (m_selectionTexture)
        m_textureHelper->deleteTexture(&m_selectionTexture);

    m_selectionTexture = m_textureHelper->createSelectionTexture(this->size(),
                                                                 m_selectionFrameBuffer,
                                                                 m_selectionDepthBuffer);
}

#if !defined(QT_OPENGL_ES_2)
void Maps3DController::initDepthShader()
{
    if (m_depthShader)
        delete m_depthShader;
    m_depthShader = new ShaderHelper(this, QStringLiteral(":/shaders/vertexDepth"),
                                     QStringLiteral(":/shaders/fragmentDepth"));
    m_depthShader->initialize();
}

void Maps3DController::initDepthBuffer()
{
    if (!m_isInitialized)
        return;

    if (m_depthTexture) {
        m_textureHelper->deleteTexture(&m_depthTexture);
        m_depthTexture = 0;
    }

    if (m_shadowQuality > QDataVis::ShadowNone) {
        m_depthTexture = m_textureHelper->createDepthTexture(this->size(), m_depthFrameBuffer,
                                                             m_shadowQuality);
        if (!m_depthTexture) {
            switch (m_shadowQuality) {
            case QDataVis::ShadowHigh:
                qWarning("Creating high quality shadows failed. Changing to medium quality.");
                (void)setShadowQuality(QDataVis::ShadowMedium);
                break;
            case QDataVis::ShadowMedium:
                qWarning("Creating medium quality shadows failed. Changing to low quality.");
                (void)setShadowQuality(QDataVis::ShadowLow);
                break;
            case QDataVis::ShadowLow:
                qWarning("Creating low quality shadows failed. Switching shadows off.");
                (void)setShadowQuality(QDataVis::ShadowNone);
                break;
            default:
                // Cannot get here
                break;
            }
        }
    }
}
#endif

void Maps3DController::initBackgroundShaders(const QString &vertexShader,
                                             const QString &fragmentShader)
{
    if (m_backgroundShader)
        delete m_backgroundShader;
    m_backgroundShader = new ShaderHelper(this, vertexShader, fragmentShader);
    m_backgroundShader->initialize();
}

void Maps3DController::initLabelShaders(const QString &vertexShader, const QString &fragmentShader)
{
    if (m_labelShader)
        delete m_labelShader;
    m_labelShader = new ShaderHelper(this, vertexShader, fragmentShader);
    m_labelShader->initialize();
}

void Maps3DController::updateTextures()
{
    // Drawer has changed; this flag needs to be checked when checking if we need to update labels
    m_updateLabels = true;
}

void Maps3DController::calculateSceneScalingFactors(const QRect &areaRect)
{
    m_areaSize = areaRect.size();
    // Calculate scaling factor so that we can be sure the whole area fits to positive z space
    if (zComp > 1.0f)
        m_scaleFactor = qMax(m_areaSize.width(), m_areaSize.height()) / zComp;
    else
        m_scaleFactor = qMax(m_areaSize.width(), m_areaSize.height());
    //qDebug() << "scaleFactor" << m_scaleFactor;
}

void Maps3DController::calculateHeightAdjustment(const QPair<GLfloat, GLfloat> &limits)
{
    // 2.0f = max difference between minimum and maximum value after scaling with m_heightNormalizer
    m_yAdjustment = 2.0f - ((limits.second - limits.first) / m_heightNormalizer);
    //qDebug() << m_yAdjustment;
}

void Maps3DController::calculateTranslation(MapRenderItem &item)
{
    // We need to convert position (which is in coordinates), to translation (which has origin in the center and is scaled)
    // -> move pos(center, center) to trans(0, 0) and pos(0, 0) to trans(left, top)
    GLfloat xTrans = 2.0f * (item.mapPosition().x() - (m_areaSize.width() / 2.0f))
                     / m_scaleFactor;
    GLfloat zTrans = 2.0f * (item.mapPosition().y() - (m_areaSize.height() / 2.0f))
                     / m_scaleFactor;
    //qDebug() << "x, y" << item.mapPosition().x() << item.mapPosition().y();
    item.setTranslation(QVector3D(xTrans, 0.0f, zTrans + zComp));
    //qDebug() << item.translation();
}

Maps3DController::SelectionType Maps3DController::isSelected(GLint bar, const QVector3D &selection)
{
    GLubyte barIdxRed = 0;
    GLubyte barIdxGreen = 0;
    GLubyte barIdxBlue = 0;
    //static QVector3D prevSel = selection; // TODO: For debugging
    SelectionType isSelectedType = SelectionNone;

    if (selection == skipColor)
        return isSelectedType; // skip window

    if (bar <= 255) {
        barIdxRed = bar;
    } else if (bar <= 65535) {
        barIdxGreen = bar / 256;
        barIdxRed = bar % 256;
    } else {
        barIdxBlue = bar / 65535;
        barIdxGreen = bar % 65535;
        barIdxRed = bar % 256;
    }

    QVector3D current = QVector3D(barIdxRed, barIdxGreen, barIdxBlue);

    // TODO: For debugging
    //if (selection != prevSel) {
    //    qDebug() << selection.x() << selection .y() << selection.z();
    //    prevSel = selection;
    //}

    if (current == selection)
        isSelectedType = SelectionBar;

    return isSelectedType;
}

bool Maps3DController::isValid(const MapRenderItem &item)
{
    bool retval = true;
    if (item.value() < 0) {
        qCritical("Data item value out of range");
        retval = false;
    } else if (item.mapPosition().x() < 0 || item.mapPosition().x() > m_areaSize.width()) {
        qCritical("Data item x position out of range");
        retval = false;
    } else if (item.mapPosition().y() < 0 || item.mapPosition().y() > m_areaSize.height()) {
        qCritical("Data item y position out of range");
        retval = false;
    }
    return retval;
}

void Maps3DController::setDataProxy(QMapDataProxy *proxy)
{
    delete m_data;
    m_data = proxy;

    QObject::connect(m_data, &QMapDataProxy::arrayReset, this, &Maps3DController::handleArrayReset);
    QObject::connect(m_data, &QMapDataProxy::itemsAdded, this, &Maps3DController::handleItemsAdded);
    QObject::connect(m_data, &QMapDataProxy::itemsChanged, this, &Maps3DController::handleItemsChanged);
    QObject::connect(m_data, &QMapDataProxy::itemsRemoved, this, &Maps3DController::handleItemsRemoved);
    QObject::connect(m_data, &QMapDataProxy::itemsInserted, this, &Maps3DController::handleItemsInserted);

    // emit something? Renderer might be interested?
}

QMapDataProxy *Maps3DController::dataProxy()
{
    return m_data;
}

void Maps3DController::handleLimitChange()
{
    QPair<GLfloat, GLfloat> limits = m_data->dptr()->limitValues();
    m_heightNormalizer = qMax(qAbs(limits.second), qAbs(limits.first));
    calculateHeightAdjustment(limits);

    //emit limitsChanged(limits);
}

void Maps3DController::handleArrayReset()
{
    handleLimitChange();
    m_valuesDirty = true;
}

void Maps3DController::handleItemsAdded(int startIndex, int count)
{
    Q_UNUSED(startIndex)
    Q_UNUSED(count)
    // TODO should dirty only affected values?
    handleLimitChange();
    m_valuesDirty = true;
}

void Maps3DController::handleItemsChanged(int startIndex, int count)
{
    Q_UNUSED(startIndex)
    Q_UNUSED(count)
    // TODO should dirty only affected values?
    handleLimitChange();
    m_valuesDirty = true;
}

void Maps3DController::handleItemsRemoved(int startIndex, int count)
{
    Q_UNUSED(startIndex)
    Q_UNUSED(count)
    // TODO should dirty only affected values?
    handleLimitChange();
    m_valuesDirty = true;
}

void Maps3DController::handleItemsInserted(int startIndex, int count)
{
    Q_UNUSED(startIndex)
    Q_UNUSED(count)
    // TODO should dirty only affected values?
    handleLimitChange();
    m_valuesDirty = true;
}

QT_DATAVIS3D_END_NAMESPACE
