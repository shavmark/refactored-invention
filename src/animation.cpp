#include "2552software.h"
#include "animation.h"
#include "model.h"
#include "scenes.h"

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
	void ColorHelper::setColor(int hex) {
		ofSetColor(colorAnimation->getColorObject(hex));
	}
	bool BaseAnimation::myReadFromScript(const Json::Value &data) {
		float from = minValue;
		float to = maxValue;
		READFLOAT(from, data);
		READFLOAT(to, data);
		animateFromTo(from, to);// full range
		return true;// fill this in as needed
	}
	bool Rotation::readFromScript(const Json::Value &data) {
		x.readFromScript(data["x"]);
		y.readFromScript(data["y"]);
		z.readFromScript(data["z"]);
		return true;
	}
	// need "scale"{}, animation etc wrapers in json
	bool FloatAnimation::readFromScript(const Json::Value &data) {
		if (data.size() == 0) {
			return true;
		}
		//bugbug how to make common read animation items one function?
		float duration = 0;
		READFLOAT(duration, data);
		setObjectLifetime(duration);

		float wait = 0;
		READFLOAT(wait, data);
		setWait(wait);

		float start = 0.0f, end = 0.0f;
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

		string repeatType = "LOOP_BACK_AND_FORTH";
		READSTRING(repeatType, data);
		setRepeatType(getRepeatTypeFromString());

		float animationDuration = 0.55f;
		READFLOAT(animationDuration, data);
		setDuration(animationDuration);

		return myReadFromScript(data);
	}
	// helper
	AnimRepeat getRepeatTypeFromString() {
		string repeatType = "PLAY_ONCE";

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

		bool enable = false;
		READBOOL(enable, data);
		setAnimationEnabled(enable);

		Point3D pointEnd; // defaults to 0,0,0
		pointEnd.readFromScript(data["finish"]);
		animateTo(pointEnd);

		string curveName = "EASE_IN"; // there is a boat load of coolness here
		READSTRING(curveName, data);
		setCurve(ofxAnimatable::getCurveFromName(curveName));

		int repeat = 0;
		READINT(repeat, data);
		setRepeatTimes(repeat);

		string repeatType = "LOOP_BACK_AND_FORTH";
		READSTRING(repeatType, data);
		setRepeatType(getRepeatTypeFromString());

		float animationDuration = 0.55f;
		READFLOAT(animationDuration, data);
		setDuration(animationDuration);

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
	int ColorHelper::getForeground() { return colorAnimation->getColorSet()->getForeground(); }
	int ColorHelper::getBackground() { return colorAnimation->getColorSet()->getBackground(); }
	int ColorHelper::getFontColor() { return colorAnimation->getColorSet()->getFontColor(); }
	int ColorHelper::getLightest() { return colorAnimation->getColorSet()->getLightest(); }
	int ColorHelper::getDarkest() { return colorAnimation->getColorSet()->getDarkest(); }
	int ColorHelper::getOther() { return colorAnimation->getColorSet()->getOther(); }
	int ColorHelper::getAlpha() { return colorAnimation->getAlpha(); }
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

	bool ActorRole::readActorFromScript(const Json::Value &data) {
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
		font.readFromScript(data);
		colorHelper.readFromScript(data);

		// read derived class data
		return myReadFromScript(data);
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
			if (getType() == draw2d) {
				applyColor(); // in 3d color comes from lights etc
			}
			myDraw();
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
		return ofFloatColor().fromHex(hex, colorAnimation->getAlpha());
	}
	ofColor ColorHelper::getColorObject(int hex) { 
		return ofFloatColor().fromHex(hex, colorAnimation->getAlpha());
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
	bool FontHelper::readFromScript(const Json::Value &data) {
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
			setColor(ofColor(getColorSet()->getLightest(), getAlpha()));
			animateTo(ofColor(getColorSet()->getDarkest(), getAlpha()));
		}
	}

	void AnimiatedColor::getNextColors() {
		setColorSet(ColorList::getNextColors(getColorSet()->getGroup(), false));
	}
	void AnimiatedColor::update() {
		float dt = 1.0f / 60.0f;//bugbug does this time to frame count? I think so
		ofxAnimatableOfColor::update(dt);
		if (getColorSet()) {//bugbug can we advance color if desired here? 
		}
	}
	void AnimiatedColor::draw() {
		if (useAnimation()) {
			applyCurrentColor();
		}
		else {
			ofSetColor(ofColor::fromHex(getColorSet()->getForeground(), getAlpha()));//background set by background manager
		}
	}
	// all drawing is done using AnimiatedColor, even if no animation is used, color info still stored
	bool AnimiatedColor::readFromScript(const Json::Value &data) {
		if (!data.empty()) {
			READBOOL(usingAnimation, data);
			READFLOAT(alpha, data);
			string colorGroup;
			READSTRING(colorGroup, data);
			if (colorGroup.size()> 0) {
				setColorSet(ColorList::getNextColors(ColorSet::convertStringToGroup(colorGroup), false));
			}
		}

		// set defaults or read from data bugbug add more data reads as needed
		setDuration(0.5f);
		setRepeatType(LOOP_BACK_AND_FORTH);
		setCurve(LINEAR);
		return true;
	}

}