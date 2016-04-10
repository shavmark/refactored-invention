#include "2552software.h"
#include "model.h"
#include "scenes.h"
#include "animation.h"

//By default, the screen is cleared with every draw() call.We can change that with 
//  ofSetBackgroundAuto(...).Passing in a value of false turns off the automatic background clearing.

namespace Software2552 {
	
	shared_ptr<Stage> stage=nullptr; // one global stage, makes it easy to change between stages
	// always valid stage pointer
	shared_ptr<Stage> getStage() {
		if (stage == nullptr) {
			getScene("Generic"); // default
		}
		return stage;
	}
	// convert name to object
	shared_ptr<Stage> getScene(const string&name)
	{
		if (name == "Test") {
			return stage = std::make_shared<TestScene>();
		}
		if (name == "Generic") {
			return stage = std::make_shared<GenericScene>();
		}
		logTrace("name not known (ignored) using default scene " + name);
		return stage = std::make_shared<GenericScene>();
	}

	// return a possibly modifed version such as camera moved
	shared_ptr<Camera> Director::pickem(vector<shared_ptr<Camera>>&cameras, bool orbiting) {
		for (auto it = cameras.begin(); it != cameras.end(); ++it) {
			if (orbiting) {
				if ((*it)->isOrbiting()) {
					return (*it);
				}
			}
			else {
				if (!(*it)->isOrbiting()) {
					//bugbug return only fixed cameras
					return (*it);// stuff data in the Camera class
									  // lots to do here to make things nice, learn and see how to bring this in
									  //void lookAt(const ofNode& lookAtNode, const ofVec3f& upVector = ofVec3f(0, 1, 0));
									  //if (j == 1) {
									  //cameras[j].move(-100,0,0);
									  //cameras[j].pan(5);
									  //}
									  //else {
									  //cameras[j].move(100, 0, 0);
									  //}
									  //bugbug just one thing we can direct http://halfdanj.github.io/ofDocGenerator/ofEasyCam.html#CameraSettings
									  //float f = cameras[j].getDistance();
									  //figure all this out, with times, animation etc:
									  //cameras[j].setDistance(cameras[j].getDistance() + ofRandom(-100,100));
									  //return &cameras[j];
				}

			}
		}
		logErrorString("no camera");
		return nullptr;
	}
	// set background object at location 0 every time
	shared_ptr<Background> Stage::CreateReadAndaddBackgroundItems(const Json::Value &data) {
		shared_ptr<Background> b = std::make_shared<Background>();
		if (b != nullptr) {
			if (b->setup(data)) {
				// only save if data was read in 
				addToAnimatable(b, true);
			}
		}
		return b;
	}
	///bigger the number the more forward
	bool compareOrder(shared_ptr<ActorRole>first, shared_ptr<ActorRole>second)	{
		return (first->drawOrder < second->drawOrder);
	}
	// samples https://sites.google.com/site/ofauckland/examples
#define ADDANIMATION(name,type)	if (!data[STRINGIFY(name)].empty()) CreateReadAndaddAnimatable<type>(data[STRINGIFY(name)])
#define ADDLIGHT(name,type)	if (!data[STRINGIFY(name)].empty()) CreateReadAndaddLight<type>(data[STRINGIFY(name)])

	bool Stage::setup(const Json::Value &data) {
		//bool b = pic.loadImage("hubble1.jpg");
		//ADDANIMATION(rainbow, Rainbow);
		//ADDANIMATION(picture, Picture);
		//ADDANIMATION(arrow, Arrow);
		ADDANIMATION(ball, Ball);
		getAnimatables().sort(compareOrder);
		//if (!data["background"].empty()) {
			//CreateReadAndaddBackgroundItems(data["background"]);
		//}
		return true;
		//ADDANIMATION(sphere, Sphere);// need to make sure there is a camera and light (maybe do an error check)
		//ADDLIGHT(light, Light);
		
		ADDANIMATION(picture, Picture);
		//ADDANIMATION(video, Video);
		getAnimatables().sort(compareOrder);
		// set a default camera if none exist
		if (getCameras().size() == 0) {
			CreateReadAndaddCamera(data["cameraFixed"]);
		}
		// set a default light if none exist
		if (getLights().size() == 0) {
			CreateReadAndaddLight<Light>(data["light"]);
		}
		// read back ground after sort as its objects are inserted in the front of the draw list
		//if (!data["background"].empty()) {
			//CreateReadAndaddBackgroundItems(data["background"]);
		//}
		return true;
		
		// data must Cap first char of key words
#define SETANIMATION(name)	if (!data[STRINGIFY(name)].empty()) CreateReadAndaddAnimatable<name>(data[STRINGIFY(name)])

		for (Json::ArrayIndex i = 0; i < data["drawingObjects"].size(); ++i) {
			string datastring = data["drawingObjects"][i].asString();
			SETANIMATION(Audio);
			if (datastring == "videoSphere") {
				CreateReadAndaddAnimatable<VideoSphere>(data[datastring]);
			}
			else if (datastring == "text") {
				CreateReadAndaddAnimatable<Text>(data[datastring]);
			}
			else if (datastring == "paragraph") {
				CreateReadAndaddAnimatable<Paragraph>(data[datastring]);
			}
			else if (datastring == "sphere") {
				CreateReadAndaddAnimatable<Sphere>(data[datastring]);
			}
			else if (datastring == "planet") {
				CreateReadAndaddAnimatable<Planet>(data[datastring]);
			}
			else if (datastring == "solarSystem") {
				CreateReadAndaddAnimatable<SolarSystem>(data[datastring]);
			}
			else if (datastring == "cube") {
				CreateReadAndaddAnimatable<Cube>(data[datastring]);
			}
			else if (datastring == "grabber") {
				CreateReadAndaddAnimatable<CameraGrabber>(data[datastring]);
			}
		}
		for (Json::ArrayIndex i = 0; i < data["cameraObjects"].size(); ++i) {
			string datastring = data["cameraObjects"][i].asString();
			//bugbug maybe create things like CameraLeft, Middle, front etc
			if (datastring == "cameraMoving") {
				CreateReadAndaddCamera(data[datastring], true);
			}
			else if (datastring == "cameraFixed") {
				CreateReadAndaddCamera(data[datastring]);
			}
		}
			for (Json::ArrayIndex i = 0; i < data["lightingObjects"].size(); ++i) {
			string datastring = data["lightingObjects"][i].asString();

			if (datastring == "pointLight") {
				CreateReadAndaddLight<PointLight>(data[datastring]);
			}
			else if (datastring == "directionalLight") {
				CreateReadAndaddLight<DirectionalLight>(data[datastring]);
			}
			else if (datastring == "spotLight") {
				CreateReadAndaddLight<SpotLight>(data[datastring]);
			}
			else if (datastring == "light") {
				CreateReadAndaddLight<Light>(data[datastring]);
			}
		}

		return true; 
	};

	void Stage::draw() {
		preDraw();
		if (drawIn2d) {
			ofPushStyle();
			ofPushMatrix();
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);// default position, cameras may change location
			draw2d();
			ofPopMatrix();
			ofPopStyle();
		}
		if (drawIn3dMoving || drawIn3dFixed) {
			ofPushStyle();
			ofPushMatrix();
			draw3d();
			ofPopMatrix();
			ofPopStyle();
		}
		postDraw();
	}
	// pause them all
	void Stage::pause() {
		for (auto& a : animatables) {
			a->pause();
		}
		//bugbug pause moving camera, grabber etc
		myPause();
	}
	void Stage::resume() {
		for (auto& a : animatables) {
			a->resume();
		}
		//bugbug pause moving camera, grabber etc
		myResume();
	}
	// clear objects
	void Stage::clear(bool force) {
		if (force) {
			animatables.clear();
			cameras.clear();
			lights.clear();
		}
		else {
			removeExpiredItems(animatables);
			removeExpiredItems<Camera>(cameras);
			removeExpiredItems<Light>(lights);
		}
		myClear(force);
	}

	void Stage::setup() {
		mySetup();
	}
	void Stage::update() {
		
		for (auto& a : animatables) {
			a->updateForDrawing();
		}
		myUpdate();// dervived class update

	}
	// setup light and material for drawing
	void Stage::installLightAndMaterialThenDraw(shared_ptr<Camera>cam, bool drawFixed) {
		if (cam != nullptr) {
			cam->worker.begin();
			cam->orbit(); 
			for (auto& light : lights) {
				ofPoint p = light->worker.getPosition();
				light->worker.setPosition(light->loc);
				light->worker.enable();
				light->worker.draw();
			}
			if (cam->isOrbiting()) {
				if (drawIn3dMoving && !drawFixed) {
					draw3dMoving();
				}
			}
			else {
				if (drawIn3dFixed && drawFixed) {
					draw3dFixed();
				}
			}
			cam->worker.end();
		}
		else {
			// draw w/o a camera
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2); // center when not using a camera
			for (auto& light : lights) {
				light->worker.setPosition(0, 0, 600);
				light->worker.enable();
				light->worker.draw();
			}
			if (drawIn3dMoving && !drawFixed) {
				draw3dMoving();
			}
			if (drawIn3dFixed && drawFixed) {
				draw3dFixed();
			}
		}
	}
	void Stage::addToAnimatable(shared_ptr<ActorRole>p, bool inFront) {
		// only save working pointers
		if (p != nullptr) {
			ActorRole::drawtype tp = p->getType();
			if (p->getType() == ActorRole::draw3dFixedCamera) {
				fixed3d(true); // do not set to false in case its already set
			}
			if (p->getType() == ActorRole::draw3dMovingCamera) {
				moving3d(true); // do not set to false in case its already set
			}
			if (p->getType() == ActorRole::draw2d) {
				fixed2d(true); // do not set to false in case its already set
			}
			p->setupForDrawing();
			if (inFront) {
				animatables.push_front(p);
			}
			else {
				animatables.push_back(p);
			}
		}
	}

	void Stage::preDraw() {
		ofDisableDepthTest(); // bugbug make this a var read in via json so we can make more advanced graphics later
		//ofBackground(ofColor::blue); // white enables all colors in pictures/videos
		ofSetSmoothLighting(true);
		ofDisableAlphaBlending();

		//ofEnableDepthTest();
	}
	void Stage::postDraw() {
		// clean up
		ofEnableDepthTest();
		for (auto& light : lights) {
			light->worker.disable();
		}
		ofDisableLighting();
	}
	void Stage::draw3d() {

		// fixed camera
		if (drawIn3dFixed) {
			shared_ptr<Camera> cam = director.pickem(cameras, false);
			if (cam != nullptr) {
				installLightAndMaterialThenDraw(cam, true);
			}
		}
		if (drawIn3dMoving) {
			shared_ptr<Camera> cam = director.pickem(cameras, true);
			if (cam != nullptr) {
				installLightAndMaterialThenDraw(cam, false);
			}
		}

	}
	// find overall duration of a scene
	float Stage::findMaxWait() {
		float f = 0;
		
		for (const auto& a : getAnimatables()) {
			setIfGreater(f, a->getObjectLifetime() + a->getWait());
		}

		return f;
	}
	// show light location, great for debugging and kind of fun too
	void Stage::drawlights() {
		for (const auto& light : getLights()) {
			ofSetColor(light->worker.getDiffuseColor());
			ofPoint pos = light->worker.getPosition();
			ofDrawSphere(light->worker.getPosition(), 20.f);
		}
	}
	void Stage::draw2d() {
		myDraw2d();
		for (auto& a : animatables) {
			a->drawIt(ActorRole::draw2d);
		}

		//ofBackground(ofColor::black);
		//bugbug option is to add vs replace:ofEnableBlendMode(OF_BLENDMODE_ADD);//bugbug can make these attributes somewhere
		//ofEnableAlphaBlending();
	}
	// juse need to draw the SpaceScene, base class does the rest
	void Stage::draw3dMoving() {
		myDraw3dMoving();
		for (auto& a : animatables) {
			a->drawIt(ActorRole::draw3dMovingCamera);
		}
	}
	void Stage::draw3dFixed() {
		myDraw3dFixed();
		for (auto& a : animatables) {
			a->drawIt(ActorRole::draw3dFixedCamera);
		}
	}
	shared_ptr<Camera> Stage::CreateReadAndaddCamera(const Json::Value &data, bool rotate) {
		shared_ptr<Camera> p = std::make_shared<Camera>();
		if (p == nullptr) {
			return nullptr;
		}
		if (p->setup(data)) {
			p->setOrbit(rotate);
			p->worker.setPosition(0, 0, ofRandom(100,500));//bugbug clean up the rand stuff via data and more organized random
			add(p);
		}
		return p;
	}
	template<typename T>shared_ptr<T> Stage::CreateReadAndaddLight(const Json::Value &data) {
		shared_ptr<T> p = std::make_shared<T>();
		if (p == nullptr) {
			return nullptr;
		}
		if (p->setup(data)) {
			add(p);
		}
		return p;
	}

	// bugbug all items in test() to come from json or are this is done in Director
	bool TestScene::myCreate(const Json::Value &data) {

		return true;

	}
	void TestScene::myUpdate() {
		mesh.update();
	}
	void TestScene::myDraw3dFixed() {
	
		//drawlights();

		return; // comment out to draw by vertice, less confusing but feel free to add it back in
		//bugbug item below needs to be put in a drawing class or should be removed
		ofSetHexColor(0xffffff);
		glPointSize(2);
		glEnable(GL_POINT_SMOOTH);
		mesh.drawVertices();
	}
	void TestScene::mySetup() {
		ofSetSmoothLighting(true);
		mesh.setup();
	}
	 
}