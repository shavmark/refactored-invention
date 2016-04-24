#pragma once
#include "ofApp.h"
#include "color.h"
#include "model.h"

// home of custom scenes

namespace Software2552 {
	extern shared_ptr<Stage> getStage();
	// calls the shots, uses the Animation classes to do so bugbug code this in, add it to its own file
	// it replaces time line
	class Director {
	public:
		// return a possibly changed and live value from the cameras vector
		shared_ptr<FixedCamera> pickem(vector<shared_ptr<FixedCamera>>&cameras, bool rotating);
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
		bool updateData(shared_ptr<ofxJSON>);
		void readGraphics(const Json::Value &data, shared_ptr<ActorRole> parent);
		void readLights(const Json::Value &data);
		void readCameras(const Json::Value &data);
		void clear(bool force=false);
		void pause();
		void resume();
		string &getKeyName() { return keyname; }

		template<typename T> void CreateReadAndaddAnimatable(const Json::Value &data, shared_ptr<ActorRole> parent, bool inFront = false, bool fullsize=false) {
			for (Json::ArrayIndex j = 0; j < data.size(); ++j) {
				shared_ptr<T> item = std::make_shared<T>();
				if (item) {
					Json::Value::Members m = data[j].getMemberNames();//here for debug
					if (item->setup(data[j])) {
						if (fullsize) {
							item->setFullSize();
						}
						//bugbug in drawing code parent position/color/font etc used unless child has its own
						// so child moves w/ parent in 2d like node does in 3d
						if (item->node && parent && parent->node) {
							item->parent = parent;
							item->node->setParent(*parent->node);//bugbug not sure what to do here but we need to tie things together somehow
						}
						addToAnimatable(item, inFront);
					}
				}
			}
		}
		void addToAnimatable(shared_ptr<ActorRole>p, bool inFront = false);

	protected:
		void fixed3d(bool b = true) { drawIn3dFixed = b; }
		void moving3d(bool b = true) { drawIn3dMoving = b; }
		void fixed2d(bool b = true) { drawIn2d = b; }
		float findMaxWait();
		void drawlights();
		virtual shared_ptr<Background> CreateReadAndaddBackgroundItems(const Json::Value &data);
		template<typename T> void CreateReadAndaddCamera(const Json::Value &data);
		template<typename T> void CreateReadAndaddLight(const Json::Value &data);
		list<shared_ptr<ActorRole>>& getAnimatables() { return animatables; }

		bool drawIn3dFixed = false; 
		bool drawIn3dMoving = false; 
		bool drawIn2d = true; 

		void add(shared_ptr<FixedCamera> camera) { cameras.push_back(camera); };
		void add(shared_ptr<Light> light) { lights.push_back(light); };

		vector<shared_ptr<FixedCamera>>& getCameras() { return cameras; }
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
		virtual void installLightAndMaterialThenDraw(shared_ptr<FixedCamera>, bool drawfixed); // derive to change where cameras are
		string keyname;

	private:
		bool deleteExisting(const Json::Value &data);
		void removeExpiredItems(list<shared_ptr<ActorRole>>&v) {
			v.remove_if(ActorRole::OKToRemove);
		}
		template<typename T> void removeExpiredItems(vector<shared_ptr<T>>&v) {
			v.erase(std::remove_if(v.begin(), v.end(), objectLifeTimeManager::OKToRemove), v.end());
		}

		list<shared_ptr<ActorRole>> animatables; // use list as it could be large with lots of adds/deletes over time
		vector<shared_ptr<FixedCamera>> cameras;  // expect list to be smaller and more fixed, also want index acess "camera 2"
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