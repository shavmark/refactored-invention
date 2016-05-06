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
		TCPID = 't',
		BodyIndexID = 'x',
		IrID = 'i',
		BodyID = 'b',
		JsonID = 'j',
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
		char typeOfPacket; // bytes only
		char b[1]; // validates data was properly read
	};
	struct ReadTCPPacket {
		char type; // byte only
		string data;
	};
	struct TCPMessage {
		int clientID=-1;	// -1 for all connected
		TypeOfSend typeOfSend=Message;// 's' stream or 'm' message
		shared_ptr<ofPixels> pixels=nullptr; // optional pixels to send
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
	class TCPClient : ofThread {
	public:
		void setup(const string& ip= defaultServerIP, int _port=TCP, bool blocking=false);

		shared_ptr<ReadTCPPacket> get();
	private:
		char update();
		void readPixelStream(ofPixels &pixels, float width, float height);
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