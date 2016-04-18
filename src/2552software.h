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

#include "ofxJSON.h"
#include "Poco/Foundation.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"
#include "Poco/DateTimeParser.h"
#include "Poco/DateTimeFormatter.h"


#define STRINGIFY(p) #p

namespace Software2552 {
	void init(); 
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

	// drawing related items start here
	class BaseClass2552WithDrawing : public BaseClass {
	public:
		BaseClass2552WithDrawing() { valid = false; }

		bool objectValid() { return valid; } // data is in a good state
		void setValid(bool b = true) { valid = b; };

	private:
		bool valid; // true when data is valid and ready to draw
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
#define logError(p1, p2) Trace::logError2(p1, p2, __FILE__, __LINE__)
#define logErrorString(p1) Trace::logError2(p1, __FILE__, __LINE__)
#if _DEBUG
#define logVerbose(p1) Trace::logVerbose2(p1, __FILE__, __LINE__)
#else
#define logVerbose(p1) 
#endif
#define logTrace(p1) Trace::logTrace2(p1, __FILE__, __LINE__)
#define checkPointer(p1, p2) Trace::checkPointer2(p1, p2, __FILE__, __LINE__)
#define checkPointer(p1, p2) Trace::checkPointer2(p1, p2, __FILE__, __LINE__)
#define hresultFails(p1, p2) Trace::CheckHresult2(p1, p2, __FILE__, __LINE__)
#define basicTrace(p) Trace::logTraceBasic(p, #p)
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

}
