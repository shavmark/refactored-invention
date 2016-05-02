#include "ofApp.h"
#include "inc\Kinect.h" // needed for enums

namespace Software2552 {
	PacketType mapPortToType(OurPorts port) {
		switch (port) {
		case TCP:
			return TCPID;
		case TCPKinectIR:
			return IrID;
		case TCPKinectBodyIndex:
			return BodyIndexID;
		case TCPKinectBody:
			return BodyID;
		default:
			ofLogError("mapPortToType") << "invalid port " << port;
			return UnknownID;
		}
	}
	bool Sender::kinectIREnabled() {
#ifdef _WIN64
		return enabled(TCPKinectIR);
#else
		return false;
#endif
	}
	bool Sender::KinectBodyIndexEndabled() {
#ifdef _WIN64
		return enabled(TCPKinectBodyIndex);
#else
		return false;
#endif
	}
	bool Sender::KinectBodyEnabled() {
#ifdef _WIN64
		return enabled(TCPKinectBody);
#else
		return false;
#endif
	}
#ifdef _WIN64
	void Sender::setupKinect() {
		addTCPServer(TCPKinectIR, true);
		addTCPServer(TCPKinectBodyIndex, true);
		addTCPServer(TCPKinectBody, true);
	}
#endif
	void Client::setup() {
		TCPReader::setup();
		OscReader.setup();
	}
	// read from Kinect and save data (or from any input port)
	void Client::update() {
		shared_ptr<ReadTCPPacket> packet;
		shared_ptr<IRImage>ir = nullptr;

		// this code is designed to read all set connections, validate data and process it
		if (get(TCPKinectBodyIndex, packet)) {
			shared_ptr<BodyIndexImage>bi = std::make_shared<BodyIndexImage>();
			if (bi) {
				bi->bodyIndexFromTCP(packet->data.c_str(), packet->data.size());
				bi->setup();
				lock();
				biQ.push_back(bi);
				unlock();
			}
		}

		if (get(TCPKinectBody, packet)) {
			shared_ptr<Kinect>k = std::make_shared<Kinect>();
			if (k) {
				k->bodyFromTCP(packet->data.c_str(), packet->data.size());
				k->setup();
				lock();
				kQ.push_back(k);
				unlock();
			}
		}

		if (get(TCPKinectIR, packet)) {
			shared_ptr<IRImage>ir = std::make_shared<IRImage>();
			if (ir) {
				ir->IRFromTCP((const UINT16 *)packet->data.c_str());
				ir->setup();
				lock();
				irQ.push_back(ir);
				unlock();
			}
		}

		if (get(TCP, packet)) {
			;// not defined yet
		}

	}
}