#pragma once

#include "model.h"
#include "draw.h"
#include <forward_list>

// home of custom scenes

namespace Software2552 {
	extern shared_ptr<Stage> getStage();
	// calls the shots, uses the Animation classes to do so bugbug code this in, add it to its own file
	// it replaces time line
	class Director {
	public:
		// return a possibly changed and live value from the cameras vector
		shared_ptr<Camera> pickem(vector<shared_ptr<Camera>>&cameras, bool rotating);
		// owns scenes, read, run, delete when duration is over
		//objectLifeTimeManager
	};

	// convert name to object
	shared_ptr<Stage> getScene(const string&name);

	// contains  elements of a stage
	class Stage {
	public:
		void setup();
		void update();
		void draw();
		void clear(bool force=false);
		void pause();
		void resume();
		float findMaxWait();
		void drawlights();
		string &getKeyName() { return keyname; }
		// things to draw

		template<typename T> shared_ptr<T> CreateReadAndaddAnimatable(const Json::Value &data, bool inFront=false) {
			shared_ptr<T> p = std::make_shared<T>();
			if (p != nullptr) { // try to run in low memory as much as possible for small devices
				if (p->setup(data)) {
					addToAnimatable(p, inFront);
				}
			}
			return p;
		}
		virtual shared_ptr<Background> CreateReadAndaddBackgroundItems(const Json::Value &data);
		shared_ptr<Camera> CreateReadAndaddCamera(const Json::Value &data, bool rotate = false);
		template<typename T>shared_ptr<T> CreateReadAndaddLight(const Json::Value &data);
		list<shared_ptr<ActorRole>>& getAnimatables() { return animatables; }

		void fixed3d(bool b = true) { drawIn3dFixed = b; }
		void moving3d(bool b = true) { drawIn3dMoving = b; }
		void fixed2d(bool b = true) { drawIn2d = b; }
		bool setup(const Json::Value &data);
	protected:

		bool drawIn3dFixed = false; 
		bool drawIn3dMoving = false; 
		bool drawIn2d = true; 

		void add(shared_ptr<Camera> camera) { cameras.push_back(camera); };
		void add(shared_ptr<Light> light) { lights.push_back(light); };

		vector<shared_ptr<Camera>>& getCameras() { return cameras; }
		vector<shared_ptr<Light>>& getLights() { return lights; }

		void draw2d();
		void draw3dFixed();
		void draw3dMoving();

		virtual void myDraw2d() {};
		virtual void myDraw3dFixed() {};
		virtual void myDraw3dMoving() {};

		virtual void preDraw();
		virtual void postDraw();
		virtual void mySetup() {}
		virtual void myUpdate() {}
		virtual void myPause() {}
		virtual void myResume() {}
		virtual void myClear(bool force) {}
		virtual void installLightAndMaterialThenDraw(shared_ptr<Camera>, bool drawfixed); // derive to change where cameras are
		string keyname;

	private:

		void addToAnimatable(shared_ptr<ActorRole>p, bool inFront=false);

		void removeExpiredItems(list<shared_ptr<ActorRole>>&v) {
			v.remove_if(ActorRole::OKToRemove);
		}
		template<typename T> void removeExpiredItems(vector<shared_ptr<T>>&v) {
			v.erase(std::remove_if(v.begin(), v.end(), objectLifeTimeManager::OKToRemove), v.end());
		}

		list<shared_ptr<ActorRole>> animatables; // use list as it could be large with lots of adds/deletes over time
		vector<shared_ptr<Camera>> cameras;  // expect list to be smaller and more fixed, also want index acess "camera 2"
		vector<shared_ptr<Light>> lights;    // expect list to be smaller and more fixed

		Director director;

		void draw3d();

	};

	// over time this just does whats in the data
	class GenericScene :public Stage {
	public:
	private:
	};

	class TestScene :public Stage {
	public:
		TestScene() :Stage() {
			drawIn2d = drawIn3dFixed = drawIn3dMoving = true;
		}
		void mySetup();
		void myUpdate();
		bool myCreate(const Json::Value &data);
	private:
		void myDraw3dFixed();

		CrazyMesh mesh;
	};


}