#include "ofApp.h"
#include "color.h"
#ifdef _WIN64
#include "inc\Kinect.h" // needed for enums
#endif
#include "model.h"
#include "scenes.h"
#include "sound.h"

namespace Software2552 {

	void SoundOut::setup() {
		 
		soundDataOut.sampleRate = 44100;
		soundDataOut.wavePhase = 0;
		soundDataOut.pulsePhase = 0;
		// start the sound stream with a sample rate of 44100 Hz, and a buffer
		// size of 512 samples per audioOut() call
		ofSoundStreamSetup(2, 0, soundDataOut.sampleRate, 512, 3);
	//http://openframeworks.cc/documentation/sound/ofSoundStream/
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
	void SoundOut::audioOut(ofSoundBuffer &outBuffer) {
		if (stop) {
			return;
		}
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
	void SoundOut::draw(float *f, int size) {

		if (drawMe) {
			ofBackground(ofColor::black);
			ofSetColor(ofColor::white);
			ofSetLineWidth(1 + (soundDataOut.rms * 30.0));
			soundDataOut.waveform.draw();
			//bugbug change data by using parameters
		}
	}

	void addMusic(const string& path) {
		shared_ptr<ofSoundPlayer> s1 = std::make_shared<ofSoundPlayer>();
		if (s1) { // get from json bugbug
			s1->loadSound(path);//set a sound by default but json can add them
			s1->setMultiPlay(true);
			s1->setLoop(true);
			s1->play();
			((ofApp*)ofGetAppPtr())->appconfig.sounds.push_back(s1);
		}
	}

	// helper
	vector <shared_ptr<ofSoundPlayer>> &getSounds() {
		return ((ofApp*)ofGetAppPtr())->appconfig.sounds;

	}

	// needs work, right now its hard coded too much bugbug
	void drawMusic() {
		float allBands = 0;
		for (int i = 1; i < 32; i++) {
			allBands += ((ofApp*)ofGetAppPtr())->appconfig.beat.getBand(i);
		}
		ofBackgroundGradient(ofColor::white, ofColor::gray);
		if (allBands > 5.0f) { // filter off low noise
			ofPushStyle();
			ofPushMatrix();
			ofTranslate(0, ofGetHeight() / 2);// default position, cameras may change location
			ofFill();
			int x = 0;
			float kick = ((ofApp*)ofGetAppPtr())->appconfig.beat.kick();
			float snare = ((ofApp*)ofGetAppPtr())->appconfig.beat.snare();
			float hihat = ((ofApp*)ofGetAppPtr())->appconfig.beat.hihat();
			ofColor color(ofMap(snare, 0, 1, 50, 255), ofMap(hihat, 0, 1, 50, 255), ofMap(kick, 0, 1, 50, 255));
			ofSetColor(color);
			for (int i = 0; i<32; i++) {      //Draw bandRad and bandVel by black color,      //and other by gray color 
				float selectedBand = ((ofApp*)ofGetAppPtr())->appconfig.beat.getBand(i);
				ofRect(x, ofGetHeight() / 2, ofGetWidth() / 32, -selectedBand * 100);
				x += ofGetWidth() / 32;
				ofPushMatrix();
				ofTranslate(ofGetWidth() / 2, 0);
				float r = ofMap(selectedBand, 0, 1, 20, ofGetHeight() / 2);
				if (r > 0) {
					ofDrawCircle(0, 0, r);
				}
				ofPopMatrix();
			}
			ofPopMatrix();
			ofPopStyle();
		}

	}
}