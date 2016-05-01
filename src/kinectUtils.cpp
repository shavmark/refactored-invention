#include "ofApp.h"
//C:\Program Files\Microsoft SDKs\Kinect\v2.0_1409\Lib\x64
//bugbug fix path for release and debug, and remove any "mark" dependencies
#pragma comment(lib, "C:\\Program Files\\Microsoft SDKs\\Kinect\\v2.0_1409\\Lib\\x64\\Kinect20.lib")
#pragma comment(lib, "C:\\Program Files\\Microsoft SDKs\\Kinect\\v2.0_1409\\Lib\\x64\\Kinect20.face.lib")
#pragma comment(lib, "C:\\Program Files\\Microsoft SDKs\\Kinect\\v2.0_1409\\Lib\\x64\\Kinect20.fusion.lib")


namespace Software2552 {
	// validate com stuff
	bool checkPointer(IUnknown *p, const string&  message, char*file, int line) {
		if (p == nullptr) {
			ofLogError("pointer") << message << " " << file << ":" << ofToString(line);
			return false;
		}
		return true;
	}
	bool CheckHresult(HRESULT hResult, const string& message, char*file, int line) {
		if (FAILED(hResult)) {
			if (hResult != E_PENDING) {
				ofLogError("Kinect") << message << " " << file << ":" << ofToString(line);
			}
			return true; // error found
		}
		return false; // no error
	}
}