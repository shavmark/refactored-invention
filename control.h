#pragma once

namespace Software2552 {
#define kinectWidthForColor 1920
#define kinectHeightForColor 1080
#define kinectWidthForIR 512
#define kinectHeightForIR 424
#define kinectWidthForDepth 512
#define kinectHeightForDepth 424

	PacketType mapPortToType(OurPorts ports);

	// can be used even w/o kinect installed
	class Sender : public Server {
	public:
#ifdef _WIN64
		void setupKinect();
#endif
		bool kinectIREnabled();
		bool KinectBodyIndexEndabled();
		bool KinectBodyEnabled();
	};

	class Client : public TCPReader {
	public:
		void update();
	};

}