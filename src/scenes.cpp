#include "2552software.h"
#include "model.h"
#include "scenes.h"
#include "animation.h"

//By default, the screen is cleared with every draw() call.We can change that with 
//  ofSetBackgroundAuto(...).Passing in a value of false turns off the automatic background clearing.

namespace Software2552 {

	// convert name to object
	shared_ptr<Stage> getScene(const string&name)
	{
		if (name == "TestBall") {
			return std::make_shared<TestBallScene>();
		}
		if (name == "Space") {
			return std::make_shared<SpaceScene>();
		}
		if (name == "Test") {
			return std::make_shared<TestScene>();
		}
		logTrace("name not known (ignored) using default scene " + name);
		return std::make_shared<GenericScene>();
	}

	// return a possibly modifed version such as camera moved
	shared_ptr<Camera> Director::pickem(forward_list<shared_ptr<Camera>>&cameras, bool orbiting) {
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
	void Stage::CreateReadAndaddBackgroundItems(const Json::Value &data) {
		createTimeLineItems<BackgroundItem>(settings, getAnimatables(), data, "backgrounds", this);
	}

	bool Stage::create(const Json::Value &data) { 

		// store in the order they will be displayed, every objec must be found here

		//for (Json::ArrayIndex i = 0; i < data["scenes"].size(); ++i) {
		string datastring = "test";
			if (!data["backgrounds"].empty()) {
				CreateReadAndaddBackgroundItems(data);
			}
			else if (datastring == "audio") {
				CreateReadAndaddAnimatable<Audio>(data[datastring]);
			}
			else if (datastring == "videoSphere") {
				CreateReadAndaddAnimatable<VideoSphere>(data[datastring]);
			}
			else if (datastring == "picture") {
				CreateReadAndaddAnimatable<Picture>(data[datastring]);
			}
			else if (datastring == "video") {
				CreateReadAndaddAnimatable<Video>(data[datastring]);
			}
			else if (datastring == "ball") {
				CreateReadAndaddAnimatable<Ball>(data[datastring]);
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
			else if (datastring == "video") {
				CreateReadAndaddAnimatable<Video>(data[datastring], "carride.mp4"); //bugbug remove last parameter when working
			}
			else if (datastring == "picture") {
				CreateReadAndaddAnimatable<Picture>(data[datastring], "t1_0010.jpg");
			}
			else if (datastring == "cube") {
				CreateReadAndaddAnimatable<Cube>(data[datastring]);
			}
			else if (datastring == "grabber") {
				CreateReadAndaddAnimatable<CameraGrabber>(data[datastring], "Logitech HD Pro Webcam C920");
			}
			else if (datastring == "camera") {
				CreateReadAndaddCamera(data[datastring]);
				//bugbug need to ready the fixed/moving from data CreateReadAndaddCamera(data["cam2"], true);
			}
			else if (datastring == "pointLight") {
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
		//}

		return myCreate(data); 
	};

	void Stage::draw() {

		if (drawIn2d) {
			ofPushStyle();
			draw2d();
			ofPopStyle();
		}
		if (drawIn3dMoving || drawIn3dFixed) {
			ofPushStyle();
			draw3d();
			ofPopStyle();
		}
	}
	// pause them all
	void Stage::pause() {
		for (auto& a : animatables) {
			a->getDefaultRole()->getAnimationHelper()->pause();
		}
		//bugbug pause moving camera, grabber etc
		myPause();
	}
	void Stage::resume() {
		for (auto& a : animatables) {
			a->getDefaultRole()->getAnimationHelper()->resume();
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
			removeExpiredItems(cameras);
			removeExpiredItems(lights);
		}
		myClear(force);
	}

	void Stage::setup() {

		material = std::make_shared<Material>();
		if (material != nullptr) {
			material->setShininess(90);
			material->setSpecularColor(ofColor::hotPink);//bugbug this needs to be set based on color
		}

		mySetup();
	}
	void Stage::update() {
		for (auto& a : animatables) {
			a->getDefaultRole()->updateForDrawing();
		}
		if (colors != nullptr) {
			colors->update();
		}
		myUpdate();// dervived class update

	}
	// setup light and material for drawing
	void Stage::installLightAndMaterialThenDraw(shared_ptr<Camera>cam, bool drawFixed) {
		if (cam != nullptr) {
			cam->getPlayer().begin();
			cam->orbit(); 
			for (auto& light : lights) {
				ofPoint p = light->getPlayer().getPosition();
				light->getPlayer().setPosition(light->loc);
				light->getPlayer().enable();
				light->getPlayer().draw();
			}
			if (material) {
				material->begin();
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
			cam->getPlayer().end();
		}
		else {
			// draw w/o a camera
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2); // center when not using a camera
			for (auto& light : lights) {
				light->getPlayer().setPosition(0, 0, 600);
				light->getPlayer().enable();
				light->getPlayer().draw();
			}
			if (drawIn3dMoving && !drawFixed) {
				draw3dMoving();
			}
			if (drawIn3dFixed && drawFixed) {
				draw3dFixed();
			}
		}
	}
	void Stage::pre3dDraw() {
		//ofBackground(ofColor::blue); // white enables all colors in pictures/videos
		ofSetSmoothLighting(true);
		ofDisableAlphaBlending();
		ofEnableDepthTest();
	}
	void Stage::post3dDraw() {
		// clean up
		ofDisableDepthTest();
		if (material) {
			material->end();
		}
		for (auto& light : lights) {
			light->getPlayer().disable();
		}
		ofDisableLighting();
	}
	void Stage::draw3d() {

		pre3dDraw();

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

		post3dDraw();
	}
	// find overall duration of a scene
	float Stage::findMaxWait() {
		float f = 0;
		
		for (const auto& a : getAnimatables()) {
			setIfGreater(f, a->getDefaultRole()->getAnimationHelper()->getObjectLifetime() + a->getDefaultRole()->getAnimationHelper()->getWait());
		}

		return f;
	}
	// show light location, great for debugging and kind of fun too
	void Stage::drawlights() {
		for (const auto& light : getLights()) {
			ofSetColor(light->getPlayer().getDiffuseColor());
			ofPoint pos = light->getPlayer().getPosition();
			ofDrawSphere(light->getPlayer().getPosition(), 20.f);
		}
	}
	void Stage::draw2d() {
		myDraw2d();
		for (auto& a : animatables) {
			a->getDefaultRole()->drawIt(ActorRole::draw2d);
		}

		//ofBackground(ofColor::black);
		//bugbug option is to add vs replace:ofEnableBlendMode(OF_BLENDMODE_ADD);//bugbug can make these attributes somewhere
		//ofEnableAlphaBlending();
	}
	bool GenericScene::myCreate(const Json::Value &data) {
		READSTRING(keyname, data);
		settings.readFromScript(data);
		// read needed common types
		// set the bool drawIn3dFixed = false; 	bool drawIn3dMoving = false;		bool drawIn2d = true; as needed
		createTimeLineItems<Video>(settings, getAnimatables(), data, "videos", this);
		createTimeLineItems<Audio>(settings, getAnimatables(), data, "audios", this);
		createTimeLineItems<Paragraph>(settings, getAnimatables(), data, "paragraphs", this);
		createTimeLineItems<Picture>(settings, getAnimatables(), data, "images", this);
		createTimeLineItems<Text>(settings, getAnimatables(), data, "texts", this);
		createTimeLineItems<Sphere>(settings, getAnimatables(), data, "spheres", this);
		return true;
	}
	//great animation example
	bool TestBallScene::myCreate(const Json::Value &data) {
		try {
			return CreateReadAndaddAnimatable<Ball>(data) != nullptr;
		}
		catch (std::exception e) {
			logErrorString(e.what());
			return false;
		}
	}
	// juse need to draw the SpaceScene, base class does the rest
	void Stage::draw3dMoving() {
		myDraw3dMoving();
		for (auto& a : animatables) {
			a->getDefaultRole()->drawIt(ActorRole::draw3dMovingCamera);
		}
	}
	void Stage::draw3dFixed() {
		myDraw3dFixed();
		for (auto& a : animatables) {
			a->getDefaultRole()->drawIt(ActorRole::draw3dFixedCamera);
		}
	}
	template<typename T> shared_ptr<T> Stage::CreateReadAndaddAnimatable(const Json::Value &data, const string&location, Settings*pSettings) {
		shared_ptr<T> p = std::make_shared<T>(location);
		if (p == nullptr) {
			return nullptr;
		}
		if (pSettings) {
			p->setSettings(pSettings);
		}
		else {
			p->setSettings(settings);
		}
		p->readFromScript(data, this);
		return p;
	}
	// return new location

	template<typename T> shared_ptr<T> Stage::CreateReadAndaddAnimatable(const Json::Value &data, Settings*pSettings) {
		shared_ptr<T> p = std::make_shared<T>();
		if (p == nullptr) {
			return nullptr;
		}
		if (pSettings) {
			p->setSettings(*pSettings);
		}
		else {
			p->setSettings(settings);
		}
		p->readFromScript(data, this);
		return p;
	}
	shared_ptr<Camera> Stage::CreateReadAndaddCamera(const Json::Value &data, bool rotate) {
		shared_ptr<Camera> p = std::make_shared<Camera>();
		if (p == nullptr) {
			return nullptr;
		}
		if (p->readFromScript(data)) {
			p->setOrbit(rotate);
			p->getPlayer().setPosition(0, 0, ofRandom(100,500));//bugbug clean up the rand stuff via data and more organized random
			add(p);
		}
		return p;
	}
	template<typename T>shared_ptr<T> Stage::CreateReadAndaddLight(const Json::Value &data) {
		shared_ptr<T> p = std::make_shared<T>();
		if (p == nullptr) {
			return nullptr;
		}
		if (p->readFromScript(data)) {
			add(p);
		}
		return p;
	}

	// bugbug all items in test() to come from json or are this is done in Director
	bool TestScene::myCreate(const Json::Value &data) {

		CreateReadAndaddAnimatable<Video>(data["video"], "carride.mp4");
		CreateReadAndaddAnimatable<Picture>(data["picture"], "t1_0010.jpg");
		CreateReadAndaddAnimatable<Cube>(data["cube"]);
		CreateReadAndaddAnimatable<CameraGrabber>(data["grabber"], "Logitech HD Pro Webcam C920");
		CreateReadAndaddCamera(data["cam1"]);
		CreateReadAndaddCamera(data["cam2"], true);
		CreateReadAndaddLight<PointLight>(data["pointLight1"]);
		CreateReadAndaddLight<PointLight>(data["pointLight2"]);
		CreateReadAndaddLight<DirectionalLight>(data["directionalLight"]);
		CreateReadAndaddLight<Light>(data["basicLight"]);
		CreateReadAndaddLight<Light>(data["spotLight"]);

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
	 
	bool SpaceScene::myCreate(const Json::Value &data) {


		return true;
	}

}