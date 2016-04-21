#pragma once
//how to broad cast https://forum.openframeworks.cc/t/fixed-ofxosc-broadcast-to-255-255-255-255-in-of-0-84-under-windows/21974
//https://github.com/microcosm/KinectV2-OSC Address: /bodies/{bodyId}/joints/{jointId}
//Values: -float : positionX
	//- float : positionY
	//- float : positionZ
	//- string : trackingState(Tracked, NotTracked or Inferred)

#include "ofxOsc.h"

	// phase II go to something like IoTivity or AllJoyn, let the dust settle.  

// notes: remote files can use ofImage.loadImage("http://some.com/url.png")
namespace Software2552 {
	class Message {
	public:
		static shared_ptr<ofxJSON> toJson(shared_ptr<ofxOscMessage>);
		static shared_ptr<ofxOscMessage> fromJson(ofxJSON &data, const string&address);
	};

	// deque allows push front and back and enumration so we do priorities and remove old data
	typedef std::unordered_map<string, deque<shared_ptr<ofxOscMessage>>>MessageMap;

	class WriteComms : public ofThread {
	public:
		WriteComms();
		void setup(const string &hostname = "192.168.1.255", int port = 2552);

		void threadedFunction();
		
		// add a message to be sent
		void send(ofxJSON &data, const string&address);

	private:
		bool ignoreDups(shared_ptr<ofxOscMessage> p, ofxJSON &data, const string&address);
		bool checkForDups = false;
		ofxOscSender sender;
		deque<shared_ptr<ofxOscMessage>> q;
		deque<shared_ptr<ofxOscMessage>> memory; // used to avoid dups, saves last 200 messages bugbug check this number
	};

	class ReadComms : public ofThread {
	public:
		ReadComms();
		void setup(int port = 2552);

		void threadedFunction();

		shared_ptr<ofxJSON> get(const string&address);

	private:
		ofxOscReceiver receiver;
		MessageMap q;
	};
}