#pragma once
#include "ofApp.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxSmartFont.h"
#include "ofxParagraph.h"


// supports animation

namespace Software2552 {
	// read in list of Json values
	template<typename T> shared_ptr<T> parseNoList(const Json::Value &data)
	{
		shared_ptr<T>  results = nullptr;
		if (data.size() > 0) {
			results = std::make_shared<T>();
			if (results) {
				results->setup(data);
			}
		}
		return results;
	}
	template<typename T> shared_ptr<vector<shared_ptr<T>>> parseList(const Json::Value &data)
	{
		shared_ptr<vector<shared_ptr<T>>>  results = nullptr;
		if (data.size() > 0) {
			results = std::make_shared<vector<shared_ptr<T>>>();
			if (results) {
				for (Json::ArrayIndex j = 0; j < data.size(); ++j) {
					shared_ptr<T> item = std::make_shared<T>();
					if (item) {
						item->setup(data[j]);
						results->push_back(item);
					}
				}
			}
		}
		return results;
	}

	class objectLifeTimeManager {
	public:
		objectLifeTimeManager();
		void start();
		void setRefreshRate(uint64_t rateIn) { refreshRate = rateIn; }
		float getRefreshRate() {return refreshRate;	}
		void setWait(float w) { waitTime = w; }
		float getWait() { return waitTime; }
		float getStart() { return startTime; }
		bool isExpired() const { return expired; }
		void setExpired(bool b = true) { expired = b; }
		float getObjectLifetime() { return objectlifetime; }
		void setObjectLifetime(float t) { objectlifetime=t; }
		void operator++ () { ++usageCount; }
		bool operator> (const objectLifeTimeManager& rhs) { return usageCount > rhs.usageCount; }
		int getUsage() const { return usageCount; }
		bool refreshAnimation();
		bool setup(const Json::Value &data);
		// how long to wait
		virtual void getTimeBeforeStart(float& t) {	setIfGreater(t, getObjectLifetime() + getWait());		}
		static bool OKToRemove(shared_ptr<objectLifeTimeManager> me);
		void removeExpiredItems(vector<shared_ptr<objectLifeTimeManager>>&v) {
			v.erase(std::remove_if(v.begin(), v.end(), OKToRemove), v.end());
		}
	private:
		int	    usageCount;     // number of times this animation was used
		float   objectlifetime; // bugbug drop this in favor of frameMax 0=forever, how long object lives after it starts drawing 
		bool	expired;    // object is expired
		float	startTime;
		float   waitTime;
		float	refreshRate;
	};

	class FloatAnimation : public ofxAnimatableFloat, public objectLifeTimeManager {
	public:
		FloatAnimation(float toIn = 0, float fromIn = 100);
		void update();
		virtual bool setup(const Json::Value &data);
		bool enabled = false;
	private:
		float to;
		float from;
	};
	class RotationAnimation : public FloatAnimation {
	public:
		RotationAnimation() : FloatAnimation(15.0f, 360.0f) {}
	private:
	};
	class Rotation  {
	public:
		bool setup(const Json::Value &data);
		void update();
		RotationAnimation x;
		RotationAnimation y;
		RotationAnimation z;
	private:
	};

	class ScaleAnimation : public FloatAnimation {
	public:
		ScaleAnimation() : FloatAnimation(1.0f, 5.0f) {}
	private:
	};

	class PointAnimation : public ofxAnimatableOfPoint, public objectLifeTimeManager {
	public:
		void update();
		bool setup(const Json::Value &data);
		float funXMover() { return (2 * ofGetFrameNum()) % ofGetWidth(); }
	private:
	};
	// global color data
	class ColorSet;
	class AnimiatedColor : public ofxAnimatableOfColor {
	public:
		friend class ColorList;

		AnimiatedColor() :ofxAnimatableOfColor() { }
		void draw();
		void update();
		bool setup(const Json::Value &data);
		void setAlpha(float val);
		void getNextColors();
		bool AlphaEnabled() { return (from != 255 && to != from); }

	private:
		float from = 255;// alpha
		float to = 255;

	};

	// simple helper to read in font data from json 
#define defaultFontSize 14
#define defaultFontFile "fonts/Raleway-Thin.ttf"
#define defaultFontName "Raleway-Thin"

	//bugbug do we want to expire fonts? maybe in 2.0
	class FontHelper {
	public:
		ofTrueTypeFont* get();//bugbug fix this so its not a pointer and it always return some value
		shared_ptr<ofxSmartFont> getPointer();
		bool setup(const Json::Value &data);
	private:
		shared_ptr<ofxSmartFont> font;
	};

	class FrameCounter {
	public:
		// -1 means infinite
		FrameCounter(int frames=-1, bool random=false) {
			int time;
			if (random) {
				time = frames + ofRandom(frames / 2); // mix it up a little
			}
			else {
				time = frames;
			}
			setFrameCount(time);
			frameStart = time;
		}
		void setFrameCount(int c) {
			frameMax = c;
		}
		int decrementFrameCount() {
			if (frameMax > 0) {
				--frameMax; // never go -1 when counting
			}
			return frameMax;
		}
		bool getFrameCountMaxHit() {
			return frameMax == 0;
		}
		void reset() {
			frameMax = frameStart;
		}
		// use random duration
		void rand() {
			if (frameStart != -1) {
				// cannot randomize non counting object
				setFrameCount(ofRandom(frameStart));
			}
		}
	private:
		int frameMax; // number of frames to show this item
		int frameStart; // enable easy reset

	};
	// basic drawing info, bugbug maybe color set goes here too, not sure yet
	class Stage;
	class Reference;
	// things are drawn using a sparse matrix, with rows having at least one column, this object handles rows that have more than one column
	template<class T> class RowRoles {
	public:
		// return true if shader is loaded
		bool setupRow(const Json::Value & data) {
			for (Json::ArrayIndex j = 0; j < data["draw"].size(); ++j) {
				shared_ptr<pair<FrameCounter, shared_ptr<T>>> drawer = make_shared<pair<FrameCounter, shared_ptr<T>>>();
				if (!drawer) {
					return false; // try to run in low memory as best as possible
				}
				drawer->second = make_shared<T>();
				if (drawer->second && mySetup(data["draw"][j], drawer->second)) {
					if (data["draw"][j]["seconds"].isInt()) {
						drawer->first = FrameCounter(data["draw"][j]["seconds"].asInt() * ((ofApp*)ofGetAppPtr())->appconfig.getFramerate());
					}
					else {
						drawer->first = FrameCounter();
					}
					items.push_back(drawer);
				}
			}
			if (items.size() > 0) {
				index = ofRandom(items.size() - 1);
			}
			else {
				index = -1;
			}
			return true;

		}
		void update() {
			if (index > -1) {
				items[index]->first.decrementFrameCount();
				if (items[index]->first.getFrameCountMaxHit()) {
					// try the next one, or start over if requested  bugbug add a Random() also
					if (index + 1 >= items.size()) {
						index = ofRandom(items.size() - 1); // bugbug add more here like repeat, linear etc vs. just random
						for (auto& item : items) {
							item->first.reset();
						}
					}
					else {
						++index;
					}
				}
			}
		}

		virtual void start() {};
		virtual void end() {};
	protected:
		int index = -1; // none
		vector<shared_ptr<pair<FrameCounter, shared_ptr<T>>>> items;
		virtual bool mySetup(const Json::Value &val, shared_ptr<T> item) { return true; };
	};


	class ActorRole  {
	public:
		friend class Stage;
		enum drawtype {  draw2d, draw3dFixedCamera, draw3dMovingCamera };
		ActorRole(DataType idIn = UnknownID) { id = idIn; }
		ActorRole(const string&path, DataType idIn = UnknownID) { setLocationPath(path);	id = idIn; }
		shared_ptr<ActorRole> parent = nullptr;
		ofNode *node = nullptr; // for 3d
		bool setup(const Json::Value &data);
		// avoid name clashes and wrap the most used items, else access the contained object for more
		void operator=(const ActorRole&rhs) {
			font = rhs.font;
			colorHelper = rhs.colorHelper;
		}
		void setDefaults(const ActorRole&rhs){
			*this = rhs;
		}
		ofTrueTypeFont* getFont() { return font.get(); }
		bool isAnimating();
		void animateTo(const ofPoint& p);
		void setFullSize();
		bool refreshAnimation();
		void resume();
		void pause();
		float getObjectLifetime();
		void setRefreshRate(uint64_t);
		float getWait();
		ofPoint getCurrentPosition();
		void setActorPosition(ofPoint& p);
		virtual float getTimeBeforeStart(float t);
		int getDrawOrder() { return drawOrder; }
		void setDrawOrder(int order) { drawOrder = order; }
		// helpers to wrap basic functions
		void setupForDrawing() { mySetup(); };
		void updateForDrawing();

		string &getLocationPath();
		void setLocationPath(const string&s) { locationPath = s; }

		void setType(drawtype typeIn) { type = typeIn; }
		shared_ptr<ofxSmartFont> getFontPointer() { return font.getPointer(); }
		void setFill(bool b = true) { fill = b; }

	protected:
		FrameCounter frames; // number of frames to show this item, null means always show
		static bool OKToRemove(shared_ptr<ActorRole> me);
		drawtype getType() { return type; }
		bool useFill() { return fill; }
		void rotate();
		float scale();
		void drawIt(drawtype type);
		ofPoint defaultStart;

		int drawOrder = 0; // sorted by value so any value is ok
		bool fullsize = false;
		FontHelper  font;
		shared_ptr<ColorSet> colorHelper = nullptr;
		
		void applyColor();
		string notes;// unstructured string of info, can be shown to the user
		string title; // title object
		string name; // any object can have a name, note, date, reference, duration
		void setFixed(bool b = true) { fixedPosition = b; }
		bool getFixed() { return fixedPosition; }
	private:
		virtual bool mysetup(const Json::Value &data) { return true; };
		bool fixedPosition = false;
		DataType id;// optional
		bool okToDraw(drawtype type);
		drawtype type = draw2d;
		// derived classes supply these if they need them to be called
		virtual void mySetup() {};
		virtual void myUpdate() {};
		virtual void myDraw();
		bool fill = false;
		string   locationPath;   // location of item to draw
		shared_ptr<Rotation> rotationAnimation = nullptr; // optional rotation
		shared_ptr<PointAnimation> locationAnimation = nullptr; // optional movement
		shared_ptr<ScaleAnimation> scaleAnimation = nullptr; // 2d scale multply x,y by scale, 1 by default bugbug 
		vector<shared_ptr<ActorRole>> partners; // working together bugbug test this out in data and add to drawing/update/setup etc
		shared_ptr<vector<shared_ptr<Reference>>> references = nullptr; // research reference to show where actor came from
	};

}