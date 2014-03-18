#ifdef ANDROID

#include "gl3context_egl.h"

/**
 * Initialize an EGL context for the current display.
 */
bool GL3ContextEGL::Create(void* osWnd) {
	window = (ANativeWindow*)osWnd;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);  //eglGetCurrentDisplay()

	EGLint major, minor;
	eglInitialize(display, &major, &minor);
	//LOGI("EGL Initialized: %d.%d", major, minor);

	/*
	 * Here specify the attributes of the desired configuration.
	 * Below, we select an EGLConfig with at least 8 bits per color
	 * component compatible with on-screen windows
	 */
	//const EGLint attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE };
	const EGLint attribs[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_BLUE_SIZE   , 8,
			EGL_GREEN_SIZE  , 8,
			EGL_RED_SIZE    , 8,
			EGL_DEPTH_SIZE  , 16,
			EGL_STENCIL_SIZE, 0,
			EGL_NONE };

	/* Here, the application chooses the configuration it desires. In this
	 * sample, we have a very simplified selection process, where we pick
	 * the first EGLConfig that matches our criteria */
	EGLConfig config;
	EGLint numConfigs;
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	 * As soon as we picked a EGLConfig, we can safely reconfigure the
	 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	EGLint format;
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(window, 0, 0, format);

	EGLSurface surface = eglCreateWindowSurface(display, config, window, nullptr);

	EGLint contextAttrs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 3,
			EGL_NONE };
	EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttrs);  // NULL, NULL

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		//LOGW("Unable to eglMakeCurrent");
		return false; //-1
	}

	// Turn off vsync - 0
	//eglSwapInterval(display, 1);

	EGLint w, h;
	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);
	//LOGI("EGL Surface size: %dx%d", w, h);

	this->display = display;
	this->context = context;
	this->surface = surface;
	this->mWidth = w;
	this->mHeight = h;

	// Initialize GL state.
	//glDisable(GL_DEPTH_TEST);
	//if (!gl3stubInit())
	//	return false;//LOGW("GL3 stub init failed");
	//glVersion = (const char*)glGetString(GL_VERSION);
	//LOGI("OpenGL initialized: %s", glVersion);
	//LOGI("Renderer: %s %s", (const char*)glGetString(GL_VENDOR), (const char*)glGetString(GL_RENDERER));

	return true; //0
}

bool GL3ContextEGL::HasDisplay() {
	return display != nullptr;
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void GL3ContextEGL::Destroy() {
	if (display != EGL_NO_DISPLAY) {
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (context != EGL_NO_CONTEXT) {
			eglDestroyContext(display, context);
		}
		if (surface != EGL_NO_SURFACE) {
			eglDestroySurface(display, surface);
		}
		eglTerminate(display);
	}
	display = EGL_NO_DISPLAY;
	context = EGL_NO_CONTEXT;
	surface = EGL_NO_SURFACE;
}

void GL3ContextEGL::Resize(int w, int h, bool vsync) {
	mWidth = w;
	mHeight = h;

	eglSwapInterval(display, (int)vsync);
}
void GL3ContextEGL::Swap() /*override*/ {
	eglSwapBuffers(display, surface);
}

#endif

