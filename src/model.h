#pragma once

#include "2552software.h"
#include "animation.h"
#include "draw.h"
#include "kinect.h"
#include <forward_list>

// json driven model

namespace Software2552 {
	// model helpers

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

	class Point3D : public ofPoint {
	public:
		bool setup(const Json::Value &data);
	private:
		void convert(float, float, float); // convert to real locations
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
		bool setup(const Json::Value &data);
	private:
		int timeZoneDifferential;
		int bc; // non zero if its a bc date
	};

	class Dates {
	public:
		bool setup(const Json::Value &data);
	protected:
		DateAndTime timelineDate; // date item existed
		DateAndTime lastUpdateDate; // last time object was updated
		DateAndTime itemDate; // date of reference vs the date of the referenced item 

	};
	// reference to a cited item
	class Reference : public Dates {
	public:
		bool setup(const Json::Value &data);

	protected:
		string locationPath; // can be local too
		string location;
		string source;
	};

	class TextureFromImage : public ofTexture {
	public:
		void create(const string& name, float w, float h);
		float getWidth() { return fbo.getWidth(); }
		float getHeight() { return fbo.getHeight(); }
		void bind() { fbo.getTexture().bind(); }
		void unbind() { fbo.getTexture().unbind(); }
		ofTexture& getImageTexture() { return fbo.getTexture(); }
	private:
		ofFbo fbo;
	};
	class Rectangle : public ActorRole {
	public:
		void myDraw();
	private:
		int width;
		int height;
		bool mysetup(const Json::Value &data);
	};

	// wrap drawing object with references and settings data
	class Ball : public ActorRole {
	public:
		// bouncy ball with nice colors is pretty nice, does not take too much really
		void myDraw();

	private:
		float radius = 100;
		bool mysetup(const Json::Value &data);
	};

	// camera, lights etc
	class EquipementBaseClass : public objectLifeTimeManager {
	public:
		virtual bool setup(const Json::Value &data) = 0;
	};
	// cameras (and others like it) are not actors
	class Camera : public EquipementBaseClass {
	public:
		virtual bool isOrbiting() const { return false; }
		bool setup(const Json::Value &data);
		virtual void orbit() {};
		ofEasyCam worker;
	protected:
	private:
		virtual bool mysetup(const Json::Value &data) { return true; }
	};
	class FixedCamera : public Camera {
	public:
	private:
		bool mysetup(const Json::Value &data);
	};
	class MovingCamera : public FixedCamera {
	public:
		virtual bool isOrbiting() const { return true; }
	private:
		virtual void orbit();
		bool mysetup(const Json::Value &data);
	};

	class Light : public EquipementBaseClass {
	public:
		void setX(float x) { loc.x = x; }
		void setY(float y) { loc.y = y; }
		void setZ(float z) { loc.z = z; }
		void setLoc(float x, float y = 0, float z = 0) { loc.x = x; loc.y = y; loc.z = z; }
		ofPoint loc; // light is set on object within camera scope
		bool setup(const Json::Value &data);
		ofLight worker;
	private:
		shared_ptr<ColorSet> colorHelper = nullptr;
		virtual bool mysetup(const Json::Value &data) { return true; }
	};
	class PointLight : public Light {
	public:
		PointLight() :Light() { worker.setPointLight(); }
	private:
		virtual bool mysetup(const Json::Value &data);
	};
	class DirectionalLight : public Light {
	public:
		DirectionalLight() :Light() { worker.setDirectional(); }
	private:
		virtual bool mysetup(const Json::Value &data);
	};
	class SpotLight : public Light {
	public:
		SpotLight() :Light() { worker.setSpotlight(); }
	private:
		virtual bool mysetup(const Json::Value &data);
	};
	class CameraGrabber : public ActorRole {
	public:
		void myUpdate();
		void myDraw();
		void mySetup() { loadGrabber(w, h); }
		int w = 320;
		int h = 240;
		int x = 0;
		int y = 0;

	private:
		ofVideoGrabber worker;
		bool loadGrabber(int wIn, int hIn);
		int find();
		int id = 0;
		bool mysetup(const Json::Value &data);
	};
	class Material : public ofMaterial {
	public:
		void begin();
		bool setup(const Json::Value &data);
		shared_ptr<ColorSet> colorHelper = nullptr;
	};

	class KinectPerson {
	public:
		Joint joints[JointType::JointType_Count];
		HandState leftHandState;
		HandState rightHandState;
		ofPoint leanAmount;

	};

	class DrawingPrimitive3d : public ActorRole {
	public:
		DrawingPrimitive3d(of3dPrimitive *p, drawtype type = draw3dFixedCamera);
		~DrawingPrimitive3d();
		bool mysetup(const Json::Value &data);
		void setWireframe(bool b = true) { wireFrame = b; }
		bool useWireframe() { return wireFrame; }
		void myUpdate();
		void myDraw();
		of3dPrimitive* get() { return (of3dPrimitive*)node; }
		Material  material;
	private:
		virtual bool derivedMysetup(const Json::Value &data) = 0;
		bool wireFrame = true;
		void basicDraw();
	};
	// do not use templates as its hard to make base class pointers to them
	class Cube : public DrawingPrimitive3d {
	public:
		Cube() : DrawingPrimitive3d(new ofBoxPrimitive()) {		}
		ofBoxPrimitive* get() { return (ofBoxPrimitive*)DrawingPrimitive3d::get(); }
	private:
		bool derivedMysetup(const Json::Value &data);
	};
	class Plane : public DrawingPrimitive3d {
	public:
		Plane() : DrawingPrimitive3d(new ofPlanePrimitive()) {		}
		ofPlanePrimitive* get() { return (ofPlanePrimitive*)DrawingPrimitive3d::get(); }
	private:
		bool derivedMysetup(const Json::Value &data);
	};
	class Sphere : public DrawingPrimitive3d {
	public:
		Sphere() : DrawingPrimitive3d(new ofSpherePrimitive()) {		}

		ofSpherePrimitive* get() { return (ofSpherePrimitive*)DrawingPrimitive3d::get(); }
	private:
		bool derivedMysetup(const Json::Value &data);
	};
	class Cylinder : public DrawingPrimitive3d {
	public:
		Cylinder() : DrawingPrimitive3d(new ofCylinderPrimitive()) {		}
		ofCylinderPrimitive* get() { return (ofCylinderPrimitive*)DrawingPrimitive3d::get(); }
	private:
		bool derivedMysetup(const Json::Value &data);
	};
	class Cone : public DrawingPrimitive3d {
	public:
		Cone() : DrawingPrimitive3d(new ofConePrimitive()) {		}
		ofConePrimitive* get() { return (ofConePrimitive*)DrawingPrimitive3d::get(); }
	private:
		bool derivedMysetup(const Json::Value &data);
	};

	//bugbug add other shapes
	class Text : public ActorRole {
	public:
		void myDraw();
		void drawText(const string &s, int x, int y);
		void setText(const string&t) { text = t; }
		string& getText() { return text; }

	private:
		string text;
		bool mysetup(const Json::Value &data);
	};

	class Paragraph : public ActorRole {
	public:
		void myDraw();

	private:
		ofxParagraph worker;
		bool mysetup(const Json::Value &data);
	};

	class Arrow : public ActorRole {
	public:
		void myDraw() {
			ofDrawArrow(start, end, headSize);
		};
	private:
		Point3D start;
		Point3D end;
		float headSize = 1.05f;
		bool mysetup(const Json::Value &data);
	};

	class Audio : public ActorRole {
	public:
		void mySetup();
	private:
		ofSoundPlayer worker;
		bool mysetup(const Json::Value &data);
	};

	class Visual : public ActorRole {
	public:
		void myUpdate();
	protected:
		bool isLoaded = false;
		int width = 0;
		int height = 0;
		bool mysetup(const Json::Value &data);

	};
	class Rainbow : public Visual {
	public:
		void myDraw();
		void myUpdate();
	private:
		ofImage worker;
		int sizex, sizey;
		bool mysetup(const Json::Value &data);
	};

	class Video : public Visual {
	public:
		~Video();
		friend class VideoTexture;
		// put advanced drawing in these objects
		void myUpdate();
		void myDraw();
		void mySetup();
		float getTimeBeforeStart(float t);
	private:
		ofVideoPlayer worker;
		bool mysetup(const Json::Value &data);
	};

	class TextureVideo : public ActorRole {
	public:
		TextureVideo() : ActorRole() { }
		TextureVideo(const string& path) : ActorRole(path) { }
		void myUpdate() { worker.update(); }
		bool textureReady() { return  worker.isInitialized(); }
		bool mybind();
		bool myunbind();
		ofTexture& getVideoTexture();
	private:
		ofFbo fbo;
		ofTexture defaulttexture;
		ofVideoPlayer worker;
		bool mysetup(const Json::Value &data);

	};
	class VideoSphere : public ActorRole {
	public:
		VideoSphere() : ActorRole() { }
		VideoSphere(const string& path) : ActorRole(path) { video = std::make_shared<TextureVideo>(path); }
		void myUpdate() { sphere.myUpdate(); }
		void myDraw();
		Sphere& getSphere() { return sphere; }

	private:
		bool set = false;
		Sphere sphere;
		shared_ptr<TextureVideo> video = nullptr;
		bool mysetup(const Json::Value &data);
	};

	class Planet : public ActorRole {
	public:
		Planet() : ActorRole() { texture = std::make_shared<TextureFromImage>(); }

		void myDraw();
		shared_ptr<TextureFromImage> getTexturePtr() { return texture; }
		Sphere& getSphere() { return sphere; }
		ofPoint rotateAround;// used to position relative to something else, default is center of screen
	private:
		shared_ptr<TextureFromImage> texture = nullptr;
		Sphere sphere;
		bool mysetup(const Json::Value &data);
	};

	class SolarSystem : public ActorRole {
	public:
		SolarSystem() :ActorRole() {  }

	private:
		void addPlanets(const Json::Value &data, Sphere &parent);
		bool mysetup(const Json::Value &data);
	};


	class Picture : public Visual {
	public:
		void myUpdate();
		void mySetup();
		void myDraw();
	private:
		ofImage worker;
		bool mysetup(const Json::Value &data);
	};

	class Background : public Visual {
	public:
		enum TypeOfBackground { ColorFixed, ColorChanging, none };
		enum TypeOfGradient {
			linear = ofGradientMode::OF_GRADIENT_LINEAR, circular = ofGradientMode::OF_GRADIENT_CIRCULAR,
			bar = ofGradientMode::OF_GRADIENT_BAR, flat, noGradient
		};

		void myDraw();
		void myUpdate();// make image a vector then rotate via animation
		void setType(TypeOfBackground typeIn = ColorFixed) { type = typeIn; }
		void setGradientMode(const TypeOfGradient& modeIn) { mode = modeIn; }

	private:
		ofGradientMode ofMode;
		TypeOfGradient mode = TypeOfGradient::noGradient;
		TypeOfBackground type = ColorFixed;
		bool mysetup(const Json::Value &data);
		template<typename T>void add(const Json::Value &data, int draworder) {
			shared_ptr<T> item = std::make_shared<T>();
			if (item) {
				if (item->setup(data)) {
					item->setFullSize();
					item->setDrawOrder(draworder);
					getStage()->addToAnimatable(item, true);
				}
			}

		}
	};

	// item in a play list

	class Stage;
	class Channel : public objectLifeTimeManager {
	public:
		Channel() :objectLifeTimeManager() {  }
		Channel(const string&keynameIn) { keyname = keynameIn; }
		enum ChannelType { History, Art, Sports, Any };

		bool setup(const Json::Value &data);
		bool operator==(const Channel rhs) { return rhs.keyname == keyname; }
		string &getKeyName() { return keyname; }
		shared_ptr<Stage> getStage() { return stage; }
		void setStage(shared_ptr<Stage>p) { stage = p; }
		bool getSkip() { return skip; }
		void setSkip(bool b = true) { skip = b; }

		ChannelType getType() { return channeltype; }
		void setType(ChannelType typeIn) { channeltype = typeIn; }
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

		bool setup(const Json::Value &data);
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