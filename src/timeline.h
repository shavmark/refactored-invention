#pragma once
#include "2552software.h"
#include "scenes.h"
#include "model.h"
#include "networking.h"
// join data and view, this class knows about both the data and how its shown

namespace Software2552 {
	class Timeline {
	public:
		Timeline();
		void start();
		void setup();
		void update();
		void draw();
		void pause();
		void resume();
		const int frameRate = 60;

		bool readScript(const string& path);

	private:
		ChannelList playlist;
		ColorList colorlist;
		Stage stage;
		ReadComms read;
		WriteComms write;
		MoreMesh mesh; // just for now bugbug, move into class hiearchey when ready
	};


}