#pragma once
// For M_PI and log definitions
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <memory>
// Microsoft Speech Platform SDK 11
#include <sapi.h>
#include <sphelper.h> // SpFindBestToken()
#include <strsafe.h>
#include <intsafe.h>
#include <Synchapi.h>
// keep all MS files above ofmain.h
#include "ofMain.h"
#include "ofFileUtils.h"
#include "ole2.h"

// !!keep all MS files above ofmain.h https://forum.openframeworks.cc/t/how-to-include-atl-in-vs2012-project/14672/2
#include "ofMain.h"

#include "ofxAssimpModelLoader.h"
#include "ofxSmartFont.h"
#include "ofxJSON.h"
#include "ofxParagraph.h"
#include "Poco/Foundation.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"
#include "Poco/DateTimeParser.h"
#include "Poco/DateTimeFormatter.h"


#define STRINGIFY(p) #p

namespace Software2552 {
	typedef uint64_t GraphicID;
	template<typename T>void setIfGreater(T& f1, T f2) {
		if (f2 > f1) {
			f1 = f2;
		}
	}

	// root class, basic and small but items every object needs.  Try to avoid adding data to keep it small
	class BaseClass {
	public:
	private:
	};


	// can be, but does not need to be, a base class as its all static and can just be called, could not even be a class I suppose
	class Trace : public BaseClass {
	public:
		static bool checkPointer2(IUnknown *p, const string&  message, char*file = __FILE__, int line = __LINE__);
		static bool checkPointer2(BaseClass *p, const string&  message, char*file = __FILE__, int line = __LINE__);

		static void logError2(const string& error, char*file, int line);
		static void logVerbose2(const string& message, char*file, int line) {
			if (ofGetLogLevel() >= OF_LOG_VERBOSE) {
				ofLog(OF_LOG_VERBOSE, buildString(message, file, line));
			}
		}
		static void logTraceBasic(const string& message, char *name);
		static void logTraceBasic(const string& message);
		static void logTrace2(const string& message, char*file, int line);
		static void logError2(HRESULT hResult, const string&  message, char*file, int line);
		static bool CheckHresult2(HRESULT hResult, const string& message, char*file, int line);
		static string buildString(const string& errorIn, char* file, int line);
		static std::string wstrtostr(const std::wstring &wstr);

		// get the right line number bugbug add DEBUG ONLY
#if _DEBUG
#define logError(p1, p2) Trace::logError2(p1, p2, __FILE__, __LINE__)
#define logErrorString(p1) Trace::logError2(p1, __FILE__, __LINE__)
#define logVerbose(p1) Trace::logVerbose2(p1, __FILE__, __LINE__)
#define logTrace(p1) Trace::logTrace2(p1, __FILE__, __LINE__)
#define checkPointer(p1, p2) Trace::checkPointer2(p1, p2, __FILE__, __LINE__)
#define checkPointer(p1, p2) Trace::checkPointer2(p1, p2, __FILE__, __LINE__)
#define hresultFails(p1, p2) Trace::CheckHresult2(p1, p2, __FILE__, __LINE__)
#define basicTrace(p) Trace::logTraceBasic(p, #p)
#endif
		// simple text log, even w/o debug
#define echo(p) Trace::logTraceBasic(p)
#define echoError(p) Trace::logErrorString(p)
#define tracer(p, isError) (isError ? echoError(p) : echo(p))
	};

	template<class Interface> void SafeRelease(Interface *& pInterfaceToRelease)
	{
		if (pInterfaceToRelease != NULL) {
			pInterfaceToRelease->Release();
			pInterfaceToRelease = NULL;
		}
	}

	
	// 3d model
	class Model3D : public ofxAssimpModelLoader, public Trace {
	public: 
		void setup(const string& name) {
			loadModel(name, false);
			setPosition(0, 0, 0); // assumes camera and this is the middle
			createLightsFromAiModel();
			setLoopStateForAllAnimations(OF_LOOP_NORMAL);
		}
		
		void setup(const string& name, ofPoint point) {
			loadModel(name, true);
			setPosition(point.x, point.y, point.z);
			setLoopStateForAllAnimations(OF_LOOP_NORMAL);
		}
		
		void update() {
			if (modelMeshes.size() > 0) {
				mesh = getCurrentAnimatedMesh(0);
			}
			ofxAssimpModelLoader::update();
		}
		// low level readJsonValue of annimation from another model, called before Load time
		void setAnimations(const Model3D& model)	{
			//bugbug clear existing animations
			for (int i = 0; i<model.scene->mNumAnimations; i++) {
				aiAnimation * animation = model.scene->mAnimations[i];
				animations.push_back(ofxAssimpAnimation(scene, animation));
			}
		}
		void moveRight() {
			setPosition(ofGetWidth(), (float)ofGetHeight() * 0.5, 0);
		}
		void setPlay(const aiString& myname) {
			for (unsigned int i = 0; i< getAnimationCount(); i++) {
				ofxAssimpAnimation &a = getAnimation(i);
				aiString s = a.getAnimation()->mName;
				if (a.getAnimation()->mName == myname) {
					a.play();
				}
			}
		}
		void draw() {
			//ofPushMatrix();
			//ofTranslate(getPosition().x + 100, getPosition().y, 0);
			//ofRotate(180, 1, 0, 0);
			//ofTranslate(-getPosition().x, -getPosition().y, 0);
			//ofRotate(90, 1, 0, 0);
			drawFaces();
			//ofPopMatrix();
		}
		void drawMesh() {
			
			//ofPushMatrix();
			//ofTranslate(getPosition().x + 100, getPosition().y, 0);
			//ofRotate(180, 1, 0, 0);
			//ofTranslate(-getPosition().x, -getPosition().y, 0);

			ofxAssimpMeshHelper & meshHelper = getMeshHelper(0);

			ofMultMatrix(getModelMatrix());
			ofMultMatrix(meshHelper.matrix);

			ofMaterial & material = meshHelper.material;
			if (meshHelper.hasTexture()) {
				meshHelper.getTextureRef().bind();
			}
			material.begin();
			//ofRotate(90, 1, 0, 0);
			mesh.drawFaces();
			material.end();
			if (meshHelper.hasTexture()) {
				meshHelper.getTextureRef().unbind();
			}
			//ofPopMatrix();
		}
	private:
		ofMesh mesh;
	};
}

