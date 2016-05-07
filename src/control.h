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

	class StagedClient  {
	public:
		void set(shared_ptr<Stage> stage) { backStagePass = stage; }
	protected:
		shared_ptr<Stage> backStagePass = nullptr;
	};
	class BodyIndexClient : public TCPPixels, public StagedClient {
	public:
		void update();
	};

	class TCPKinectClient : public TCPClient, public StagedClient {
	public:
		void update();


	};

}