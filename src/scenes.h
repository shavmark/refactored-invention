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
	class Stage : public Settings {
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

		template<typename T> shared_ptr<T> CreateReadAndaddAnimatable(const Json::Value &data, Stage*stage=nullptr) {
			shared_ptr<T> p = std::make_shared<T>();
			if (p != nullptr) { // try to run in low memory as much as possible for small devices
				if (stage) {
					p->setSettings(*stage);
					if (p->readActorFromScript(data, stage)) {
						appendToAnimatable(p);
					}
				}
				else {
					p->setSettings(*this);
					if (p->readActorFromScript(data, this)) {
						appendToAnimatable(p);
					}
				}
			}
			return p;
		}
		virtual shared_ptr<Background> CreateReadAndaddBackgroundItems(const Json::Value &data);
		shared_ptr<Camera> CreateReadAndaddCamera(const Json::Value &data, bool rotate = false);
		template<typename T>shared_ptr<T> CreateReadAndaddLight(const Json::Value &data);
		list<shared_ptr<Actor>>& getAnimatables() { return animatables; }

		void fixed3d(bool b = true) { drawIn3dFixed = b; }
		void moving3d(bool b = true) { drawIn3dMoving = b; }
		void fixed2d(bool b = true) { drawIn2d = b; }
		bool readFromScript(const Json::Value &data);
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
		virtual void installLightAndMaterialThenDraw(shared_ptr<Camera>, bool drawfixed); // derive to change where cameras are
		string keyname;

	private:

		void appendToAnimatable(shared_ptr<Actor>p) { animatables.push_back(p); }

		static bool OKToRemove(shared_ptr<Actor> me) {
			return me->getDefaultRole()->OKToRemoveNormalPointer(me->getDefaultRole());
		}
		void removeExpiredItems(list<shared_ptr<Actor>>&v) {
			v.remove_if(OKToRemove);
		}
		void removeExpiredItems(forward_list<shared_ptr<Camera>>&v) {
			v.remove_if(objectLifeTimeManager::OKToRemove);
		}
		void removeExpiredItems(forward_list<shared_ptr<Light>>&v) {
			v.remove_if(objectLifeTimeManager::OKToRemove);
		}
		//bugbug maybe just animatables is needed, a a typeof or such can be used
		list<shared_ptr<Actor>> animatables;
		forward_list<shared_ptr<Camera>> cameras;
		forward_list<shared_ptr<Light>> lights;
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