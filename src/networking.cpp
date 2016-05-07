#include "ofApp.h"
namespace Software2552 {
#define MAXSEND (1024*1024)

	// input buffer returned as reference
	bool compress(const char*buffer, size_t len, string&output) {
		output.clear();
		output.append(buffer, len);
		return true;
		//bug remove one more data point
		size_t size = snappy::Compress((const char*)buffer, len, &output);
		if (size <= 0) {
			ofLogError("compress") << "fails " << size;
		}
		return size > 0;
	}

	// input buffer returned as reference
	bool uncompress(const char*buffer, size_t len, string&output) {
		output.clear();
		output.append(buffer, len);
		return true;
		if (!snappy::Uncompress(buffer, len, &output)) {
			ofLogError("uncompress") << "fails";
			return false;
		}
		return true;
	}
	string getRemoteIP(shared_ptr<ofxOscMessage>m) {
		if (m) {
			return m->getRemoteIp();
		}
		return "";
	}

	void getRawString(string &buffer, shared_ptr<ofxOscMessage>m) {
		buffer.clear();
		if (m) {
			string input = m->getArgAsString(0);
			uncompress(input.c_str(), input.size(), buffer);
		}
	 }

	shared_ptr<ofxJSON> toJson(shared_ptr<ofxOscMessage> m) {
		if (m) {
			// bugbug data comes from one of http/s, string or local file but for now just a string
			shared_ptr<ofxJSON> p = std::make_shared<ofxJSON>();
			if (p) {
				string output;
				string input = m->getArgAsString(0);
				uncompress(input.c_str(), input.size(), output);
				p->parse(output); // uncompress returns input upon error
			}
			return p;
		}
		return nullptr;
	}
	shared_ptr<ofxOscMessage> fromJson(ofxJSON &data, const string&address) {
		shared_ptr<ofxOscMessage> p = std::make_shared<ofxOscMessage>();
		if (p) {
			p->setAddress(address); // use 32 bits so we can talk to everyone easiy
									//bugbug if these are used find a way to parameterize
									//bugbug put all these items in json? or instead use them
									// to ignore messages, delete old ones?
			// even compress the small ones so more messages can use UDP
			string output;
			string input = data.getRawString(false);
			compress(input.c_str(), input.size(), output);
			p->addStringArg(output); // all data is in json
		}
		return p;
	}
	void WriteOsc::setup(const string &hostname, int port) {
		sender.setup(hostname, port);
		startThread();
	}
	void WriteOsc::threadedFunction() {
		while (1) {
			lock();
			if (q.size() > 0) {
				shared_ptr<ofxOscMessage> m = q.front();
				q.pop_front();
				if (m) {
					sender.sendMessage(*m, false);
				}
			}
			unlock();
			yield();
		}
	}
	// return true to ignore messages that have been added recently
	bool WriteOsc::ignoreDups(shared_ptr<ofxOscMessage> p, ofxJSON &data, const string&address) {
		// only add if its not in the list already 
		for (int i = 0; i < memory.size(); ++i) {
			if (memory[i]->getAddress() == address && memory[i]->getArgAsString(0) == data.getRawString()) {
				return true; // ignore dup
			}
		}
		memory.push_front(p);
		if (memory.size() > 1000) {
			memory.erase(memory.end() - 200, memory.end());// find to the 1000 and 200 magic numbers bugbug
		}
		return false;
	}
	void WriteOsc::send(shared_ptr<ofxOscMessage> m, const string&address) {
		if (m) {
			m->setAddress(address);
			lock();//bugbug review useage of lock in entire app
			q.push_front(m); //bugbub do we want to add a priority? front & back? not sure
			unlock();
		}
	}
	// simples string send
	void WriteOsc::send(const string&data, const string&address) {
		if (data.size() > 0) {
			shared_ptr<ofxOscMessage> p = std::make_shared<ofxOscMessage>();
			if (p) {
				p->setAddress(address); 
				string output;
				if (compress(data.c_str(), data.size(), output)) {
					p->addStringArg(output); 
					lock();
					q.push_front(p); //bugbub do we want to add a priority? front & back? not sure
					unlock();
				}
			}
		}

	}

	// add a message to be sent, json is default
	void WriteOsc::send(ofxJSON &data, const string&address) {
		if (data.size() > 0) {
			shared_ptr<ofxOscMessage> p = fromJson(data, address);
			if (p) {
				if (checkForDups && ignoreDups(p, data, address)) {
					return;
				}
				lock();
				q.push_front(p); //bugbub do we want to add a priority? front & back? not sure
				unlock();
			}
		}
	}
	void ReadOsc::setup(int port) {
		receiver.setup(port);
		startThread();
	}

	void ReadOsc::threadedFunction() {
		while (1) {
			// check for waiting messages
			while (receiver.hasWaitingMessages()) {
				shared_ptr<ofxOscMessage> p = std::make_shared<ofxOscMessage>();
				if (p) {
					lock();
					receiver.getNextMessage(*p);
					q[p->getAddress()].push_front(p); // will create a new queue if needed
					unlock();
				}
				yield();
			}
		}
	}
	shared_ptr<ofxOscMessage> ReadOsc::getMessage(const string&address) {
		shared_ptr<ofxOscMessage> m = nullptr;
		lock();
		if (q.size() > 0) {
			MessageMap::iterator found = q.find(address);
			if (found != q.end() && found->second.size() > 0) {
				m = (found->second).back();
				found->second.pop_back();// first in first out
			}
		}
		unlock();

		return m;
	}
	string ReadOsc::getString(string &buffer, const string&address) {
		string sourceIP;
		lock();
		if (q.size() > 0) {

			MessageMap::iterator found = q.find(address);
			if (found != q.end() && found->second.size() > 0) {
				shared_ptr<ofxOscMessage> p = (found->second).back();
				getRawString(buffer, p);
				found->second.pop_back();// first in first out
				sourceIP = getRemoteIP(p);
			}
		}
		unlock();
		return sourceIP;
	}
	shared_ptr<ofxJSON> ReadOsc::getJson(const string&address) {
		shared_ptr<ofxJSON> j = nullptr;
		lock();
		if (q.size() > 0) {
			MessageMap::iterator found = q.find(address);
			if (found != q.end() && found->second.size() > 0) {
				shared_ptr<ofxOscMessage> p = (found->second).back();
				if (p && !p->getArgAsBool(0)) {//bugbug this code is not tested
					j = toJson(p);
				}
				found->second.pop_back();// first in first out
			}
		}
		unlock();
		return j;
	}

	void TCPServer::setup(int port, bool blocking) {
		//11999
		server.setup(port, blocking);
		startThread();
	}
	// input data is  deleted by this object at the right time (at least that is the plan)
	void TCPServer::update(const char * bytes, const size_t numBytes, PacketType type, TypeOfSend typeOfSend, int clientID) {
		string buffer;
		if (compress(bytes, numBytes, buffer)) { // copy and compress data so caller can free passed data 
			char *bytes = new char[sizeof(TCPMessage) + buffer.size()];
			if (bytes) {
				memset(bytes, 0, sizeof(TCPMessage) + buffer.size());
				TCPMessage *message = (TCPMessage *)bytes;
				message->clientID = -1;
				message->typeOfSend = typeOfSend;
				message->packet.typeOfPacket = type; // passed as a double check
				message->packet.b[0] = PacketFence; // help check for lost or out of sync data
				message->numberOfBytesToSend = sizeof(TCPPacket) + buffer.size();
				memcpy_s(&message->packet.b[1], buffer.size(), buffer.c_str(), buffer.size());
				lock();
				if (q.size() > maxItems) {
					q.pop_back(); // remove oldest
				}
				q.push_front(message); //bugbub do we want to add a priority? 
				unlock();
			}
		}
		return;
	}
	// no compression yet bugbug
	void TCPServer::update(shared_ptr<ofPixels>pixels, PacketType type, TypeOfSend typeOfSend, int clientID) {
		unsigned char *data = pixels->getPixels();
		TCPMessage *message = new TCPMessage;
		if (message && data) {
			message->clientID = -1;
			message->typeOfSend = typeOfSend;
			message->packet.typeOfPacket = type; // passed as a double check
			message->packet.b[0] = PacketFence; // help check for lost or out of sync data
			message->pixels = pixels;//bugbug use shared pointer to avoid uneeded data copy
			lock();
			if (q.size() > maxItems) {
				q.pop_back(); // remove oldest
			}
			q.push_front(message); //bugbub do we want to add a priority? 
			unlock();
		}
		return;
	}

	// control data deletion (why we have our own thread) to avoid data copy since this code is in a Kinect crital path 
	void TCPServer::threadedFunction() {
		while (1) {
			lock();
			if (q.size() > 0) {
				TCPMessage* m = q.back();// last in first out
				q.pop_back();
				if (m) { 
					if (m->typeOfSend == Stream) {
						sendStream(m);
						delete m;
					}
					else if (m->typeOfSend == Message) {
						sendMessage(m);
						delete (char *)m; // allocated as char buffer
					}
				}
			}
			unlock();
			yield();
		}
	}
	// client needs to know if its a stream of message
	void TCPServer::sendMessage(TCPMessage *m) {
		if (m) {
			if (m->numberOfBytesToSend > MAXSEND) {
				ofLogError("TCPServer::sendbinary") << "block too large " << ofToString(m->numberOfBytesToSend) + " max " << ofToString(MAXSEND);
				return;
			}
			if (server.getNumClients() > 0) {
				if (m->clientID > 0) {
					server.sendRawMsg(m->clientID, (const char*)&m->packet, m->numberOfBytesToSend);
				}
				else {
					for (int clientID = 0; clientID < server.getLastID(); clientID++) {
						server.sendRawMsg(clientID, (const char*)&m->packet, m->numberOfBytesToSend);
						//server.sendRawMsgToAll((const char*)&m->packet, m->numberOfBytesToSend);
					}
				}
			}
		}
	}
	void TCPServer::sendStream(TCPMessage *m) {
		if (m) {
			if (server.getNumClients() > 0 && m->pixels) {
				for (int clientID = 0; clientID < server.getLastID(); clientID++) {
					const char* index = (const char*)m->pixels->getPixels(); //start at beginning of pixel array 
					int length = m->pixels->getWidth() * 3;//length of one row of pixels in the image 
					int size = m->pixels->getHeight() * m->pixels->getWidth() * 3;
					int pixelCount = 0;
					while (pixelCount < size) {
						//if (m->clientID > 0) {
						for (int clientID = 0; clientID < server.getLastID(); clientID++) {
							server.sendRawBytes(clientID, index, length); //bugbug not sure if different sizes go
						}
						//}
						//else {
						//server.sendRawBytesToAll(index, length); //send the first row of the image 
						//}
						index += length; //increase pointer so that it points to the next image row 
						pixelCount += length; //increase pixel count by one row 
					}
				}
			}
		}
	}
	void TCPPixels::threadedFunction() {
		while (1) {
			//bugbug need to go with a list else the most recent one is all we get
			// bugbug sizes need to be set also
			readPixelStream(pixels, getDepthFrameWidth(), getDepthFrameHeight());
			yield();
		}
	}
	void TCPClient::threadedFunction() {
		while (1) {
			update();
			yield();
		}
	}
	shared_ptr<ReadTCPPacket> TCPClient::get() {
		shared_ptr<ReadTCPPacket>p = nullptr;
		lock(); // q can pop in an other thread then q.back will fail
		if (q.size() > 0) {
			p = q.back();// last in first out
			q.pop_back();
		}
		unlock();
		return p;
	}
	//Receiving loop that must ensure a frame is received as a whole
	// must know the size of item being sent
	void TCPPixels::readPixelStream(ofPixels &pixels, float width, float height) {
		unsigned char* receivePos = pixels.getPixels();
		int length = width * 3;
		int totalReceivedBytes = 0;
		int size = size = width * height * 3;
		//bugbug maybe this needs to be non blocking?
		while (totalReceivedBytes < size) {
			int receivedBytes = tcpClient.receiveRawBytes((char*)receivePos, length); //returns received bytes 
			if (receivedBytes < 0) {
				break; // try again later
			}
			totalReceivedBytes += receivedBytes;
			receivePos += receivedBytes;
		}
	}

	char TCPClient::update() {
		char type = 0;
		if (tcpClient.isConnected()) {
			char* b = (char*)std::malloc(MAXSEND);
			if (b) {
				int messageSize = 0;

				do {
					// this api will write the size of the data not the size of the buffer we pass in (ouch)
					// it buffers data beteen its markets and returns 0 until all data between 
					// markers is in its buffer which it then returns.
					messageSize = tcpClient.receiveRawMsg(b, MAXSEND);
					// only occurs due to bug or hack as we never send more than MAXSEND ourself, at this point we need to crash
					if (messageSize > MAXSEND) {
						ofExit(-2); // enables DOS, the real fix is for to not over flow in receiveRawMsg bugbug
					}
					break;
				} while (1);

				if (messageSize > 0) {
					TCPPacket*p = (TCPPacket*)b;
					if (p->b[0] == PacketFence) { // basic validation
						shared_ptr<ReadTCPPacket> returnedData = std::make_shared<ReadTCPPacket>();
						if (returnedData) {
							if (uncompress(&p->b[1], messageSize - sizeof(TCPPacket), returnedData->data)) {
								type = p->typeOfPacket; // data should change a litte
								returnedData->type = type;
								ofLogNotice("TCPClient::update") << "receiveRawMsg packet of size " << ofToString(messageSize) << " type " << type;
								lock();
								q.push_back(returnedData);
								unlock();
							}
							else {
								ofLogError("TCPClient::update") << "data ignored";
							}
						}
					}
				}

				free(b); // only delete if data not returned
			}
			return 0;

		}
		else {
			setup();
		}
		return type;
	}
	void TCPClient::setup(const string& ip, int port, bool blocking) {//192.168.1.21 or 127.0.0.1
		if (!tcpClient.isConnected()) {
			if (!tcpClient.setup(ip, port, blocking)) {
				ofSleepMillis(500); // wait before a try again if failed
				tcpClient.setup(ip, port, blocking); // caller must try again beyond this
			}
			if (tcpClient.isConnected()) {
				ofLogNotice("TCPClient::setup") << "connected " << ip << ":" << port << " blocking" << blocking;
				if (!isThreadRunning()) {
					startThread(); // start thread once connection is made
				}
			}
		}
	}
	void Server::setup() {
		comms.setup();
	}
	void Server::addTCPServer(OurPorts port, bool blocking) {
		shared_ptr<TCPServer> s = std::make_shared<TCPServer>();
		if (s) {
			s->setup(port, blocking);
			servers[port] = s; // will create a new queue if needed
		}
	}

	void Server::sendTCP(const char * bytes, const size_t numBytes, OurPorts port, TypeOfSend typeOfSend, int clientID) {
		if (numBytes > 0) {
			ServerMap::const_iterator s = servers.find(port);
			if (s != servers.end()) {
				s->second->update(bytes, numBytes, mapPortToType(port),  typeOfSend, clientID);
			}
		}
	}
	void Server::sendTCP(shared_ptr<ofPixels>pixels, OurPorts port, TypeOfSend typeOfSend, int clientID) {
		if (pixels->size() > 0) {
			ServerMap::const_iterator s = servers.find(port);
			if (s != servers.end()) {
				//bugbug go with shared pointered to avoid data copy
				s->second->update(pixels, mapPortToType(port), typeOfSend, clientID);
			}
		}
	}

	bool Server::tcpEnabled() {
		ServerMap::const_iterator s = servers.find(TCP);
		return s != servers.end();
	}
	bool Server::enabled(OurPorts port) {
		ServerMap::const_iterator s = servers.find(port);
		if (s != servers.end() && s->second.get()) {
			return s->second.get()->clientCount()  > 0;
		}
		return false;
	}

}