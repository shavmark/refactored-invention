#include "timeline.h"
#include "model.h"
namespace Software2552 {

	Timeline::Timeline(){
		//bugbug should we get this from data too at some point? these items cannot be set for each object ofColor colorOne(255, 0, 0);
		// bugbug a timeline json?

	}
	// read one act and save it to the list of acts (ie a story)
	bool Timeline::readScript(const string& path) {

		return playlist.read(path);
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
		return;

		mesh.setup();
	}
	// keep this super fast
	void Timeline::update() { 
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
		//mesh.draw();
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->draw();
		}
	};

}