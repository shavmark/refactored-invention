#pragma once
//how to broad cast https://forum.openframeworks.cc/t/fixed-ofxosc-broadcast-to-255-255-255-255-in-of-0-84-under-windows/21974
//https://github.com/microcosm/KinectV2-OSC Address: /bodies/{bodyId}/joints/{jointId}
//Values: -float : positionX
	//- float : positionY
	//- float : positionZ
	//- string : trackingState(Tracked, NotTracked or Inferred)

#include "ofxOsc.h"
#include "ofxJSON.h"

	// phase II go to something like IoTivity or AllJoyn, let the dust settle.  

// notes: remote files can use ofImage.loadImage("http://some.com/url.png")
namespace Software2552 {
	class Message {
	public:
		static shared_ptr<ofxJSON> toJson(shared_ptr<ofxOscMessage>);
		static shared_ptr<ofxOscMessage> fromJson(ofxJSON &data, const string&name);
	};

	class WriteComms : public ofThread {
	public:
		WriteComms();
		void setup(const string &hostname = "192.168.1.255", int port = 2552);

		void threadedFunction();
		
		// add a message to be sent
		void send(ofxJSON &data, const string&name);

	private:
		ofxOscSender sender;
		queue<shared_ptr<ofxOscMessage>> q;
	};

	class ReadComms : public ofThread {
	public:
		ReadComms();
		void setup(int port = 2552);

		void threadedFunction();

		shared_ptr<ofxJSON> get(const string&name);

	private:
		ofxOscReceiver receiver;
		queue<shared_ptr<ofxOscMessage>> q;
	};
}