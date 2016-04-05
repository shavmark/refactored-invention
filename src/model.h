#pragma once

#include "2552software.h"
#include "animation.h"
#include "color.h"
#include "draw.h"
#include <forward_list>

// json driven model

namespace Software2552 {
	// model helpers

	void echoValue(const Json::Value &data, bool isError = false);
	bool echoJSONTree(const string& functionname, const Json::Value &root, bool isError = false);
#define ECHOAll(data) echoJSONTree(__FUNCTION__, data);
#define ERROR_ECHOAll(data) echoJSONTree(__FUNCTION__, data, true);
	template<typename T> shared_ptr<vector<shared_ptr<T>>> parse(const Json::Value &data, const string&location);

#if _DEBUG
	template<typename T> void traceVector(T& vec);
#endif // _DEBUG

	// readJsonValue only if value in json, readJsonValue does not support string due to templatle issues
	template<typename T> bool readJsonValue(T &value, const Json::Value& data);
	bool readStringFromJson(string &value, const Json::Value& data);

	// will only readJsonValue vars if there is a string to readJsonValue, ok to preserve existing values
	// always use this macro or the readJsonValue function to be sure errors are handled consitantly
#define READFLOAT(var, data) readJsonValue(var, data[#var])
#define READINT(var, data) readJsonValue(var, data[#var])
#define READBOOL(var, data) readJsonValue(var, data[#var])
#define READSTRING(var, data) readStringFromJson(var, data[#var])
#define READDATE(var, data) readStringFromJson(var, data[#var])

	class Point3D : public ofVec3f {
	public:
		bool readFromScript(const Json::Value &data);
	};
	
	//http://pocoproject.org/slides/070-DateAndTime.pdf
	class DateAndTime : public Poco::DateTime {
	public:
		// default to no no date to avoid the current date being set for bad data
		DateAndTime();
		void operator=(const DateAndTime& rhs);
		const string format = "%dd %H:%M:%S.%i";
		string getDate() {
			return Poco::DateTimeFormatter::format(timestamp(), format);
		}
		bool readFromScript(const Json::Value &data);
	private:
		int timeZoneDifferential;
		int bc; // non zero if its a bc date
	};

	// simple helper to read in font data from json 
#define defaultFontSize 14
#define defaultFontFile "fonts/Raleway-Thin.ttf"
#define defaultFontName "Raleway-Thin"

	//bugbug do we want to expire fonts? maybe in 2.0
	class Font  { 
	public:
		ofTrueTypeFont& get();
		shared_ptr<ofxSmartFont> getPointer();
		bool readFromScript(const Json::Value &data);
	private:
		shared_ptr<ofxSmartFont> font;
	};


	//  settings get copied a lot as they are the default data for all classes so they need to stay small
	class Settings {
	public:
		Settings() {	}
		Settings(const string& nameIn) {name = nameIn;	}
		void operator=(const Settings& rhs) {setSettings(rhs);	}
		bool readFromScript(const Json::Value &data);

		ofTrueTypeFont& getFont() { return font.get(); }
		shared_ptr<ofxSmartFont> getFontPointer() { return font.getPointer(); }
		bool operator==(const Settings& rhs) { return rhs.name == name; }
		string &getName() { return name; }
		virtual void setSettings(const Settings& rhs);
		virtual void setSettings(Settings* rhs);
		
	protected:
		
		shared_ptr<ColorSet> getColor();// drawing using animatedcolor
		Font   font;
		//		colors = std::make_shared<Colors>(ColorSet::ColorGroup::Extreme);// setup colors bugbug get from json
		shared_ptr<ColorSet>  colors=nullptr;
		string notes;// unstructured string of info, can be shown to the user
		string title; // title object
		string name; // any object can have a name, note, date, reference, duration

	private:
	};

	class Dates : public Settings {
	public:
		bool readFromScript(const Json::Value &data);
	protected:
		DateAndTime timelineDate; // date item existed
		DateAndTime lastUpdateDate; // last time object was updated
		DateAndTime itemDate; // date of reference vs the date of the referenced item 

	};
	// reference to a cited item
	class Reference : public Dates {
	public:
		bool readFromScript(const Json::Value &data);

	protected:
		string locationPath; // can be local too
		string location;
		string source;
	};

	// an actor is a drawable that contains the drawing object for that graphic
	class Stage;
	class Actor : public Settings {
	public:
		Actor(ActorRole *p) :Settings() {
			references = nullptr;
			player = p;
		}
		~Actor();

		template<typename T> T* getRole() { return (T*)getDefaultRole(); }
		ActorRole* getDefaultRole() { return player; }
		template<typename T> T& getPlayer() { return *getRole<Role>()->player; } // assume player always allocated
		bool readActorFromScript(const Json::Value &data, Stage*);

		shared_ptr<vector<shared_ptr<Reference>>>  getReferences() { return references; }

		void setAnimation(bool f = true) { if (isValid()) getDefaultRole()->getAnimationHelper()->setAnimationEnabled(f); }
		void setType(ActorRole::drawtype type) { if (getDefaultRole())getDefaultRole()->setType(type); }
		void animateTo(const ofPoint& p) { if (isValid())getDefaultRole()->getAnimationHelper()->animateTo(p); }
		void setAnimationPosition(const ofPoint& p) { if (isValid())getDefaultRole()->getAnimationHelper()->setPosition(p); }
		void setAnimationPositionX(float x) { if (isValid())getDefaultRole()->getAnimationHelper()->setPositionX(x); }
		void setAnimationPositionY(float y) { if (isValid())getDefaultRole()->getAnimationHelper()->setPositionX(y); }
		void setAnimationPositionZ(float z) { if (isValid())getDefaultRole()->getAnimationHelper()->setPositionX(z); }
		shared_ptr<PointAnimation> getAnimation() {		return getDefaultRole()->getAnimationHelper();	}
		void setupActor() { if (getDefaultRole()) getDefaultRole()->setupForDrawing(); }
		void updateActor() { if (getDefaultRole()) getDefaultRole()->updateForDrawing(); }
		void setStage(Stage*s) { stage = s; }
		Stage* getStage() { return stage; }
	private: 
		virtual bool myReadFromScript(const Json::Value &data) { return true; };

		class Stage* stage=nullptr;// where object is to live
		bool isValid() { return getDefaultRole() && getDefaultRole()->getAnimationHelper(); }
		
		ActorRole* player=nullptr; // need a down cast to get specific items
		shared_ptr<vector<shared_ptr<Reference>>> references=nullptr; // research reference to show where actor came from
	};
	class TextureFromImage : public ofTexture {
	public:
		void create(const string& name, float w, float h);
		float getWidth() { return fbo.getWidth(); }
		float getHeight() { return fbo.getHeight(); }
		void bind() { fbo.getTexture().bind(); }
		void unbind() { fbo.getTexture().unbind(); }
		ofTexture& getTexture() { return fbo.getTexture(); }
	private:
		ofFbo fbo;
	};



	// wrap drawing object with references and settings data
	class Ball : public Actor {
	public:
		// bouncy ball with nice colors is pretty nice, does not take too much really
		class Role : public ActorRole {
		public:
			void myDraw();

			int floorLine = 630;
			int xMargin = 0;
			int widthCol = 60;
			float radius = 100;
		};

		// data read during scene creation bugbug move all data reads to scene creation?
		Ball() :Actor(new Role()) {  }
		Role* role() { return getRole<Role>(); }

	private:
		bool myReadFromScript(const Json::Value &data);
	};

	// camera, lights etc
	class EquipementBaseClass : public objectLifeTimeManager {
	public:
		bool readFromScript(const Json::Value &data) { return myReadFromScript(data); }
	private:
		virtual bool myReadFromScript(const Json::Value &data) =0;// for derived classes
	};
	// cameras (and others like it) are not actors
	class Camera : public EquipementBaseClass {
	public:
		void orbit();
		void setOrbit(bool b = true) { useOrbit = b; }
		bool isOrbiting() const { return useOrbit; }
		ofEasyCam& getPlayer() { return player; }
	private:
		ofEasyCam player;
		bool myReadFromScript(const Json::Value &data);
		bool useOrbit = false;
	};
	class Light : public EquipementBaseClass {
	public:
		ofLight &getPlayer() { return player; }
		void setX(float x) { loc.x = x; }
		void setY(float y) { loc.y = y; }
		void setZ(float z) { loc.z = z; }
		void setLoc(float x, float y = 0, float z = 0) { loc.x = x; loc.y = y; loc.z = z; }
		ofPoint loc; // light is set on object within camera scope
	private:
		ofLight player;
		virtual bool myReadFromScript(const Json::Value &data);
	};
	class PointLight : public Light {
	public:
		PointLight() :Light() { getPlayer().setPointLight(); }
	private:
		virtual bool myReadFromScript(const Json::Value &data);
	};
	class DirectionalLight : public Light {
	public:
		DirectionalLight() :Light() { getPlayer().setDirectional(); }
	private:
		virtual bool myReadFromScript(const Json::Value &data);
	};
	class SpotLight : public Light {
	public:
		SpotLight() :Light() { getPlayer().setSpotlight(); }
	private:
		virtual bool myReadFromScript(const Json::Value &data);
	};
	class CameraGrabber : public Actor {
	public:
		class Role : public ActorRole {
		public:
			void myUpdate();
			void myDraw();
			void mySetup() { loadGrabber(w, h); }
			int w = 320;
			int h = 240;
			int x = 0;
			int y = 0;
			ofVideoGrabber player;
		private:
			bool loadGrabber(int wIn, int hIn);
			int find();
			int id = 0;
		};
		CameraGrabber() :Actor(new Role()) {}
	private:
		bool myReadFromScript(const Json::Value &data);
	};
	class ActorWithPrimativeBaseClass;
	class DrawingPrimitive3d : public ActorRole {
	public:
		DrawingPrimitive3d(of3dPrimitive *p, drawtype type = draw3dFixedCamera);
		~DrawingPrimitive3d();

		void setWireframe(bool b = true) { wireFrame = b; }
		bool useWireframe() { return wireFrame; }
		void setFill(bool b = true) { fill = b; }
		bool useFill() { return fill; }

		void myDraw();
		of3dPrimitive* getPlayer() { return player; }
		bool myReadFromScript(const Json::Value &data);
	private:
		virtual bool DerivedMyReadFromScript(const Json::Value &data) { return true; }
		of3dPrimitive *player = nullptr; // allow derived pointers and polymorphism
		bool wireFrame = true;
		bool fill = false;
	};
	class ActorWithPrimativeBaseClass : public Actor {
	public:
		ActorWithPrimativeBaseClass(DrawingPrimitive3d *p = nullptr) :Actor(p){}
		DrawingPrimitive3d *getPrimative() { return (DrawingPrimitive3d*)getDefaultRole();  }
	};
	// do not use templates as its hard to make base class pointers to them
	class Cube : public ActorWithPrimativeBaseClass {
	public:
		class Role : public DrawingPrimitive3d {
		public:
			Role() : DrawingPrimitive3d(new ofBoxPrimitive) {}
		};
		Cube() : ActorWithPrimativeBaseClass(new Role()) {		}
		ofBoxPrimitive* getPlayer() { return (ofBoxPrimitive*)(getRole<Role>()->getPlayer()); }
		Role* role() { return getRole<Role>(); }

	private:
		bool DerivedMyReadFromScript(const Json::Value &data);
	};
	class Plane : public ActorWithPrimativeBaseClass {
	public:
		class Role : public DrawingPrimitive3d {
		public:
			Role() : DrawingPrimitive3d(new ofPlanePrimitive) {}
		};
		Plane() : ActorWithPrimativeBaseClass(new Role()) {		}
		ofPlanePrimitive* getPlayer() { return (ofPlanePrimitive*)(getRole<Role>()->getPlayer()); }
		Role* role() { return getRole<Role>(); }

	private:
		bool DerivedMyReadFromScript(const Json::Value &data);
	};
	class Sphere : public ActorWithPrimativeBaseClass {
	public:
		class Role : public DrawingPrimitive3d {
		public:
			Role() : DrawingPrimitive3d(new ofSpherePrimitive) {}
		};
		Sphere() : ActorWithPrimativeBaseClass(new Role()) {		}
		ofSpherePrimitive* getPlayer() { return (ofSpherePrimitive*)(getRole<Role>()->getPlayer()); }
		Role* role() { return getRole<Role>(); }

	private:
		bool DerivedMyReadFromScript(const Json::Value &data);
	};
	//bugbug add other shapes
	class Text : public Actor {
	public:
		class Role : public ActorRole {
		public:
			void myDraw();
			void drawText(const string &s, int x, int y);
			void setText(const string&t) { text = t; }
			string& getText() { return text; }
			string text;
		};
		Text() : Actor(new Role) {  }
		void drawText(const string &s, int x, int y) { getRole<Role>()->drawText(s, x, y); };
		void setText(const string&t) { getRole<Role>()->setText(t); }
		Role* role() { return getRole<Role>(); }

	private:
		bool myReadFromScript(const Json::Value &data);
	};

	class Paragraph : public Actor {
	public:
		// put advanced drawing in these objects
		class Role : public ActorRole {
		public:
			Role() : ActorRole() {}
			void myDraw();
			ofxParagraph player;
		private:
		};

		Paragraph() :Actor(new Role()) {  }
		ofxParagraph& getPlayer() { return getRole<Role>()->player; }
		Role* role() { return getRole<Role>(); }

	private:
		bool myReadFromScript(const Json::Value &data);
	};

	
	class Audio : public Actor {
	public:
		// sound gets drawing basics for path and possibly other items in the future
		class Role : public ActorRole {
		public:
			//bugbug tie into the main sound code we added
			void mySetup();
			ofSoundPlayer player;
		};

		Audio() :Actor(new Role()) {  }
		ofSoundPlayer& getPlayer() { return getRole<Role>()->player; }
	private:
		bool myReadFromScript(const Json::Value &data);
	};

	class Visual : public ActorRole {
	public:
		void setFullSize();
	protected:
		bool isLoaded = false;
		bool fullsize = false; // keep full size of screen, like for a background
	};

	class Video : public Actor {
	public:
		// put advanced drawing in these objects
		class Role :public Visual {
		public:
			void myUpdate();
			void myDraw();
			void mySetup();
			float getTimeBeforeStart(float t);
			ofVideoPlayer player;
		};
		Video() :Actor(new Role()) {  }
		Role* role() { return getRole<Role>(); }
		ofVideoPlayer& getPlayer() { return getRole<Role>()->player; }
	private:
		bool myReadFromScript(const Json::Value &data);
	};

	class TextureVideo : public Actor {
	public:
		class Role : public ActorRole {
		public:
			Role() : ActorRole() {  }
			Role(const string& path) : ActorRole(path) { }
			void myUpdate() { player.update(); }
			void myDraw();
			void mySetup();
			bool textureReady() { return  player.isInitialized(); }
			bool mybind();
			bool myunbind();
			ofVideoPlayer player;
			ofTexture& getTexture();
			ofFbo fbo;
		private:
			ofTexture defaulttexture;
		};
		TextureVideo(const string&s) :Actor(new Role(s)) {  }
		TextureVideo() :Actor(new Role()) {  }
		ofVideoPlayer& getPlayer() { return getRole<Role>()->player; }
		ofTexture& getTexture() { return getRole<Role>()->getTexture(); }
		Role* role() { return getRole<Role>(); }

	private:
		bool myReadFromScript(const Json::Value &data);

	};
	class VideoSphere : public Actor {
	public:
		class Role : public ActorRole {
		public:
			friend VideoSphere;
			Role() : ActorRole() {	}
			Role(const string& path) : ActorRole(path) { video = std::make_shared<TextureVideo>(path); }
			void myDraw();
			Sphere sphere;
			shared_ptr<TextureVideo> video = nullptr;
		private:
			bool set = false;
		};
		VideoSphere(const string&s) :Actor(new Role(s)) {		}
		VideoSphere() :Actor(new Role()) {  }
		void setSettings(const Settings& rhs);
		Sphere& getSphere() { return getRole<Role>()->sphere; }
		Role* role() { return getRole<Role>(); }

	private:
		bool myReadFromScript(const Json::Value &data);
	};

	class Planet : public Actor {
	public:
		class Role : public ActorRole {
		public:
			Role() : ActorRole() { texture = std::make_shared<TextureFromImage>(); }
			void myDraw();
			Sphere sphere;
			shared_ptr<TextureFromImage> getTexturePtr() { return texture; }
		private:
			shared_ptr<TextureFromImage> texture = nullptr;
		};
		Planet() :Actor(new Role()) {  }
		Sphere& getSphere() { return getRole<Role>()->sphere; }
		Role* role() { return getRole<Role>(); }

	private:
		bool myReadFromScript(const Json::Value &data);
	};

	class SolarSystem : public Actor {
	public:
		class Role : public ActorRole {
		public:
		};
		SolarSystem() :Actor(new Role()) {  }
		void addPlanets(const Json::Value &data, ofPoint& min, Settings& settings);

	private:
		bool myReadFromScript(const Json::Value &data);
	};


	class Picture : public Actor {
	public:
		class Role : public Visual {
		public:
			void myUpdate();
			void mySetup();
			void myDraw();
			ofImage player;
		};

		Picture() :Actor(new Role()) {  }
		Role* role() { return getRole<Role>(); }
		ofImage& getPlayer() { return getRole<Role>()->player; }
	private:
		bool myReadFromScript(const Json::Value &data);
	};
	
	class Background : public Actor {
	public:
		enum TypeOfBackground { ColorFixed, ColorChanging,  none };
		enum TypeOfGradient {	linear = ofGradientMode::OF_GRADIENT_LINEAR,	circular = ofGradientMode::OF_GRADIENT_CIRCULAR,
			bar = ofGradientMode::OF_GRADIENT_BAR,	flat, noGradient	};
		class Role : public ActorRole {
		public:
			void myDraw();
			void myUpdate();// make image a vector then rotate via animation
			void setType(TypeOfBackground typeIn = ColorFixed) { type = typeIn; }
			void setGradientMode(const TypeOfGradient& modeIn) { mode = modeIn; }
			ofGradientMode ofMode;
		private:
			TypeOfGradient mode = TypeOfGradient::noGradient;
			TypeOfBackground type = ColorFixed;
		};
		Background() :Actor(new Role()) {  }
		Role* role() { return getRole<Role>(); }

	private:
		bool myReadFromScript(const Json::Value &data);

	};

	// item in a play list

	class Stage;
	class Channel : public objectLifeTimeManager {
	public:
		Channel() :objectLifeTimeManager() {  }
		Channel(const string&keynameIn) { keyname = keynameIn;  }
		enum ChannelType{History, Art, Sports, Any};
		
		bool readFromScript(const Json::Value &data);
		bool operator==(const Channel rhs) { return rhs.keyname == keyname; }
		string &getKeyName() { return keyname; }
		shared_ptr<Stage> getStage() { return stage; }
		void setStage(shared_ptr<Stage>p) { stage = p; }
		bool getSkip() { return skip; }
		void setSkip(bool b = true) { skip = b; }

		ChannelType getType() { return channeltype; }
		void setType(ChannelType typeIn) { channeltype =typeIn; }
	private:
		shared_ptr<Stage> stage = nullptr;
		string keyname;
		bool skip = false;
		ChannelType channeltype = History;
	};
	class ChannelList {
	public:
		shared_ptr<Channel> getCurrent();
		shared_ptr<Channel> getbyType(Channel::ChannelType, int number = 0);
		shared_ptr<Channel> getbyName(const string&name);
		shared_ptr<Channel> getbyNumber(int i = 0);

		bool readFromScript(const Json::Value &data);
		void removeExpiredItems() {
			list.erase(std::remove_if(list.begin(), list.end(), objectLifeTimeManager::OKToRemove), list.end());
		}
		bool read(const string&path);
		bool setStage(shared_ptr<Stage> p);
		bool skipChannel(const string&keyname);
		vector<shared_ptr<Channel>>& getList();
	private:
		vector<shared_ptr<Channel>> list;
	};

}