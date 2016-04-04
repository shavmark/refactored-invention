#include "2552software.h"
#include "animation.h"
#include "model.h"

namespace Software2552 {


	//ColorSet Animator::getFirstColor(ColorSet::ColorGroup group) {
	//	std::vector<ColorSet>::iterator itr = std::find(data.begin(), data.end(), ColorSet(group));
	//	if (itr != data.end()) {
	//		++(*itr); // reflect usage
	//		return *itr;
	//	}
	//	return ColorSet(group); // always do something
	//}

	objectLifeTimeManager::objectLifeTimeManager() {
		usageCount = 0;     // number of times this animation was used
		objectlifetime = 0; // 0=forever, how long object lives after it starts drawing
		expired = false;    // object is expired
		startTime = 0;
		waitTime = 0;
		refreshRate = 0;
	}

	void objectLifeTimeManager::start() { 
		startTime = ofGetElapsedTimef(); 
	}

	bool objectLifeTimeManager::OKToRemove(shared_ptr<objectLifeTimeManager> me) {
		if (me->isExpired()) {
			return true;
		}
		// duration == 0 means never go away, and start == 0 means we have not started yet
		if (me->getObjectLifetime() == 0 || me->startTime == 0) {
			return false; // no time out ever, or we have not started yet
		}
		float t = ofGetElapsedTimef();
		float elapsed = ofGetElapsedTimef() - me->startTime;
		if (me->getWait() > elapsed) {
			return false;
		}
		if (me->getObjectLifetime() > elapsed) {
			return false;
		}
		return true;

	}
	// return true if a refresh was done
	bool objectLifeTimeManager::refreshAnimation() {
		if (startTime == 0) {
			start();
			return false;
		}
		float t = ofGetElapsedTimef() - startTime;

		if (t < getWait()) {
			return false;// waiting to start
		}
		//wait = 0; // skip all future usage of wait once we start
				  // check for expired flag 
		if (getObjectLifetime() > 0 && t > getObjectLifetime()) {
			expired = true; // duration of 0 means no expiration
			return false;
		}
		// at this point we can start the time over w/o a wait
		if (t > getRefreshRate()) {
			startTime = ofGetElapsedTimeMillis();
			return true;
		}
		return false;
	}
	
	void PointAnimation::update() {
		if (isAnimationEnabled()) {
			float dt = 1.0f / 60.0f;
			ofxAnimatableOfPoint::update(dt);
		}
	}
	shared_ptr<PointAnimation> ActorRole::getAnimationHelper() {
		// allocate on demand, then objects not in need of animation will be smaller
		if (ani == nullptr) {
			ani = std::make_shared<PointAnimation>();
		}
		return ani;
	}
	void ActorRole::updateForDrawing() {
		
		if (getColorAnimation() != nullptr) {
			getColorAnimation()->update();
		}
		if (getAnimationHelper()) {
			getAnimationHelper()->update();
		}
		myUpdate(); // call derived classes
	};
	void ActorRole::applyColor() {
		if (getColorAnimation()) {
			getColorAnimation()->draw();
		}
	}
	string &ActorRole::getLocationPath() { 
		return locationPath; 
	}

	void ActorRole::drawIt(drawtype type) {//DrawingBasics::drawtype
		if (okToDraw(type)) {
			applyColor();
			myDraw();
		}
	};

	bool ActorRole::okToDraw(drawtype type) {
		drawtype dt = getType();
		if (type != getType() || getAnimationHelper()->paused() || getAnimationHelper()->isExpired()) {
			return false;
		}
		// if still in wait threshold
		float t = ofGetElapsedTimef() - getAnimationHelper()->getStart();
		if (t < getAnimationHelper()->getWait()) {
			return false; // in wait mode, nothing else to do
		}
		//wait = 0; // skip all future usage of wait once we start
				  // duration 0 means always draw
		if (getAnimationHelper()->getObjectLifetime() == 0) {
			return true;
		}
		if (t < getAnimationHelper()->getObjectLifetime()) {
			return true;
		}
		else {
			float olt = getAnimationHelper()->getObjectLifetime();
			getAnimationHelper()->setExpired(true);
			return false;
		}
	}

}