/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "enumtostringmap_p.h"

#ifdef VERBOSE_STATE_STORE

#include <QDebug>

static EnumToStringMap *theInstance = 0;
static unsigned int theInstanceCount = 0;

EnumToStringMap *EnumToStringMap::newInstance()
{
    if (theInstance) {
        theInstanceCount++;
        return theInstance;
    }

    theInstance = new EnumToStringMap();
    theInstanceCount++;
    return theInstance;
}

void EnumToStringMap::deleteInstance()
{
    theInstanceCount--;
    if (theInstanceCount <= 0) {
        delete theInstance;
        theInstance = 0;
    }
}

EnumToStringMap::EnumToStringMap() :
    m_unknown("<unknown>")
{
    m_map[GL_ZERO] = "ZERO/null";

    m_map[GL_DEPTH_BUFFER_BIT] = "DEPTH_BUFFER_BIT";
    m_map[GL_STENCIL_BUFFER_BIT] = "STENCIL_BUFFER_BIT";
    m_map[GL_COLOR_BUFFER_BIT] = "COLOR_BUFFER_BIT";

    m_map[GL_STENCIL_BUFFER_BIT] = "STENCIL_BUFFER_BIT";
    m_map[GL_COLOR_BUFFER_BIT] = "COLOR_BUFFER_BIT";

    m_map[GL_POINTS] = "POINTS";
    m_map[GL_LINES] = "LINES";
    m_map[GL_LINE_LOOP] = "LINE_LOOP";
    m_map[GL_LINE_STRIP] = "LINE_STRIP";
    m_map[GL_TRIANGLES] = "TRIANGLES";
    m_map[GL_TRIANGLE_STRIP] = "TRIANGLE_STRIP";
    m_map[GL_TRIANGLE_FAN] = "TRIANGLE_FAN";

    m_map[GL_SRC_COLOR] = "SRC_COLOR";
    m_map[GL_ONE_MINUS_SRC_COLOR] = "ONE_MINUS_SRC_COLOR";
    m_map[GL_SRC_ALPHA] = "SRC_ALPHA";
    m_map[GL_ONE_MINUS_SRC_ALPHA] = "ONE_MINUS_SRC_ALPHA";
    m_map[GL_DST_ALPHA] = "DST_ALPHA";
    m_map[GL_ONE_MINUS_DST_ALPHA] = "ONE_MINUS_DST_ALPHA";

    m_map[GL_DST_COLOR] = "DST_COLOR";
    m_map[GL_ONE_MINUS_DST_COLOR] = "ONE_MINUS_DST_COLOR";
    m_map[GL_SRC_ALPHA_SATURATE] = "SRC_ALPHA_SATURATE";

    m_map[GL_FUNC_ADD] = "FUNC_ADD";
    m_map[GL_BLEND_EQUATION] = "BLEND_EQUATION";
    m_map[GL_BLEND_EQUATION_RGB] = "BLEND_EQUATION_RGB";
    m_map[GL_BLEND_EQUATION_ALPHA] = "BLEND_EQUATION_ALPHA";

    m_map[GL_FUNC_SUBTRACT] = "FUNC_SUBTRACT";
    m_map[GL_FUNC_REVERSE_SUBTRACT] = "FUNC_REVERSE_SUBTRACT";

    m_map[GL_BLEND_DST_RGB] = "BLEND_DST_RGB";
    m_map[GL_BLEND_SRC_RGB] = "BLEND_SRC_RGB";
    m_map[GL_BLEND_DST_ALPHA] = "BLEND_DST_ALPHA";
    m_map[GL_BLEND_SRC_ALPHA] = "BLEND_SRC_ALPHA";
    m_map[GL_CONSTANT_COLOR] = "CONSTANT_COLOR";
    m_map[GL_ONE_MINUS_CONSTANT_COLOR] = "ONE_MINUS_CONSTANT_COLOR";
    m_map[GL_CONSTANT_ALPHA] = "CONSTANT_ALPHA";
    m_map[GL_ONE_MINUS_CONSTANT_ALPHA] = "ONE_MINUS_CONSTANT_ALPHA";
    m_map[GL_BLEND_COLOR] = "BLEND_COLOR";

    m_map[GL_ARRAY_BUFFER] = "ARRAY_BUFFER";
    m_map[GL_ELEMENT_ARRAY_BUFFER] = "ELEMENT_ARRAY_BUFFER";
    m_map[GL_ARRAY_BUFFER_BINDING] = "ARRAY_BUFFER_BINDING";
    m_map[GL_ELEMENT_ARRAY_BUFFER_BINDING] = "ELEMENT_ARRAY_BUFFER_BINDING";

    m_map[GL_STREAM_DRAW] = "STREAM_DRAW";
    m_map[GL_STATIC_DRAW] = "STATIC_DRAW";
    m_map[GL_DYNAMIC_DRAW] = "DYNAMIC_DRAW";

    m_map[GL_BUFFER_SIZE] = "BUFFER_SIZE";
    m_map[GL_BUFFER_USAGE] = "BUFFER_USAGE";

    m_map[GL_CURRENT_VERTEX_ATTRIB] = "CURRENT_VERTEX_ATTRIB";

    m_map[GL_FRONT] = "FRONT";
    m_map[GL_BACK] = "BACK";
    m_map[GL_FRONT_AND_BACK] = "FRONT_AND_BACK";

    m_map[GL_CULL_FACE] = "CULL_FACE";
    m_map[GL_BLEND] = "BLEND";
    m_map[GL_DITHER] = "DITHER";
    m_map[GL_STENCIL_TEST] = "STENCIL_TEST";
    m_map[GL_DEPTH_TEST] = "DEPTH_TEST";
    m_map[GL_SCISSOR_TEST] = "SCISSOR_TEST";
    m_map[GL_POLYGON_OFFSET_FILL] = "POLYGON_OFFSET_FILL";
    m_map[GL_SAMPLE_ALPHA_TO_COVERAGE] = "SAMPLE_ALPHA_TO_COVERAGE";
    m_map[GL_SAMPLE_COVERAGE] = "SAMPLE_COVERAGE";

    m_map[GL_NO_ERROR] = "NO_ERROR";
    m_map[GL_INVALID_ENUM] = "INVALID_ENUM";
    m_map[GL_INVALID_VALUE] = "INVALID_VALUE";
    m_map[GL_INVALID_OPERATION] = "INVALID_OPERATION";
    m_map[GL_OUT_OF_MEMORY] = "OUT_OF_MEMORY";

    m_map[GL_CW] = "CW";
    m_map[GL_CCW] = "CCW";

    m_map[GL_LINE_WIDTH] = "LINE_WIDTH";
    m_map[GL_ALIASED_POINT_SIZE_RANGE] = "ALIASED_POINT_SIZE_RANGE";
    m_map[GL_ALIASED_LINE_WIDTH_RANGE] = "ALIASED_LINE_WIDTH_RANGE";
    m_map[GL_CULL_FACE_MODE] = "CULL_FACE_MODE";
    m_map[GL_FRONT_FACE] = "FRONT_FACE";
    m_map[GL_DEPTH_RANGE] = "DEPTH_RANGE";
    m_map[GL_DEPTH_WRITEMASK] = "DEPTH_WRITEMASK";
    m_map[GL_DEPTH_CLEAR_VALUE] = "DEPTH_CLEAR_VALUE";
    m_map[GL_DEPTH_FUNC] = "DEPTH_FUNC";
    m_map[GL_STENCIL_CLEAR_VALUE] = "STENCIL_CLEAR_VALUE";
    m_map[GL_STENCIL_FUNC] = "STENCIL_FUNC";
    m_map[GL_STENCIL_FAIL] = "STENCIL_FAIL";
    m_map[GL_STENCIL_PASS_DEPTH_FAIL] = "STENCIL_PASS_DEPTH_FAIL";
    m_map[GL_STENCIL_PASS_DEPTH_PASS] = "STENCIL_PASS_DEPTH_PASS";
    m_map[GL_STENCIL_REF] = "STENCIL_REF";
    m_map[GL_STENCIL_VALUE_MASK] = "STENCIL_VALUE_MASK";
    m_map[GL_STENCIL_WRITEMASK] = "STENCIL_WRITEMASK";
    m_map[GL_STENCIL_BACK_FUNC] = "STENCIL_BACK_FUNC";
    m_map[GL_STENCIL_BACK_FAIL] = "STENCIL_BACK_FAIL";
    m_map[GL_STENCIL_BACK_PASS_DEPTH_FAIL] = "STENCIL_BACK_PASS_DEPTH_FAIL";
    m_map[GL_STENCIL_BACK_PASS_DEPTH_PASS] = "STENCIL_BACK_PASS_DEPTH_PASS";
    m_map[GL_STENCIL_BACK_REF] = "STENCIL_BACK_REF";
    m_map[GL_STENCIL_BACK_VALUE_MASK] = "STENCIL_BACK_VALUE_MASK";
    m_map[GL_STENCIL_BACK_WRITEMASK] = "STENCIL_BACK_WRITEMASK";
    m_map[GL_VIEWPORT] = "VIEWPORT";
    m_map[GL_SCISSOR_BOX] = "SCISSOR_BOX";

    m_map[GL_COLOR_CLEAR_VALUE] = "COLOR_CLEAR_VALUE";
    m_map[GL_COLOR_WRITEMASK] = "COLOR_WRITEMASK";
    m_map[GL_UNPACK_ALIGNMENT] = "UNPACK_ALIGNMENT";
    m_map[GL_PACK_ALIGNMENT] = "PACK_ALIGNMENT";
    m_map[GL_MAX_TEXTURE_SIZE] = "MAX_TEXTURE_SIZE";
    m_map[GL_MAX_VIEWPORT_DIMS] = "MAX_VIEWPORT_DIMS";
    m_map[GL_SUBPIXEL_BITS] = "SUBPIXEL_BITS";
    m_map[GL_RED_BITS] = "RED_BITS";
    m_map[GL_GREEN_BITS] = "GREEN_BITS";
    m_map[GL_BLUE_BITS] = "BLUE_BITS";
    m_map[GL_ALPHA_BITS] = "ALPHA_BITS";
    m_map[GL_DEPTH_BITS] = "DEPTH_BITS";
    m_map[GL_STENCIL_BITS] = "STENCIL_BITS";
    m_map[GL_POLYGON_OFFSET_UNITS] = "POLYGON_OFFSET_UNITS";

    m_map[GL_POLYGON_OFFSET_FACTOR] = "POLYGON_OFFSET_FACTOR";
    m_map[GL_TEXTURE_BINDING_2D] = "TEXTURE_BINDING_2D";
    m_map[GL_SAMPLE_BUFFERS] = "SAMPLE_BUFFERS";
    m_map[GL_SAMPLES] = "SAMPLES";
    m_map[GL_SAMPLE_COVERAGE_VALUE] = "SAMPLE_COVERAGE_VALUE";
    m_map[GL_SAMPLE_COVERAGE_INVERT] = "SAMPLE_COVERAGE_INVERT";

    m_map[GL_COMPRESSED_TEXTURE_FORMATS] = "COMPRESSED_TEXTURE_FORMATS";

    m_map[GL_DONT_CARE] = "DONT_CARE";
    m_map[GL_FASTEST] = "FASTEST";
    m_map[GL_NICEST] = "NICEST";

    m_map[GL_GENERATE_MIPMAP_HINT] = "GENERATE_MIPMAP_HINT";

    m_map[GL_BYTE] = "BYTE";
    m_map[GL_UNSIGNED_BYTE] = "UNSIGNED_BYTE";
    m_map[GL_SHORT] = "SHORT";
    m_map[GL_UNSIGNED_SHORT] = "UNSIGNED_SHORT";
    m_map[GL_INT] = "INT";
    m_map[GL_UNSIGNED_INT] = "UNSIGNED_INT";
    m_map[GL_FLOAT] = "FLOAT";

    m_map[GL_DEPTH_COMPONENT] = "DEPTH_COMPONENT";
    m_map[GL_ALPHA] = "ALPHA";
    m_map[GL_RGB] = "RGB";
    m_map[GL_RGBA] = "RGBA";
    m_map[GL_LUMINANCE] = "LUMINANCE";
    m_map[GL_LUMINANCE_ALPHA] = "LUMINANCE_ALPHA";

    m_map[GL_UNSIGNED_SHORT_4_4_4_4] = "UNSIGNED_SHORT_4_4_4_4";
    m_map[GL_UNSIGNED_SHORT_5_5_5_1] = "UNSIGNED_SHORT_5_5_5_1";
    m_map[GL_UNSIGNED_SHORT_5_6_5] = "UNSIGNED_SHORT_5_6_5";

    m_map[GL_FRAGMENT_SHADER] = "FRAGMENT_SHADER";
    m_map[GL_VERTEX_SHADER] = "VERTEX_SHADER";
    m_map[GL_MAX_VERTEX_ATTRIBS] = "MAX_VERTEX_ATTRIBS";
    m_map[GL_MAX_VERTEX_UNIFORM_VECTORS] = "MAX_VERTEX_UNIFORM_VECTORS";
    m_map[GL_MAX_VARYING_VECTORS] = "MAX_VARYING_VECTORS";
    m_map[GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS] = "MAX_COMBINED_TEXTURE_IMAGE_UNITS";
    m_map[GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS] = "MAX_VERTEX_TEXTURE_IMAGE_UNITS";
    m_map[GL_MAX_TEXTURE_IMAGE_UNITS] = "MAX_TEXTURE_IMAGE_UNITS";
    m_map[GL_MAX_FRAGMENT_UNIFORM_VECTORS] = "MAX_FRAGMENT_UNIFORM_VECTORS";
    m_map[GL_SHADER_TYPE] = "SHADER_TYPE";
    m_map[GL_DELETE_STATUS] = "DELETE_STATUS";
    m_map[GL_LINK_STATUS] = "LINK_STATUS";
    m_map[GL_VALIDATE_STATUS] = "VALIDATE_STATUS";
    m_map[GL_ATTACHED_SHADERS] = "ATTACHED_SHADERS";
    m_map[GL_ACTIVE_UNIFORMS] = "ACTIVE_UNIFORMS";
    m_map[GL_ACTIVE_ATTRIBUTES] = "ACTIVE_ATTRIBUTES";
    m_map[GL_SHADING_LANGUAGE_VERSION] = "SHADING_LANGUAGE_VERSION";
    m_map[GL_CURRENT_PROGRAM] = "CURRENT_PROGRAM";

    m_map[GL_NEVER] = "NEVER";
    m_map[GL_LESS] = "LESS";
    m_map[GL_EQUAL] = "EQUAL";
    m_map[GL_LEQUAL] = "LEQUAL";
    m_map[GL_GREATER] = "GREATER";
    m_map[GL_NOTEQUAL] = "NOTEQUAL";
    m_map[GL_GEQUAL] = "GEQUAL";
    m_map[GL_ALWAYS] = "ALWAYS";

    m_map[GL_KEEP] = "KEEP";
    m_map[GL_REPLACE] = "REPLACE";
    m_map[GL_INCR] = "INCR";
    m_map[GL_DECR] = "DECR";
    m_map[GL_INVERT] = "INVERT";
    m_map[GL_INCR_WRAP] = "INCR_WRAP";
    m_map[GL_DECR_WRAP] = "DECR_WRAP";

    m_map[GL_VENDOR] = "VENDOR";
    m_map[GL_RENDERER] = "RENDERER";
    m_map[GL_VERSION] = "VERSION";

    m_map[GL_NEAREST] = "NEAREST";
    m_map[GL_LINEAR] = "LINEAR";

    m_map[GL_NEAREST_MIPMAP_NEAREST] = "NEAREST_MIPMAP_NEAREST";
    m_map[GL_LINEAR_MIPMAP_NEAREST] = "LINEAR_MIPMAP_NEAREST";
    m_map[GL_NEAREST_MIPMAP_LINEAR] = "NEAREST_MIPMAP_LINEAR";
    m_map[GL_LINEAR_MIPMAP_LINEAR] = "LINEAR_MIPMAP_LINEAR";

    m_map[GL_TEXTURE_MAG_FILTER] = "TEXTURE_MAG_FILTER";
    m_map[GL_TEXTURE_MIN_FILTER] = "TEXTURE_MIN_FILTER";
    m_map[GL_TEXTURE_WRAP_S] = "TEXTURE_WRAP_S";
    m_map[GL_TEXTURE_WRAP_T] = "TEXTURE_WRAP_T";

    m_map[GL_TEXTURE_2D] = "TEXTURE_2D";
    m_map[GL_TEXTURE] = "TEXTURE";

    m_map[GL_TEXTURE_CUBE_MAP] = "TEXTURE_CUBE_MAP";
    m_map[GL_TEXTURE_BINDING_CUBE_MAP] = "TEXTURE_BINDING_CUBE_MAP";
    m_map[GL_TEXTURE_CUBE_MAP_POSITIVE_X] = "TEXTURE_CUBE_MAP_POSITIVE_X";
    m_map[GL_TEXTURE_CUBE_MAP_NEGATIVE_X] = "TEXTURE_CUBE_MAP_NEGATIVE_X";
    m_map[GL_TEXTURE_CUBE_MAP_POSITIVE_Y] = "TEXTURE_CUBE_MAP_POSITIVE_Y";
    m_map[GL_TEXTURE_CUBE_MAP_NEGATIVE_Y] = "TEXTURE_CUBE_MAP_NEGATIVE_Y";
    m_map[GL_TEXTURE_CUBE_MAP_POSITIVE_Z] = "TEXTURE_CUBE_MAP_POSITIVE_Z";
    m_map[GL_TEXTURE_CUBE_MAP_NEGATIVE_Z] = "TEXTURE_CUBE_MAP_NEGATIVE_Z";
    m_map[GL_MAX_CUBE_MAP_TEXTURE_SIZE] = "MAX_CUBE_MAP_TEXTURE_SIZE";

    m_map[GL_TEXTURE0] = "TEXTURE0";
    m_map[GL_TEXTURE1] = "TEXTURE1";
    m_map[GL_TEXTURE2] = "TEXTURE2";
    m_map[GL_TEXTURE3] = "TEXTURE3";
    m_map[GL_TEXTURE4] = "TEXTURE4";
    m_map[GL_TEXTURE5] = "TEXTURE5";
    m_map[GL_TEXTURE6] = "TEXTURE6";
    m_map[GL_TEXTURE7] = "TEXTURE7";
    m_map[GL_TEXTURE8] = "TEXTURE8";
    m_map[GL_TEXTURE9] = "TEXTURE9";
    m_map[GL_TEXTURE10] = "TEXTURE10";
    m_map[GL_TEXTURE11] = "TEXTURE11";
    m_map[GL_TEXTURE12] = "TEXTURE12";
    m_map[GL_TEXTURE13] = "TEXTURE13";
    m_map[GL_TEXTURE14] = "TEXTURE14";
    m_map[GL_TEXTURE15] = "TEXTURE15";
    m_map[GL_TEXTURE16] = "TEXTURE16";
    m_map[GL_TEXTURE17] = "TEXTURE17";
    m_map[GL_TEXTURE18] = "TEXTURE18";
    m_map[GL_TEXTURE19] = "TEXTURE19";
    m_map[GL_TEXTURE20] = "TEXTURE20";
    m_map[GL_TEXTURE21] = "TEXTURE21";
    m_map[GL_TEXTURE22] = "TEXTURE22";
    m_map[GL_TEXTURE23] = "TEXTURE23";
    m_map[GL_TEXTURE24] = "TEXTURE24";
    m_map[GL_TEXTURE25] = "TEXTURE25";
    m_map[GL_TEXTURE26] = "TEXTURE26";
    m_map[GL_TEXTURE27] = "TEXTURE27";
    m_map[GL_TEXTURE28] = "TEXTURE28";
    m_map[GL_TEXTURE29] = "TEXTURE29";
    m_map[GL_TEXTURE30] = "TEXTURE30";
    m_map[GL_TEXTURE31] = "TEXTURE31";
    m_map[GL_ACTIVE_TEXTURE] = "ACTIVE_TEXTURE";

    m_map[GL_REPEAT] = "REPEAT";
    m_map[GL_CLAMP_TO_EDGE] = "CLAMP_TO_EDGE";
    m_map[GL_MIRRORED_REPEAT] = "MIRRORED_REPEAT";

    m_map[GL_FLOAT_VEC2] = "FLOAT_VEC2";
    m_map[GL_FLOAT_VEC3] = "FLOAT_VEC3";
    m_map[GL_FLOAT_VEC4] = "FLOAT_VEC4";
    m_map[GL_INT_VEC2] = "INT_VEC2";
    m_map[GL_INT_VEC3] = "INT_VEC3";
    m_map[GL_INT_VEC4] = "INT_VEC4";
    m_map[GL_BOOL] = "BOOL";
    m_map[GL_BOOL_VEC2] = "BOOL_VEC2";
    m_map[GL_BOOL_VEC3] = "BOOL_VEC3";
    m_map[GL_BOOL_VEC4] = "BOOL_VEC4";
    m_map[GL_FLOAT_MAT2] = "FLOAT_MAT2";
    m_map[GL_FLOAT_MAT3] = "FLOAT_MAT3";
    m_map[GL_FLOAT_MAT4] = "FLOAT_MAT4";
    m_map[GL_SAMPLER_2D] = "SAMPLER_2D";
    m_map[GL_SAMPLER_CUBE] = "SAMPLER_CUBE";

    m_map[GL_VERTEX_ATTRIB_ARRAY_ENABLED] = "VERTEX_ATTRIB_ARRAY_ENABLED";
    m_map[GL_VERTEX_ATTRIB_ARRAY_SIZE] = "VERTEX_ATTRIB_ARRAY_SIZE";
    m_map[GL_VERTEX_ATTRIB_ARRAY_STRIDE] = "VERTEX_ATTRIB_ARRAY_STRIDE";
    m_map[GL_VERTEX_ATTRIB_ARRAY_TYPE] = "VERTEX_ATTRIB_ARRAY_TYPE";
    m_map[GL_VERTEX_ATTRIB_ARRAY_NORMALIZED] = "VERTEX_ATTRIB_ARRAY_NORMALIZED";
    m_map[GL_VERTEX_ATTRIB_ARRAY_POINTER] = "VERTEX_ATTRIB_ARRAY_POINTER";
    m_map[GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING] = "VERTEX_ATTRIB_ARRAY_BUFFER_BINDING";

    m_map[GL_COMPILE_STATUS] = "COMPILE_STATUS";

    m_map[GL_LOW_FLOAT] = "LOW_FLOAT";
    m_map[GL_MEDIUM_FLOAT] = "MEDIUM_FLOAT";
    m_map[GL_HIGH_FLOAT] = "HIGH_FLOAT";
    m_map[GL_LOW_INT] = "LOW_INT";
    m_map[GL_MEDIUM_INT] = "MEDIUM_INT";
    m_map[GL_HIGH_INT] = "HIGH_INT";

    m_map[GL_FRAMEBUFFER] = "FRAMEBUFFER";
    m_map[GL_RENDERBUFFER] = "RENDERBUFFER";

    m_map[GL_RGBA4] = "RGBA4";
    m_map[GL_RGB5_A1] = "RGB5_A1";
    m_map[GL_RGB565] = "RGB565";
    m_map[GL_DEPTH_COMPONENT16] = "DEPTH_COMPONENT16";
    m_map[GL_STENCIL_INDEX] = "STENCIL_INDEX";
    m_map[GL_STENCIL_INDEX8] = "STENCIL_INDEX8";

    m_map[GL_RENDERBUFFER_WIDTH] = "RENDERBUFFER_WIDTH";
    m_map[GL_RENDERBUFFER_HEIGHT] = "RENDERBUFFER_HEIGHT";
    m_map[GL_RENDERBUFFER_INTERNAL_FORMAT] = "RENDERBUFFER_INTERNAL_FORMAT";
    m_map[GL_RENDERBUFFER_RED_SIZE] = "RENDERBUFFER_RED_SIZE";
    m_map[GL_RENDERBUFFER_GREEN_SIZE] = "RENDERBUFFER_GREEN_SIZE";
    m_map[GL_RENDERBUFFER_BLUE_SIZE] = "RENDERBUFFER_BLUE_SIZE";
    m_map[GL_RENDERBUFFER_ALPHA_SIZE] = "RENDERBUFFER_ALPHA_SIZE";
    m_map[GL_RENDERBUFFER_DEPTH_SIZE] = "RENDERBUFFER_DEPTH_SIZE";
    m_map[GL_RENDERBUFFER_STENCIL_SIZE] = "RENDERBUFFER_STENCIL_SIZE";

    m_map[GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE] = "FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE";
    m_map[GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME] = "FRAMEBUFFER_ATTACHMENT_OBJECT_NAME";
    m_map[GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL] = "FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL";
    m_map[GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE] =
            "FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE";

    m_map[GL_COLOR_ATTACHMENT0] = "COLOR_ATTACHMENT0";
    m_map[GL_DEPTH_ATTACHMENT] = "DEPTH_ATTACHMENT";
    m_map[GL_STENCIL_ATTACHMENT] = "STENCIL_ATTACHMENT";

    m_map[GL_FRAMEBUFFER_COMPLETE] = "FRAMEBUFFER_COMPLETE";
    m_map[GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT] = "FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    m_map[GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT] =
            "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    m_map[GL_FRAMEBUFFER_UNSUPPORTED] = "FRAMEBUFFER_UNSUPPORTED";

    m_map[GL_FRAMEBUFFER_BINDING] = "FRAMEBUFFER_BINDING";
#if !defined(QT_OPENGL_ES_2)
    m_map[GL_RENDERBUFFER_BINDING] = "RENDERBUFFER_BINDING";
#else
    m_map[GL_RENDERBUFFER] = "RENDERBUFFER_BINDING";
#endif
    m_map[GL_MAX_RENDERBUFFER_SIZE] = "MAX_RENDERBUFFER_SIZE";

    m_map[GL_INVALID_FRAMEBUFFER_OPERATION] = "INVALID_FRAMEBUFFER_OPERATION";
}

QString EnumToStringMap::lookUp(const GLuint value) const
{
    if (m_map.contains(value))
        return m_map[value];

    return QString("0x0%1").arg(value, 0, 16);
}
#endif
