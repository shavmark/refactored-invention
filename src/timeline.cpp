#include "timeline.h"
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

		mesh.setup();
	}
	// keep this super fast
	void Timeline::update() { 
		//kinect/joints kinect/face kinect/audio kinect/body kinect/audioCommand
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