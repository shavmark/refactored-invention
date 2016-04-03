#include "2552software.h"
#include <istream>
#pragma comment( lib, "sapi.lib" )

// helpers, base classes etc

namespace Software2552 {

	bool Trace::checkPointer2(IUnknown *p, const string&  message, char*file, int line) {
		logVerbose2(message, file, line); // should give some good trace
		if (p == nullptr) {
			logError2("invalid pointer " + message, file, line);
			return false;
		}
		return true;
	}
	bool Trace::checkPointer2(BaseClass *p, const string&  message, char*file, int line) {
		logVerbose2(message, file, line); // should give some good trace
		if (p == nullptr) {
			logError2("invalid pointer " + message, file, line);
			return false;
		}
		return true;
	}
	string Trace::buildString(const string& text, char* file, int line) {
		return text + " " + file + ": " + ofToString(line);
	}
	void Trace::logError2(const string& errorIn, char* file, int line ) {
		ofLog(OF_LOG_FATAL_ERROR, buildString(errorIn, file,  line));
	}
	void Trace::logError2(HRESULT hResult, const string&  message, char*file, int line) {

		logError2(message + ":  " + ofToHex(hResult), file, line);

	}
	// use when source location is not needed
	void Trace::logTraceBasic(const string& message, char *name){
		string text = name;
		text += " " + message + ";"; // dirty dump bugbug maybe clean up some day
		ofLog(OF_LOG_NOTICE, text);
	}
	void Trace::logTraceBasic(const string& message) {
#if _DEBUG
		ofLog(OF_LOG_VERBOSE, message); //OF_LOG_VERBOSE could dump a lot but in debug thats what we want?
#else
		ofLog(OF_LOG_NOTICE, message);
#endif
	}
	void Trace::logTrace2(const string& message,  char*file, int line) {
		if (ofGetLogLevel() >= OF_LOG_NOTICE) {
			ofLog(OF_LOG_NOTICE, buildString(message, file, line));
		}
	}
	bool Trace::CheckHresult2(HRESULT hResult, const string& message, char*file, int line) {
		if (FAILED(hResult)) {
			if (hResult != E_PENDING) {
				logError2(hResult, message, file, line);
			}
			return true; // error found
		}
		logVerbose2(message, file, line);
		return false; // no error
	}
	// allow wide chars this way, bugbug do we need to make wstring seamless?
	std::string Trace::wstrtostr(const std::wstring &wstr)	{
		std::string strTo;
		char *szTo = new char[wstr.length() + 1];
		szTo[wstr.size()] = '\0';
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
		strTo = szTo;
		delete[] szTo;
		return strTo;
	}


}