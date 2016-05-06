#pragma once

// tries to to things together

namespace Software2552 {

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

	class TCPKinectClient : public TCPReader {
	public:
		void setup();
		void update();
		
		void deleteFromIrq(int index);
		void deleteFromBi(int index);
		void deleteFromBody(int index);

		deque<shared_ptr<IRImage>>irQ;
		deque<shared_ptr<BodyIndexImage>>biQ;
		deque<shared_ptr<Kinect>>kQ;
	};

}