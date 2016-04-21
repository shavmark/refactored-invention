#include "ofxJSON.h"
#include "networking.h"

namespace Software2552 {

	shared_ptr<ofxJSON> Message::toJson(shared_ptr<ofxOscMessage> m) {
		if (m) {
			// bugbug data comes from one of http/s, string or local file but for now just a string
			shared_ptr<ofxJSON> p = std::make_shared<ofxJSON>();
			if (p) {
				p->parse(m->getArgAsString(0));
			}
			return p;
		}
		return nullptr;
	}
	shared_ptr<ofxOscMessage> Message::fromJson(ofxJSON &data, const string&name) {
		shared_ptr<ofxOscMessage> p = std::make_shared<ofxOscMessage>();
		if (p) {
			p->setAddress(name); // use 32 bits so we can talk to everyone easiy
									//bugbug if these are used find a way to parameterize
									//bugbug put all these items in json? or instead use them
									// to ignore messages, delete old ones?
			p->addStringArg(data.getRawString(false)); // all data is in json
		}
		return p;
	}
	WriteComms::WriteComms() {
	}
	void WriteComms::setup(const string &hostname, int port) {
		sender.setup(hostname, port);
		startThread();
	}
	void WriteComms::threadedFunction() {
		while (1) {
			if (q.size() > 0) {
				lock();
				shared_ptr<ofxOscMessage> m = q.front();
				q.pop();
				unlock();
				if (m) {
					sender.sendMessage(*m, false);
				}
			}
			ofSleepMillis(10);
		}
	}
	// add a message to be sent
	void WriteComms::send(ofxJSON &data, const string&name) {
		if (data.size() > 0) {
			shared_ptr<ofxOscMessage> p = Message::fromJson(data, name);
			if (p) {
				lock();
				q.push(p);
				unlock();
			}
		}
	}
	ReadComms::ReadComms() {
	}
	void ReadComms::setup(int port) {
		receiver.setup(port);
		startThread();
	}

	void ReadComms::threadedFunction() {
		while (1) {
			// check for waiting messages
			while (receiver.hasWaitingMessages()) {
				shared_ptr<ofxOscMessage> p = std::make_shared<ofxOscMessage>();
				if (p) {
					receiver.getNextMessage(*p);
					lock();
					// add new one if not there
					q[p->getAddress()].push(p);
					unlock();
				
#if 0
						p->destination = m.getArgAsInt32(0);//bugbug code in time out, priority etc
						p->source = m.getArgAsInt32(1);
						p->priority = m.getArgAsInt32(2);
						p->weight = m.getArgAsInt32(3);
						p->expires = m.getArgAsInt32(4);

#endif // 0						
				}
			}
			ofSleepMillis(10);
		}
	}
	shared_ptr<ofxJSON> ReadComms::get(const string&name) {
		shared_ptr<ofxJSON> j = nullptr;
		if (q.size() > 0) {
			lock();
			MessageMap::iterator m = q.find(name);
			if (m != q.end() && m->second.size() > 0) {
				j = Message::toJson((m->second).front());
				m->second.pop();
			}
			unlock();
		}
		return j;
	}


}