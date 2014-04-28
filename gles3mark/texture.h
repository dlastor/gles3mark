
#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#ifdef _WIN32
#define KTX_OPENGL 1
#else
#define KTX_OPENGL_ES2 1 //ES3 ?
#endif
#include <ktx.h>

#include "glquery.h"
#include "rendertarget.h"
#include "log.h"

// http://docs.unity3d.com/Documentation/ScriptReference/Texture.html
// RenderTexture (target) - http://docs.unity3d.com/Documentation/ScriptReference/RenderTexture.html
// Reflective Shader - cubemap
/*
class Texture { // : public RenderTarget?
protected:    
    enum class FilterMode {
        Point,
        Bilinear,
        Trilinear    
    };

    enum class WrapMode {
        Repeat,
        Clamp,    
    };
    //enum class Dimension {}; // 1,2,3d, array
};*/


class Texture : public RenderTarget { 
    friend class Framebuffer;
    
    GLuint textureObject;
    GLenum target;

public:
    std::string path;

public:
    Texture(const std::string& path = std::string()) : path(path) {
        glGenTextures(1, &textureObject);
        maxSize = GLQuery::Get<GLint>(GL_MAX_TEXTURE_SIZE);
    }
    ~Texture() {
        glDeleteTextures(1, &textureObject);    
    }

    void InitStorage(GLenum internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height, GLint level = 0);

    void FromKTXdata(const std::vector<char>& ktxData);
    void FromBitmapData(const std::vector<char>& rawData, int width, int height);

    void Bind(GLenum textureUnit = GL_TEXTURE0);

private:
    static bool isPowerOfTwo(unsigned x) {
        return (x != 0) && ((x & (x - 1)) == 0);
    }
};
