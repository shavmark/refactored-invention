#pragma once
//how to broad cast https://forum.openframeworks.cc/t/fixed-ofxosc-broadcast-to-255-255-255-255-in-of-0-84-under-windows/21974
//https://github.com/microcosm/KinectV2-OSC Address: /bodies/{bodyId}/joints/{jointId}
//Values: -float : positionX
	//- float : positionY
	//- float : positionZ
	//- string : trackingState(Tracked, NotTracked or Inferred)
#include "2552software.h"
#include "ofxOsc.h"

namespace Software2552 {
	class Comms {
	public:
		Comms() { sender.setup("192.168.1.255", 2552); receiver.setup(2552);
		}
		class Message {
		public:
			int32_t destination = 0;
			int32_t source = 0;
			int32_t priority = 0;
			int32_t weight = 0;
			int32_t expires = 0;
			string message;
		};

		void update() {
			// check for waiting messages
			while (receiver.hasWaitingMessages()) {
				shared_ptr<Message> p = std::make_shared<Message>();
				if (p) {
					ofxOscMessage m;
					receiver.getNextMessage(m);
					// check for json
					if (m.getAddress() == "/json") {
						// both the arguments are int32's
						p->destination = m.getArgAsInt32(0);
						p->source = m.getArgAsInt32(1);
						p->priority = m.getArgAsInt32(2);
						p->weight = m.getArgAsInt32(3);
						p->expires = m.getArgAsInt32(4);
						p->message = m.getArgAsString(5);
						q.push(p);
					}
				}
			}
		}
		shared_ptr<Message> get() { 
			shared_ptr<Message> p = q.front();
			q.pop();
			return p;
		}
		void send(const Json::Value &data) {
			ofxOscMessage m;
			m.setAddress("/json"); // use 32 bits so we can talk to everyone easiy
			//bugbug if these are used find a way to parameterize
			m.addInt32Arg(1); // destination ID
			m.addInt32Arg(2); // source ID
			m.addInt32Arg(0); // priority
			m.addInt32Arg(0); // weight 
			unsigned int t = ofGetUnixTime();
			m.addInt32Arg(t); // send time, gmt
			m.addInt32Arg(0); // expires, 0 is never
			m.addStringArg(data.asString());
			sender.sendMessage(m, false);
		}

	private:
		ofxOscSender sender;
		ofxOscReceiver receiver;
		queue<shared_ptr<Message>> q;
	};
}