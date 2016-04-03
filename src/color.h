#pragma once
#include "2552software.h"
#include "animation.h"
#include <forward_list>

#include <cstdarg>

namespace Software2552 {
	// matching colors, this class controls colors, no data or others selected colors
	// this way its easier to manage things
	//http://openframeworks.cc/documentation/types/ofColor/#show_fromHsb
	// there is a lot we can do 
	//http://www.color-hex.com/ http://www.colorhexa.com/web-safe-colors
	// http://cloford.com/resources/colours/500col.htm

	// colors can have a time out or a wait count via Animator
	class ColorSet : public objectLifeTimeManager {
	public:
		enum ColorGroup {
			Modern, Smart, Extreme, EarthTone, BuiltIn, Default, Black, White, Blue, RedBlue, Random//only modern so far, ArtDeco, Warm, Cool, Stark, Pastel, LightValue, DarkValue, MediumValue, Random
		};
		enum ColorType {
			Fore, Back, Text, Other, Lightest, Darkest
		};
		//bugbug color set may need 4 or more colors once we do more with graphics
		// something like fore/back/text/other[n], not sure, or maybe we
		// just use multiple ColorSets, find out more as we continue on
		ColorSet(const ColorGroup groupIn, int fore, int back, int text, int other, int lightest, int darkest);
		ColorGroup setGroup(const string&name);
		ColorGroup getGroup() const {return group;}
		int getHex(int index) const  {return colors[index];	}
		bool operator== (const ColorSet& rhs) {	return getGroup() == rhs.getGroup();}
		// return true if less than, and both of the desired type or Random
		int size() { return colors.size(); }
		ColorSet& operator++() { usage++; return *this; }
		int getUsage() { return usage; }
	private:
		void setSetcolors(int c, ...);
		ColorGroup group;
		int usage = 0;// usage count
		vector<int> colors; //hex values of all matching colors
	};

	// global color data
	class ColorList {
	public:
		class ColorList(ColorSet::ColorGroup group) {
			// there must always be at least one color
			privateData = std::make_shared<colordata>();
			getNextColors(group);
			setup();
		}
		//color naming modeled after http://www.creativecolorschemes.com/products/ccs1/rgbColorGuide.shtml
		// names of customer colors, when paired they are a color set

		void update();
		void setup();

		class colordata {
		public:
			forward_list<shared_ptr<ColorSet>> colorlist;
			shared_ptr<ColorSet> currentColor = nullptr;
		};

		// call getNext at start up and when ever colors should change
		// do not break colors up or things will not match
		// get next color based on type and usage count
		// example: type==cool gets the next cool type, type=Random gets any next color
		static shared_ptr<ColorSet> getNextColors(ColorSet::ColorGroup group = ColorSet::ColorGroup::Default);
		static shared_ptr<ColorSet> getCurrentColor();

	protected:
		template<typename T> void removeExpiredItems(forward_list<shared_ptr<T>>&v) {
			v.remove_if(objectLifeTimeManager::OKToRemove);
		}


		void add(const ColorSet::ColorGroup group, int fore, int back, int text, int other, int lightest, int darkest);

	private:
		static shared_ptr<colordata> privateData;// never call directly to assure allocation
		static void setCurrentColor(shared_ptr<ColorSet>c) { privateData->currentColor = c; }
		static forward_list<shared_ptr<ColorSet>>  getList() { return privateData->colorlist; }
	};


	// color helpers
	class Colors : public ColorList {
	public:
		Colors(ColorSet::ColorGroup group) : ColorList(group){		}
		// hue helpers, example getHue(getBackground()) bugbug maybe cache some data if needed
		static float getSaturation(int index) {
			return ofColor().fromHex(getCurrentColor()->getHex(index)).getSaturation();
		}
		static float getBrightness(int index) {
			return ofColor().fromHex(getCurrentColor()->getHex(index)).getBrightness();
		}
		static float getHue(int index) {
			return ofColor().fromHex(getCurrentColor()->getHex(index)).getHue();
		}
		static ofColor getOfColor(int index) {
			return ofColor().fromHex(getCurrentColor()->getHex(index));
		}
		static ofFloatColor getFloatColor(int index) {
			return ofFloatColor().fromHex(getCurrentColor()->getHex(index));
		}
		// more like painting
		static float getHueAngle(int index)	{
			return ofColor().fromHex(getCurrentColor()->getHex(index)).getHueAngle();
		}
		static int getForeground() {
			return getCurrentColor()->getHex(ColorSet::ColorType::Fore);
		}
		static  int getBackground() {
			return getCurrentColor()->getHex(ColorSet::ColorType::Back);
		}
		static  int getFontColor() {
			return getCurrentColor()->getHex(ColorSet::ColorType::Text);
		}
		static int getLightest() {
			return getCurrentColor()->getHex(ColorSet::ColorType::Lightest);
		}
		static int getDarkest() {
			return getCurrentColor()->getHex(ColorSet::ColorType::Darkest);
		}
		static int getOther() {
			return getCurrentColor()->getHex(ColorSet::ColorType::Other);
		}

	};


	template <class T> class ColorBase {
	public:
		ColorBase() {}
		ColorBase(const T& colorIn) {
			color = colorIn;
		}
		bool readFromScript(const Json::Value &data) {
			readJsonValue(color.r, data["r"]);
			readJsonValue(color.g, data["g"]);
			readJsonValue(color.g, data["b"]);
			return true;
		}
		operator T() {
			return color;
		}
		T& get() { return color; }
	protected:
		T color;
	};

	// color support of explict color is needed
	class Color : public ColorBase<ofColor> {
	public:
		Color() :ColorBase() {
		}
		Color(const ofColor& color) :ColorBase(color) {
		}
	};
	class floatColor : public ColorBase<ofFloatColor> {
	public:
		floatColor() :ColorBase() {
		}
		floatColor(const ofFloatColor& color) :ColorBase(color) {
		}

	};


}