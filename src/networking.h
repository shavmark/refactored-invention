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
	static const string defaultServerIP = "192.168.1.25";

	enum PacketType : char {
		TCPID = 't',
		BodyIndexID = 'x',
		IrID = 'i',
		BodyID = 'b',
		JsonID = 'j',
		UnknownID ='k'
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

	class OSCMessage {
	public:
		static shared_ptr<ofxJSON> toJson(shared_ptr<ofxOscMessage>);
		static shared_ptr<ofxOscMessage> fromJson(ofxJSON &data, const string&address);
	};

	const char PacketFence = 'f'; // used to validate packet were set properly

	// what gets sent over the wire
	struct TCPPacket {
		char type; // byte only
		char b[1]; // validates data was properly read
	};
	struct ReadTCPPacket {
		char type; // byte only
		string data;
	};
	struct TCPMessage {
		int clientID;			// -1 for all connected
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

		shared_ptr<ofxJSON> get(const string&address);

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
		void update(const char * rawBytes, const size_t numBytes, PacketType type, int clientID = -1);
		int maxItems = 10; // max size of q
	private:
		ofxTCPServer server;
		void threadedFunction();
		void sendbinary(TCPMessage *m);
		deque<TCPMessage*> q;
	};
	class TCPClient : ofThread {
	public:
		void setup(const string& ip= defaultServerIP, int _port=TCP, bool blocking=false);

		shared_ptr<ReadTCPPacket> get();
	private:
		char update();
		void threadedFunction();
		deque<shared_ptr<ReadTCPPacket>> q;
		ofxTCPClient tcpClient; 
	};

	typedef std::unordered_map<OurPorts, shared_ptr<TCPClient>> ClientMap;

	class TCPReader : public ofThread {
	public:
		void setup(const string& ip = defaultServerIP);
		void add(const string& ip = defaultServerIP, OurPorts port = TCP, bool blocking = false);
		bool get(OurPorts port, shared_ptr<ReadTCPPacket>& packet);

	private:
		void threadedFunction();
		virtual void update()=0; // let the thread do this
		ClientMap clients;
	};

	typedef std::unordered_map<OurPorts, shared_ptr<TCPServer>> ServerMap;

	// send in tpc or osc
	class Server {
	public:
		void setup();
		void sendTCP(const char * bytes, const size_t numBytes, OurPorts port, int clientID = -1);
		void addTCPServer(OurPorts port = TCP, bool blocking = false);
		bool tcpEnabled();
		bool enabled(OurPorts port);
		void sendOsc(ofxJSON &data, const string&address) { comms.send(data, address); }
	private:
		WriteOsc comms;
		ServerMap servers;
	};

}