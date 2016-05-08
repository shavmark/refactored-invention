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
		addTCPServer(TCPKinectBodyIndex, true);
		addTCPServer(TCPKinectBody, true);
	}
#endif
	void BodyIndexClient::update() {
		// if no pass the buffer will grow and go... so be sure to set one
		if (backStagePass) {
			TCPPixels::update(); //reads the data
			shared_ptr<ofPixels> pixels;
			do {
				pixels = get();
				ofPoint pt;// start at 0,0
				if (pixels) {
					// map data to stage
					pt.x = getDepthFrameWidth();// *ratioDepthToScreenX();
					pt.y = 0;
					shared_ptr<BodyIndexImage>p = std::make_shared<BodyIndexImage>();
					if (p) {
						p->pixels = pixels; // drawing needs to occur in main thread to make 
						p->setActorPosition(pt);//bugbug not set in object yet
						backStagePass->addToAnimatable(p);
					}
				}
			} while (pixels);
		}
	}
	// read from Kinect and save data (or from any input port)
	void TCPKinectClient::update() {
		if (backStagePass) {
			TCPClient::update(); //reads the data
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
	
}