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
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->setup();
			playlist.getCurrent()->start();
		}
	}
	void Timeline::resume() {
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->resume();
		}
				
	}
	void Timeline::pause() {
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->pause();
		}
	}

	void Timeline::setup() {
		//ofSeedRandom(); // turn of to debug if needed
		client.setup(); // uses a thread to read
		client.add(defaultServerIP, TCPKinectIR, true); //bugbug get server ip via osc broad cast or such
		client.add(defaultServerIP, TCPKinectBody, true);
		client.add(defaultServerIP, TCPKinectBodyIndex, true);

		router = std::make_shared<Software2552::Sender>();
		if (!router) {
			return; //things would be really messed up...
		}

		router->addTCPServer(TCP, true); // general server

#ifdef _WIN64
		// there is not a special Kinect build, all builds will support a Kinect if they are 64 bits
		router->addTCPServer(TCPKinectIR, true);
		router->addTCPServer(TCPKinectBodyIndex, true);
		router->addTCPServer(TCPKinectBody, true);

		kinectDevice.setup(router);
		kinectBody = std::make_shared<Software2552::KinectBody>(&kinectDevice);
#endif

		ofSetVerticalSync(false);
		ofSetFrameRate(frameRate);
		colorlist.setup();
		stage.setup();
		read.setup();
		//write.setup();
		ofxJSON data;
		data.open("json4.json");// use json editor vs. coding it up
		//write.send(data, "graphics");
		return;

	}
	// keep this super fast
	void Timeline::update() { 
		return;
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
		stage.draw();
		//mesh.draw();
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->draw();
		}
	};

}