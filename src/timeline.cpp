#include "ofapp.h"
#include "color.h"
#include "model.h"

namespace Software2552 {

	Timeline::Timeline(){
		//bugbug should we get this from data too at some point? these items cannot be set for each object ofColor colorOne(255, 0, 0);
		// bugbug a timeline json?

	}
	// read one act and save it to the list of acts (ie a story)
	bool Timeline::readScript(const string& path) {

		//return playlist.read(path);
		return true;
	}
	void Timeline::start() {
	}
	void Timeline::resume() {
	}
	void Timeline::pause() {
	}

	void Timeline::setup() {
		//ofSeedRandom(); // turn of to debug if needed

		// design to run in low mem on slow devices, many items can not be allocated and graphics should still display

		//bugbug use ofxOscMessage ofxOscReceiver (cool way to find server ip for all things, server may need to broad cast this now and then
		// or advertise I guess for new folks that come on line
		stage = std::make_shared<Software2552::Stage>();
		if (!stage) {
			return; //things would be really messed up...
		}

		oscClient = std::make_shared<Software2552::ReadOsc>();
		if (oscClient) {
			oscClient->setup();
		}

		router = std::make_shared<Software2552::Sender>();
		if (router) {
			router->setup();
			router->addTCPServer(TCP, true); // general server
			router->sendOsc("client", SignOnClientOscAddress);
		}
#ifdef _WIN64
		if (((ofApp*)ofGetAppPtr())->seekKinect) {
			kinectDevice = std::make_shared<KinectDevice>();
			if (kinectDevice) {
				// build out a full kinect
				router->setupKinect();
				if (kinectDevice->setup(router, stage, 5)) {
					kinectBody = std::make_shared<Software2552::KinectBody>(kinectDevice);
					if (kinectBody) {
						kinectBody->useFaces(std::make_shared<Software2552::KinectFaces>(kinectDevice));
						kinectBody->useAudio(std::make_shared<Software2552::KinectAudio>(kinectDevice));
					}
					router->sendOsc(kinectDevice->getId(), SignOnKinectServerOscAddress);
				}
			}
		}
#endif

		ofSetVerticalSync(false);
		ofSetFrameRate(frameRate);
		colorlist.setup();
		
		//write.setup();
		ofxJSON data;
		data.open("json4.json");// use json editor vs. coding it up
		//write.send(data, "graphics");
		return;

	}
	// keep this super fast
	void Timeline::update() { 
#ifdef _WIN64
		// kinect can only go 30fps 
		if (kinectBody) {
			kinectBody->update();
		}
#endif
		// router updates itself and builds a queue of input
		// check for sign on/off etc of things
		string signon;
		string source = oscClient->getString(signon, SignOnKinectServerOscAddress);
		// if there is a valid message, if I am not the kinect sending the sign on is requested then ...
		if (source.size() > 0 && signon.size() > 0 && source != kinectServerIP) {
			kinectServerIP = source;//bugbug only 1 kinect for now, needs to be a vector for > 1 kinect
			if (!kinectDevice || signon != kinectDevice->getId()) {
				tcpKinectClient = std::make_shared<Software2552::TCPKinectClient>(); // frees of any existing
				ofLogNotice("Timeline::update()") << " client sign on for kinect ID " << signon;
				//tcpKinectClient->add(kinectServerIP, TCPKinectIR, true); //bugbug get server ip via osc broad cast or such, osc sign on from kinect likely to contain ip
				tcpKinectClient->add(kinectServerIP, TCPKinectBody, true);
				tcpKinectClient->add(kinectServerIP, TCPKinectBodyIndex, true);
				if (tcpKinectClient) {
					tcpKinectClient->setup(); // uses a thread to read
				}
				stage->setup(tcpKinectClient);
			}
		}
		else if (tcpKinectClient) {
			tcpKinectClient->update();
		}
		if (stage) {
			stage->update();
		}
		return;
#if 0
		//kinect/joints kinect/face kinect/audio kinect/body kinect/audioCommand kinect/install
		shared_ptr<ofxJSON> joints = read.get("kinect/joints");
		string s;
		if (joints) {
			s = joints->getRawString();
			int type = (*joints)["jointType"].asInt();
			x = (*joints)["depth"]["x"].asFloat();
			y = (*joints)["depth"]["y"].asFloat();
			x2 = (*joints)["color"]["x"].asFloat();
			y2 = (*joints)["color"]["y"].asFloat();
			x3 = (*joints)["cam"]["x"].asFloat();
			y3 = (*joints)["cam"]["y"].asFloat();
			z3 = (*joints)["cam"]["z"].asFloat();
		}
		shared_ptr<ofxJSON> face = read.get("kinect/face");
		if (face) {
			s = face->getRawString();
		}
		shared_ptr<ofxJSON> install = read.get("kinect/install");
		if (install) {
			s = install->getRawString();
		}
		shared_ptr<ofxJSON> body = read.get("kinect/body");
		if (body) {
			s = body->getRawString();
		}
		shared_ptr<ofxJSON> audio = read.get("kinect/audio");
		if (audio) {
			s = audio->getRawString();
		}
		shared_ptr<ofxJSON> audioCommand = read.get("kinect/audioCommand");
		if (audioCommand) {
			s = audioCommand->getRawString();
		}
		// add a drawing object like a small square that will only draw 5 frames or such using
		// the new frame count code
		//stage.updateData(read.get("graphics")); // data can come from files, http/s, osc ++
		stage.update();
		return;

#endif // 0
		playlist.removeExpiredItems();
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->update();
		}
		else {
			ofExit();
		}
		colorlist.update();
	};

	// keep as fast as possible
	void Timeline::draw() {
		if (stage) {
			stage->draw();
		}
		//mesh.draw();
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->draw();
		}
	};

}