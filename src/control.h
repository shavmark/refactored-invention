#pragma once

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

	class Client : public TCPReader {
	public:
		void setup();
		void update();
		
		deque<shared_ptr<IRImage>>&IRQ() { return irQ; }
		deque<shared_ptr<BodyIndexImage>>&bodyIndexQ() { return biQ; }
		deque<shared_ptr<Kinect>>&kinectQ() { return kQ; }
		shared_ptr<ofxJSON> get(const string&address) { return OscReader.get(address); };
	private:
		deque<shared_ptr<IRImage>>irQ;
		deque<shared_ptr<BodyIndexImage>>biQ;
		deque<shared_ptr<Kinect>>kQ;
		ReadOsc OscReader;
	};

}