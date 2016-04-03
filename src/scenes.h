#pragma once
#include "2552software.h"
#include "model.h"
#include "ofxBox2d.h"
#include "draw.h"
#include <forward_list>

// home of custom scenes

namespace Software2552 {

	// calls the shots, uses the Animation classes to do so bugbug code this in, add it to its own file
	// it replaces time line
	class Director {
	public:
		// return a possibly changed and live value from the cameras vector
		shared_ptr<Camera> pickem(forward_list<shared_ptr<Camera>>&cameras, bool rotating);
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
		virtual bool create(const Json::Value &data);
		void clear(bool force=false);
		void pause();
		void resume();
		float findMaxWait();
		void drawlights();
		Settings settings;
		string &getKeyName() { return keyname; }

		void addAnimatable(shared_ptr<Actor>p) { animatables.push_front(p); }
		// things to draw
		template<typename T> shared_ptr<T> CreateReadAndaddAnimatable(const Json::Value &data, const string&location);
		template<typename T> shared_ptr<T> CreateReadAndaddAnimatable(const Json::Value &data);
		virtual void CreateReadAndaddBackgroundItems(const Json::Value &data);
		shared_ptr<Camera> CreateReadAndaddCamera(const Json::Value &data, bool rotate = false);
		template<typename T>shared_ptr<T> CreateReadAndaddLight(const Json::Value &data);
		forward_list<shared_ptr<Actor>>& getAnimatables() { return animatables; }

		void fixed3d(bool b = true) { drawIn3dFixed = b; }
		void moving3d(bool b = true) { drawIn3dMoving = b; }
		void fixed2d(bool b = true) { drawIn2d = b; }

	protected:

		bool drawIn3dFixed = false; 
		bool drawIn3dMoving = false; 
		bool drawIn2d = true; 

		void add(shared_ptr<Camera> camera) { cameras.push_front(camera); };
		void add(shared_ptr<Light> light) { lights.push_front(light); };

		forward_list<shared_ptr<Camera>>& getCameras() { return cameras; }
		forward_list<shared_ptr<Light>>& getLights() { return lights; }

		void draw2d();
		void draw3dFixed();
		void draw3dMoving();

		virtual void myDraw2d() {};
		virtual void myDraw3dFixed() {};
		virtual void myDraw3dMoving() {};

		virtual void pre3dDraw();
		virtual void post3dDraw();
		virtual void mySetup() {}
		virtual void myUpdate() {}
		virtual void myPause() {}
		virtual void myResume() {}
		virtual void myClear(bool force) {}
		virtual bool myCreate(const Json::Value &data) { return true; }
		virtual void installLightAndMaterialThenDraw(shared_ptr<Camera>, bool drawfixed); // derive to change where cameras are
		string keyname;

	private:
		static bool OKToRemove(shared_ptr<Actor> me) {
			return me->getDefaultRole()->OKToRemoveNormalPointer(me->getDefaultRole());
		}
		void removeExpiredItems(forward_list<shared_ptr<Actor>>&v) {
			v.remove_if(OKToRemove);
		}
		void removeExpiredItems(forward_list<shared_ptr<Camera>>&v) {
			v.remove_if(objectLifeTimeManager::OKToRemove);
		}
		void removeExpiredItems(forward_list<shared_ptr<Light>>&v) {
			v.remove_if(objectLifeTimeManager::OKToRemove);
		}
		//bugbug maybe just animatables is needed, a a typeof or such can be used
		forward_list<shared_ptr<Actor>> animatables;
		forward_list<shared_ptr<Camera>> cameras;
		forward_list<shared_ptr<Light>> lights;
		shared_ptr<Colors> colors = nullptr;
		shared_ptr<Material> material = nullptr;//bugbug need to learn this but I expect it pairs with Light, just make a vector<pair<>>

		Director director;

		void draw3d();

	};

	// over time this just does whats in the data
	class GenericScene :public Stage {
	public:
		bool myCreate(const Json::Value &data);

	private:
	};

	class TestBallScene :public Stage {
	public:
		bool myCreate(const Json::Value &data);
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
	class SpaceScene : public Stage {
	public:
		SpaceScene() :Stage() {
			drawIn3dFixed = drawIn3dMoving = true;
		}
		bool myCreate(const Json::Value &data);
	private:
	};


}