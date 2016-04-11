#include "2552software.h"
#include "animation.h"
#include "model.h"
#include "scenes.h"

namespace Software2552 {

	void setAnimationValues(ofxAnimatable*p, const Json::Value &data, string& curveName, string& repeatType);
	AnimRepeat getRepeatTypeFromString(const string& repeatType);

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
		if (me == nullptr) {
			return false;
		}
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

	void ActorRole::setPosition(ofPoint& p) {
		if (locationAnimation) {
			locationAnimation->setPosition(p);
		}
		else {
			defaultStart = p;
		}
	}

	ofPoint ActorRole::getCurrentPosition() {
		if (locationAnimation) {
			return locationAnimation->getCurrentPosition();
		}
		return defaultStart;// 0,0,0 by default bugbug set on of object vs this saved one
	}
	bool Rotation::setup(const Json::Value &data) {
		FloatAnimation::setup(data);
		x.setup(data["x"]);
		y.setup(data["y"]);
		z.setup(data["z"]);
		return true;
	}
	FloatAnimation::FloatAnimation(float fromIn, float toIn) :ofxAnimatableFloat(), objectLifeTimeManager() {
		from = fromIn;
		to = toIn;
	}

	// need "scale"{}, animation etc wrapers in json
	bool FloatAnimation::setup(const Json::Value &data) {
		if (data.size() > 0) {
			objectLifeTimeManager::setup(data);

			setAnimationValues(this, data, string("LINEAR"), string("LOOP_BACK_AND_FORTH"));

			READFLOAT(from, data);
			READFLOAT(to, data);

			animateFromTo(from, to);

			bool enableAnimation = false;
			READBOOL(enableAnimation, data);
			setAnimationEnabled(enableAnimation);
		}

		return true;
	}
	// helper
	AnimRepeat getRepeatTypeFromString(const string& repeatType) {
		if (repeatType == "LOOP") {
			return LOOP;
		}
		if (repeatType == "PLAY_ONCE") {
			return PLAY_ONCE;
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH") {
			return LOOP_BACK_AND_FORTH;
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH_ONCE") {
			return LOOP_BACK_AND_FORTH_ONCE;
		}
		else if (repeatType == "PLAY_N_TIMES") {
			return PLAY_N_TIMES;
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH_N_TIMES") {
			return LOOP_BACK_AND_FORTH_N_TIMES;
		}
		logTrace("using default loop");
		return LOOP; // default
	}
	void setAnimationValues(ofxAnimatable*p, const Json::Value &data, string& curveName, string& repeatType) {
		if (p) {
			READSTRING(curveName, data);
			p->setCurve(ofxAnimatable::getCurveFromName(curveName));

			int repeat = 0;
			READINT(repeat, data);
			p->setRepeatTimes(repeat);

			READSTRING(repeatType, data);
			p->setRepeatType(getRepeatTypeFromString(repeatType));

			float duration = 0.55f;
			READFLOAT(duration, data);
			p->setDuration(duration);
		}
	}
	
	bool objectLifeTimeManager::setup(const Json::Value &data) {
		usageCount = 0;     // number of times this animation was used
		objectlifetime = 0; // 0=forever, how long object lives after it starts drawing
		expired = false;    // object is expired
		startTime = 0;
		waitTime = 0;
		refreshRate = 0;

		READFLOAT(waitTime, data);
		READFLOAT(objectlifetime, data);
		READFLOAT(refreshRate, data);
		
		return true;
	}

		// only read in one time, to make things more dynamic change at run time
	bool PointAnimation::setup(const Json::Value &data) {
		if (data.size() > 0) {
			objectLifeTimeManager::setup(data);

			setAnimationValues(this, data, string("EASE_IN"), string("LOOP_BACK_AND_FORTH"));

			bool enable = false;
			READBOOL(enable, data);
			setAnimationEnabled(enable);

			Point3D point0; // defaults to 0,0,0
			point0.setup(data["from"]);
			setPosition(point0);

			Point3D pointEnd; // defaults to 0,0,0
			pointEnd.setup(data["to"]);
			animateTo(pointEnd);

		}
		return true;
	}
	void FloatAnimation::update() {
		if (isAnimationEnabled()) {
			float dt = 1.0f / 60.0f;
			ofxAnimatableFloat::update(dt);
		}
	}

	void PointAnimation::update() {
		if (isAnimationEnabled()) {
			float dt = 1.0f / 60.0f;
			ofxAnimatableOfPoint::update(dt);
		}
	}
	// try to keep wrappers out of site to avoid clutter
	// we want to run w/o crashing in very low memory so we need to check all our pointers, we can chug along
	// until memory frees up, a crash would be very bad
	
	void ActorRole::setAnimationPosition(const ofPoint& p) { if (locationAnimation)locationAnimation->setPosition(p); }
	void ActorRole::setAnimationPositionX(float x) { if (locationAnimation)locationAnimation->setPositionX(x); }
	void ActorRole::setAnimationPositionY(float y) { if (locationAnimation)locationAnimation->setPositionY(y); }
	void ActorRole::setAnimationPositionZ(float z) { if (locationAnimation)locationAnimation->setPositionZ(z); }
	void ActorRole::animateTo(const ofPoint& p) { if (locationAnimation)locationAnimation->animateTo(p); }
	bool ActorRole::refreshAnimation() { return (locationAnimation) ? locationAnimation->refreshAnimation() : false; }
	void ActorRole::setAnimationEnabled(bool f) { if (locationAnimation)locationAnimation->setAnimationEnabled(f); }
	float ActorRole::getTimeBeforeStart(float t) { return (locationAnimation) ? locationAnimation->getWait() : 0; }
	void ActorRole::pause(){ if (locationAnimation)locationAnimation->pause();}
	void ActorRole::resume() { if (locationAnimation)locationAnimation->pause(); }
	float ActorRole::getObjectLifetime() { return (locationAnimation) ? locationAnimation->getObjectLifetime() : 0; }
	void ActorRole::setRefreshRate(uint64_t rateIn) { if (locationAnimation)locationAnimation->setRefreshRate(rateIn); }
	float ActorRole::getWait() { return (locationAnimation) ? locationAnimation->getWait() : 0; }

	void AnimiatedColor::setAlpha(float val) {
		setAlphaOnly(val);
	}

	bool ActorRole::setup(const Json::Value &data) {
		fill = true; // set default
		drawOrder = 0;
		w = h = 0;
		references = nullptr;
		locationAnimation = nullptr;
		scaleAnimation = nullptr;
		rotationAnimation = nullptr;

		if (data.size()) {
			READSTRING(name, data);
			READSTRING(title, data);
			READSTRING(notes, data);
			READSTRING(locationPath, data);
			READBOOL(fill, data);
			READINT(drawOrder, data);
			string s = getLocationPath();//just for debug
			// optional sizes, locations, durations for animation etc
			readJsonValue(w, data["width"]);
			readJsonValue(h, data["height"]);
			// any actor can have a reference
			references = parseList<Reference>(data["references"]);
			locationAnimation = parseNoList<PointAnimation>(data["animation"]);
			scaleAnimation = parseNoList<ScaleAnimation>(data["scale"]);
			rotationAnimation = parseNoList<Rotation>(data["rotation"]);
		}

		colorHelper = parseColor(data); // always some kind of helper present

		// let helper objects deal with empty data in their own way

		// actors can have a lot of attributes, but if not no memory is used
		font.setup(data);

		// read derived class data
		return mysetup(data);
	}
	// return current color, track its usage count
	shared_ptr<ColorSet> ColorList::getCurrentColor() {
		if (privateData) {
			if (privateData->currentColorSet) {
				++(*(privateData->currentColorSet)); // mark usage if data has been set
				return privateData->currentColorSet;
			}
		}
		// should never happen ... but return a reasonble value if it does
		return nullptr;
	}

	

	void ActorRole::updateForDrawing() {

		if (colorHelper) {
			colorHelper->update();
		}
		if (locationAnimation) {
			locationAnimation->update();
		}
		if (scaleAnimation) {
			scaleAnimation->update();
		}
		if (rotationAnimation) {
			rotationAnimation->update();
		}
		
		myUpdate(); // call derived classes
	};
	void Rotation::update() {
		x.update();
		y.update();
		z.update();
	}
	void ActorRole::applyColor() {
		if (colorHelper) {
			colorHelper->draw(); // always returns a pointer
		}
	}
	string &ActorRole::getLocationPath() {
		return locationPath;
	}
	float ActorRole::rotate() {
		//bugbug use rotationAnimation and make sure w and h are set for ration
		if (w && h) {
			ofTranslate(w / 2, h / 2, 0);//move pivot to centre
			ofRotate(ofGetFrameNum() * .05, 0, 0, 1);//rotate from centre
			ofTranslate(-w / 2, -w / 2, 0);//move back by the centre offset
		}
		return 0;

		if (rotationAnimation) {
			//bugbug convert to a proper rotate
			return rotationAnimation->x.getCurrentValue();//bugbug need to rotate object not screen
		}
		return 0.0f;
	}
	float ActorRole::scale() {
		if (scaleAnimation) {
			return scaleAnimation->getCurrentValue();
		}
		return 1.0f;
	}
	void ActorRole::drawIt(drawtype type) {
		if (okToDraw(type)) {
			if (useFill()) {
				ofFill();
			}
			else {
				ofNoFill();
			}
			bool disableEAP = false;
			if (colorHelper->alphaEnbled()) {
				disableEAP = true;
				ofEnableAlphaBlending(); // only use when needed for performance
			}
			if (getType() == draw2d) {
				applyColor(); // in 3d color comes from lights etc
			}
			//rotate();
			myDraw();
			if (disableEAP) {
				ofDisableAlphaBlending(); 
			}
		}
	};

	bool ActorRole::okToDraw(drawtype type) {
		drawtype dt = getType();
		if (type != getType()){
			return false;
		}
		if (locationAnimation == nullptr) {
			return true;
		}
		if (locationAnimation->paused() || locationAnimation->isExpired()) {
			return false;
		}
		// if still in wait threshold
		float t = ofGetElapsedTimef() - locationAnimation->getStart();
		if (t < locationAnimation->getWait()) {
			return false; // in wait mode, nothing else to do
		}
		//wait = 0; // skip all future usage of wait once we start
				  // duration 0 means always draw
		if (locationAnimation->getObjectLifetime() == 0) {
			return true;
		}
		if (t < locationAnimation->getObjectLifetime()) {
			return true;
		}
		else {
			float olt = locationAnimation->getObjectLifetime();
			locationAnimation->setExpired(true);
			return false;
		}
	}
	// helpers
	ofTrueTypeFont* FontHelper::get() {
		if (font == nullptr) {
			getPointer();
		}
		if (font != nullptr) {
			return &font->ttf;
		}
		return nullptr;
	}
	ofFloatColor AnimiatedColor::getFloatObject() { 
		return ofFloatColor(getCurrentColor());
	}
	ofColor AnimiatedColor::getColorObject() { 
		return getCurrentColor();
	}

	shared_ptr<ofxSmartFont> FontHelper::getPointer() {
		if (font == nullptr) {
			font = ofxSmartFont::get(defaultFontFile, defaultFontSize);
			if (font == nullptr) {
				// name is not unqiue, just a helper of some kind I guess
				font = ofxSmartFont::add(defaultFontFile, defaultFontSize, defaultFontName);
			}
		}
		if (font == nullptr) {
			logErrorString("font is null");
		}
		return font;
	}
	bool FontHelper::setup(const Json::Value &data) {
		if (data.size() > 0 && !data["font"].empty()) {

			string name;
			int size = defaultFontSize;
			string filename;

			readStringFromJson(name, data["font"]["name"]);
			readStringFromJson(filename, data["font"]["file"]);
			readJsonValue(size, data["font"]["size"]);

			// filename required to create a font, else default font is used
			if (filename.size() != 0) {
				font = ofxSmartFont::get(filename, size);
				if (font == nullptr) {
					// name is not unqiue, just a helper of some kind I guess
					font = ofxSmartFont::add(filename, size, name);
				}
				if (font == nullptr) {
					logErrorString("font file issue");
					return false;
				}
			}
		}
		return true;
	}
	void AnimiatedColor::update() {
		float dt = 1.0f / 60.0f;//bugbug does this time to frame count? I think so
		ofxAnimatableOfColor::update(dt);
	}
	void AnimiatedColor::draw() {
		if (isAnimationEnabled()) {
			applyCurrentColor();
		}
	}
	// all drawing is done using AnimiatedColor, even if no animation is used, color info still stored by doing a set color
	bool AnimiatedColor::setup(const Json::Value &data) {
		if (!data.empty()) {
			READFLOAT(from, data);
			READFLOAT(to, data);
			if (from != 255) {
				setAlphaOnly(from);
			}
			// anmation requested
			bool enable = false;
			if (from != to && from != 255) {
				animateToAlpha(to); // will not animate color
				enable = true; // set on by default
			}
			READBOOL(enable, data);
			setAnimationEnabled(true);

			int color;//hex color
			if (data["colorTo"].size() > 0) {
				if (READINT(color, data["colorTo"])) {
					animateTo(ofColor().fromHex(color, to));
				}
			}
			if (data["colorFrom"].size() > 0) {
				if (READINT(color, data["colorFrom"])) {
					setColor(ofColor().fromHex(color, from));
				}
			}
		}
		setAnimationValues(this, data, string("LINEAR"), string("LOOP_BACK_AND_FORTH"));
		return true;
	}

}