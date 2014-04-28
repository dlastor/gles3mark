
#include "framebuffer.h"

bool Framebuffer::CheckCompleteness() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Log::E() << "Framebuffer incomplete: " << GetFramebufferStatusString(status);
        return false; // throw        
    }

    return true;
}

std::string Framebuffer::GetFramebufferStatusString(GLenum status) {
#define FBSTATUS(e) case e : return std::string(#e)
    switch (status) {
        FBSTATUS(GL_FRAMEBUFFER_COMPLETE);
        FBSTATUS(GL_FRAMEBUFFER_UNDEFINED);
        FBSTATUS(GL_FRAMEBUFFER_UNSUPPORTED);
        FBSTATUS(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
        FBSTATUS(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
        FBSTATUS(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
        //FBSTATUS(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
        //FBSTATUS(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
        //FBSTATUS(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
        default: return std::string("GL_FRAMEBUFFER_STATUS_UNKNOWN");
    }
#undef FBSTATUS    
}