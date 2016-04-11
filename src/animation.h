#pragma once
#include "2552software.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxEasing.h"


// supports animation

namespace Software2552 {
	AnimRepeat getRepeatTypeFromString();
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
		float   objectlifetime; // 0=forever, how long object lives after it starts drawing
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
		RotationAnimation() : FloatAnimation(0.0f, 0.0f) {}
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
		float funXMover() { (2 * ofGetFrameNum()) % ofGetWidth(); }
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

	// basic drawing info, bugbug maybe color set goes here too, not sure yet
	class Stage;
	class Reference;
	class ActorRole  {
	public:
		friend class Stage;
		enum drawtype {  draw2d, draw3dFixedCamera, draw3dMovingCamera };
		ActorRole() {  }
		ActorRole(const string&path) { 	setLocationPath(path);	}
		
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
		void setPosition(ofPoint& p);
		virtual float getTimeBeforeStart(float t);
		int getDrawOrder() { return drawOrder; }
		// helpers to wrap basic functions
		void setupForDrawing() { mySetup(); };
		void updateForDrawing();

		string &getLocationPath();
		void setLocationPath(const string&s) { locationPath = s; }

		void setType(drawtype typeIn) { type = typeIn; }
		shared_ptr<ofxSmartFont> getFontPointer() { return font.getPointer(); }
		void setFill(bool b = true) { fill = b; }

	protected:
		static bool OKToRemove(shared_ptr<ActorRole> me) {
			return objectLifeTimeManager::OKToRemove(me->locationAnimation);
		}
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

	private:
		virtual bool mysetup(const Json::Value &data) { return true; };

		bool okToDraw(drawtype type);
		drawtype type = draw2d;
		// derived classes supply these if they need them to be called
		virtual void mySetup() {};
		virtual void myUpdate() {};
		virtual void myDraw() {};
		bool fill = false;
		string   locationPath;   // location of item to draw
		shared_ptr<Rotation> rotationAnimation = nullptr; // optional rotation
		shared_ptr<PointAnimation> locationAnimation = nullptr; // optional movement
		shared_ptr<ScaleAnimation> scaleAnimation = nullptr; // 2d scale multply x,y by scale, 1 by default bugbug 
		shared_ptr<vector<shared_ptr<Reference>>> references = nullptr; // research reference to show where actor came from
		
	};



}