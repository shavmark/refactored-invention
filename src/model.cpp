#include "2552software.h"
#include "scenes.h"
#include "model.h"
#include "animation.h"
#include <algorithm>

// maps json to drawing and animation tools

namespace Software2552 {
	// data read in as a % of screen x,y; z is not converted
	void Point3D::convert(float xpercent, float ypercent, float zpercent) {
		x = (float)ofGetWidth() * xpercent;
		y = (float)ofGetWidth() * ypercent;
		z = zpercent;//not sure how to do this yet
	}
	bool Point3D::setup(const Json::Value &data) {
		float x=0.0f, y=00.0f, z=00.0f;
		if (data.size() > 0) {
			READFLOAT(x, data);
			READFLOAT(y, data);
			READFLOAT(z, data);
			convert(x*0.01f, y*0.01f, z*0.01f);
		}
		return true;
	}

	vector<shared_ptr<Channel>>& ChannelList::getList() {
		return list;
	}
	// since list is maintained 0 is always current
	shared_ptr<Channel> ChannelList::getCurrent() {
		// find first non skipped channel
		for (const auto& channel : list) {
			if (!channel->getSkip()){
				return channel;
			}
		}
		return nullptr;
	}
	shared_ptr<Channel> ChannelList::getbyNumber(int i) {
		if (i < list.size() && i > 0) {
			return list[i];
		}
		return nullptr;
	}

	// first channel of type 
	shared_ptr<Channel> ChannelList::getbyType(Channel::ChannelType type, int number) {
		// find first non skipped channel
		int count = 0;
		for (const auto& channel : list) {
			if (!channel->getSkip() && channel->getType() == type) {
				if (count == number) {
					return channel;
				}
				++count;
			}
		}
		return nullptr;
	}
	shared_ptr<Channel> ChannelList::getbyName(const string&name) {
		// find first non skipped channel
		for (const auto& channel : list) {
			if (!channel->getSkip() && channel->getKeyName() == name) {
				return channel;
			}
		}
		return nullptr;
	}

	bool ChannelList::setup(const Json::Value &data) {

		for (Json::ArrayIndex j = 0; j < data.size(); ++j) {
			shared_ptr<Channel> channel = std::make_shared<Channel>();
			channel->setup(data[j]);
			if (!channel->getSkip()) {
				list.push_back(channel);
			}
		}
		return true;
	}



	//bugbug todo weave into errors, even on release mode as anyone can break a json file
	void echoValue(const Json::Value &data, bool isError) {
		//Json::Value::Members m = data.getMemberNames();
		try {
			if (data.isString()) {
				tracer("type=string, value=\"" + data.asString() + "\">", isError);
			}
			else if (data.isBool()) {
				tracer("type=bool, value=" + ofToString(data.asBool()) + ">", isError);
			}
			else if (data.isInt()) {
				tracer("type=int, value=" + ofToString(data.asInt()) + ">", isError);
			}
			else if (data.isDouble()) {
				tracer("type=float, value=" + ofToString(data.asDouble()) + ">", isError);
			}
			else if (data.isArray()) {
				tracer("type=array>", isError);
			}
			else if (data.isObject()) {
				tracer("type=objectvalue(name/value pair), value=" + ofToString(data.asString()) + ">", isError);
			}
			else if (data.isNull()) {
				tracer("type=null>", isError);
			}
			else {
				tracer("type=unsupported, type is " + ofToString(data.type()) + ">", isError);
			}
		}
		catch (std::exception e) {
			logErrorString(e.what());
		}


	}
	bool echoJSONTree(const string& functionname, const Json::Value &root, bool isError)
	{
		tracer("<Parse name=\"" + functionname + "\">", isError); // kick it back as xml, easier to read by a human? 

		if (root.size() > 0) {

			for (Json::ValueIterator itr = root.begin(); itr != root.end(); itr++) {
				string member = itr.memberName();
				tracer("<subvalue name=\"" + member + "\">", isError);
				echoValue(itr.key());
				tracer("</subvalue>", isError);
				echoJSONTree(functionname, *itr, isError);
			}
			return true;
		}
		else {
			echoValue(root);
		}
		tracer("</Parse>", isError);
		return true;
	}

#if _DEBUG
	template<typename T> void traceVector(T& vec) {
		for (auto a : vec) {
			a->trace();
		}
	}
#endif // _DEBUG

	// get a string from json
	bool readStringFromJson(string &value, const Json::Value& data) {
		if (readJsonValue(value, data)) {
			value = data.asString();
			return true;
		}
		return false;
	}
	template<typename T> bool readJsonValue(T &value, const Json::Value& data) {
		try {
			if (!data.empty()) {
				switch (data.type()) {
				case Json::nullValue:
					logTrace("nullValue");
					break;
				case Json::booleanValue:
					value = data.asBool();
					break;
				case Json::stringValue:
					return true; // templates get confused to readJsonValue the string else where, use this to validate
					break;
				case Json::intValue:
					value = data.asInt();
					break;
				case Json::realValue:
					value = data.asFloat();
					break;
				case Json::objectValue:
					logErrorString("objectValue called at wrong time");
					break;
				case Json::arrayValue:
					logErrorString("arrayValue called at wrong time");
					break;
				default:
					// ignore?
					logErrorString("value not found");
					break;
				}
				return true;
			}
		}
		catch (std::exception e) {
			echoJSONTree(__FUNCTION__, data, true);
			logErrorString(e.what());
		}
		return false;
	}
	DateAndTime::DateAndTime() : Poco::DateTime(0, 1, 1) {
		timeZoneDifferential = 0;
		bc = 0;
	}
	void DateAndTime::operator=(const DateAndTime& rhs) {
		timeZoneDifferential = rhs.timeZoneDifferential;
		bc = rhs.bc;
		assign(rhs.year(), rhs.month(), rhs.day(), rhs.hour(), rhs.minute(), rhs.second(), rhs.microsecond(), rhs.microsecond());
	}
	bool Ball::mysetup(const Json::Value &data) {
		// can read any of these items from json here
		readJsonValue(radius, data["radius"]);
		return true;
	}
	bool Channel::setup(const Json::Value &data) {
		READSTRING(keyname, data);
		float lifetime=0;
		READFLOAT(lifetime, data);
		setObjectLifetime(lifetime);
		READBOOL(skip, data);
		string type= "any";
		readStringFromJson(type, data["channelType"]);
		if (type == "history") {
			setType(History);
		}
		else if (type == "sports") {
			setType(Sports);
		}
		else  {
			setType(Any);
		}
		return true;
	}

	bool Dates::setup(const Json::Value &data) {
		timelineDate.setup(data["timelineDate"]); // date item existed
		lastUpdateDate.setup(data["lastUpdateDate"]); // last time object was updated
		itemDate.setup(data["itemDate"]);
		return true;
	}

	bool Reference::setup(const Json::Value &data) {
		Dates::setup(data["dates"]);
		if (Dates::setup(data)) { // ignore reference as an array or w/o data at this point
			// no base class so it repeats some data in base class ReferencedItem
			READSTRING(location, data[STRINGIFY(Reference)]);
			READSTRING(locationPath, data[STRINGIFY(Reference)]);
			READSTRING(source, data[STRINGIFY(Reference)]);
			return true;
		}
		return false;
	}

	bool DateAndTime::setup(const Json::Value &data) {

		if (READINT(bc, data)) {
			return true;
		}

		string str; // scratch varible, enables either string or ints to pull a date
		if (READSTRING(str, data)) {
			if (!Poco::DateTimeParser::tryParse(str, *this, timeZoneDifferential)) {
				logErrorString("invalid AD date " + str);
				return false;
			}
			makeUTC(timeZoneDifferential);
		}

		return true;
	}
	
	bool Text::mysetup(const Json::Value &data) {
		readStringFromJson(getText(), data["text"]["str"]);
		return true;
	}

	// return true if text read in
	bool Paragraph::mysetup(const Json::Value &data) {

		string str;
		readStringFromJson(str, data["text"]["str"]);
		worker.setText(str);

		int indent;
		int leading;
		int spacing;
		string alignment; // paragraph is a data type in this usage

		if (READINT(indent, data)) {
			worker.setIndent(indent);
		}
		if (READINT(leading, data)) {
			worker.setLeading(leading);
		}
		if (READINT(spacing, data)) {
			worker.setSpacing(spacing);
		}
		READSTRING(alignment, data);
		if (alignment == "center") { //bugbug ignore case
			worker.setAlignment(ofxParagraph::ALIGN_CENTER);
		}
		else if (alignment == "right") { //bugbug ignore case
			worker.setAlignment(ofxParagraph::ALIGN_RIGHT);
		}

		worker.setFont(getFontPointer());
		
		// object holds it own color bugbug maybe just set current color right before draw?
		worker.setColor(colorHelper->getForeground());

		return true;
	}

	//, "carride.mp4"
	bool Video::mysetup(const Json::Value &data) {
		setType(ActorRole::draw2d);
		setAnimationEnabled(true);
		float speed = 0;
		READFLOAT(speed, data);
		if (speed != 0) {
			worker.setSpeed(speed);
		}
		float volume=1;//default
		READFLOAT(volume, data);
		worker.setVolume(volume);
		return true;
	}
	bool Audio::mysetup(const Json::Value &data) {
		float volume = 1;//default
		READFLOAT(volume, data);
		worker.setVolume(volume);
		worker.setMultiPlay(true);
		worker.setSpeed(ofRandom(0.8, 1.2));// get from data

		return true;
	}
	void Camera::orbit() {
		if (useOrbit) {
			float time = ofGetElapsedTimef();
			float longitude = 10 * time;
			float latitude = 10 * sin(time*0.8);
			float radius = 600 + 50 * sin(time*0.4);
			worker.orbit(longitude, latitude, radius, ofPoint(0, 0, 0));
		}
	}
	
	bool Camera::setup(const Json::Value &data) {
		//bugbug fill in
		setOrbit(false); // not rotating
		//player.setScale(-1, -1, 1); // showing video
		worker.setPosition(ofRandom(-100, 200), ofRandom(60, 70), ofRandom(600, 700));
		worker.setFov(60);
		return true;
	}
	void Material::begin() {
		// the light highlight of the material  
		if (colorHelper){
			setSpecularColor(colorHelper->getForeground());
		}
		//setSpecularColor(ofColor(255, 255, 255, 255)); // does white work wonders? or what does color do?
		ofMaterial::begin();
	}

	bool Material::setup(const Json::Value &data) {
		// shininess is a value between 0 - 128, 128 being the most shiny // 
		float shininess = 90;
		READFLOAT(shininess, data);
		setShininess(shininess);
		colorHelper = parseColor(data["colorAnimation"]);

		return true;
	}
	bool Light::setup(const Json::Value &data) {
		//bugbug fill in as an option, use Settings for color, or the defaults
		//get from json player.setDiffuseColor(ofColor(0.f, 255.f, 0.f));
		// specular color, the highlight/shininess color //
		//get from json player.setSpecularColor(ofColor(255.f, 0, 0));
		//could get from json? not sure yet getAnimationHelper()->setPositionX(ofGetWidth()*.2);
		setLoc(ofRandom(-100,100), 0, ofRandom(300,500));
		// help http://www.glprogramming.com/red/chapter05.html
		colorHelper = parseColor(data["colorAnimation"]);
		if (colorHelper) {
			worker.setDiffuseColor(colorHelper->getLightest());
			worker.setSpecularColor(colorHelper->getDarkest());
			worker.setAmbientColor(colorHelper->getBackground());
		}
		//bugbug what about alpha?
		return mysetup(data);
	}
	bool PointLight::mysetup(const Json::Value &data) {
		//bugbug fill in as an option, use Settings for color, or the defaults
		//get from json player.setDiffuseColor(ofColor(0.f, 255.f, 0.f));
		// specular color, the highlight/shininess color //
		//get from json player.setSpecularColor(ofColor(255.f, 0, 0));
		// specular color, the highlight/shininess color //
		setLoc(ofRandom(ofGetWidth()*.2, ofGetWidth()*.4), ofRandom(ofGetHeight()*.2, ofGetHeight()*.4), ofRandom(500,700));
		//could get from json? not sure yet getAnimationHelper()->setPositionY(ofGetHeight()*.2);
		return true;
	}
	bool DirectionalLight::mysetup(const Json::Value &data) {
		//bugbug fill in as an option, use Settings for color, or the defaults
		//get from json player.setDiffuseColor(ofColor(0.f, 255.f, 0.f));
		// specular color, the highlight/shininess color //
		//get from json player.setSpecularColor(ofColor(255.f, 0, 0));
		worker.setOrientation(ofVec3f(0, 90, 0));
		setLoc(ofGetWidth() / 2, ofGetHeight() / 2, 260);
		return true;
	}
	bool SpotLight::mysetup(const Json::Value &data) {
		//bugbug fill in as an option, use Settings for color, or the defaults
		//get from json player.setDiffuseColor(ofColor(0.f, 255.f, 0.f));
		// specular color, the highlight/shininess color //
		//get from json player.setSpecularColor(ofColor(255.f, 0, 0));
		//could get from json? not sure yet getAnimationHelper()->setPositionX(ofGetWidth()*.2);
		//could get from json? not sure yet getAnimationHelper()->setPositionY(ofGetHeight()*.2);
		//directionalLight->player.setOrientation(ofVec3f(0, 90, 0));
		setLoc(ofGetWidth()*.1, ofGetHeight()*.1, 220);
		// size of the cone of emitted light, angle between light axis and side of cone //
		// angle range between 0 - 90 in degrees //
		worker.setSpotlightCutOff(50);

		// rate of falloff, illumitation decreases as the angle from the cone axis increases //
		// range 0 - 128, zero is even illumination, 128 is max falloff //
		worker.setSpotConcentration(2);
		setLoc(-ofGetWidth()*.1, ofGetHeight()*.1, 100);
		return true;
	}
	void Ball::myDraw() {
		
		float y = getCurrentPosition().y;
		ofCircle((2 * ofGetFrameNum()) % ofGetWidth(), y, radius*scale());

	}
	bool Arrow::mysetup(const Json::Value &data) {
		end.x = ofGetWidth() / 2;
		end.z = 600;
		if (data.size() > 0) {
			start.setup(data);
			end.setup(data);
			READFLOAT(headSize, data);
		}
		return true;
	}

	void Text::myDraw() {
		//bugbug add in some animation
		drawText(text, getCurrentPosition().x, getCurrentPosition().y);
	}

	void Text::drawText(const string &s, int x, int y) {
		FontHelper font;
		font.get()->drawString(s, x, y);
	}

	bool Plane::derivedMysetup(const Json::Value &data) {
		return true;
	}
	DrawingPrimitive3d::~DrawingPrimitive3d() { 
		if (worker) {
			delete worker;
			worker = nullptr;
		}
	}

	DrawingPrimitive3d::DrawingPrimitive3d(of3dPrimitive *p, drawtype type) : ActorRole() {
		worker = p;
		setType(type); 
		if (worker) {
			worker->enableColors();
		}
	}
	
	bool DrawingPrimitive3d::mysetup(const Json::Value &data) {
		///ofPolyRenderMode renderType = OF_MESH_WIREFRAME; //bugbug enable phase II
		bool wireFrame = true;
		READBOOL(wireFrame, data);
		setWireframe(wireFrame);
		// pass on current animation
		material.colorHelper = colorHelper;
		material.setup(data);
		return derivedMysetup(data);
	}
	void DrawingPrimitive3d::myUpdate() {
		if (get()) {
			get()->setPosition(getCurrentPosition());
		}
	}
	// private draw helper
	void DrawingPrimitive3d::basicDraw() {
		if (get() && worker) {
			//worker->setScale(ofRandom(100));
			worker->rotate(180, 0, 1, 0.0);
			if (useWireframe()) {
				ofPushMatrix();
				worker->setScale(1.01f);
				worker->drawWireframe();
				worker->setScale(1.f);
				ofPopMatrix();
			}
			else {
				worker->draw();
			}
		}
	}
	// assumes push/pop handled by caller
	void DrawingPrimitive3d::myDraw() {
		ofSetColor(0, 0, 0);//color comes from the light
		material.begin();
		if (!useFill()) {
			ofNoFill();
		}
		else {
			ofFill();
		}
		basicDraw();
		material.end();
	}

	bool Cube::derivedMysetup(const Json::Value &data) {
		float size = 100;//default
		READFLOAT(size, data);
		if (get()) {
			setWireframe(true);
			get()->set(size);
			get()->roll(20.0f);// just as an example
		}
		return true;
	}
	bool Cylinder::derivedMysetup(const Json::Value &data) {
		return true;
	}
	bool Cone::derivedMysetup(const Json::Value &data) {
		return true;
	}
	bool Sphere::derivedMysetup(const Json::Value &data) {
		if (get()) {
			float radius = 100;//default
			READFLOAT(radius, data);
			get()->setRadius(radius);

			float resolution = 100;//default
			READFLOAT(resolution, data);
			get()->setResolution(resolution);

			// can be moving too, let json decide, need camera too
			setType(ActorRole::draw3dFixedCamera);
			setFill();
			get()->setMode(OF_PRIMITIVE_TRIANGLES);
		}
		return true;
	}
	bool Background::mysetup(const Json::Value &data) {

		string type;
		readStringFromJson(type, data["colortype"]);
		if (type == "fixed") {
			setType(ColorFixed);
		}
		else if (type == "changing") {
			setType(ColorChanging);
		}
		else {
			setType(none);
		}
		type = "";
		readStringFromJson(type, data["gradient"]);
		if (type == "linear") {
			ofMode = OF_GRADIENT_LINEAR;
			setGradientMode(linear);
		}
		else if (type == "bar") {
			ofMode = OF_GRADIENT_BAR;
			setGradientMode(bar);
		}
		else if (type == "circular") {
			ofMode = OF_GRADIENT_CIRCULAR;
			setGradientMode(circular);
		}
		else if (type == "flat") {
			setGradientMode(flat);
		}
		else {
			setGradientMode(noGradient);
		}
		
		//bugbug finish this 
		setRefreshRate(60000);// just set something different while in dev

		if (!data["image"].empty()) {
			shared_ptr<Picture> p = getStage()->CreateReadAndaddAnimatable<Picture>(data["image"], true);
			if (p) {
				p->setFullSize();
			}
		}
		
		if (!data["video"].empty()) {
			shared_ptr<Video> p = getStage()->CreateReadAndaddAnimatable<Video>(data["video"], true);
			if (p) {
				p->setFullSize();
			}
		}

		if (!data["rainbow"].empty()) {
			getStage()->CreateReadAndaddAnimatable<Rainbow>(data["rainbow"], true);
		}
		return true;
	}
	void Visual::myUpdate() {
		if (fullsize) {
			w = ofGetWidth();
			h = ofGetHeight();
			ofPoint pt;// upper left in a centered world
			pt.x = -ofGetWidth() / 2;
			pt.y = -ofGetHeight() / 2;
			setPosition(pt);
		}
	}
	void Visual::setFullSize() {
		fullsize = true;
		setAnimationEnabled(false);
		//bugbug get this somewhere setSpeed(0.25f);

	}
	void Background::myDraw() {
		if (colorHelper) {
			if (mode == flat) {
				// just a plane background
				ofBackground(colorHelper->getBackground());
			}
			else if (mode != noGradient) {
				ofBackgroundGradient(colorHelper->getLightest(), colorHelper->getDarkest(), ofMode);
			}
			if (type == none) {
				return;
			}
			// set by default since this is set first other usage of fore color will override
			ofSetColor(colorHelper->getForeground());
		}
	}
	bool Rainbow::mysetup(const Json::Value &data) {
		return true;
	}
	void Rainbow::myUpdate()	{
		if (w != ofGetWidth() && h != ofGetHeight()) {
			w = ofGetWidth();
			h = ofGetHeight();

			worker.allocate(w, h, OF_IMAGE_COLOR);

			for (float y = 0; y<h; y++) {
				for (float x = 0; x<w; x++) {

					float hue = x / w * 255;
					float sat = ofMap(y, 0, h / 2, 0, 255, true);
					float bri = ofMap(y, h / 2, h, 255, 0, true);

					worker.setColor(x, y, ofColor::fromHsb(hue, sat, bri));
				}
			}
			worker.update();
		}
	}
	void Rainbow::myDraw() {
		ofSetColor(ofColor::white);
		worker.draw(0, 0);
	}
	// colors and background change over time but not at the same time
	void Background::myUpdate() {
		if (type == ColorChanging && refreshAnimation()) {
			if (mode != noGradient) {
				//bugbug test out refreshAnimation
				switch ((int)ofRandom(0, 3)) {
				case 0:
					ofMode = OF_GRADIENT_LINEAR;
					break;
				case 1:
					ofMode = OF_GRADIENT_CIRCULAR;
					break;
				case 2:
					ofMode = OF_GRADIENT_BAR;
					break;
				}
			}
		}

	}
	void Paragraph::myDraw() {
		worker.setPosition(getCurrentPosition().x, getCurrentPosition().y);
		worker.draw();
	}
	bool ChannelList::skipChannel(const string&keyname) {
		for (auto& item : list) {
			if (item->getKeyName() == keyname) {
				return item->getSkip();
			}
		}
		return true;
	}
	// match the keynames 
	bool ChannelList::setStage(shared_ptr<Stage> p) {
		if (p != nullptr) {
			for (auto& item : list) {
				if (item->getKeyName() == p->getKeyName()) {
					item->setStage(p);
					return true;
				}
			}
		}
		return false;
	}
	bool ChannelList::read(const string&path) {
		ofxJSON json;
		
		logTrace("parse " + path);

		if (!json.open(path)) {
			logErrorString("Failed to parse JSON " + path);
			return false;
		}
		try {

			setup(json["channelList"]);
			if (getList().size() == 0) {
				logErrorString("missing channelList");
				return false;
			}

			// read all the scenes
			for (Json::ArrayIndex i = 0; i < json["scenes"].size(); ++i) {
				logTrace("create look json[scenes][" + ofToString(i) + "][keyname]");
				string sceneType;
				if (readStringFromJson(sceneType, json["scenes"][i]["sceneType"])) {
					shared_ptr<Stage> p = getScene(sceneType);
					// read common items here
					//p->settings.setup(json["scenes"][i]["settings"]);
					readStringFromJson(p->getKeyName(), json["scenes"][i]["keyname"]);
					if (skipChannel(p->getKeyName())) {
						continue;
					}
					if (p->getKeyName() == "ClydeBellecourt") {
						int i = 1; // just for debugging
					}
					// save with right playitem
					if (p->setup(json["scenes"][i])) {
						// find stage and set it
						if (!setStage(p)) {
							logTrace("scene not in playlist (ignored) " + p->getKeyName());
						}
					}
				}
			}
			// remove unattached stages, user forgot them in the input file
			std::vector<shared_ptr<Channel>>::const_iterator iter = list.begin();
			while (iter != list.end())	{
				if ((*iter)->getStage() == nullptr) {
					iter = list.erase(iter);
					logTrace("item in playlist not found in json (ignored) " + (*iter)->getKeyName());
				}
				else {
					++iter;
				}
			}
		}
		catch (std::exception e) {
			logErrorString(e.what());
			return false;
		}
		return true;
	}

	// add this one http://clab.concordia.ca/?page_id=944
	void Video::myDraw() {
		ofPoint& pt = getCurrentPosition();
		if (w == 0 || h == 0) {
			worker.draw(pt.x, pt.y);
		}
		else {
			worker.draw(pt.x, pt.y, w, h);
		}
	}
	void Video::mySetup() {
		string debug = getLocationPath();
		if (!isLoaded) {
			if (!worker.load(getLocationPath())) {
				logErrorString("setup video Player");
			}
			isLoaded = true; // avoid keep on trying
		}
		worker.play();

	}
	void Video::myUpdate() {
		Visual::myUpdate();
		worker.update();
	}

	void Picture::myUpdate() {
		Visual::myUpdate();
		worker.update();
	}

	void Picture::mySetup() { 
		if (!isLoaded) {
			if (!ofLoadImage(worker, getLocationPath())) {
				logErrorString("setup Picture Player");
			}
			isLoaded = true; // avoid keep on trying
		}
	}

	float Video::getTimeBeforeStart(float t) {

		// if json sets a wait use it
		if (getWait() > 0) {
			setIfGreater(t, getWait());
		}
		else {
			// will need to load it now to get the true lenght
			if (!worker.isLoaded()) {
				worker.load(getLocationPath());
			}
			float duration = getObjectLifetime();
			setIfGreater(t, duration);
		}
		return t;
	}
	void Picture::myDraw() {
		ofPoint& pt = getCurrentPosition();
		if (w == 0 || h == 0) {
			worker.draw(pt.x, pt.y);
		}
		else {
			worker.draw(pt.x, pt.y, w, h);
		}
	}
	void Audio::mySetup() {
		if (!worker.load(getLocationPath())) {
			logErrorString("setup audio Player");
		}
		// some of this data could come from data in the future
		worker.play();
	}
	int CameraGrabber::find() {
		//bugbug does Kintect show up?
		vector<ofVideoDevice> devices = worker.listDevices();
		for (vector<ofVideoDevice>::const_iterator it = devices.begin(); it != devices.end(); ++it) {
			if (it->deviceName == getLocationPath()) {
				return it->id;
			}
		}
		return 0;// try first found as a default
	}
	void CameraGrabber::myUpdate() { 
		if (worker.isInitialized()) {
			worker.update();
		}
	}

	bool CameraGrabber::loadGrabber(int wIn, int hIn) {
		id = find();
		worker.setDeviceID(id);
		worker.setDesiredFrameRate(30);
		bool b = worker.initGrabber(wIn, hIn);
		return b;
	}

	void CameraGrabber::myDraw() {
		if (worker.isInitialized()) {
			worker.draw(getCurrentPosition().x, getCurrentPosition().y);
		}
	}
	bool CameraGrabber::mysetup(const Json::Value &data) {
		//"Logitech HD Pro Webcam C920"

		return true;
	}
	bool Picture::mysetup(const Json::Value &data) { 
		setType(ActorRole::draw2d);
		return true;
	}
	void TextureVideo::myDraw() {
		return; // not using fbo for video bugbug clean this up;

	}
	void TextureVideo::mySetup() {
	}
	bool TextureVideo::mybind() {
		if (worker.isInitialized() && fbo.isUsingTexture()) {
			worker.getTexture().bind();
			return true;
		}
		return false;
	}
	bool TextureVideo::myunbind() {
		if (worker.isInitialized() && worker.isUsingTexture()) {
			worker.getTexture().unbind();
			return true;
		}
		return false;
	}
	bool TextureVideo::mysetup(const Json::Value &data) {
		//bugbug fill this in with json reads as needed
		if (!worker.isLoaded()) {
			if (!worker.load(getLocationPath())) {
				logErrorString("setup TextureVideo Player");
				return false;
			}
			worker.play();
		}
		return true;
	}
	ofTexture& TextureVideo::getTexture() { 
		return worker.getTexture();
	}

	void VideoSphere::myDraw() {
		//bugbug just need to do this one time, maybe set a flag
		if (video->getTexture().isAllocated() && !set) {
			sphere.get()->mapTexCoordsFromTexture(video->getTexture());
			sphere.get()->rotate(180, 0, 1, 0.0);
			set = true;
		}
		video->mybind();
		getSphere().myDraw();
		video->myunbind();
	}
	bool VideoSphere::mysetup(const Json::Value &data) {

		setType(ActorRole::draw3dFixedCamera);
		video = getStage()->CreateReadAndaddAnimatable<TextureVideo>(data);
		getSphere().setWireframe(false);
		getSphere().get()->set(250, 180);// set default
		return true;
	}
	void TextureFromImage::create(const string& name, float w, float h) {
		// create texture
		ofLoadImage(*this, name);
		fbo.allocate(w, h, GL_RGB);
		fbo.begin();//fbo does drawing
		ofSetColor(ofColor::white); // no image color change when using white
		draw(0, 0, w, h);
		fbo.end();// normal drawing resumes
	}

	bool SolarSystem::mysetup(const Json::Value &data) {
		shared_ptr<VideoSphere> p = getStage()->CreateReadAndaddAnimatable<VideoSphere>(data);
 		if (p) {
			ofPoint min(p->getSphere().get()->getRadius() * 2, 0, 200);
			addPlanets(data["Planets"], min);
			return true;
		}
		//bugbug if there are no camareas a this point add in two defaults, one fixed and the other moving
		// do the same for VideoSphere, post a trace if this is done
		return false;
	}
	void SolarSystem::addPlanets(const Json::Value &data, ofPoint& min) {
		ofPoint point;
		for (Json::ArrayIndex j = 0; j < data.size(); ++j) {
			shared_ptr<Planet> p = getStage()->CreateReadAndaddAnimatable<Planet>(data);
			if (p) {
				point.x = ofRandom(min.x + point.x*1.2, point.x * 2.8);
				point.y = ofRandom(min.y, 500);
				point.z = ofRandom(min.z, 500);
				p->getSphere().get()->setPosition(point); // data stored as pointer so this updates the list
			}
		}
	}

	bool Planet::mysetup(const Json::Value &data) {
		setType(ActorRole::draw3dMovingCamera);
		getSphere().setWireframe(false);
		float r = ofRandom(5, 100);
		getSphere().get()->set(r, 40);
		//bugbug could get sphere location here

		getTexturePtr()->create(getLocationPath(), r * 2, r * 2);

		getSphere().get()->mapTexCoordsFromTexture(getTexturePtr()->getTexture());
		return true;
	}
	void Planet::myDraw() {
		getTexturePtr()->bind();
		sphere.get()->rotate(30, 0, 2.0, 0.0);
		sphere.myDraw();
		getTexturePtr()->unbind();
	}
}