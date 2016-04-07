#pragma once
#include "2552software.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"


// supports animation

namespace Software2552 {
	// read in list of Json values
	template<typename T> shared_ptr<T> parseNoList(const Json::Value &data)
	{
		shared_ptr<T>  results = nullptr;
		if (data.size() > 0) {
			results = std::make_shared<T>();
			if (results) {
				results->readFromScript(data);
			}
		}
		return results;
	}
	template<typename T> shared_ptr<vector<shared_ptr<T>>> parseList(const Json::Value &data)
	{
		shared_ptr<vector<shared_ptr<T>>>  results = nullptr;
		if (data.size() > 0) {
			results = std::make_shared<vector<shared_ptr<T>>>();
			for (Json::ArrayIndex j = 0; j < data.size(); ++j) {
				shared_ptr<T> item = std::make_shared<T>();
				item->readFromScript(data[j]);
				results->push_back(item);
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
		int getAnimationUsageCount() const { return usageCount; }
		bool refreshAnimation();
		// how long to wait
		virtual void getTimeBeforeStart(float& t) {	setIfGreater(t, getObjectLifetime() + getWait());		}
		static bool OKToRemove(shared_ptr<objectLifeTimeManager> me);
		void removeExpiredItems(vector<shared_ptr<objectLifeTimeManager>>&v) {
			v.erase(std::remove_if(v.begin(), v.end(), OKToRemove), v.end());
		}

	private:
		int	    usageCount=0;     // number of times this animation was used
		float   objectlifetime=0; // 0=forever, how long object lives after it starts drawing
		bool	expired=false;    // object is expired
		float	startTime = 0;
		float   waitTime = 0;
		float	refreshRate = 0;
	};

	class FloatAnimation : public ofxAnimatableFloat, public objectLifeTimeManager {
	public:
		void update();
		bool readFromScript(const Json::Value &data);
		void startAnimationAfterDelay(float delay) { ofxAnimatableFloat::startAnimationAfterDelay(delay); }
		bool paused() { return paused_; }
		bool isAnimationEnabled() { return animating_; }
		void setAnimationEnabled(bool b = true) { animating_ = b; }
	private:
	};
	class RotationAnimation : public ofxAnimatableFloat {
	public:
		void update();
		bool readFromScript(const Json::Value &data) {
			return true;//bugbug fill this in
		}
		bool isAnimationEnabled() { return animating_; }
		void setAnimationEnabled(bool b = true) { animating_ = b; }
		float angle=1;
		float x=0, y = 0, z = 0;
	};
	class PointAnimation : public ofxAnimatableOfPoint, public objectLifeTimeManager {
	public:
		void update();
		bool readFromScript(const Json::Value &data);
		void startAnimationAfterDelay(float delay) { ofxAnimatableOfPoint::startAnimationAfterDelay(delay); }
		bool paused() {	return paused_;	}
		bool isAnimationEnabled() { return animating_; }
		void setAnimationEnabled(bool b= true) { animating_ =b; }
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
		bool paused() { return paused_; }
		bool readFromScript(const Json::Value &data);
		shared_ptr<ColorSet> getColorSet();
		float getAlpha() { return alpha; }//bugbug flush this out, read it in, set it etc
		void setAlpha(float a) { alpha = a; }
		void getNextColors();
		void setColorSet(shared_ptr<ColorSet>p);
		bool useAnimation() { return usingAnimation; }
		void setAnimation(bool b = true) { usingAnimation = true; }
		// avoid long access code
		ofFloatColor getFloatObject(int hex) { return ofFloatColor().fromHex(hex, getAlpha()); }
		ofColor getColorObject(int hex) { return ofFloatColor().fromHex(hex, getAlpha()); }

	private:
		shared_ptr<ColorSet> colorSet = nullptr; // use a pointer to the global color for dynamic color or set a fixed one
		bool usingAnimation = false; // force  to set this to make sure its understood
		float alpha = 255;
	};

	class ColorHelper {
	public:
		bool readFromScript(const Json::Value &data);
		void setColor(int hex);
		int getForeground();
		int getBackground();
		int getFontColor();
		int getLightest();
		int getDarkest();
		int getOther();
		ofFloatColor getFloatObject(int hex);
		ofColor getColorObject(int hex);
		int getAlpha();
		void getNextColors();
		shared_ptr<AnimiatedColor> getAnimatedColorPtr() const;
		void setAnimatedColorPtr(shared_ptr<AnimiatedColor>p) { colorAnimation = p; }
	private:
		shared_ptr<AnimiatedColor> colorAnimation = nullptr; // optional color
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
		bool readFromScript(const Json::Value &data);
	private:
		shared_ptr<ofxSmartFont> font;
	};

	// basic drawing info, bugbug maybe color set goes here too, not sure yet
	class Stage;
	class Reference;
	class ActorRole  {
	public:
		enum drawtype {  draw2d, draw3dFixedCamera, draw3dMovingCamera };
		ActorRole() {  }
		ActorRole(const string&path) { 	setLocationPath(path);	}
		
		bool readActorFromScript(const Json::Value &data);
		// avoid name clashes and wrap the most used items, else access the contained object for more
		void operator=(const ActorRole&rhs) {
			font = rhs.font;
			colorHelper.setAnimatedColorPtr(rhs.getColorAnimationPtr());
		}
		void setDefaults(const ActorRole&rhs){
			*this = rhs;
		}
		bool operator==(const ActorRole& rhs) { return rhs.name == name; }
		//void setupActor() { if (getDefaultRole()) getDefaultRole()->setupForDrawing(); }
		//void updateActor() { if (getDefaultRole()) getDefaultRole()->updateForDrawing(); }

		shared_ptr<RotationAnimation> getRotationAnimationHelper() { return rotationAnimation; }
		shared_ptr<FloatAnimation> getScaleAnimationHelper() { return scaleAnimation; }
		shared_ptr<PointAnimation> getLocationAnimationHelper() { return locationAnimation; }
		shared_ptr<AnimiatedColor> getColorAnimationPtr() const { return colorHelper.getAnimatedColorPtr(); }
		shared_ptr<ofxSmartFont> getFontPointer() { return font.getPointer(); }
		ofTrueTypeFont* getFont() { return font.get(); }

		void setAnimationPosition(const ofPoint& p) { if (getLocationAnimationHelper())getLocationAnimationHelper()->setPosition(p); }
		void setAnimationPositionX(float x) { if (getLocationAnimationHelper())getLocationAnimationHelper()->setPositionX(x); }
		void setAnimationPositionY(float y) { if (getLocationAnimationHelper())getLocationAnimationHelper()->setPositionX(y); }
		void setAnimationPositionZ(float z) { if (getLocationAnimationHelper())getLocationAnimationHelper()->setPositionX(z); }
		void animateTo(const ofPoint& p) { if (getLocationAnimationHelper())getLocationAnimationHelper()->animateTo(p); }

		void setAnimationEnabled(bool f) { if (getLocationAnimationHelper())getLocationAnimationHelper()->setAnimationEnabled(f); }
		ofPoint& getCurrentPosition();
		void setPosition(ofPoint& p);
		virtual float getTimeBeforeStart(float t) { return (getLocationAnimationHelper()) ? getLocationAnimationHelper()->getWait():0; }

		// helpers to wrap basic functions
		void setupForDrawing() { mySetup(); };
		void updateForDrawing();
		void drawIt(drawtype type);

		int w = 0;
		int h = 0;
		ofPoint defaultStart;

		int drawOrder = 0; // sorted by value so any value is ok

		string &getLocationPath();
		void setLocationPath(const string&s) { locationPath = s; }

		static bool OKToRemove(shared_ptr<ActorRole> me) {
			return objectLifeTimeManager::OKToRemove(me->getLocationAnimationHelper());
		}
		void setType(drawtype typeIn) { type = typeIn; }


	protected:
		FontHelper  font;
		ColorHelper colorHelper;
		drawtype getType() { return type; }
		void applyColor();
		string notes;// unstructured string of info, can be shown to the user
		string title; // title object
		string name; // any object can have a name, note, date, reference, duration

	private:
		virtual bool myReadFromScript(const Json::Value &data) { return true; };

		bool okToDraw(drawtype type);
		drawtype type = draw2d;
		// derived classes supply these if they need them to be called
		virtual void mySetup() {};
		virtual void myUpdate() {};
		virtual void myDraw() {};
		string   locationPath;   // location of item to draw
		shared_ptr<RotationAnimation> rotationAnimation = nullptr; // optional rotation
		shared_ptr<PointAnimation> locationAnimation = nullptr; // optional movement
		shared_ptr<FloatAnimation> scaleAnimation = nullptr; // 2d scale multply x,y by scale, 1 by default bugbug 
		shared_ptr<vector<shared_ptr<Reference>>> references = nullptr; // research reference to show where actor came from

	};



}