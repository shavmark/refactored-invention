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
}