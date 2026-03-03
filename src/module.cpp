#include <napi.h>
#include "egl_context.h"
#include "gl_bindings.h"

static GLESContext g_ctx = {};

static Napi::Value createContext(const Napi::CallbackInfo& info) {
    int width = info[0].As<Napi::Number>().Int32Value();
    int height = info[1].As<Napi::Number>().Int32Value();
    bool ok = gles_context_create(&g_ctx, width, height);
    return Napi::Boolean::New(info.Env(), ok);
}

static Napi::Value destroyContext(const Napi::CallbackInfo& info) {
    gles_context_destroy(&g_ctx);
    return info.Env().Undefined();
}

static Napi::Value resizeContext(const Napi::CallbackInfo& info) {
    int width = info[0].As<Napi::Number>().Int32Value();
    int height = info[1].As<Napi::Number>().Int32Value();
    bool ok = gles_context_resize(&g_ctx, width, height);
    return Napi::Boolean::New(info.Env(), ok);
}

static Napi::Value makeCurrent(const Napi::CallbackInfo& info) {
    bool ok = gles_context_make_current(&g_ctx);
    return Napi::Boolean::New(info.Env(), ok);
}

static Napi::Value getContextInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto obj = Napi::Object::New(env);
    obj.Set("valid", Napi::Boolean::New(env, g_ctx.valid));
    obj.Set("width", Napi::Number::New(env, g_ctx.width));
    obj.Set("height", Napi::Number::New(env, g_ctx.height));
    return obj;
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
    // Context management
    exports.Set("createContext", Napi::Function::New<createContext>(env));
    exports.Set("destroyContext", Napi::Function::New<destroyContext>(env));
    exports.Set("resizeContext", Napi::Function::New<resizeContext>(env));
    exports.Set("makeCurrent", Napi::Function::New<makeCurrent>(env));
    exports.Set("getContextInfo", Napi::Function::New<getContextInfo>(env));

    // State
    exports.Set("glEnable", Napi::Function::New<gl::_enable>(env));
    exports.Set("glDisable", Napi::Function::New<gl::_disable>(env));
    exports.Set("glGetError", Napi::Function::New<gl::_getError>(env));
    exports.Set("glGetIntegerv", Napi::Function::New<gl::_getIntegerv>(env));
    exports.Set("glGetString", Napi::Function::New<gl::_getString>(env));
    exports.Set("glFinish", Napi::Function::New<gl::_finish>(env));
    exports.Set("glFlush", Napi::Function::New<gl::_flush>(env));
    exports.Set("glHint", Napi::Function::New<gl::_hint>(env));
    exports.Set("glPixelStorei", Napi::Function::New<gl::_pixelStorei>(env));

    // Viewport/Clear
    exports.Set("glViewport", Napi::Function::New<gl::_viewport>(env));
    exports.Set("glScissor", Napi::Function::New<gl::_scissor>(env));
    exports.Set("glClear", Napi::Function::New<gl::_clear>(env));
    exports.Set("glClearColor", Napi::Function::New<gl::_clearColor>(env));
    exports.Set("glClearDepthf", Napi::Function::New<gl::_clearDepthf>(env));
    exports.Set("glClearStencil", Napi::Function::New<gl::_clearStencil>(env));

    // Blending
    exports.Set("glBlendFunc", Napi::Function::New<gl::_blendFunc>(env));
    exports.Set("glBlendFuncSeparate", Napi::Function::New<gl::_blendFuncSeparate>(env));
    exports.Set("glBlendEquation", Napi::Function::New<gl::_blendEquation>(env));
    exports.Set("glBlendEquationSeparate", Napi::Function::New<gl::_blendEquationSeparate>(env));
    exports.Set("glBlendColor", Napi::Function::New<gl::_blendColor>(env));
    exports.Set("glColorMask", Napi::Function::New<gl::_colorMask>(env));

    // Depth/Stencil
    exports.Set("glDepthFunc", Napi::Function::New<gl::_depthFunc>(env));
    exports.Set("glDepthMask", Napi::Function::New<gl::_depthMask>(env));
    exports.Set("glDepthRangef", Napi::Function::New<gl::_depthRangef>(env));
    exports.Set("glStencilFunc", Napi::Function::New<gl::_stencilFunc>(env));
    exports.Set("glStencilFuncSeparate", Napi::Function::New<gl::_stencilFuncSeparate>(env));
    exports.Set("glStencilOp", Napi::Function::New<gl::_stencilOp>(env));
    exports.Set("glStencilOpSeparate", Napi::Function::New<gl::_stencilOpSeparate>(env));
    exports.Set("glStencilMask", Napi::Function::New<gl::_stencilMask>(env));
    exports.Set("glStencilMaskSeparate", Napi::Function::New<gl::_stencilMaskSeparate>(env));

    // Face culling
    exports.Set("glCullFace", Napi::Function::New<gl::_cullFace>(env));
    exports.Set("glFrontFace", Napi::Function::New<gl::_frontFace>(env));
    exports.Set("glPolygonOffset", Napi::Function::New<gl::_polygonOffset>(env));
    exports.Set("glLineWidth", Napi::Function::New<gl::_lineWidth>(env));

    // Buffers
    exports.Set("glGenBuffers", Napi::Function::New<gl::_genBuffers>(env));
    exports.Set("glDeleteBuffers", Napi::Function::New<gl::_deleteBuffers>(env));
    exports.Set("glBindBuffer", Napi::Function::New<gl::_bindBuffer>(env));
    exports.Set("glBufferData", Napi::Function::New<gl::_bufferData>(env));
    exports.Set("glBufferSubData", Napi::Function::New<gl::_bufferSubData>(env));

    // Textures
    exports.Set("glGenTextures", Napi::Function::New<gl::_genTextures>(env));
    exports.Set("glDeleteTextures", Napi::Function::New<gl::_deleteTextures>(env));
    exports.Set("glBindTexture", Napi::Function::New<gl::_bindTexture>(env));
    exports.Set("glActiveTexture", Napi::Function::New<gl::_activeTexture>(env));
    exports.Set("glTexImage2D", Napi::Function::New<gl::_texImage2D>(env));
    exports.Set("glTexSubImage2D", Napi::Function::New<gl::_texSubImage2D>(env));
    exports.Set("glTexParameteri", Napi::Function::New<gl::_texParameteri>(env));
    exports.Set("glTexParameterf", Napi::Function::New<gl::_texParameterf>(env));
    exports.Set("glGenerateMipmap", Napi::Function::New<gl::_generateMipmap>(env));
    exports.Set("glCompressedTexImage2D", Napi::Function::New<gl::_compressedTexImage2D>(env));

    // Shaders
    exports.Set("glCreateShader", Napi::Function::New<gl::_createShader>(env));
    exports.Set("glDeleteShader", Napi::Function::New<gl::_deleteShader>(env));
    exports.Set("glShaderSource", Napi::Function::New<gl::_shaderSource>(env));
    exports.Set("glCompileShader", Napi::Function::New<gl::_compileShader>(env));
    exports.Set("glGetShaderiv", Napi::Function::New<gl::_getShaderiv>(env));
    exports.Set("glGetShaderInfoLog", Napi::Function::New<gl::_getShaderInfoLog>(env));

    // Programs
    exports.Set("glCreateProgram", Napi::Function::New<gl::_createProgram>(env));
    exports.Set("glDeleteProgram", Napi::Function::New<gl::_deleteProgram>(env));
    exports.Set("glAttachShader", Napi::Function::New<gl::_attachShader>(env));
    exports.Set("glDetachShader", Napi::Function::New<gl::_detachShader>(env));
    exports.Set("glLinkProgram", Napi::Function::New<gl::_linkProgram>(env));
    exports.Set("glUseProgram", Napi::Function::New<gl::_useProgram>(env));
    exports.Set("glGetProgramiv", Napi::Function::New<gl::_getProgramiv>(env));
    exports.Set("glGetProgramInfoLog", Napi::Function::New<gl::_getProgramInfoLog>(env));
    exports.Set("glValidateProgram", Napi::Function::New<gl::_validateProgram>(env));
    exports.Set("glBindAttribLocation", Napi::Function::New<gl::_bindAttribLocation>(env));
    exports.Set("glGetAttribLocation", Napi::Function::New<gl::_getAttribLocation>(env));
    exports.Set("glGetUniformLocation", Napi::Function::New<gl::_getUniformLocation>(env));

    // Uniforms
    exports.Set("glUniform1i", Napi::Function::New<gl::_uniform1i>(env));
    exports.Set("glUniform2i", Napi::Function::New<gl::_uniform2i>(env));
    exports.Set("glUniform3i", Napi::Function::New<gl::_uniform3i>(env));
    exports.Set("glUniform4i", Napi::Function::New<gl::_uniform4i>(env));
    exports.Set("glUniform1f", Napi::Function::New<gl::_uniform1f>(env));
    exports.Set("glUniform2f", Napi::Function::New<gl::_uniform2f>(env));
    exports.Set("glUniform3f", Napi::Function::New<gl::_uniform3f>(env));
    exports.Set("glUniform4f", Napi::Function::New<gl::_uniform4f>(env));
    exports.Set("glUniform1iv", Napi::Function::New<gl::_uniform1iv>(env));
    exports.Set("glUniform2iv", Napi::Function::New<gl::_uniform2iv>(env));
    exports.Set("glUniform3iv", Napi::Function::New<gl::_uniform3iv>(env));
    exports.Set("glUniform4iv", Napi::Function::New<gl::_uniform4iv>(env));
    exports.Set("glUniform1fv", Napi::Function::New<gl::_uniform1fv>(env));
    exports.Set("glUniform2fv", Napi::Function::New<gl::_uniform2fv>(env));
    exports.Set("glUniform3fv", Napi::Function::New<gl::_uniform3fv>(env));
    exports.Set("glUniform4fv", Napi::Function::New<gl::_uniform4fv>(env));
    exports.Set("glUniformMatrix2fv", Napi::Function::New<gl::_uniformMatrix2fv>(env));
    exports.Set("glUniformMatrix3fv", Napi::Function::New<gl::_uniformMatrix3fv>(env));
    exports.Set("glUniformMatrix4fv", Napi::Function::New<gl::_uniformMatrix4fv>(env));

    // Vertex attribs
    exports.Set("glEnableVertexAttribArray", Napi::Function::New<gl::_enableVertexAttribArray>(env));
    exports.Set("glDisableVertexAttribArray", Napi::Function::New<gl::_disableVertexAttribArray>(env));
    exports.Set("glVertexAttribPointer", Napi::Function::New<gl::_vertexAttribPointer>(env));

    // Drawing
    exports.Set("glDrawArrays", Napi::Function::New<gl::_drawArrays>(env));
    exports.Set("glDrawElements", Napi::Function::New<gl::_drawElements>(env));

    // FBOs
    exports.Set("glGenFramebuffers", Napi::Function::New<gl::_genFramebuffers>(env));
    exports.Set("glDeleteFramebuffers", Napi::Function::New<gl::_deleteFramebuffers>(env));
    exports.Set("glBindFramebuffer", Napi::Function::New<gl::_bindFramebuffer>(env));
    exports.Set("glCheckFramebufferStatus", Napi::Function::New<gl::_checkFramebufferStatus>(env));
    exports.Set("glFramebufferTexture2D", Napi::Function::New<gl::_framebufferTexture2D>(env));
    exports.Set("glFramebufferRenderbuffer", Napi::Function::New<gl::_framebufferRenderbuffer>(env));

    // RBOs
    exports.Set("glGenRenderbuffers", Napi::Function::New<gl::_genRenderbuffers>(env));
    exports.Set("glDeleteRenderbuffers", Napi::Function::New<gl::_deleteRenderbuffers>(env));
    exports.Set("glBindRenderbuffer", Napi::Function::New<gl::_bindRenderbuffer>(env));
    exports.Set("glRenderbufferStorage", Napi::Function::New<gl::_renderbufferStorage>(env));

    // Readback
    exports.Set("glReadPixels", Napi::Function::New<gl::_readPixels>(env));

    // ES3 / VAO
    exports.Set("glGenVertexArrays", Napi::Function::New<gl::_genVertexArrays>(env));
    exports.Set("glDeleteVertexArrays", Napi::Function::New<gl::_deleteVertexArrays>(env));
    exports.Set("glBindVertexArray", Napi::Function::New<gl::_bindVertexArray>(env));
    exports.Set("glDrawArraysInstanced", Napi::Function::New<gl::_drawArraysInstanced>(env));
    exports.Set("glDrawElementsInstanced", Napi::Function::New<gl::_drawElementsInstanced>(env));
    exports.Set("glVertexAttribDivisor", Napi::Function::New<gl::_vertexAttribDivisor>(env));
    exports.Set("glDrawBuffers", Napi::Function::New<gl::_drawBuffers>(env));

    // Additional GLES 3.0 functions
    exports.Set("glGetStringi", Napi::Function::New<gl::_getStringi>(env));
    exports.Set("glGetBooleanv", Napi::Function::New<gl::_getBooleanv>(env));
    exports.Set("glGetFloatv", Napi::Function::New<gl::_getFloatv>(env));
    exports.Set("glTexParameteriv", Napi::Function::New<gl::_texParameteriv>(env));
    exports.Set("glCopyTexSubImage2D", Napi::Function::New<gl::_copyTexSubImage2D>(env));
    exports.Set("glVertexAttrib4fv", Napi::Function::New<gl::_vertexAttrib4fv>(env));
    exports.Set("glVertexAttribIPointer", Napi::Function::New<gl::_vertexAttribIPointer>(env));
    exports.Set("glGenSamplers", Napi::Function::New<gl::_genSamplers>(env));
    exports.Set("glDeleteSamplers", Napi::Function::New<gl::_deleteSamplers>(env));
    exports.Set("glBindSampler", Napi::Function::New<gl::_bindSampler>(env));
    exports.Set("glSamplerParameteri", Napi::Function::New<gl::_samplerParameteri>(env));
    exports.Set("glSamplerParameterf", Napi::Function::New<gl::_samplerParameterf>(env));
    exports.Set("glFenceSync", Napi::Function::New<gl::_fenceSync>(env));
    exports.Set("glClientWaitSync", Napi::Function::New<gl::_clientWaitSync>(env));
    exports.Set("glDeleteSync", Napi::Function::New<gl::_deleteSync>(env));
    exports.Set("glMapBufferRange", Napi::Function::New<gl::_mapBufferRange>(env));
    exports.Set("glUnmapBuffer", Napi::Function::New<gl::_unmapBuffer>(env));

    // Occlusion queries
    exports.Set("glGenQueries", Napi::Function::New<gl::_genQueries>(env));
    exports.Set("glDeleteQueries", Napi::Function::New<gl::_deleteQueries>(env));
    exports.Set("glBeginQuery", Napi::Function::New<gl::_beginQuery>(env));
    exports.Set("glEndQuery", Napi::Function::New<gl::_endQuery>(env));
    exports.Set("glGetQueryObjectiv", Napi::Function::New<gl::_getQueryObjectiv>(env));
    exports.Set("glGetQueryObjectuiv", Napi::Function::New<gl::_getQueryObjectuiv>(env));

    // Compressed textures
    exports.Set("glCompressedTexSubImage2D", Napi::Function::New<gl::_compressedTexSubImage2D>(env));

    // FBO extensions
    exports.Set("glBlitFramebuffer", Napi::Function::New<gl::_blitFramebuffer>(env));
    exports.Set("glRenderbufferStorageMultisample", Napi::Function::New<gl::_renderbufferStorageMultisample>(env));

    // Shader introspection
    exports.Set("glGetActiveUniform", Napi::Function::New<gl::_getActiveUniform>(env));
    exports.Set("glGetActiveAttrib", Napi::Function::New<gl::_getActiveAttrib>(env));
    exports.Set("glGetShaderSource", Napi::Function::New<gl::_getShaderSource>(env));

    // State queries
    exports.Set("glIsEnabled", Napi::Function::New<gl::_isEnabled>(env));
    exports.Set("glGetVertexAttribiv", Napi::Function::New<gl::_getVertexAttribiv>(env));
    exports.Set("glGetVertexAttribfv", Napi::Function::New<gl::_getVertexAttribfv>(env));
    exports.Set("glGetVertexAttribPointerv", Napi::Function::New<gl::_getVertexAttribPointerv>(env));
    exports.Set("glGetRenderbufferParameteriv", Napi::Function::New<gl::_getRenderbufferParameteriv>(env));
    exports.Set("glGetFramebufferAttachmentParameteriv", Napi::Function::New<gl::_getFramebufferAttachmentParameteriv>(env));
    exports.Set("glGetBufferParameteriv", Napi::Function::New<gl::_getBufferParameteriv>(env));
    exports.Set("glIsTexture", Napi::Function::New<gl::_isTexture>(env));
    exports.Set("glIsBuffer", Napi::Function::New<gl::_isBuffer>(env));
    exports.Set("glIsFramebuffer", Napi::Function::New<gl::_isFramebuffer>(env));
    exports.Set("glIsRenderbuffer", Napi::Function::New<gl::_isRenderbuffer>(env));
    exports.Set("glIsProgram", Napi::Function::New<gl::_isProgram>(env));
    exports.Set("glIsShader", Napi::Function::New<gl::_isShader>(env));
    exports.Set("glGetTexParameteriv", Napi::Function::New<gl::_getTexParameteriv>(env));
    exports.Set("glGetTexParameterfv", Napi::Function::New<gl::_getTexParameterfv>(env));
    exports.Set("glGetUniformiv", Napi::Function::New<gl::_getUniformiv>(env));
    exports.Set("glGetUniformfv", Napi::Function::New<gl::_getUniformfv>(env));
    exports.Set("glVertexAttrib1f", Napi::Function::New<gl::_vertexAttrib1f>(env));
    exports.Set("glVertexAttrib2f", Napi::Function::New<gl::_vertexAttrib2f>(env));
    exports.Set("glVertexAttrib3f", Napi::Function::New<gl::_vertexAttrib3f>(env));
    exports.Set("glVertexAttrib4f", Napi::Function::New<gl::_vertexAttrib4f>(env));
    exports.Set("glSampleCoverage", Napi::Function::New<gl::_sampleCoverage>(env));

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, init)
