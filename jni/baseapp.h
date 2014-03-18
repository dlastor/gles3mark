/*
 * baseapp.h
 */

#ifdef ANDROID

#pragma once

#include <jni.h>
#include <android_native_app_glue.h>
#include <memory>
#include "../gles3mark_src/log.h"
#include "../gles3mark_src/gles3mark.h"

/**
 * Shared state for our app.
 */
class BaseApp {
	android_app*	state;
	std::unique_ptr<GLES3Mark> gles3mark;

	JNIEnv 			*env; // game thread env (vs main thread env: state->activity->env)
	jobject			thiz;
	jclass			clazz;

	bool			animating;
	bool			quit;

	/**
	 * Our saved state data.
	 */
	struct SavedState {
		int x, y;
		SavedState():x(0),y(0){}
	} savedState;


	/**
	 * Process the next main command.
	 */
	static void handle_cmd(android_app* app, int32_t cmd) {
		BaseApp* me = (BaseApp*)app->userData;
		me->HandleCommand(cmd);
	}

	/**
	 * Process the next main command.
	 */
	void HandleCommand(int32_t cmd) {
		switch (cmd) {

		// The system has asked us to save our current state.  Do so.
		case APP_CMD_SAVE_STATE:
			state->savedState = malloc(sizeof(SavedState));
			*((SavedState*)state->savedState) = savedState;
			state->savedStateSize = sizeof(SavedState);
			Log::Stream() << "State saved: x: " << savedState.x << ", y: " << savedState.y;
			break;

		// The window is being shown, get it ready.
		case APP_CMD_INIT_WINDOW:
			if (state->window) {
				Log::Msg("<<- CMD INIT WINDOW ->>");
				//if (!gles3mark)
				//	gles3mark = new GLES3Mark();
				gles3mark = std::unique_ptr<GLES3Mark>(new GLES3Mark()); // gles3mark = std::make_unique<GLES3Mark>();
				gles3mark->OnInit(state->window, state->activity->assetManager);
				gles3mark->OnResize(gles3mark->GetGLContext()->GetWidth(), gles3mark->GetGLContext()->GetHeight());
				//glContext->Create(state->window);
				animating = true;

				//OnIdle();
			}
			break;

		// The window is being hidden or closed, clean it up.
		case APP_CMD_TERM_WINDOW:
			Log::Msg("<<- CMD TERM WINDOW ->>");
			//glContext->Destroy();
			gles3mark.reset();
			animating = false;
			break;

		// When our app gains focus, we start monitoring the accelerometer.
		case APP_CMD_GAINED_FOCUS:
			Log::Msg("<<- CMD GAINED FOCUS  ->>");
			animating = true;
			break;

		// When our app loses focus, we stop monitoring the accelerometer. This is to avoid consuming battery while not being used.
		case APP_CMD_LOST_FOCUS:
			Log::Msg("<<- CMD LOST FOCUS ->>");
			animating = false; // Also stop animating.
			//OnIdle();
			break;
		}
	}

	/**
	 * Process the next input event.
	 */
	static int32_t handle_input(android_app* app, AInputEvent* event) {
		BaseApp* me = (BaseApp*)app->userData;
		return me->HandleInput(event);
	}

	/**
	 * Process the next input event.
	 */
	int32_t HandleInput(AInputEvent* event) {
		if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
			animating = !animating;
			savedState.x = AMotionEvent_getX(event, 0);
			savedState.y = AMotionEvent_getY(event, 0);

			Exit(934);

			return 1;
		}
		return 0;
	}

protected:

	virtual void OnStartup() {}
	virtual void OnQuit()   {}
	virtual void OnResize(int w, int h) {}
	virtual void OnIdle() {
		//if (!glContext->HasDisplay()) return;
		gles3mark->OnStep();
	}

	void Exit(int score) {
		jmethodID FinishMe = env->GetMethodID(clazz, "FinishMe", "(I)V");
		env->CallVoidMethod(thiz, FinishMe, score);
		quit = true;
	}


	// JNI is running the equivalent of the following Java code: activity.showToastAlert(text);
	bool showToast(const char* text) {
		jmethodID showToastAlert = env->GetMethodID(clazz, "showToastAlert", "(Ljava/lang/String;)V");
		jstring jniText = env->NewStringUTF(text);
		env->CallVoidMethod(thiz, showToastAlert, jniText);
		env->DeleteLocalRef(jniText);

		// Check nvidia's own native_app_glue implementation for encapsulation
/*		jstring jniText = mApp->appThreadEnv->NewStringUTF(text);   				        EXCEPTION_RETURN(mApp->appThreadEnv);
	    jclass thisClass = mApp->appThreadEnv->GetObjectClass(mApp->appThreadThis);         EXCEPTION_RETURN(mApp->appThreadEnv);
		jmethodID showToastAlert = mApp->appThreadEnv->GetMethodID(thisClass, "showToastAlert", "(Ljava/lang/String;)V"); EXCEPTION_RETURN(mApp->appThreadEnv);
		mApp->appThreadEnv->CallVoidMethod(mApp->appThreadThis, showToastAlert, jniText); 	EXCEPTION_RETURN(mApp->appThreadEnv);
*/
		return true;
	}



public:
	BaseApp(android_app* _state)
		: state(_state), animating(false), quit(false) {

		state->userData = this;
		state->onAppCmd = handle_cmd;
		state->onInputEvent = handle_input;

		// We are starting with a previous saved state; restore from it.
		if (state->savedState != nullptr) {
			savedState = *(SavedState*)state->savedState;
			Log::Stream() << "State loaded: x: " << savedState.x << ", y: " << savedState.y;
		}

		//gles3mark = std::unique_ptr<GLES3Mark>(new GLES3Mark());  // gles3mark = std::make_unique<GLES3Mark>();

		//state->activity->internalDataPath;

		if (state->activity->vm->AttachCurrentThread(&env, nullptr) == 0) {
			thiz = state->activity->clazz;
			clazz = env->GetObjectClass(thiz);
		} else {
			Log::Msg("AttachCurrentThread Failed");
		}
	}

	virtual ~BaseApp() {}

	void Run() {
		// our main loop for the app. Will only return once the game is really finished.
		while (true) {
	        // Read all pending events.
	        int ident, events;
	        android_poll_source* source;

	        // If not animating, we will block forever waiting for events.
	        // If animating, we loop until all events are read, then continue to draw the next frame of animation.
	        while ((ident = ALooper_pollAll(animating ? 0 : -1, nullptr, &events, (void**)&source)) >= 0) {

	            // Process this event.
	            if (source != nullptr) {
	                source->process(state, source);
	            }

	            // If a sensor has data, process it now.
	            if (ident == LOOPER_ID_USER) {
	            }

	            // Check if we are exiting.  Which is the case once we called ANativeActivity_finish
	            if (state->destroyRequested != 0) {
	            	Log::Msg("<<- DESTROY REQUESTED ->>"); // Quit our app stuff here

	            	//animating = false;
	    			//showToast("Exitting");

	    			// detach from current thread (when thread exists) - else error: "native thread exited without detaching"
	    			state->activity->vm->DetachCurrentThread();

	                return; // return the main, so we get back to our java activity which called the nativeactivity
	            }
	        }

	        // Done with events; draw next animation frame.
	        if (animating) {
	            // Update game state

	            // Drawing is throttled to the screen update rate, so there is no need to do timing here.
	        	OnIdle();
	        }

	        // if our app told us to finish
	        if (quit) {
	        	ANativeActivity_finish(state->activity);
	        }
	    }
	}
};

#endif

