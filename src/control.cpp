#include "ofApp.h"
#include "inc\Kinect.h" // needed for enums

// connect things together

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
		addTCPServer(TCPKinectBodyIndex, false);
		addTCPServer(TCPKinectBody, true);
	}
#endif
	void BodyIndexClient::update() {
		// if no pass the buffer will grow and go... so be sure to set one
		if (backStagePass) {
			shared_ptr<ReadTCPPacket> packet;
			do {
				packet = get();
				ofPoint pt;// start at 0,0

				if (packet) {
					// map data to stage
					shared_ptr<BodyIndexImage>bi = std::make_shared<BodyIndexImage>();
					if (bi) {
						bi->bodyIndexFromTCP(packet->data.c_str(), packet->data.size());
						pt.x = getDepthFrameWidth();// *ratioDepthToScreenX();
						pt.y = 0;
						bi->setActorPosition(pt);
						bi->setup();
						bi->setLoaded(); //avoi
						backStagePass->addToAnimatable(bi);
					}
				}
			} while (packet);
		}
	}
	// read from Kinect and save data (or from any input port)
	void TCPKinectClient::update() {
		if (backStagePass) {
			shared_ptr<IRImage>ir = nullptr;
			ofPoint pt;// start at 0,0
			shared_ptr<ReadTCPPacket> packet;
			do {

				packet = get();
				if (packet) {
					// map data to stage
					shared_ptr<Kinect>k = std::make_shared<Kinect>();
					if (k) {
						k->bodyFromTCP(packet->data.c_str(), packet->data.size());
						k->setup();
						pt.x = 0;
						pt.y = getDepthFrameHeight();// *ratioDepthToScreenY();
						k->setActorPosition(pt);
						backStagePass->addToAnimatable(k);
					}
				}
			} while (packet);

		}
	}
#if 0
						// this code is designed to read all set connections, validate data and process it
		if (get(TCPKinectBodyIndex, packet)) {
			shared_ptr<BodyIndexImage>bi = std::make_shared<BodyIndexImage>();
			if (bi) {
				bi->bodyIndexFromTCP(packet->data.c_str(), packet->data.size());
				pt.x = getDepthFrameWidth();// *ratioDepthToScreenX();
				pt.y = 0;
				bi->setActorPosition(pt);
				bi->setup();
				lock();
				biQ.push_back(bi);
				unlock();
			}
		}

		if (get(TCPKinectIR, packet)) {
			shared_ptr<IRImage>ir = std::make_shared<IRImage>();
			if (ir) {
				ir->IRFromTCP((const UINT16 *)packet->data.c_str(), packet->data.size() / sizeof(UINT16));
				ir->setup();
				pt.x = 0;
				pt.y = 0;
				ir->setActorPosition(pt); // upper left
				lock();
				//irQ.push_back(ir);
				unlock();
			}
		}

		if (get(TCPKinectBody, packet)) {
			shared_ptr<Kinect>k = std::make_shared<Kinect>();
			if (k) {
				k->bodyFromTCP(packet->data.c_str(), packet->data.size());
				k->setup();
				pt.x = 0;
				pt.y = getDepthFrameHeight();// *ratioDepthToScreenY();
				k->setActorPosition(pt);
				lock();
				kQ.push_back(k);
				unlock();
			}
		}

		if (get(TCP, packet)) {
			;// not defined yet
		}

#endif // 0
	
}