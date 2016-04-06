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
	bool FloatAnimation::readFromScript(const Json::Value &data){
		//bugbug how to make common read animation items one function?
		float duration = 0;
		READFLOAT(duration, data);
		setObjectLifetime(duration);

		float wait = 0;
		READFLOAT(wait, data);
		setWait(wait);

		float start=0.0f, end = 0.0f;
		READFLOAT(start, data);
		READFLOAT(end, data);

		animateFromTo(start, end);

		bool enableAnimation = false;
		READBOOL(enableAnimation, data);
		setAnimationEnabled(enableAnimation);

		string curveName = "EASE_IN"; // there is a boat load of coolness here
		READSTRING(curveName, data);
		setCurve(ofxAnimatable::getCurveFromName(curveName));

		int repeat = 1;
		READINT(repeat, data);
		setRepeatTimes(repeat);

		string repeatType = "PLAY_ONCE";
		READSTRING(repeatType, data);

		if (repeatType == "LOOP") {
			setRepeatType(LOOP);
		}
		if (repeatType == "PLAY_ONCE") {
			setRepeatType(PLAY_ONCE);
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH") {
			setRepeatType(LOOP_BACK_AND_FORTH);
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH_ONCE") {
			setRepeatType(LOOP_BACK_AND_FORTH_ONCE);
		}
		else if (repeatType == "PLAY_N_TIMES") {
			setRepeatType(PLAY_N_TIMES);
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH_N_TIMES") {
			setRepeatType(LOOP_BACK_AND_FORTH_N_TIMES);
		}

		float animationDuration = 0.55f;
		READFLOAT(animationDuration, data);
		setDuration(animationDuration);


		return true;
	}
	// only read in one time, to make things more dynamic change at run time
	bool PointAnimation::readFromScript(const Json::Value &data) {

		float duration = 0;
		READFLOAT(duration, data);
		setObjectLifetime(duration);

		float wait = 0;
		READFLOAT(wait, data);
		setWait(wait);

		Point3D point0; // defaults to 0,0,0
		point0.readFromScript(data["start"]);
		setPosition(point0);

		bool enableAnimation = false;
		READBOOL(enableAnimation, data);
		setAnimationEnabled(enableAnimation);

		Point3D pointEnd; // defaults to 0,0,0
		pointEnd.readFromScript(data["finish"]);
		animateTo(pointEnd);

		string curveName="EASE_IN"; // there is a boat load of coolness here
		READSTRING(curveName, data);
		setCurve(ofxAnimatable::getCurveFromName(curveName));

		int repeat = 1;
		READINT(repeat, data);
		setRepeatTimes(repeat);

		string repeatType = "PLAY_ONCE";
		READSTRING(repeatType, data);

		if (repeatType == "LOOP") {
			setRepeatType(LOOP);
		}
		if (repeatType == "PLAY_ONCE") {
			setRepeatType(PLAY_ONCE);
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH") {
			setRepeatType(LOOP_BACK_AND_FORTH);
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH_ONCE") {
			setRepeatType(LOOP_BACK_AND_FORTH_ONCE);
		}
		else if (repeatType == "PLAY_N_TIMES") {
			setRepeatType(PLAY_N_TIMES);
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH_N_TIMES") {
			setRepeatType(LOOP_BACK_AND_FORTH_N_TIMES);
		}

		float animationDuration = 0.55f;
		READFLOAT(animationDuration, data);
		setDuration(animationDuration);

		return true;
	}
	void PointAnimation::update() {
		if (isAnimationEnabled()) {
			float dt = 1.0f / 60.0f;
			ofxAnimatableOfPoint::update(dt);
		}
	}

	shared_ptr<FloatAnimation>  ActorRole::getScaleAnimationHelper() {
		// allocate on demand, then objects not in need of animation will be smaller
		if (scaleAnimation == nullptr) {
			scaleAnimation = std::make_shared<FloatAnimation>();
		}
		return scaleAnimation;
	}

	shared_ptr<PointAnimation> ActorRole::getLocationAnimationHelper() {
		// allocate on demand, then objects not in need of animation will be smaller
		if (locationAnimation == nullptr) {
			locationAnimation = std::make_shared<PointAnimation>();
		}
		return locationAnimation;
	}
	bool ActorRole::readFromScript(const Json::Value &data) {

		READSTRING(locationPath, data);
		READINT(drawOrder, data);
		string s = getLocationPath();//just for debug
		// optional sizes, locations, durations for animation etc
		readJsonValue(w, data["width"]);
		readJsonValue(h, data["height"]);

		getLocationAnimationHelper()->readFromScript(data);
		getScaleAnimationHelper()->readFromScript(data);
		//bugbug should I move color helper here too? 
		//bugbug add in some rotate too? its afloat across x,y,z
		return true;
	}
	void ActorRole::updateForDrawing() {
		
		if (getColorAnimationHelper()) {
			getColorAnimationHelper()->update();
		}
		if (getLocationAnimationHelper()) {
			getLocationAnimationHelper()->update();
		}
		if (getScaleAnimationHelper()) {
			getScaleAnimationHelper()->update();
		}
		myUpdate(); // call derived classes
	};
	void ActorRole::applyColor() {
		if (getColorAnimationHelper()) {
			getColorAnimationHelper()->draw();
		}
	}
	string &ActorRole::getLocationPath() { 
		return locationPath; 
	}

	void ActorRole::drawIt(drawtype type) {
		if (okToDraw(type)) {
			if (getType() == draw2d) {
				applyColor(); // in 3d color comes from lights etc
			}
			myDraw();
		}
	};

	bool ActorRole::okToDraw(drawtype type) {
		drawtype dt = getType();
		if (type != getType() || getLocationAnimationHelper()->paused() || getLocationAnimationHelper()->isExpired()) {
			return false;
		}
		// if still in wait threshold
		float t = ofGetElapsedTimef() - getLocationAnimationHelper()->getStart();
		if (t < getLocationAnimationHelper()->getWait()) {
			return false; // in wait mode, nothing else to do
		}
		//wait = 0; // skip all future usage of wait once we start
				  // duration 0 means always draw
		if (getLocationAnimationHelper()->getObjectLifetime() == 0) {
			return true;
		}
		if (t < getLocationAnimationHelper()->getObjectLifetime()) {
			return true;
		}
		else {
			float olt = getLocationAnimationHelper()->getObjectLifetime();
			getLocationAnimationHelper()->setExpired(true);
			return false;
		}
	}

}