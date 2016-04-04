#pragma once
#include "2552software.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"

// supports animation

namespace Software2552 {

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
	class PointAnimation : public ofxAnimatableOfPoint, public objectLifeTimeManager {
	public:
		void update();
		void startAnimationAfterDelay(float delay) { ofxAnimatableOfPoint::startAnimationAfterDelay(delay); }
		bool paused() {	return paused_;	}
		bool isAnimationEnabled() { return animationEnabled; }
		void setAnimationEnabled(bool b= true) { animationEnabled=b; }
	private:
		bool animationEnabled = false;
	};
	// basic drawing info, bugbug maybe color set goes here too, not sure yet
	class AnimiatedColor;
	class ActorRole  {
	public:
		enum drawtype { drawBackground, draw2d, draw3dFixedCamera, draw3dMovingCamera };
		ActorRole() {  }
		ActorRole(const string&path) { 	setLocationPath(path); 		}

		// avoid name clashes and wrap the most used items, else access the contained object for more
		shared_ptr<PointAnimation> getAnimationHelper();
		virtual float getTimeBeforeStart(float t) { return getAnimationHelper()->getWait(); }

		// helpers to wrap basic functions
		void setupForDrawing() { mySetup(); };
		void updateForDrawing();
		void drawIt(drawtype type);

		void setColorAnimation(shared_ptr<AnimiatedColor>p) { colorAnimation = p;		}
		shared_ptr<AnimiatedColor> getColorAnimation() {return colorAnimation;}

		int w = 0;
		int h = 0;

		string &getLocationPath();
		void setLocationPath(const string&s) { locationPath = s; }

		static bool OKToRemove(shared_ptr<ActorRole> me) {
			return me->getAnimationHelper()->OKToRemove(me->getAnimationHelper());
		}
		static bool OKToRemoveNormalPointer(ActorRole* me) {
			return me->getAnimationHelper()->OKToRemove(me->getAnimationHelper());
		}
		void setType(drawtype typeIn) { type = typeIn; }
	protected:
		drawtype getType() { return type; }
		void applyColor();

	private:
		bool okToDraw(drawtype type);
		drawtype type = draw2d;
		// derived classes supply these if they need them to be called
		virtual void mySetup() {};
		virtual void myUpdate() {};
		virtual void myDraw() {};
		string   locationPath;   // location of item to draw
		shared_ptr<AnimiatedColor> colorAnimation = nullptr; // optional color
		shared_ptr<PointAnimation> ani = nullptr; // call all the other items via here
	};



}