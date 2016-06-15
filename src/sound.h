#pragma once
#include "ofApp.h"
namespace Software2552 {

	class SoundOut {
	public:
		void setup();
		void update();
		void draw(float *, int);
		virtual void audioOut(ofSoundBuffer &outBuffer);

		class data {
		public:
			double wavePhase;
			double pulsePhase;
			double sampleRate;
			mutex audioMutex;
			ofSoundBuffer lastBuffer;
			ofPolyline waveform;
			float rms;
		};
		data soundDataOut;
		void setDraw(bool b = true) { drawMe = b; }
		void setStop(bool b = true) { stop = b; }
	private:
		bool drawMe = true;
		bool stop = false;
	};
}
