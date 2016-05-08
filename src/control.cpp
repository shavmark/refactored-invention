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
	void PixelsClient::myUpdate(shared_ptr<ofPixels> pixels) {
		if (backStagePass && pixels) {
			// map data to stage
			ofPoint pt;// start at 0,0
			pt.x = getDepthFrameWidth();// *ratioDepthToScreenX();
			pt.y = 0;
			shared_ptr<PixelsManager>p = std::make_shared<PixelsManager>();
			if (p) {
				p->pixels = pixels; // drawing needs to occur in main thread to make 
				p->setActorPosition(pt);//bugbug not set in object yet
				backStagePass->addToAnimatable(p);
			}
		}
	}
	void TCPKinectClient::myUpdate(shared_ptr<ReadTCPPacket> packet) {
		if (backStagePass && packet) {
			shared_ptr<Kinect>k = std::make_shared<Kinect>();
			if (k) {
				ofPoint pt;// start at 0,0
				k->bodyFromTCP(packet->data.c_str(), packet->data.size());
				k->setup();
				pt.x = 0;
				pt.y = getDepthFrameHeight();// *ratioDepthToScreenY();
				k->setActorPosition(pt);
				backStagePass->addToAnimatable(k);
			}
		}
	}
}