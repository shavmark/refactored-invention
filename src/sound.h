#pragma once
#include "ofApp.h"
namespace Software2552 {
	
	void addMusic(const string& path= "keep.mp3");
	void graphsound();

	vector <shared_ptr<ofSoundPlayer>> &getSounds();

	class Spiral : public ActorRole {
	public:
		Spiral():ActorRole(){ setFill(false); }
		void myDraw();
		bool mysetup(const Json::Value &data);
		float numurator=250;
		float denominator=810;
		float step=80;
	};

	class VisibleSound : public ActorRole {
	public:
		bool mysetup(const Json::Value &data);
		void myDraw();
		void drawGraph();
	protected:
	private:
		bool showGraph = false;
		virtual void drawMusic() {}
	};

	class VisibleMusic : public VisibleSound {
	public:
		void drawMusic();
	};
	class GraphMusic : public VisibleSound {
	public:
		void drawMusic() { drawGraph(); };
	};

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
