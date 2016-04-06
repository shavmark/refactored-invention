#include "2552software.h"
#include "animation.h"
#include "draw.h"
#include "model.h"

// this is the drawing  module where most drawing work is done

namespace Software2552 {
	SoundIn::dataIn SoundIn::soundDataIn; // only one sound per app instance
	SoundOut::dataOut SoundOut::soundDataOut;// only one sound per app instance

	void SoundOut::setup() {
		
		soundDataOut.sampleRate = 44100;
		soundDataOut.wavePhase = 0;
		soundDataOut.pulsePhase = 0;
		// start the sound stream with a sample rate of 44100 Hz, and a buffer
		// size of 512 samples per audioOut() call
		ofSoundStreamSetup(2, 0, soundDataOut.sampleRate, 512, 3);

	}
	void SoundIn::setup() {
		// 0 output channels, 
		// 2 input channels
		// 44100 samples per second
		// 256 samples per buffer
		// 4 num buffers (latency)

		soundDataIn.soundStream.printDeviceList();

		//if you want to set a different device id 
		//soundStream.setDeviceID(0); //bear in mind the device id corresponds to all audio devices, including  input-only and output-only devices.

		int bufferSize = 256;


		soundDataIn.left.assign(bufferSize, 0.0);
		soundDataIn.right.assign(bufferSize, 0.0);
		soundDataIn.volHistory.assign(400, 0.0);

		soundDataIn.bufferCounter = 0;
		soundDataIn.drawCounter = 0;
		soundDataIn.smoothedVol = 0.0;
		soundDataIn.scaledVol = 0.0;

		soundDataIn.soundStream.setup(ofGetAppPtr(), 0, 2, 44100, bufferSize, 4);

	}

	void SoundOut::update() {

		// sound out

		// "lastBuffer" is shared between update() and audioOut(), which are called
		// on two different threads. This lock makes sure we don't use lastBuffer
		// from both threads simultaneously (see the corresponding lock in audioOut())
		unique_lock<mutex> lock(soundDataOut.audioMutex);

		// this loop is building up a polyline representing the audio contained in
		// the left channel of the buffer

		// the x coordinates are evenly spaced on a grid from 0 to the window's width
		// the y coordinates map each audio sample's range (-1 to 1) to the height
		// of the window

		soundDataOut.waveform.clear();
		for (size_t i = 0; i < soundDataOut.lastBuffer.getNumFrames(); i++) {
			float sample = soundDataOut.lastBuffer.getSample(i, 0);
			float x = ofMap(i, 0, soundDataOut.lastBuffer.getNumFrames(), 0, ofGetWidth());
			float y = ofMap(sample, -1, 1, 0, ofGetHeight());
			soundDataOut.waveform.addVertex(x, y);
		}

		soundDataOut.rms = soundDataOut.lastBuffer.getRMSAmplitude();

		
	}
	void SoundIn::audioIn(float * input, int bufferSize, int nChannels) {

		float curVol = 0.0;

		// samples are "interleaved"
		int numCounted = 0;

		//lets go through each sample and calculate the root mean square which is a rough way to calculate volume	
		for (int i = 0; i < bufferSize; i++) {
			soundDataIn.left[i] = input[i * 2] * 0.5;
			soundDataIn.right[i] = input[i * 2 + 1] * 0.5;

			curVol += soundDataIn.left[i] * soundDataIn.left[i];
			curVol += soundDataIn.right[i] * soundDataIn.right[i];
			numCounted += 2;
		}

		//this is how we get the mean of rms :) 
		curVol /= (float)numCounted;

		// this is how we get the root of rms :) 
		curVol = sqrt(curVol);

		soundDataIn.smoothedVol *= 0.93;
		soundDataIn.smoothedVol += 0.07 * curVol;

		soundDataIn.bufferCounter++;

	}

	void SoundOut::audioOut(ofSoundBuffer &outBuffer) {

		// base frequency of the lowest sine wave in cycles per second (hertz)
		float frequency = 172.5;

		// mapping frequencies from Hz into full oscillations of sin() (two pi)
		float wavePhaseStep = (frequency / soundDataOut.sampleRate) * TWO_PI;
		float pulsePhaseStep = (0.5 / soundDataOut.sampleRate) * TWO_PI;

		// this loop builds a buffer of audio containing 3 sine waves at different
		// frequencies, and pulses the volume of each sine wave individually. In
		// other words, 3 oscillators and 3 LFOs.

		for (size_t i = 0; i < outBuffer.getNumFrames(); i++) {

			// build up a chord out of sine waves at 3 different frequencies
			float sampleLow = sin(soundDataOut.wavePhase);
			float sampleMid = sin(soundDataOut.wavePhase * 1.5);
			float sampleHi = sin(soundDataOut.wavePhase * 2.0);

			// pulse each sample's volume
			sampleLow *= sin(soundDataOut.pulsePhase);
			sampleMid *= sin(soundDataOut.pulsePhase * 1.04);
			sampleHi *= sin(soundDataOut.pulsePhase * 1.09);

			float fullSample = (sampleLow + sampleMid + sampleHi);

			// reduce the full sample's volume so it doesn't exceed 1
			fullSample *= 0.3;

			// write the computed sample to the left and right channels
			outBuffer.getSample(i, 0) = fullSample;
			outBuffer.getSample(i, 1) = fullSample;

			// get the two phase variables ready for the next sample
			soundDataOut.wavePhase += wavePhaseStep;
			soundDataOut.pulsePhase += pulsePhaseStep;
		}

		unique_lock<mutex> lock(soundDataOut.audioMutex);
		soundDataOut.lastBuffer = outBuffer;
	}

	//--------------------------------------------------------------
	void SoundOut::draw() {
		ofBackground(ofColor::black);
		ofSetColor(ofColor::white);
		ofSetLineWidth(1 + (soundDataOut.rms * 30.));
		soundDataOut.waveform.draw();
	}
	void SoundIn::update() {
		//lets scale the vol up to a 0-1 range 
		soundDataIn.scaledVol = ofMap(soundDataIn.smoothedVol, 0.0, 0.17, 0.0, 1.0, true);

		//lets record the volume into an array
		soundDataIn.volHistory.push_back(soundDataIn.scaledVol);

		//if we are bigger the the size we want to record - lets drop the oldest value
		if (soundDataIn.volHistory.size() >= 400) {
			soundDataIn.volHistory.erase(soundDataIn.volHistory.begin(), soundDataIn.volHistory.begin() + 1);
		}
	}
	void RandomDots::draw() {
		for (int i = 0; i < ofGetMouseX() * 5; i++) {
			ofSetColor(ofRandom(96));
			ofRect(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), 4, 4);
		}
	}
	void ComplexLines::draw() {
		for (const auto& line : lines) {
			ofDrawLine(line.a, line.b);
		}
	}
	//bugbug convert to code vs mouse using random
	void ComplexLines::mouseDragged(int x, int y, int button) {
		for (auto point : drawnPoints) {
			ofPoint mouse;
			mouse.set(x, y);
			float dist = (mouse - point).length();
			if (dist < 30) {
				Line lineTemp;
				lineTemp.a = mouse;
				lineTemp.b = point;
				lines.push_back(lineTemp);
			}
		}
		drawnPoints.push_back(ofPoint(x, y));
	}
	void VectorPattern::matrix(int twistx, int shifty) {
		ofPushMatrix();
		for (int i = -50; i < 50; ++i) {
			ofTranslate(0, i*shifty);
			ofRotate(i*twistx);
			stripe(true);
		}
		ofPopMatrix();
	}
	void VectorPattern::stripe(bool rotate) {
		ofSetColor(ofColor::black);
		ofSetLineWidth(3);
		for (int i = -50; i < 50; ++i) {
			ofPushMatrix();
			ofTranslate(i * 20, 0);
			if (rotate) {
				ofRotate(i * 5);
			}
			ofLine(0, -100, 0, 100);
			ofPopMatrix();
		}
	}
	void VectorPattern::triangle(bool rotate) {
		ofSetColor(ofColor::black);
		ofSetLineWidth(3);
		ofNoFill();
		for (int i = -50; i < 50; ++i) {
			ofPushMatrix();
			ofTranslate(i * 20, 0);
			if (rotate) {
				ofRotate(i * 5);
			}
			ofScale(6, 6); // enlarge 6x
			ofTriangle(0, 0, -50, 100, 50, 100);
			ofPopMatrix();
		}
	}
	void VectorPattern::shape(int twistx, int shifty, bool rect, bool fill, int rotate, int alpha) {
		ofColor color = ofColor::black;
		color.a = alpha;
		ofSetColor(color);
		ofSetLineWidth(1);
		if (fill) {
			ofFill();
		}
		else {
			ofNoFill();
		}
		for (int i = -50; i < 50; ++i) {
			ofPushMatrix();
			ofRotate(i * twistx);
			ofTranslate(i * 20, shifty);
			ofRotate(rotate);
			ofScale(6, 6); // enlarge 6x
			if (rect) {
				ofRect(-50, -50, 100, 100);
			}
			else {
				ofTriangle(0, 0, -50, 100, 50, 100);
			}
			ofPopMatrix();
		}
		ofScale(6, 6); // enlarge 6x
		ofTriangle(0, 0, -50, 100, 50, 100);
	}



	void Line3D::setup() {
		//required ofEnableDepthTest();
		baseNode.setPosition(0, 0, 0);
		childNode.setParent(baseNode);
		childNode.setPosition(0, 0, 200);
		grandChildNode.setParent(childNode);
		grandChildNode.setPosition(0, 50, 0);
	}
	void Line3D::update() {
		baseNode.pan(1);
		childNode.tilt(3);

		line.addVertex(grandChildNode.getGlobalPosition());
		if (line.size() > 200) {
			line.getVertices().erase(
				line.getVertices().begin()
				);
		}
	}
	void Line3D::draw() {
		//uncomment these 3 lines to understand how nodes are moving
		//baseNode.draw();
		//childNode.draw();
		//grandChildNode.draw();
		line.draw();
	}

	void Rectangle::draw() {
		ofFill();
		ofSetRectMode(OF_RECTMODE_CENTER);	// center around the position
		ofSetColor(255, 0, 0);
		ofRect(getLocationAnimationHelper()->getCurrentPosition().x, getLocationAnimationHelper()->getCurrentPosition().y, w, h);
	}


	void CrazyMesh::setup() {
		for (int i = 0;  i < w; ++i) {
			for (int j = 0; j < h; ++j) {
				addColor(color);
			}
		}
	}
	void CrazyMesh::update() {
		clearVertices();
		for (int i = 0; i < w; ++i) {
			for (int j = 0; j < h; ++j) {
				float x = sin(i*0.1+ofGetElapsedTimef())*1000;
				float y = sin(j*0.15 + ofGetElapsedTimef()) * 1000;
				float z = x + y;
				addVertex(ofVec3f(x,y,z));
			}
		}
	}
	void CrazyMesh::draw() {
		ofPushStyle();
		ofSetHexColor(0xffffff);
		glPointSize(2);
		glEnable(GL_POINT_SMOOTH);
		drawVertices();
		ofPopStyle();
	}

	void MeshEngine::setup() {
		//setMode(OF_PRIMITIVE_POINTS);
		setMode(OF_PRIMITIVE_LINES);
		//setMode(OF_PRIMITIVE_LINE_STRIP);
		//setMode(OF_PRIMITIVE_LINE_LOOP);
		enableColors();
		enableIndices();
		ofVec3f top(100.0, 50.0, 0.0);
		ofVec3f left(50.0, 150.0, 0.0);
		ofVec3f right(150.0, 150.0, 0.0);
		addVertex(top);
//		addColor(Colors::getFloatColor(Colors::getDarkest()));

		addVertex(left);
//		addColor(Colors::getFloatColor(Colors::getLightest()));

		addVertex(right);
		//bugbug this is where we add in more colors, lets see how many before we make 
		// changes, but somthing like Color1, Color2 etc
	//	addColor(Colors::getFloatColor(Colors::getDarkest()));

		addIndex(0);
		addIndex(1);
		addIndex(1);
		addIndex(2);
		addIndex(2);
		addIndex(0);

	}
	void MoreMesh::setup() {
		image.load("stars.png");
		image.resize(200, 200); // use less data
		setMode(OF_PRIMITIVE_LINES);
		enableColors();

		float intensityThreshold = 150.0;
		int w = image.getWidth();
		int h = image.getHeight();
		for (int x = 0; x<w; ++x) {
			for (int y = 0; y<h; ++y) {
				ofColor c = image.getColor(x, y);
				float intensity = c.getLightness();
				if (intensity >= intensityThreshold) {
					float saturation = c.getSaturation();
					float z = ofMap(saturation, 0, 255, -100, 100);
					// We shrunk our image by a factor of 4, so we need to multiply our pixel
					// locations by 4 in order to have our mesh cover the openFrameworks window
					ofVec3f pos(x * 4, y * 4,z);
					addVertex(pos);
					// When addColor(...), the mesh will automatically convert
					// the ofColor to an ofFloatColor
					addColor(c);
				}
			}
		}
		// Let's add some lines!
		float connectionDistance = 30;
		int numVerts = getNumVertices();
		for (int a = 0; a < numVerts; ++a) {
			ofVec3f verta = getVertex(a);
			for (int b = a + 1; b < numVerts; ++b) {
				ofVec3f vertb = getVertex(b);
				float distance = verta.distance(vertb);
				if (distance <= connectionDistance) {
					// In OF_PRIMITIVE_LINES, every pair of vertices or indices will be
					// connected to form a line
					addIndex(a);
					addIndex(b);
				}
			}
		}
	}
	void MoreMesh::draw(){
		ofColor centerColor = ofColor(85, 78, 68);
		ofColor edgeColor(0, 0, 0);
		ofBackgroundGradient(centerColor, edgeColor, OF_GRADIENT_CIRCULAR);
		easyCam.begin();
		ofPushMatrix();
		ofTranslate(-ofGetWidth() / 2, -ofGetHeight() / 2);
		ofMesh::draw();
		ofPopMatrix();
		easyCam.end();
	}

	// its ok also if Controller passes in an object such as a paragraph to copy in
	// bugbug move to a text object
	void Graphics2552::rotateToNormal(ofVec3f normal) {
		normal.normalize();

		float rotationAmount;
		ofVec3f rotationAngle;
		ofQuaternion rotation;

		ofVec3f axis(0, 0, 1);
		rotation.makeRotate(axis, normal);
		rotation.getRotate(rotationAmount, rotationAngle);
		logVerbose("ofRotate " + ofToString(rotationAmount));
		ofRotate(rotationAmount, rotationAngle.x, rotationAngle.y, rotationAngle.z);
	}

}
