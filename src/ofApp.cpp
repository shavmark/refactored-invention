#include "ofApp.h"
#include "2552software.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowTitle("Story Teller");
	//Software2552::SoundIn::setup();// move to timeline or scene
	//Software2552::SoundOut::setup();// move to timeline or scene

	ofSetLogLevel(OF_LOG_NOTICE);//OF_LOG_VERBOSE
	timeline.setup();
	timeline.readScript("json3.json");
	timeline.start();
	//paragraphs.build(ofGetWidth());
	
	//ofSetFullscreen(true);
	//ofSetFrameRate(60);
	//ofBackgroundHex(0x00000);
	//ofSetLogLevel(OF_LOG_NOTICE);//ofSetLogLevel(OF_LOG_VERBOSE);
	
	// we add this listener before setting up so the initial circle resolution is correct
#if 0
	circleResolution.addListener(this, &ofApp::circleResolutionChanged);
	ringButton.addListener(this, &ofApp::ringButtonPressed);

	gui.setup(); // most of the time you don't need a name
	gui.add(filled.setup("fill", true));
	gui.add(radius.setup("radius", 140, 10, 300));
	gui.add(center.setup("center", ofVec2f(ofGetWidth()*.5, ofGetHeight()*.5), ofVec2f(0, 0), ofVec2f(ofGetWidth(), ofGetHeight())));
	gui.add(color.setup("color", ofColor(100, 100, 140), ofColor(0, 0), ofColor(255, 255)));
	gui.add(circleResolution.setup("circle res", 5, 3, 90));
	gui.add(twoCircles.setup("two circles"));
	gui.add(ringButton.setup("ring"));
	gui.add(screenSize.setup("screen size", ofToString(ofGetWidth()) + "x" + ofToString(ofGetHeight())));

	bHide = false;

	ring.load("ring.wav");
#endif // 0

	years = 0;
	return;
	// show images, then wash them away

#if 0
	images.push_back(ofImage("C:\\Users\\mark\\Pictures\\maps\\Res37.jpe"));
	images.push_back(ofImage("C:\\Users\\mark\\Pictures\\maps\\Res37-2.jpe"));
	images.push_back(ofImage("C:\\Users\\mark\\Pictures\\maps\\Res37-2.jpe"));
	franklinBook14.load("frabk.ttf", 14, true, true, true);
	franklinBook14.setLineHeight(18.0f);
	franklinBook14.setLetterSpacing(1.037);
	ofDisableDepthTest();
	return;
	backgroundImage.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR);
	backgroundImage.loadImage("C:\\Users\\mark\\Documents\\iclone\\images\\robot.jpg");
	// read the directory for the images
	// we know that they are named in seq
	ofDirectory dir;

	int nFiles = dir.listDir("C:\\Users\\mark\\Documents\\iclone\\images");
	if (nFiles) {

		for (int i = 0; i < dir.size(); i++) {

			// add the image to the vector
			string filePath = dir.getPath(i);
			images.push_back(ofImage());
			images.back().load(filePath);

		}

	}
	else ofLog(OF_LOG_WARNING) << "Could not find folder";

	// this toggle will tell the sequence
		// be be indepent of the app fps
	bFrameIndependent = true;

	// this will readJsonValue the speed to play 
	// the animation back we readJsonValue the
	// default to 24fps
	sequenceFPS = 24;

	// readJsonValue the app fps 
	appFPS = 18;
	ofSetFrameRate(appFPS);

	myPlayer.loadMovie("C:\\Users\\mark\\Documents\\iclone\\background2.mp4");
	myPlayer.setLoopState(OF_LOOP_NORMAL);
	myPlayer.play();

	robot.loadMovie("C:\\Users\\mark\\Documents\\iclone\\images\\robot2.mp4");
	robot.setLoopState(OF_LOOP_NORMAL);
	robot.play();


	pathLines.setMode(OF_PRIMITIVE_LINE_STRIP);
	ofDisableArbTex(); // we need GL_TEXTURE_2D for our models coords.
	ofEnableDepthTest();
	light.enable();
	cam.setPosition(0, 0, 100);

	headTrackedCamera.setNearClip(0.01f);
	headTrackedCamera.setFarClip(1000.0f);

	//defining the real world coordinates of the window which is being headtracked is important for visual accuracy
	windowWidth = 0.3f;
	windowHeight = 0.2f;

	windowTopLeft = ofVec3f(-windowWidth / 2.0f,
		+windowHeight / 2.0f,
		0.0f);
	windowBottomLeft = ofVec3f(-windowWidth / 2.0f,
		-windowHeight / 2.0f,
		0.0f);
	windowBottomRight = ofVec3f(+windowWidth / 2.0f,
		-windowHeight / 2.0f,
		0.0f);

	//we use this constant since we're using a really hacky headtracking in this example
	//if you use something that can properly locate the head in 3d (like a kinect), you don't need this fudge factor
	viewerDistance = 0.4f;


	//From2552Software::Sound sound;
	//sound.test();

	myKinect.open();
	//audio.setup(&myKinect);
	//audio.setup(&myKinect);
	//audio.setup(&myKinect);

	//faces.setup(&myKinect);
	bodies.useFaces();
	bodies.setup(&myKinect);

	//ofSetWindowShape(640 * 2, 480 * 2);
	ofDisableDepthTest(); // draws in order vs z order

#endif // 0

}


//--------------------------------------------------------------
void ofApp::update(){
	timeline.update();
	Software2552::SoundOut::update();// move to timeline or scene
	Software2552::SoundIn::update();// move to timeline or scene
	return;
	ofSetCircleResolution(circleResolution);
	return;
//	myPlayer.update(); // get all the new frames
	//robot.update();
	//faces.update();
	//bodies.update();
	return;
	ofVec3f headPosition(0, 0, viewerDistance);
	
#if 0
	if (finder.blobs.size() > 0) {
		//get the head position in camera pixel coordinates
		float cameraHeadX = blob.centroid.x;
		float cameraHeadY = blob.centroid.y;

		//do a really hacky interpretation of this, really you should be using something better to find the head (e.g. kinect skeleton tracking)

		//since camera isn't mirrored, high x in camera means -ve x in world
		float worldHeadX = ofMap(cameraHeadX, 0, video.getWidth(), windowBottomRight.x, windowBottomLeft.x);

		//low y in camera is +ve y in world
		float worldHeadY = ofMap(cameraHeadY, 0, video.getHeight(), windowTopLeft.y, windowBottomLeft.y);

		//readJsonValue position in a pretty arbitrary way
		headPosition = ofVec3f(worldHeadX, worldHeadY, viewerDistance);
	}
	else {
		if (!video.isInitialized()) {
			//if video isn't working, just make something up
			headPosition = ofVec3f(0.5f * windowWidth * sin(ofGetElapsedTimef()), 0.5f * windowHeight * cos(ofGetElapsedTimef()), viewerDistance);
		}
	}

#endif // 0

	//if video isn't working, just make something up REPLACE with Kinect
	headPosition = ofVec3f(0.5f * windowWidth * sin(ofGetElapsedTimef()), 0.5f * windowHeight * cos(ofGetElapsedTimef()), viewerDistance);
	headPositionHistory.push_back(headPosition);
	while (headPositionHistory.size() > 50.0f) {
		headPositionHistory.pop_front();
	}

	//these 2 lines of code must be called every time the head position changes
	headTrackedCamera.setPosition(headPosition);
	headTrackedCamera.setupOffAxisViewPortal(windowTopLeft, windowBottomLeft, windowBottomRight);

	previous = current;

	// generate a noisy 3d position over time 
	float t = (2 + ofGetElapsedTimef()) * .1;
	current.x = ofSignedNoise(t, 0, 0);
	current.y = ofSignedNoise(0, t, 0);
	current.z = ofSignedNoise(0, 0, t);
	current *= 400; // scale from -1,+1 range to -400,+400

					// add the current position to the pathVertices deque
	pathVertices.push_back(current);
	// if we have too many vertices in the deque, get rid of the oldest ones
	while (pathVertices.size() > 200) {
		pathVertices.pop_front();
	}

	// clear the pathLines ofMesh from any old vertices
	pathLines.clear();
	// add all the vertices from pathVertices
	for (unsigned int i = 0; i < pathVertices.size(); i++) {
		pathLines.addVertex(pathVertices[i]);
	}	// here's where the transformation happens, using the orbit and roll member functions of the ofNode class, 
	// since angleH and distance are initialised to 0 and 500, we start up as how we want it

	//faces.baseline(); //use to debug, can do what ever needed to get things to work, to create a working base line
	//faces.update();
	//bodies.update();
	//audio.update();
	//audio.update();
#if sample	
	//--
	//Getting joint positions (skeleton tracking)
	//--
	//
	{
		auto bodies = kinect.getBodySource()->getBodies();
		for (auto body : bodies) {
			for (auto joint : body.joints) {
				//now do something with the joints
			}
		}
	}
	//
	//--



	//--
	//Getting bones (connected joints)
	//--
	//
	{
		// Note that for this we need a reference of which joints are connected to each other.
		// We call this the 'boneAtlas', and you can ask for a reference to this atlas whenever you like
		auto bodies = kinect.getBodySource()->getBodies();
		auto boneAtlas = ofxKinectForWindows2::Data::Body::getBonesAtlas();

		for (auto body : bodies) {
			for (auto bone : boneAtlas) {
				auto firstJointInBone = body.joints[bone.first];
				auto secondJointInBone = body.joints[bone.second];

				//now do something with the joints
			}
		}
	}
	//
	//--
#endif
}
void ofApp::audioIn(float * input, int bufferSize, int nChannels) {
	Software2552::SoundIn::audioIn(input, bufferSize, nChannels);
}
//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer &outBuffer) {
	Software2552::SoundOut::audioOut(outBuffer);
}

//--------------------------------------------------------------
void ofApp::draw(){
	timeline.draw();
	//Software2552::SoundOut::draw();//bugbug move to timeline
	return;
	//for (int i = 0; i< paragraphs.get().size(); i++) {
	//	paragraphs.get(i).draw();
	//}
	
	//string info = "";
	//info += "Mouse Drag for particles\n";
	//info += "Total Particles: " + ofToString(particles.getParticleCount()) + "\n\n";
	//info += "FPS: " + ofToString(ofGetFrameRate(), 1) + "\n";
	//ofSetHexColor(0xffffff);
	//ofDrawBitmapString(info, 30, 30);
#if 0
	if (filled) {
		ofFill();
	}
	else {
		ofNoFill();
	}

	ofSetColor(color);
	if (twoCircles) {
		ofDrawCircle(center->x - radius*.5, center->y, radius);
		ofDrawCircle(center->x + radius*.5, center->y, radius);
	}
	else {
		ofDrawCircle((ofVec2f)center, radius);
	}

	// auto draw?
	// should the gui control hiding?
	if (!bHide) {
		gui.draw();
	}
#endif // 0

	return;
#if 0
	//backgroundImage.draw(0, 0, ofGetWidth(), ofGetHeight());
//myPlayer.draw(0, 0, 300, 300);
	myPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
	robot.draw(300, 0, 300, 300);
	backgroundImage.draw(0, 0, 300, 300);
	return;
	// we need some images if not return
	if ((int)images.size() <= 0) {
		ofSetColor(255);
		ofDrawBitmapString("No Images...", 150, ofGetHeight() / 2);
		return;
	}

	// this is the total time of the animation based on fps
	//float totalTime = images.size() / sequenceFPS;

	uint64_t frameIndex = 0;

	if (bFrameIndependent) {
		// calculate the frame index based on the app time
		// and the desired sequence fps. then mod to wrap
		frameIndex = (int)(ofGetElapsedTimef() * sequenceFPS) % images.size();
	}
	else {
		// readJsonValue the frame index based on the app frame
		// count. then mod to wrap.
		frameIndex = ofGetFrameNum() % images.size();
	}


	// draw the image sequence at the new frame count
	ofSetColor(255);
	images[frameIndex].draw(900, 36);
	// draw where we are in the sequence
	float x = 0;
	for (int offset = 0; offset < 5; offset++) {
		int i = (frameIndex + offset) % images.size();
		ofSetColor(255);
		images[i].draw(700 + x, ofGetHeight() - 40, 40, 40);
		x += 40;
	}
	//faces.draw();
	bodies.draw();
	return;
	headTrackedCamera.begin();
	//drawScene();
	headTrackedCamera.end();

	//
	//------
	ofColor cyan = ofColor::fromHex(0x00abec);
	ofColor magenta = ofColor::fromHex(0xec008c);
	ofColor yellow = ofColor::fromHex(0xffee00);

	ofBackgroundGradient(magenta * .6, magenta * .4);
	ofNoFill();

	easyCam.begin();
	ofRotateX(15);

	ofSetColor(0);
	ofDrawGrid(500, 10, false, false, true, false);

	// draw the path of the box
	ofSetLineWidth(2);
	ofSetColor(cyan);
	pathLines.draw();

	// draw a line connecting the box to the grid
	ofSetColor(yellow);
	ofDrawLine(current.x, current.y, current.z, current.x, 0, current.z);

	// translate and rotate to the current position and orientation
	ofTranslate(current.x, current.y, current.z);
	if ((current - previous).length() > 0.0) {
		From2552Software::Graphics2552::rotateToNormal(current - previous);
	}
	ofSetColor(255);
	ofDrawBox(32);
	ofDrawAxis(32);
	ofFill();
	easyCam.end();
	bodies.draw();

	material.begin();

	ofNode node = camera.getTarget();

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	//ofEnableDepthTest();

#ifndef TARGET_PROGRAMMABLE_GL    
	glShadeModel(GL_SMOOTH); //some model / light stuff
#endif


#endif // 0

	/*
	kinect.getDepthSource()->draw(0, 0, previewWidth, previewHeight);  // note that the depth texture is RAW so may appear dark

																	   // Color is at 1920x1080 instead of 512x424 so we should fix aspect ratio

	kinect.getColorSource()->draw(previewWidth, 0 + colorTop, previewWidth, colorHeight);
	kinect.getBodySource()->drawProjected(previewWidth, 0 + colorTop, previewWidth, colorHeight);

	kinect.getInfraredSource()->draw(0, previewHeight, previewWidth, previewHeight);

	kinect.getBodyIndexSource()->draw(previewWidth, previewHeight, previewWidth, previewHeight);
	kinect.getBodySource()->drawProjected(previewWidth, previewHeight, previewWidth, previewHeight, ofxKFW2::ProjectionCoordinates::DepthCamera);
	*/
	//float colorHeight = previewWidth * (kinect.getColorSource()->getHeight() / kinect.getColorSource()->getWidth());
	//float colorTop = (previewHeight - colorHeight) / 2.0;
	//kinect.getBodySource()->drawProjected(previewWidth, 0 + colorTop, previewWidth, colorHeight, ofxKFW2::ProjectionCoordinates::DepthCamera);
	//faces.drawProjected(kinect.getBodySource()->getBodies(), previewWidth, 0 + colorTop, previewWidth, colorHeight, ofxKFW2::ProjectionCoordinates::DepthCamera);
	//faces.draw();
	/*
	bodies.draw();
	ofDisableDepthTest();
	camera.end();
	material.begin();
	light.disable();

	ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate(), 2), 10, 15);
	ofDrawBitmapString("keys 1-5 load models, spacebar to trigger animation", 10, 30);
	ofDrawBitmapString("drag to control animation with mouseY", 10, 45);
	ofDrawBitmapString("num animations for this model: " + ofToString(model.getAnimationCount()), 10, 60);
	*/
}

void ofApp::mouseDragged(int x, int y, int button) {
	for (int i = 0; i < 20; i++) {
		float radius = ofRandom(60, 80);
		float x = cos(ofRandom(PI * 2.0)) * radius + mouseX;
		float y = sin(ofRandom(PI * 2.0)) * radius + mouseY;
		ofVec2f position = ofVec2f(x, y);
		ofVec2f velocity = ofVec2f(0, 0);
		ofColor color;
		int hue = int(ofGetFrameNum() / 4.0) % 255;
		color.setHsb(hue, 180, 200);
	}
}

void ofApp::exit() {
	ringButton.removeListener(this, &ofApp::ringButtonPressed);
}

//--------------------------------------------------------------
void ofApp::circleResolutionChanged(int &circleResolution) {
	ofSetCircleResolution(circleResolution);
}

//--------------------------------------------------------------
void ofApp::ringButtonPressed() {
	ring.play();
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}


//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::keyPressed(int key) {
	if (key == 'c') {
		float r = ofRandom(4, 20);
		///circles.push_back(ofPtr<ofxBox2dCircle>(new ofxBox2dCircle));
		//circles.back().get()->setPhysics(0.9, 0.9, 0.1);
		//circles.back().get()->setup(box2d.getWorld(), mouseX, mouseY, r);
	}
	if (key == 'b') {
		float w = ofRandom(20, 40);
		float h = ofRandom(20, 40);
		//boxes.push_back(ofPtr<ofxBox2dRect>(new ofxBox2dRect));
		//boxes.back().get()->setPhysics(4.0, 0.53, 0.1);
		//boxes.back().get()->setup(box2d.getWorld(), mouseX, mouseY, w, h);
	}
}
