#pragma once
//how to broad cast https://forum.openframeworks.cc/t/fixed-ofxosc-broadcast-to-255-255-255-255-in-of-0-84-under-windows/21974
//https://github.com/microcosm/KinectV2-OSC Address: /bodies/{bodyId}/joints/{jointId}
//Values: -float : positionX
	//- float : positionY
	//- float : positionZ
	//- string : trackingState(Tracked, NotTracked or Inferred)

#include "2552software.h"
#include "ofxOsc.h"
#include <algorithm>

namespace Software2552 {
	class Message {
	public:
		static shared_ptr<ofxJSON> toJson(shared_ptr<ofxOscMessage>) {
			// bugbug data comes from one of http/s, string or local file but for now just a string
			shared_ptr<ofxJSON> p = std::make_shared<ofxJSON>();
			if (p) {
				//bugbug hard code for now
				(*p)["scenes"][0]["code"] = "44";
			}
			return p;
		}
		static shared_ptr<ofxOscMessage> fromJson(ofxJSON &data) {
			shared_ptr<ofxOscMessage> p = std::make_shared<ofxOscMessage>();
			if (p) {
				p->setAddress("/json"); // use 32 bits so we can talk to everyone easiy
										 //bugbug if these are used find a way to parameterize
				//bugbug put all these items in json? or instead use them
				// to ignore messages, delete old ones?
				p->addInt32Arg(1); // destination ID
				p->addInt32Arg(2); // source ID
				p->addInt32Arg(0); // priority
				p->addInt32Arg(0); // weight 
				unsigned int t = ofGetUnixTime();
				p->addInt32Arg(t); // send time, gmt
				p->addInt32Arg(0); // expires, 0 is never
				p->addStringArg(data.getRawString(false));
			}
		}
	};

	class WriteComms : public ofThread {
	public:
		WriteComms() { sender.setup("192.168.1.255", 2552); startThread();
		}

		void threadedFunction() {
			lock();
			shared_ptr<ofxOscMessage> m = q.front();
			q.pop();
			unlock();
			if (m) {
				sender.sendMessage(*m, false);
				unlock();
				ofSleepMillis(500);
			}
		}

		void send(ofxJSON &data) {
			shared_ptr<ofxOscMessage> p = Message::fromJson(data);
			if (p) {
				lock();
				q.push(p);
				unlock();
			}
		}

	private:
		ofxOscSender sender;
		queue<shared_ptr<ofxOscMessage>> q;
	};

	class ReadComms : public ofThread {
	public:
		ReadComms() {
			receiver.setup(2552);
		}

		void threadedFunction() {
			// check for waiting messages
			while (receiver.hasWaitingMessages()) {
				shared_ptr<ofxOscMessage> p = std::make_shared<ofxOscMessage>();
				if (p) {
					receiver.getNextMessage(*p);
					// check for json
					if (p->getAddress() == "/json") {
						lock();
						q.push(p); 
						unlock();
						// figure out priority and removing old data bugbug
								   // both the arguments are int32's
#if 0
						p->destination = m.getArgAsInt32(0);
						p->source = m.getArgAsInt32(1);
						p->priority = m.getArgAsInt32(2);
						p->weight = m.getArgAsInt32(3);
						p->expires = m.getArgAsInt32(4);

#endif // 0						
					}
				}
			}
		}
		shared_ptr<ofxJSON> get() {
			lock();
			shared_ptr<ofxOscMessage> p = q.front();
			q.pop();
			unlock();
			if (p){
				return Message::toJson(p);
			}
			return nullptr;
		}

	private:
		ofxOscReceiver receiver;
		queue<shared_ptr<ofxOscMessage>> q;
	};
}