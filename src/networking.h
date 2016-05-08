#pragma once
//how to broad cast https://forum.openframeworks.cc/t/fixed-ofxosc-broadcast-to-255-255-255-255-in-of-0-84-under-windows/21974
//https://github.com/microcosm/KinectV2-OSC Address: /bodies/{bodyId}/joints/{jointId}
//Values: -float : positionX
	//- float : positionY
	//- float : positionZ
	//- string : trackingState(Tracked, NotTracked or Inferred)
#include "ofApp.h"
#include "ofxOsc.h"
#include "ofxNetwork.h"
#include "snappy.h"


// phase II go to something like IoTivity or AllJoyn, let the dust settle.  

// notes: remote files can use ofImage.loadImage("http://some.com/url.png")
namespace Software2552 {
	static const string defaultServerIP = "192.168.1.25";//bugbug working to make this dynamic
	static const string SignOnKinectServerOscAddress="storyteller/kinect/server/signon";
	static const string SignOnClientOscAddress = "storyteller/client/signon";

	enum PacketType : char {
		TCPID = 't',	//116
		BodyIndexID = 'x',//120
		IrID = 'i',		//105
		BodyID = 'b',	// 98
		JsonID = 'j',	//106
		Pixels='p',		//12
		UnknownID ='k'
	};
	enum TypeOfSend : char {
		Stream = 's',
		Message = 'm',
		Unknown = '?'
	};

	enum OurPorts : int {
		OSC = 2552, 
		TCP, // generic TCP
		TCPKinectIR,
		TCPKinectBodyIndex,
		TCPKinectBody
	};

	bool compress(const char*buffer, size_t len, string&output);
	bool uncompress(const char*buffer, size_t len, string&output);
	shared_ptr<ofxJSON> toJson(shared_ptr<ofxOscMessage>);
	shared_ptr<ofxOscMessage> fromJson(ofxJSON &data, const string&address);
	void getRawString(string &buffer, shared_ptr<ofxOscMessage>);
	string getRemoteIP(shared_ptr<ofxOscMessage>m);

	const char PacketFence = 'f'; // used to validate packet were set properly

	// what gets sent over the wire
	struct TCPPacket {
		PacketType typeOfPacket; // bytes only
		char b[1]; // validates data was properly read
	};
	struct ReadTCPPacket {
		PacketType typeOfPacket; // byte only
		string data;
	};
	class TCPMessage {
	public:
		void setup(size_t bytesToSend, PacketType type, TypeOfSend typeOfSend=Message, int clientID = -1);
		
		int clientID;	// -1 for all connected
		TypeOfSend typeOfSend;// 's' stream or 'm' message
		shared_ptr<ofPixels> pixels; // optional pixels to send
		size_t numberOfBytesToSend;
		TCPPacket packet;		// data that is sent
	};

	// deque allows push front and back and enumration so we do priorities and remove old data
	typedef std::unordered_map<string, deque<shared_ptr<ofxOscMessage>>>MessageMap;

	class WriteOsc : public ofThread {
	public:
		void setup(const string &hostname = "192.168.1.255", int port = OSC);

		// add a message to be sent
		void send(ofxJSON &data, const string&address);
		void send(const string&data, const string&address);
		void send(shared_ptr<ofxOscMessage>, const string&address);
	private:
		void threadedFunction();
		bool ignoreDups(shared_ptr<ofxOscMessage> p, ofxJSON &data, const string&address);
		bool checkForDups = false;
		ofxOscSender sender;
		deque<shared_ptr<ofxOscMessage>> q;
		deque<shared_ptr<ofxOscMessage>> memory; // used to avoid dups, saves last 200 messages bugbug check this number
	};

	class ReadOsc : public ofThread {
	public:
		void setup(int port = OSC);

		// caller can use the address to determine how things are sent
		shared_ptr<ofxJSON> getJson(const string&address);
		// returns source ip
		string getString(string &buffer, const string&address);
		shared_ptr<ofxOscMessage> getMessage(const string&address);
	private:
		void threadedFunction();
		ofxOscReceiver receiver;
		MessageMap q;
	};

	class TCPServer : public ofThread {
	public:
		void setup(int port= TCP, bool blocking = false);
		int  port() { return server.getPort(); }
		int  clientCount() { return server.getNumClients(); }
		void update(const char * rawBytes, const size_t numBytes, PacketType type, TypeOfSend typeOfSend=Message, int clientID = -1);
		void update(shared_ptr<ofPixels>pixels, PacketType type, TypeOfSend typeOfSend=Stream, int clientID=-1);
		int maxItems = 10; // max size of q
	private:
		ofxTCPServer server;
		void threadedFunction();
		void sendMessage(TCPMessage *m);
		void sendStream(TCPMessage *m);
		deque<TCPMessage*> q;
	};
	template <class T>
	class TCPBase : public ofThread {
	public:
		shared_ptr<T> get() {
			shared_ptr<T>p = nullptr;
			lock(); // q can pop in an other thread then q.back will fail
			if (q.size() > 0) {
				p = q.back();// last in first out
				q.pop_back();
			}
			unlock();
			return p;
		}
		void setup(const string& ipIn, int portIn, bool blockingIn) {//192.168.1.21 or 127.0.0.1
			ip = ipIn; // save to retry setup via thread
			port = portIn;
			blocking = blockingIn;
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

	protected:
		ofxTCPClient tcpClient;
		deque<shared_ptr<T>> q;
	private:
		void setup() { setup(ip, port, blocking); }// enables setup retry in the thread loop, not for external callers
		void threadedFunction() {
			while (1) {
				if (tcpClient.isConnected()) {
					update();
				}
				else {
					setup();
				}
				yield();
			}
		}
		virtual void update()=0;
		string ip; // save to retry setup via thread
		int port=0;
		bool blocking=true;
	};

	class TCPClient : public TCPBase<ReadTCPPacket> {
	public:
	protected:
		void update();
	private:
	};
	class TCPPixels : public TCPBase<ofPixels> {
	public:
	// ex: ofTexture::readToPixels(pixels); // now all the pixels from tex are in pix
		float width = getDepthFrameWidth();
		float height= getDepthFrameHeight();
	protected:
		void update();
	private:
	};

	typedef std::unordered_map<OurPorts, shared_ptr<TCPServer>> ServerMap;

	// send in tpc or osc, server may be a bit too complicated since it trys to be one server source for all things, not sure yet
	class Server {
	public:
		void setup();
		void sendTCP(shared_ptr<ofPixels>pixels, OurPorts port, TypeOfSend typeOfSend=Stream, int clientID=-1);
		void sendTCP(const char * bytes, const size_t numBytes, OurPorts port, TypeOfSend typeOfSend = Message, int clientID = -1);
		void addTCPServer(OurPorts port = TCP, bool blocking = false);
		bool tcpEnabled();
		bool enabled(OurPorts port);
		void sendOsc(ofxJSON &data, const string&address) { comms.send(data, address); }
		void sendOsc(const string &data, const string&address) { comms.send(data, address); }
		void sendOsc(shared_ptr<ofxOscMessage>data, const string&address) { comms.send(data, address); }
	private:
		WriteOsc comms;
		ServerMap servers;
	};

}