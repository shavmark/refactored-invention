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

	ofPoint& ActorRole::getCurrentPosition() {
		if (locationAnimation) {
			return locationAnimation->getCurrentPosition();
		}
		return defaultStart;// 0,0,0 by default bugbug set on of object vs this saved one
	}
	void ColorHelper::setColor(int hex, int alpha) {
		ofSetColor(colorAnimation->getColorObject(hex, alpha));
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

			float animationDuration = 0.55f;
			READFLOAT(animationDuration, data);
			p->setDuration(animationDuration);
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

			return true;
		}
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
	int ColorHelper::getAlpha() { return colorAnimation->getAlpha(); }
	
	int ColorHelper::getForeground() { return colorAnimation->getColorSet()->getForeground(); }
	int ColorHelper::getBackground() { return colorAnimation->getColorSet()->getBackground(); }
	int ColorHelper::getFontColor() { return colorAnimation->getColorSet()->getFontColor(); }
	int ColorHelper::getLightest() { return colorAnimation->getColorSet()->getLightest(); }
	int ColorHelper::getDarkest() { return colorAnimation->getColorSet()->getDarkest(); }
	int ColorHelper::getOther() { return colorAnimation->getColorSet()->getOther(); }
	void ColorHelper::getNextColors() { colorAnimation->getNextColors(); }
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

	int AnimiatedColor::getAlpha() { return (alphaAnimation) ? alphaAnimation->getCurrentValue() : 255; }
	void AnimiatedColor::setAlpha(int val) {
		if (alphaAnimation) {
			alphaAnimation->reset(val); // set and stop animation
		}
		// if no animation helper no alpha
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

		// let helper objects deal with empty data in their own way

		// actors can have a lot of attributes, but if not no memory is used
		font.setup(data);
		colorHelper.setup(data);

		// read derived class data
		return mysetup(data);
	}
	// return current color, track its usage count
	shared_ptr<AnimiatedColor> ColorList::getCurrentColor() {
		if (privateData) {
			if (privateData->currentColor && privateData->currentColor->colorSet) {
				++(*(privateData->currentColor->colorSet)); // mark usage if data has been set
				return privateData->currentColor;
			}
		}
		return nullptr;
	}

	ColorHelper::ColorHelper() { colorAnimation = ColorList::getCurrentColor(); } // default to global color

	void ActorRole::updateForDrawing() {

		colorHelper.colorAnimation->update(); // always returns a pointer

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
		colorHelper.colorAnimation->draw(); // always returns a pointer
	}
	string &ActorRole::getLocationPath() {
		return locationPath;
	}
	float ActorRole::rotate() {
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
			
			if (colorHelper.getAlpha() != 255) {
				ofEnableAlphaBlending(); // only use when needed for performance
			}
			if (getType() == draw2d) {
				applyColor(); // in 3d color comes from lights etc
			}
			myDraw();
			if (colorHelper.getAlpha() != 255) {
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
	ofFloatColor ColorHelper::getFloatObject(int hex) {
		return ofFloatColor().fromHex(hex, getAlpha());
	}
	ofColor ColorHelper::getColorObject(int hex) {
		return ofFloatColor().fromHex(hex, getAlpha());
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
	// always return a valid pointer
	shared_ptr<ColorSet> AnimiatedColor::getColorSet() {
		if (colorSet == nullptr) {
			return ColorList::getCurrentColor()->getColorSet(); // use the global color
		}
		return colorSet;
	}
	// reset the object
	void AnimiatedColor::setColorSet(shared_ptr<ColorSet>p) {
		if (p) {
			colorSet = p;
			setColor(ofColor(getColorSet()->getLightest()));
			animateTo(ofColor(getColorSet()->getDarkest()));
		}
	}

	void AnimiatedColor::getNextColors() {
		setColorSet(ColorList::getNextColors(getColorSet()->getGroup(), false));
	}
	void AnimiatedColor::update() {
		float dt = 1.0f / 60.0f;//bugbug does this time to frame count? I think so
		ofxAnimatableOfColor::update(dt);
		if (alphaAnimation) {
			alphaAnimation->update();
		}
		if (getColorSet()) {//bugbug can we advance color if desired here? 
		}
	}
	void AnimiatedColor::draw() {
		if (useAnimation()) {
			applyCurrentColor();
		}
		else {
			ofSetColor(ofColor::fromHex(getColorSet()->getForeground(), getAlpha()));
			//background set by background manager
		}
	}
	// all drawing is done using AnimiatedColor, even if no animation is used, color info still stored
	bool AnimiatedColor::setup(const Json::Value &data) {
		if (!data.empty()) {
			string colorGroup;
			READSTRING(colorGroup, data);
			if (colorGroup.size()> 0) {
				// must be set before alpha
				setColorSet(ColorList::getNextColors(ColorSet::convertStringToGroup(colorGroup), false));
			}
			float from = 255;
			READFLOAT(from, data);
			float to = 255;
			READFLOAT(to, data);
			if (from != 255) {
				setAlphaOnly(from);
			}
			if (to != 255) {
				animateToAlpha(to); // will not animate color
				usingAnimation = true; // can still be turned off below
			}
			READBOOL(usingAnimation, data);
		}
		setAnimationValues(this, data, string("LINEAR"), string("LOOP_BACK_AND_FORTH"));
		return true;
	}

}