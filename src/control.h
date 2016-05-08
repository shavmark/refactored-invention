#pragma once

// ties to to things together

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

	class StagedClient  {
	public:
		void set(shared_ptr<Stage> stage) { backStagePass = stage; }
	protected:
		shared_ptr<Stage> backStagePass = nullptr;
	};
	// read known size ofPixels
	class PixelsClient : public TCPPixels, public StagedClient {
	public:
		virtual void myUpdate(shared_ptr<ofPixels> pixels);

	};
	
	// easily add in more types here such as...

	class TCPKinectClient : public TCPClient, public StagedClient {
	public:
		virtual void myUpdate(shared_ptr<ReadTCPPacket> packet);// let dervied class do their thing, q not used

	};

}