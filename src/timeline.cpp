#include "ofapp.h"
#include "color.h"
#include "model.h"


namespace Software2552 {
	//bugbug maybe add this https://github.com/arturoc/ofxDepthStreamCompression
	Timeline::Timeline(){
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
	void  Timeline::sendClientSigon(shared_ptr<Sender> sender) {
		if (sender) {
			router->sendOsc(((ofApp*)ofGetAppPtr())->appconfig.getsignon(), SignOnClientOscAddress);
		}
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
			router->addTCPServer(TCP, false); // general server
		}

#ifdef _WIN64
		if (((ofApp*)ofGetAppPtr())->appconfig.getseekKinect()) {
			kinectDevice = std::make_shared<KinectDevice>();
			if (kinectDevice) {
				// build out a full kinect
				router->setupKinect();
				if (kinectDevice->setup(router, stage, 25)) { // give after a while
					kinectBody = std::make_shared<Software2552::KinectBody>(kinectDevice);
					if (kinectBody) {
						kinectBody->useFaces(std::make_shared<Software2552::KinectFaces>(kinectDevice));
						kinectBody->useAudio(std::make_shared<Software2552::KinectAudio>(kinectDevice));
					}
					shared_ptr<ofxOscMessage> signon = std::make_shared<ofxOscMessage>();
					if (signon) {
						signon->addStringArg(kinectDevice->getId());
						router->sendOsc(signon, SignOnKinectServerOscAddress);
					}
				}
			}
		}
#endif

		ofSetVerticalSync(false);
		((ofApp*)ofGetAppPtr())->appconfig.setFrameRateinOF();
		colorlist.setup();
		
		//write.setup();
		ofxJSON data;
		for (const auto& file : ((ofApp*)ofGetAppPtr())->appconfig.jsonFile) {
			//bugbug write a better iterator etc so we can loop through jsons for ever
			data.open(file);// use json editor vs. coding it up
		}
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
		shared_ptr<ofxOscMessage> signon = oscClient->getMessage(SignOnKinectServerOscAddress);
		// if there is a valid message, if I am not the kinect sending the sign on is requested then ...
		if ((!((ofApp*)ofGetAppPtr())->appconfig.getseekKinect()) && signon) {
			ofLogNotice("Timeline::update()") << " client sign on for kinect ID " << signon->getArgAsString(0);
			kinectServerIP = signon->getRemoteIp();//bugbug only 1 kinect for now, needs to be a vector for > 1 kinect
			tcpKinectClient = std::make_shared<Software2552::TCPKinectClient>(); // frees of any existing
			if (tcpKinectClient) {
				tcpKinectClient->set(stage);
				tcpKinectClient->setup(kinectServerIP, TCPKinectBody, true);
			}
			stage->setup(tcpKinectClient);
			tcpBodyIndex = std::make_shared<Software2552::PixelsClient>(BodyIndexID); // frees of any existing
			if (tcpBodyIndex) {
				tcpBodyIndex->set(stage);
				tcpBodyIndex->setup(kinectServerIP, TCPKinectBodyIndex, true);
			}
			/* ir takes too much from our little Kinect server, maybe the server needs to be high powered, like this dev box (or the one I gave away :() 
			tcpIRIndex = std::make_shared<Software2552::PixelsClient>(); // frees of any existing
			if (tcpIRIndex) {
				tcpIRIndex->set(stage);
				tcpIRIndex->setup(kinectServerIP, TCPKinectIR, true);
				tcpIRIndex->pt.x = getDepthFrameWidth();// *ratioDepthToScreenX();
			}
			*/
		}
		if (stage) {
			stage->update();
		}
		// let new people know where are here once time per minute (every window will call this)
		uint64_t frame = ofGetFrameNum();
		uint64_t rate = ((ofApp*)ofGetAppPtr())->appconfig.getFramerate() * 60;
		if ((frame % rate ) == 0 && router) {
			sendClientSigon(router);
		}
		// see if someone else checked in, every window is a client (but we do not care really, we just care that its a client)
		// its a bit bizzare in that we may talk to ourself via osc but thats ok on an internal network with out much traffic usign osc
		// and these sign on items are not time sensative
		signon = oscClient->getMessage(SignOnClientOscAddress);
		if (signon) {
			// add or update client bubug we do not use this data yet
			//MachineConfiguration
			string name;
			AppConfiguration::getName(name, signon);
			name += ofToString(AppConfiguration::getWindowNumber(signon));
			if (name != ((ofApp*)ofGetAppPtr())->appconfig.getName()+ ofToString(((ofApp*)ofGetAppPtr())->appconfig.getWindowNumber())) { // skip our own signon messages
				std::vector <shared_ptr<AppConfiguration>>::iterator it = std::find_if(others.begin(), others.end(), [&name](shared_ptr<AppConfiguration>p) {
					return p->getName() == name;
				});
				if (it == others.end()) {
					shared_ptr<AppConfiguration> p = std::make_shared<AppConfiguration>(signon);
					if (p) {
						others.push_back(p);
					}
				}
				else {
					*it = std::make_shared<AppConfiguration>(signon); //bugbug does this really work? It should
				}
			}
		}
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
		if (stage) {
			stage->draw();
		}
		//mesh.draw();
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->draw();
		}
	};

}