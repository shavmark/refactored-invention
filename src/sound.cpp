#include "ofApp.h"
#include "color.h"
#include "inc\Kinect.h" // needed for enums
#include "model.h"
#include "scenes.h"
#include "sound.h"

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
	//http://openframeworks.cc/documentation/sound/ofSoundStream/
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

}