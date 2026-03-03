#include "gl_bindings.h"
#include <GLES3/gl3.h>
#include <vector>

// Helper: read a TypedArray's data pointer
static inline uint8_t* getArrayData(const Napi::CallbackInfo& info, size_t argIndex) {
    return info[argIndex].As<Napi::TypedArray>().As<Napi::Uint8Array>().Data();
}

static inline size_t getArrayByteLength(const Napi::CallbackInfo& info, size_t argIndex) {
    return info[argIndex].As<Napi::TypedArray>().ByteLength();
}

namespace gl {

// ─── State ──────────────────────────────────────────────────────────────

Napi::Value _enable(const Napi::CallbackInfo& info) {
    glEnable(info[0].As<Napi::Number>().Uint32Value());
    return info.Env().Undefined();
}

Napi::Value _disable(const Napi::CallbackInfo& info) {
    glDisable(info[0].As<Napi::Number>().Uint32Value());
    return info.Env().Undefined();
}

Napi::Value _getError(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), glGetError());
}

Napi::Value _getIntegerv(const Napi::CallbackInfo& info) {
    GLenum pname = info[0].As<Napi::Number>().Uint32Value();
    if (info.Length() >= 2 && info[1].IsTypedArray()) {
        auto arr = info[1].As<Napi::Int32Array>();
        glGetIntegerv(pname, arr.Data());
        return info.Env().Undefined();
    }
    GLint value = 0;
    glGetIntegerv(pname, &value);
    return Napi::Number::New(info.Env(), value);
}

Napi::Value _getString(const Napi::CallbackInfo& info) {
    const GLubyte* str = glGetString(info[0].As<Napi::Number>().Uint32Value());
    if (!str) return info.Env().Null();
    return Napi::String::New(info.Env(), reinterpret_cast<const char*>(str));
}

void _finish(const Napi::CallbackInfo&) { glFinish(); }
void _flush(const Napi::CallbackInfo&) { glFlush(); }

void _hint(const Napi::CallbackInfo& info) {
    glHint(info[0].As<Napi::Number>().Uint32Value(),
           info[1].As<Napi::Number>().Uint32Value());
}

void _pixelStorei(const Napi::CallbackInfo& info) {
    glPixelStorei(info[0].As<Napi::Number>().Uint32Value(),
                  info[1].As<Napi::Number>().Int32Value());
}

// ─── Viewport / Clear ───────────────────────────────────────────────────

void _viewport(const Napi::CallbackInfo& info) {
    glViewport(info[0].As<Napi::Number>().Int32Value(),
               info[1].As<Napi::Number>().Int32Value(),
               info[2].As<Napi::Number>().Int32Value(),
               info[3].As<Napi::Number>().Int32Value());
}

void _scissor(const Napi::CallbackInfo& info) {
    glScissor(info[0].As<Napi::Number>().Int32Value(),
              info[1].As<Napi::Number>().Int32Value(),
              info[2].As<Napi::Number>().Int32Value(),
              info[3].As<Napi::Number>().Int32Value());
}

void _clear(const Napi::CallbackInfo& info) {
    glClear(info[0].As<Napi::Number>().Uint32Value());
}

void _clearColor(const Napi::CallbackInfo& info) {
    glClearColor(info[0].As<Napi::Number>().FloatValue(),
                 info[1].As<Napi::Number>().FloatValue(),
                 info[2].As<Napi::Number>().FloatValue(),
                 info[3].As<Napi::Number>().FloatValue());
}

void _clearDepthf(const Napi::CallbackInfo& info) {
    glClearDepthf(info[0].As<Napi::Number>().FloatValue());
}

void _clearStencil(const Napi::CallbackInfo& info) {
    glClearStencil(info[0].As<Napi::Number>().Int32Value());
}

// ─── Blending ───────────────────────────────────────────────────────────

void _blendFunc(const Napi::CallbackInfo& info) {
    glBlendFunc(info[0].As<Napi::Number>().Uint32Value(),
                info[1].As<Napi::Number>().Uint32Value());
}

void _blendFuncSeparate(const Napi::CallbackInfo& info) {
    glBlendFuncSeparate(info[0].As<Napi::Number>().Uint32Value(),
                        info[1].As<Napi::Number>().Uint32Value(),
                        info[2].As<Napi::Number>().Uint32Value(),
                        info[3].As<Napi::Number>().Uint32Value());
}

void _blendEquation(const Napi::CallbackInfo& info) {
    glBlendEquation(info[0].As<Napi::Number>().Uint32Value());
}

void _blendEquationSeparate(const Napi::CallbackInfo& info) {
    glBlendEquationSeparate(info[0].As<Napi::Number>().Uint32Value(),
                            info[1].As<Napi::Number>().Uint32Value());
}

void _blendColor(const Napi::CallbackInfo& info) {
    glBlendColor(info[0].As<Napi::Number>().FloatValue(),
                 info[1].As<Napi::Number>().FloatValue(),
                 info[2].As<Napi::Number>().FloatValue(),
                 info[3].As<Napi::Number>().FloatValue());
}

void _colorMask(const Napi::CallbackInfo& info) {
    glColorMask(info[0].As<Napi::Boolean>().Value(),
                info[1].As<Napi::Boolean>().Value(),
                info[2].As<Napi::Boolean>().Value(),
                info[3].As<Napi::Boolean>().Value());
}

// ─── Depth / Stencil ────────────────────────────────────────────────────

void _depthFunc(const Napi::CallbackInfo& info) {
    glDepthFunc(info[0].As<Napi::Number>().Uint32Value());
}

void _depthMask(const Napi::CallbackInfo& info) {
    glDepthMask(info[0].As<Napi::Boolean>().Value());
}

void _depthRangef(const Napi::CallbackInfo& info) {
    glDepthRangef(info[0].As<Napi::Number>().FloatValue(),
                  info[1].As<Napi::Number>().FloatValue());
}

void _stencilFunc(const Napi::CallbackInfo& info) {
    glStencilFunc(info[0].As<Napi::Number>().Uint32Value(),
                  info[1].As<Napi::Number>().Int32Value(),
                  info[2].As<Napi::Number>().Uint32Value());
}

void _stencilFuncSeparate(const Napi::CallbackInfo& info) {
    glStencilFuncSeparate(info[0].As<Napi::Number>().Uint32Value(),
                          info[1].As<Napi::Number>().Uint32Value(),
                          info[2].As<Napi::Number>().Int32Value(),
                          info[3].As<Napi::Number>().Uint32Value());
}

void _stencilOp(const Napi::CallbackInfo& info) {
    glStencilOp(info[0].As<Napi::Number>().Uint32Value(),
                info[1].As<Napi::Number>().Uint32Value(),
                info[2].As<Napi::Number>().Uint32Value());
}

void _stencilOpSeparate(const Napi::CallbackInfo& info) {
    glStencilOpSeparate(info[0].As<Napi::Number>().Uint32Value(),
                        info[1].As<Napi::Number>().Uint32Value(),
                        info[2].As<Napi::Number>().Uint32Value(),
                        info[3].As<Napi::Number>().Uint32Value());
}

void _stencilMask(const Napi::CallbackInfo& info) {
    glStencilMask(info[0].As<Napi::Number>().Uint32Value());
}

void _stencilMaskSeparate(const Napi::CallbackInfo& info) {
    glStencilMaskSeparate(info[0].As<Napi::Number>().Uint32Value(),
                          info[1].As<Napi::Number>().Uint32Value());
}

// ─── Face culling ───────────────────────────────────────────────────────

void _cullFace(const Napi::CallbackInfo& info) {
    glCullFace(info[0].As<Napi::Number>().Uint32Value());
}

void _frontFace(const Napi::CallbackInfo& info) {
    glFrontFace(info[0].As<Napi::Number>().Uint32Value());
}

void _polygonOffset(const Napi::CallbackInfo& info) {
    glPolygonOffset(info[0].As<Napi::Number>().FloatValue(),
                    info[1].As<Napi::Number>().FloatValue());
}

void _lineWidth(const Napi::CallbackInfo& info) {
    glLineWidth(info[0].As<Napi::Number>().FloatValue());
}

// ─── Buffers ────────────────────────────────────────────────────────────

void _genBuffers(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glGenBuffers(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _deleteBuffers(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glDeleteBuffers(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _bindBuffer(const Napi::CallbackInfo& info) {
    glBindBuffer(info[0].As<Napi::Number>().Uint32Value(),
                 info[1].As<Napi::Number>().Uint32Value());
}

void _bufferData(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    auto data = info[1].As<Napi::Uint8Array>();
    GLenum usage = info[2].As<Napi::Number>().Uint32Value();
    glBufferData(target, data.ByteLength(), data.Data(), usage);
}

void _bufferSubData(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLintptr offset = info[1].As<Napi::Number>().Int64Value();
    auto data = info[2].As<Napi::Uint8Array>();
    glBufferSubData(target, offset, data.ByteLength(), data.Data());
}

// ─── Textures ───────────────────────────────────────────────────────────

void _genTextures(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glGenTextures(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _deleteTextures(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glDeleteTextures(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _bindTexture(const Napi::CallbackInfo& info) {
    glBindTexture(info[0].As<Napi::Number>().Uint32Value(),
                  info[1].As<Napi::Number>().Uint32Value());
}

void _activeTexture(const Napi::CallbackInfo& info) {
    glActiveTexture(info[0].As<Napi::Number>().Uint32Value());
}

void _texImage2D(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLint level = info[1].As<Napi::Number>().Int32Value();
    GLint internalformat = info[2].As<Napi::Number>().Int32Value();
    GLsizei width = info[3].As<Napi::Number>().Int32Value();
    GLsizei height = info[4].As<Napi::Number>().Int32Value();
    GLint border = info[5].As<Napi::Number>().Int32Value();
    GLenum format = info[6].As<Napi::Number>().Uint32Value();
    GLenum type = info[7].As<Napi::Number>().Uint32Value();
    const void* data = nullptr;
    if (!info[8].IsNull() && !info[8].IsUndefined()) {
        data = info[8].As<Napi::Uint8Array>().Data();
    }
    glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
}

void _texSubImage2D(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLint level = info[1].As<Napi::Number>().Int32Value();
    GLint xoffset = info[2].As<Napi::Number>().Int32Value();
    GLint yoffset = info[3].As<Napi::Number>().Int32Value();
    GLsizei width = info[4].As<Napi::Number>().Int32Value();
    GLsizei height = info[5].As<Napi::Number>().Int32Value();
    GLenum format = info[6].As<Napi::Number>().Uint32Value();
    GLenum type = info[7].As<Napi::Number>().Uint32Value();
    auto data = info[8].As<Napi::Uint8Array>();
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, data.Data());
}

void _texParameteri(const Napi::CallbackInfo& info) {
    glTexParameteri(info[0].As<Napi::Number>().Uint32Value(),
                    info[1].As<Napi::Number>().Uint32Value(),
                    info[2].As<Napi::Number>().Int32Value());
}

void _texParameterf(const Napi::CallbackInfo& info) {
    glTexParameterf(info[0].As<Napi::Number>().Uint32Value(),
                    info[1].As<Napi::Number>().Uint32Value(),
                    info[2].As<Napi::Number>().FloatValue());
}

void _generateMipmap(const Napi::CallbackInfo& info) {
    glGenerateMipmap(info[0].As<Napi::Number>().Uint32Value());
}

void _compressedTexImage2D(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLint level = info[1].As<Napi::Number>().Int32Value();
    GLenum internalformat = info[2].As<Napi::Number>().Uint32Value();
    GLsizei width = info[3].As<Napi::Number>().Int32Value();
    GLsizei height = info[4].As<Napi::Number>().Int32Value();
    GLint border = info[5].As<Napi::Number>().Int32Value();
    auto data = info[6].As<Napi::Uint8Array>();
    glCompressedTexImage2D(target, level, internalformat, width, height, border,
                           data.ByteLength(), data.Data());
}

// ─── Shaders ────────────────────────────────────────────────────────────

Napi::Value _createShader(const Napi::CallbackInfo& info) {
    GLuint shader = glCreateShader(info[0].As<Napi::Number>().Uint32Value());
    return Napi::Number::New(info.Env(), shader);
}

void _deleteShader(const Napi::CallbackInfo& info) {
    glDeleteShader(info[0].As<Napi::Number>().Uint32Value());
}

void _shaderSource(const Napi::CallbackInfo& info) {
    GLuint shader = info[0].As<Napi::Number>().Uint32Value();
    std::string source = info[1].As<Napi::String>().Utf8Value();
    const char* src = source.c_str();
    GLint length = static_cast<GLint>(source.length());
    glShaderSource(shader, 1, &src, &length);
}

void _compileShader(const Napi::CallbackInfo& info) {
    glCompileShader(info[0].As<Napi::Number>().Uint32Value());
}

Napi::Value _getShaderiv(const Napi::CallbackInfo& info) {
    GLint result = 0;
    glGetShaderiv(info[0].As<Napi::Number>().Uint32Value(),
                  info[1].As<Napi::Number>().Uint32Value(), &result);
    return Napi::Number::New(info.Env(), result);
}

Napi::Value _getShaderInfoLog(const Napi::CallbackInfo& info) {
    GLuint shader = info[0].As<Napi::Number>().Uint32Value();
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length <= 0) return Napi::String::New(info.Env(), "");
    std::string log(length, '\0');
    glGetShaderInfoLog(shader, length, nullptr, &log[0]);
    return Napi::String::New(info.Env(), log);
}

// ─── Programs ───────────────────────────────────────────────────────────

Napi::Value _createProgram(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), glCreateProgram());
}

void _deleteProgram(const Napi::CallbackInfo& info) {
    glDeleteProgram(info[0].As<Napi::Number>().Uint32Value());
}

void _attachShader(const Napi::CallbackInfo& info) {
    glAttachShader(info[0].As<Napi::Number>().Uint32Value(),
                   info[1].As<Napi::Number>().Uint32Value());
}

void _detachShader(const Napi::CallbackInfo& info) {
    glDetachShader(info[0].As<Napi::Number>().Uint32Value(),
                   info[1].As<Napi::Number>().Uint32Value());
}

void _linkProgram(const Napi::CallbackInfo& info) {
    glLinkProgram(info[0].As<Napi::Number>().Uint32Value());
}

void _useProgram(const Napi::CallbackInfo& info) {
    glUseProgram(info[0].As<Napi::Number>().Uint32Value());
}

Napi::Value _getProgramiv(const Napi::CallbackInfo& info) {
    GLint result = 0;
    glGetProgramiv(info[0].As<Napi::Number>().Uint32Value(),
                   info[1].As<Napi::Number>().Uint32Value(), &result);
    return Napi::Number::New(info.Env(), result);
}

Napi::Value _getProgramInfoLog(const Napi::CallbackInfo& info) {
    GLuint program = info[0].As<Napi::Number>().Uint32Value();
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length <= 0) return Napi::String::New(info.Env(), "");
    std::string log(length, '\0');
    glGetProgramInfoLog(program, length, nullptr, &log[0]);
    return Napi::String::New(info.Env(), log);
}

void _validateProgram(const Napi::CallbackInfo& info) {
    glValidateProgram(info[0].As<Napi::Number>().Uint32Value());
}

void _bindAttribLocation(const Napi::CallbackInfo& info) {
    glBindAttribLocation(info[0].As<Napi::Number>().Uint32Value(),
                         info[1].As<Napi::Number>().Uint32Value(),
                         info[2].As<Napi::String>().Utf8Value().c_str());
}

Napi::Value _getAttribLocation(const Napi::CallbackInfo& info) {
    GLint loc = glGetAttribLocation(info[0].As<Napi::Number>().Uint32Value(),
                                    info[1].As<Napi::String>().Utf8Value().c_str());
    return Napi::Number::New(info.Env(), loc);
}

Napi::Value _getUniformLocation(const Napi::CallbackInfo& info) {
    GLint loc = glGetUniformLocation(info[0].As<Napi::Number>().Uint32Value(),
                                     info[1].As<Napi::String>().Utf8Value().c_str());
    return Napi::Number::New(info.Env(), loc);
}

// ─── Uniforms ───────────────────────────────────────────────────────────

void _uniform1i(const Napi::CallbackInfo& info) {
    glUniform1i(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().Int32Value());
}
void _uniform2i(const Napi::CallbackInfo& info) {
    glUniform2i(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().Int32Value(),
                info[2].As<Napi::Number>().Int32Value());
}
void _uniform3i(const Napi::CallbackInfo& info) {
    glUniform3i(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().Int32Value(),
                info[2].As<Napi::Number>().Int32Value(), info[3].As<Napi::Number>().Int32Value());
}
void _uniform4i(const Napi::CallbackInfo& info) {
    glUniform4i(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().Int32Value(),
                info[2].As<Napi::Number>().Int32Value(), info[3].As<Napi::Number>().Int32Value(),
                info[4].As<Napi::Number>().Int32Value());
}

void _uniform1f(const Napi::CallbackInfo& info) {
    glUniform1f(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().FloatValue());
}
void _uniform2f(const Napi::CallbackInfo& info) {
    glUniform2f(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().FloatValue(),
                info[2].As<Napi::Number>().FloatValue());
}
void _uniform3f(const Napi::CallbackInfo& info) {
    glUniform3f(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().FloatValue(),
                info[2].As<Napi::Number>().FloatValue(), info[3].As<Napi::Number>().FloatValue());
}
void _uniform4f(const Napi::CallbackInfo& info) {
    glUniform4f(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().FloatValue(),
                info[2].As<Napi::Number>().FloatValue(), info[3].As<Napi::Number>().FloatValue(),
                info[4].As<Napi::Number>().FloatValue());
}

void _uniform1iv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Int32Array>();
    glUniform1iv(loc, arr.ElementLength(), arr.Data());
}
void _uniform2iv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Int32Array>();
    glUniform2iv(loc, arr.ElementLength() / 2, arr.Data());
}
void _uniform3iv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Int32Array>();
    glUniform3iv(loc, arr.ElementLength() / 3, arr.Data());
}
void _uniform4iv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Int32Array>();
    glUniform4iv(loc, arr.ElementLength() / 4, arr.Data());
}

void _uniform1fv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Float32Array>();
    glUniform1fv(loc, arr.ElementLength(), arr.Data());
}
void _uniform2fv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Float32Array>();
    glUniform2fv(loc, arr.ElementLength() / 2, arr.Data());
}
void _uniform3fv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Float32Array>();
    glUniform3fv(loc, arr.ElementLength() / 3, arr.Data());
}
void _uniform4fv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Float32Array>();
    glUniform4fv(loc, arr.ElementLength() / 4, arr.Data());
}

void _uniformMatrix2fv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    GLboolean transpose = info[1].As<Napi::Boolean>().Value();
    auto arr = info[2].As<Napi::Float32Array>();
    glUniformMatrix2fv(loc, arr.ElementLength() / 4, transpose, arr.Data());
}
void _uniformMatrix3fv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    GLboolean transpose = info[1].As<Napi::Boolean>().Value();
    auto arr = info[2].As<Napi::Float32Array>();
    glUniformMatrix3fv(loc, arr.ElementLength() / 9, transpose, arr.Data());
}
void _uniformMatrix4fv(const Napi::CallbackInfo& info) {
    GLint loc = info[0].As<Napi::Number>().Int32Value();
    GLboolean transpose = info[1].As<Napi::Boolean>().Value();
    auto arr = info[2].As<Napi::Float32Array>();
    glUniformMatrix4fv(loc, arr.ElementLength() / 16, transpose, arr.Data());
}

// ─── Vertex attribs ─────────────────────────────────────────────────────

void _enableVertexAttribArray(const Napi::CallbackInfo& info) {
    glEnableVertexAttribArray(info[0].As<Napi::Number>().Uint32Value());
}

void _disableVertexAttribArray(const Napi::CallbackInfo& info) {
    glDisableVertexAttribArray(info[0].As<Napi::Number>().Uint32Value());
}

void _vertexAttribPointer(const Napi::CallbackInfo& info) {
    GLuint index = info[0].As<Napi::Number>().Uint32Value();
    GLint size = info[1].As<Napi::Number>().Int32Value();
    GLenum type = info[2].As<Napi::Number>().Uint32Value();
    GLboolean normalized = info[3].As<Napi::Boolean>().Value();
    GLsizei stride = info[4].As<Napi::Number>().Int32Value();
    // Offset passed as integer (pointer-sized), cast to void*
    intptr_t offset = info[5].As<Napi::Number>().Int64Value();
    glVertexAttribPointer(index, size, type, normalized, stride, reinterpret_cast<const void*>(offset));
}

// ─── Drawing ────────────────────────────────────────────────────────────

void _drawArrays(const Napi::CallbackInfo& info) {
    glDrawArrays(info[0].As<Napi::Number>().Uint32Value(),
                 info[1].As<Napi::Number>().Int32Value(),
                 info[2].As<Napi::Number>().Int32Value());
}

void _drawElements(const Napi::CallbackInfo& info) {
    GLenum mode = info[0].As<Napi::Number>().Uint32Value();
    GLsizei count = info[1].As<Napi::Number>().Int32Value();
    GLenum type = info[2].As<Napi::Number>().Uint32Value();
    intptr_t offset = info[3].As<Napi::Number>().Int64Value();
    glDrawElements(mode, count, type, reinterpret_cast<const void*>(offset));
}

// ─── FBOs ───────────────────────────────────────────────────────────────

void _genFramebuffers(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glGenFramebuffers(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _deleteFramebuffers(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glDeleteFramebuffers(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _bindFramebuffer(const Napi::CallbackInfo& info) {
    glBindFramebuffer(info[0].As<Napi::Number>().Uint32Value(),
                      info[1].As<Napi::Number>().Uint32Value());
}

Napi::Value _checkFramebufferStatus(const Napi::CallbackInfo& info) {
    GLenum status = glCheckFramebufferStatus(info[0].As<Napi::Number>().Uint32Value());
    return Napi::Number::New(info.Env(), status);
}

void _framebufferTexture2D(const Napi::CallbackInfo& info) {
    glFramebufferTexture2D(info[0].As<Napi::Number>().Uint32Value(),
                           info[1].As<Napi::Number>().Uint32Value(),
                           info[2].As<Napi::Number>().Uint32Value(),
                           info[3].As<Napi::Number>().Uint32Value(),
                           info[4].As<Napi::Number>().Int32Value());
}

void _framebufferRenderbuffer(const Napi::CallbackInfo& info) {
    glFramebufferRenderbuffer(info[0].As<Napi::Number>().Uint32Value(),
                              info[1].As<Napi::Number>().Uint32Value(),
                              info[2].As<Napi::Number>().Uint32Value(),
                              info[3].As<Napi::Number>().Uint32Value());
}

// ─── RBOs ───────────────────────────────────────────────────────────────

void _genRenderbuffers(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glGenRenderbuffers(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _deleteRenderbuffers(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glDeleteRenderbuffers(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _bindRenderbuffer(const Napi::CallbackInfo& info) {
    glBindRenderbuffer(info[0].As<Napi::Number>().Uint32Value(),
                       info[1].As<Napi::Number>().Uint32Value());
}

void _renderbufferStorage(const Napi::CallbackInfo& info) {
    glRenderbufferStorage(info[0].As<Napi::Number>().Uint32Value(),
                          info[1].As<Napi::Number>().Uint32Value(),
                          info[2].As<Napi::Number>().Int32Value(),
                          info[3].As<Napi::Number>().Int32Value());
}

// ─── Readback ───────────────────────────────────────────────────────────

void _readPixels(const Napi::CallbackInfo& info) {
    GLint x = info[0].As<Napi::Number>().Int32Value();
    GLint y = info[1].As<Napi::Number>().Int32Value();
    GLsizei width = info[2].As<Napi::Number>().Int32Value();
    GLsizei height = info[3].As<Napi::Number>().Int32Value();
    GLenum format = info[4].As<Napi::Number>().Uint32Value();
    GLenum type = info[5].As<Napi::Number>().Uint32Value();
    auto pixels = info[6].As<Napi::Uint8Array>();

    // GLES 3.0 only allows reading float FBOs with the implementation-defined
    // format/type (usually GL_RGBA/GL_FLOAT for RGBA16F).  Desktop GL allows
    // GL_UNSIGNED_BYTE on any FBO.  Many engines (ioq3, etc.) call glReadPixels
    // with GL_UNSIGNED_BYTE on RGBA16F FBOs expecting automatic conversion.
    // Detect when the requested type would fail and do the conversion here.
    if (type == GL_UNSIGNED_BYTE) {
        GLint implType = 0;
        glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &implType);
        if (implType == GL_FLOAT || implType == GL_HALF_FLOAT) {
            // Read as float, then convert to unsigned byte
            size_t numPixels = static_cast<size_t>(width) * height;
            // Determine channel count from format
            size_t channels = 4;
            if (format == GL_RGB) channels = 3;
            else if (format == GL_RG) channels = 2;
            else if (format == GL_RED || format == GL_ALPHA) channels = 1;

            size_t count = numPixels * channels;
            std::vector<float> tmp(count);
            glReadPixels(x, y, width, height, format, GL_FLOAT, tmp.data());
            uint8_t* dst = pixels.Data();
            for (size_t i = 0; i < count; i++) {
                float v = tmp[i];
                if (v < 0.0f) v = 0.0f;
                if (v > 1.0f) v = 1.0f;
                dst[i] = static_cast<uint8_t>(v * 255.0f + 0.5f);
            }
            return;
        }
    }

    glReadPixels(x, y, width, height, format, type, pixels.Data());
}

// ─── ES3 / VAO ──────────────────────────────────────────────────────────

void _genVertexArrays(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glGenVertexArrays(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _deleteVertexArrays(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glDeleteVertexArrays(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _bindVertexArray(const Napi::CallbackInfo& info) {
    glBindVertexArray(info[0].As<Napi::Number>().Uint32Value());
}

void _drawArraysInstanced(const Napi::CallbackInfo& info) {
    glDrawArraysInstanced(info[0].As<Napi::Number>().Uint32Value(),
                          info[1].As<Napi::Number>().Int32Value(),
                          info[2].As<Napi::Number>().Int32Value(),
                          info[3].As<Napi::Number>().Int32Value());
}

void _drawElementsInstanced(const Napi::CallbackInfo& info) {
    GLenum mode = info[0].As<Napi::Number>().Uint32Value();
    GLsizei count = info[1].As<Napi::Number>().Int32Value();
    GLenum type = info[2].As<Napi::Number>().Uint32Value();
    intptr_t offset = info[3].As<Napi::Number>().Int64Value();
    GLsizei instanceCount = info[4].As<Napi::Number>().Int32Value();
    glDrawElementsInstanced(mode, count, type, reinterpret_cast<const void*>(offset), instanceCount);
}

void _vertexAttribDivisor(const Napi::CallbackInfo& info) {
    glVertexAttribDivisor(info[0].As<Napi::Number>().Uint32Value(),
                          info[1].As<Napi::Number>().Uint32Value());
}

void _drawBuffers(const Napi::CallbackInfo& info) {
    auto arr = info[0].As<Napi::Uint32Array>();
    glDrawBuffers(arr.ElementLength(), reinterpret_cast<GLenum*>(arr.Data()));
}

// ─── Additional GLES 3.0 functions ──────────────────────────────────────

Napi::Value _getStringi(const Napi::CallbackInfo& info) {
    GLenum name = info[0].As<Napi::Number>().Uint32Value();
    GLuint index = info[1].As<Napi::Number>().Uint32Value();
    const GLubyte* str = glGetStringi(name, index);
    if (!str) return info.Env().Null();
    return Napi::String::New(info.Env(), reinterpret_cast<const char*>(str));
}

Napi::Value _getBooleanv(const Napi::CallbackInfo& info) {
    GLenum pname = info[0].As<Napi::Number>().Uint32Value();
    if (info.Length() >= 2 && info[1].IsTypedArray()) {
        auto arr = info[1].As<Napi::Uint8Array>();
        // Write directly to the typed array — GL writes GLboolean (1 byte each)
        // For multi-value queries (e.g. GL_COLOR_WRITEMASK returns 4 booleans)
        glGetBooleanv(pname, reinterpret_cast<GLboolean*>(arr.Data()));
        return info.Env().Undefined();
    }
    GLboolean value = GL_FALSE;
    glGetBooleanv(pname, &value);
    return Napi::Boolean::New(info.Env(), value == GL_TRUE);
}

Napi::Value _getFloatv(const Napi::CallbackInfo& info) {
    GLenum pname = info[0].As<Napi::Number>().Uint32Value();
    if (info.Length() >= 2 && info[1].IsTypedArray()) {
        auto arr = info[1].As<Napi::Float32Array>();
        glGetFloatv(pname, arr.Data());
        return info.Env().Undefined();
    }
    GLfloat value = 0.0f;
    glGetFloatv(pname, &value);
    return Napi::Number::New(info.Env(), value);
}

void _texParameteriv(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLenum pname = info[1].As<Napi::Number>().Uint32Value();
    auto arr = info[2].As<Napi::Int32Array>();
    glTexParameteriv(target, pname, arr.Data());
}

void _copyTexSubImage2D(const Napi::CallbackInfo& info) {
    glCopyTexSubImage2D(info[0].As<Napi::Number>().Uint32Value(),
                        info[1].As<Napi::Number>().Int32Value(),
                        info[2].As<Napi::Number>().Int32Value(),
                        info[3].As<Napi::Number>().Int32Value(),
                        info[4].As<Napi::Number>().Int32Value(),
                        info[5].As<Napi::Number>().Int32Value(),
                        info[6].As<Napi::Number>().Int32Value(),
                        info[7].As<Napi::Number>().Int32Value());
}

void _vertexAttrib4fv(const Napi::CallbackInfo& info) {
    GLuint index = info[0].As<Napi::Number>().Uint32Value();
    auto arr = info[1].As<Napi::Float32Array>();
    glVertexAttrib4fv(index, arr.Data());
}

void _vertexAttribIPointer(const Napi::CallbackInfo& info) {
    GLuint index = info[0].As<Napi::Number>().Uint32Value();
    GLint size = info[1].As<Napi::Number>().Int32Value();
    GLenum type = info[2].As<Napi::Number>().Uint32Value();
    GLsizei stride = info[3].As<Napi::Number>().Int32Value();
    intptr_t offset = info[4].As<Napi::Number>().Int64Value();
    glVertexAttribIPointer(index, size, type, stride, reinterpret_cast<const void*>(offset));
}

Napi::Value _genSamplers(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glGenSamplers(n, reinterpret_cast<GLuint*>(arr.Data()));
    return info.Env().Undefined();
}

void _deleteSamplers(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glDeleteSamplers(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _bindSampler(const Napi::CallbackInfo& info) {
    glBindSampler(info[0].As<Napi::Number>().Uint32Value(),
                  info[1].As<Napi::Number>().Uint32Value());
}

void _samplerParameteri(const Napi::CallbackInfo& info) {
    glSamplerParameteri(info[0].As<Napi::Number>().Uint32Value(),
                        info[1].As<Napi::Number>().Uint32Value(),
                        info[2].As<Napi::Number>().Int32Value());
}

void _samplerParameterf(const Napi::CallbackInfo& info) {
    glSamplerParameterf(info[0].As<Napi::Number>().Uint32Value(),
                        info[1].As<Napi::Number>().Uint32Value(),
                        info[2].As<Napi::Number>().FloatValue());
}

Napi::Value _fenceSync(const Napi::CallbackInfo& info) {
    GLenum condition = info[0].As<Napi::Number>().Uint32Value();
    GLbitfield flags = info[1].As<Napi::Number>().Uint32Value();
    GLsync sync = glFenceSync(condition, flags);
    // Return sync as a numeric handle (cast pointer to integer)
    return Napi::Number::New(info.Env(), static_cast<double>(reinterpret_cast<intptr_t>(sync)));
}

Napi::Value _clientWaitSync(const Napi::CallbackInfo& info) {
    GLsync sync = reinterpret_cast<GLsync>(static_cast<intptr_t>(info[0].As<Napi::Number>().Int64Value()));
    GLbitfield flags = info[1].As<Napi::Number>().Uint32Value();
    // timeout comes as two 32-bit halves from WASM, but from JS we get a single number
    GLuint64 timeout = static_cast<GLuint64>(info[2].As<Napi::Number>().Int64Value());
    GLenum result = glClientWaitSync(sync, flags, timeout);
    return Napi::Number::New(info.Env(), result);
}

void _deleteSync(const Napi::CallbackInfo& info) {
    GLsync sync = reinterpret_cast<GLsync>(static_cast<intptr_t>(info[0].As<Napi::Number>().Int64Value()));
    glDeleteSync(sync);
}

Napi::Value _mapBufferRange(const Napi::CallbackInfo& info) {
    // Note: mapBufferRange returns a host pointer which doesn't make sense for WASM carts.
    // Return null — carts should use bufferSubData instead.
    return info.Env().Null();
}

Napi::Value _unmapBuffer(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLboolean result = glUnmapBuffer(target);
    return Napi::Boolean::New(info.Env(), result == GL_TRUE);
}

// ─── Occlusion queries ──────────────────────────────────────────────────

void _genQueries(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glGenQueries(n, reinterpret_cast<GLuint*>(arr.Data()));
}

void _deleteQueries(const Napi::CallbackInfo& info) {
    GLsizei n = info[0].As<Napi::Number>().Int32Value();
    auto arr = info[1].As<Napi::Uint32Array>();
    glDeleteQueries(n, reinterpret_cast<const GLuint*>(arr.Data()));
}

void _beginQuery(const Napi::CallbackInfo& info) {
    glBeginQuery(info[0].As<Napi::Number>().Uint32Value(),
                 info[1].As<Napi::Number>().Uint32Value());
}

void _endQuery(const Napi::CallbackInfo& info) {
    glEndQuery(info[0].As<Napi::Number>().Uint32Value());
}

Napi::Value _getQueryObjectiv(const Napi::CallbackInfo& info) {
    GLuint id = info[0].As<Napi::Number>().Uint32Value();
    GLenum pname = info[1].As<Napi::Number>().Uint32Value();
    GLint result = 0;
    // GLES3 only has glGetQueryObjectuiv, but we can cast
    GLuint uresult = 0;
    glGetQueryObjectuiv(id, pname, &uresult);
    result = static_cast<GLint>(uresult);
    return Napi::Number::New(info.Env(), result);
}

Napi::Value _getQueryObjectuiv(const Napi::CallbackInfo& info) {
    GLuint id = info[0].As<Napi::Number>().Uint32Value();
    GLenum pname = info[1].As<Napi::Number>().Uint32Value();
    GLuint result = 0;
    glGetQueryObjectuiv(id, pname, &result);
    return Napi::Number::New(info.Env(), result);
}

// ─── Compressed textures ────────────────────────────────────────────────

void _compressedTexSubImage2D(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLint level = info[1].As<Napi::Number>().Int32Value();
    GLint xoffset = info[2].As<Napi::Number>().Int32Value();
    GLint yoffset = info[3].As<Napi::Number>().Int32Value();
    GLsizei width = info[4].As<Napi::Number>().Int32Value();
    GLsizei height = info[5].As<Napi::Number>().Int32Value();
    GLenum format = info[6].As<Napi::Number>().Uint32Value();
    auto data = info[7].As<Napi::Uint8Array>();
    glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format,
                               data.ByteLength(), data.Data());
}

// ─── FBO extensions ─────────────────────────────────────────────────────

void _blitFramebuffer(const Napi::CallbackInfo& info) {
    glBlitFramebuffer(info[0].As<Napi::Number>().Int32Value(),
                      info[1].As<Napi::Number>().Int32Value(),
                      info[2].As<Napi::Number>().Int32Value(),
                      info[3].As<Napi::Number>().Int32Value(),
                      info[4].As<Napi::Number>().Int32Value(),
                      info[5].As<Napi::Number>().Int32Value(),
                      info[6].As<Napi::Number>().Int32Value(),
                      info[7].As<Napi::Number>().Int32Value(),
                      info[8].As<Napi::Number>().Uint32Value(),
                      info[9].As<Napi::Number>().Uint32Value());
}

void _renderbufferStorageMultisample(const Napi::CallbackInfo& info) {
    glRenderbufferStorageMultisample(info[0].As<Napi::Number>().Uint32Value(),
                                     info[1].As<Napi::Number>().Int32Value(),
                                     info[2].As<Napi::Number>().Uint32Value(),
                                     info[3].As<Napi::Number>().Int32Value(),
                                     info[4].As<Napi::Number>().Int32Value());
}

// ─── Shader introspection ───────────────────────────────────────────────

Napi::Value _getActiveUniform(const Napi::CallbackInfo& info) {
    GLuint program = info[0].As<Napi::Number>().Uint32Value();
    GLuint index = info[1].As<Napi::Number>().Uint32Value();
    (void)info[2]; // bufSize — we use fixed 256
    char name[256];
    GLsizei length = 0;
    GLint size = 0;
    GLenum type = 0;
    glGetActiveUniform(program, index, sizeof(name), &length, &size, &type, name);
    auto env = info.Env();
    auto obj = Napi::Object::New(env);
    obj.Set("name", Napi::String::New(env, name, length));
    obj.Set("size", Napi::Number::New(env, size));
    obj.Set("type", Napi::Number::New(env, type));
    return obj;
}

Napi::Value _getActiveAttrib(const Napi::CallbackInfo& info) {
    GLuint program = info[0].As<Napi::Number>().Uint32Value();
    GLuint index = info[1].As<Napi::Number>().Uint32Value();
    (void)info[2]; // bufSize — we use fixed 256
    char name[256];
    GLsizei length = 0;
    GLint size = 0;
    GLenum type = 0;
    glGetActiveAttrib(program, index, sizeof(name), &length, &size, &type, name);
    auto env = info.Env();
    auto obj = Napi::Object::New(env);
    obj.Set("name", Napi::String::New(env, name, length));
    obj.Set("size", Napi::Number::New(env, size));
    obj.Set("type", Napi::Number::New(env, type));
    return obj;
}

Napi::Value _getShaderSource(const Napi::CallbackInfo& info) {
    GLuint shader = info[0].As<Napi::Number>().Uint32Value();
    GLint srcLen = 0;
    glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &srcLen);
    if (srcLen <= 0) return Napi::String::New(info.Env(), "");
    std::string source(srcLen, '\0');
    glGetShaderSource(shader, srcLen, nullptr, &source[0]);
    return Napi::String::New(info.Env(), source);
}

// ─── State queries ──────────────────────────────────────────────────────

Napi::Value _isEnabled(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), glIsEnabled(info[0].As<Napi::Number>().Uint32Value()));
}

void _getVertexAttribiv(const Napi::CallbackInfo& info) {
    GLuint index = info[0].As<Napi::Number>().Uint32Value();
    GLenum pname = info[1].As<Napi::Number>().Uint32Value();
    GLint params[4] = {0};
    glGetVertexAttribiv(index, pname, params);
    auto buf = info[2].As<Napi::Buffer<uint8_t>>();
    memcpy(buf.Data(), params, sizeof(GLint));
}

void _getVertexAttribfv(const Napi::CallbackInfo& info) {
    GLuint index = info[0].As<Napi::Number>().Uint32Value();
    GLenum pname = info[1].As<Napi::Number>().Uint32Value();
    GLfloat params[4] = {0};
    glGetVertexAttribfv(index, pname, params);
    auto buf = info[2].As<Napi::Buffer<uint8_t>>();
    memcpy(buf.Data(), params, sizeof(GLfloat));
}

void _getVertexAttribPointerv(const Napi::CallbackInfo& info) {
    GLuint index = info[0].As<Napi::Number>().Uint32Value();
    GLenum pname = info[1].As<Napi::Number>().Uint32Value();
    void *pointer = nullptr;
    glGetVertexAttribPointerv(index, pname, &pointer);
    auto buf = info[2].As<Napi::Buffer<uint8_t>>();
    uintptr_t val = (uintptr_t)pointer;
    memcpy(buf.Data(), &val, sizeof(uint32_t));
}

void _getRenderbufferParameteriv(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLenum pname = info[1].As<Napi::Number>().Uint32Value();
    GLint param = 0;
    glGetRenderbufferParameteriv(target, pname, &param);
    auto buf = info[2].As<Napi::Buffer<uint8_t>>();
    memcpy(buf.Data(), &param, sizeof(GLint));
}

void _getFramebufferAttachmentParameteriv(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLenum attachment = info[1].As<Napi::Number>().Uint32Value();
    GLenum pname = info[2].As<Napi::Number>().Uint32Value();
    GLint param = 0;
    glGetFramebufferAttachmentParameteriv(target, attachment, pname, &param);
    auto buf = info[3].As<Napi::Buffer<uint8_t>>();
    memcpy(buf.Data(), &param, sizeof(GLint));
}

void _getBufferParameteriv(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLenum pname = info[1].As<Napi::Number>().Uint32Value();
    GLint param = 0;
    glGetBufferParameteriv(target, pname, &param);
    auto buf = info[2].As<Napi::Buffer<uint8_t>>();
    memcpy(buf.Data(), &param, sizeof(GLint));
}

Napi::Value _isTexture(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), glIsTexture(info[0].As<Napi::Number>().Uint32Value()));
}

Napi::Value _isBuffer(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), glIsBuffer(info[0].As<Napi::Number>().Uint32Value()));
}

Napi::Value _isFramebuffer(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), glIsFramebuffer(info[0].As<Napi::Number>().Uint32Value()));
}

Napi::Value _isRenderbuffer(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), glIsRenderbuffer(info[0].As<Napi::Number>().Uint32Value()));
}

Napi::Value _isProgram(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), glIsProgram(info[0].As<Napi::Number>().Uint32Value()));
}

Napi::Value _isShader(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), glIsShader(info[0].As<Napi::Number>().Uint32Value()));
}

void _getTexParameteriv(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLenum pname = info[1].As<Napi::Number>().Uint32Value();
    GLint param = 0;
    glGetTexParameteriv(target, pname, &param);
    auto buf = info[2].As<Napi::Buffer<uint8_t>>();
    memcpy(buf.Data(), &param, sizeof(GLint));
}

void _getTexParameterfv(const Napi::CallbackInfo& info) {
    GLenum target = info[0].As<Napi::Number>().Uint32Value();
    GLenum pname = info[1].As<Napi::Number>().Uint32Value();
    GLfloat param = 0;
    glGetTexParameterfv(target, pname, &param);
    auto buf = info[2].As<Napi::Buffer<uint8_t>>();
    memcpy(buf.Data(), &param, sizeof(GLfloat));
}

void _getUniformiv(const Napi::CallbackInfo& info) {
    GLuint program = info[0].As<Napi::Number>().Uint32Value();
    GLint location = info[1].As<Napi::Number>().Int32Value();
    GLint params[16] = {0};
    glGetUniformiv(program, location, params);
    auto buf = info[2].As<Napi::Buffer<uint8_t>>();
    memcpy(buf.Data(), params, sizeof(GLint));
}

void _getUniformfv(const Napi::CallbackInfo& info) {
    GLuint program = info[0].As<Napi::Number>().Uint32Value();
    GLint location = info[1].As<Napi::Number>().Int32Value();
    GLfloat params[16] = {0};
    glGetUniformfv(program, location, params);
    auto buf = info[2].As<Napi::Buffer<uint8_t>>();
    memcpy(buf.Data(), params, sizeof(GLfloat));
}

void _vertexAttrib1f(const Napi::CallbackInfo& info) {
    glVertexAttrib1f(info[0].As<Napi::Number>().Uint32Value(),
                     info[1].As<Napi::Number>().FloatValue());
}

void _vertexAttrib2f(const Napi::CallbackInfo& info) {
    glVertexAttrib2f(info[0].As<Napi::Number>().Uint32Value(),
                     info[1].As<Napi::Number>().FloatValue(),
                     info[2].As<Napi::Number>().FloatValue());
}

void _vertexAttrib3f(const Napi::CallbackInfo& info) {
    glVertexAttrib3f(info[0].As<Napi::Number>().Uint32Value(),
                     info[1].As<Napi::Number>().FloatValue(),
                     info[2].As<Napi::Number>().FloatValue(),
                     info[3].As<Napi::Number>().FloatValue());
}

void _vertexAttrib4f(const Napi::CallbackInfo& info) {
    glVertexAttrib4f(info[0].As<Napi::Number>().Uint32Value(),
                     info[1].As<Napi::Number>().FloatValue(),
                     info[2].As<Napi::Number>().FloatValue(),
                     info[3].As<Napi::Number>().FloatValue(),
                     info[4].As<Napi::Number>().FloatValue());
}

void _sampleCoverage(const Napi::CallbackInfo& info) {
    glSampleCoverage(info[0].As<Napi::Number>().FloatValue(),
                     info[1].As<Napi::Boolean>().Value());
}

} // namespace gl
