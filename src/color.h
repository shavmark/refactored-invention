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
		//convertStringToGroup must be updated to match ColorGroup
		enum ColorGroup {
			Modern, Smart, Orange, Extreme, EarthTone, BuiltIn, Default, Black, White, Blue, RedBlue, Random//only modern so far, ArtDeco, Warm, Cool, Stark, Pastel, LightValue, DarkValue, MediumValue, Random
		};

		//bugbug color set may need 4 or more colors once we do more with graphics
		// something like fore/back/text/other[n], not sure, or maybe we
		// just use multiple ColorSets, find out more as we continue on
		ColorSet(const ColorGroup groupIn, int fore, int back, int text, int other, int lightest, int darkest);
		int getForeground() { return getHex(ColorSet::ColorType::Fore); }
		int getBackground() { return getHex(ColorSet::ColorType::Back); }
		int getFontColor() { return getHex(ColorSet::ColorType::Text); }
		int getLightest() { return getHex(ColorSet::ColorType::Lightest); }
		int getDarkest() { return getHex(ColorSet::ColorType::Darkest); }
		int getOther() { return getHex(ColorSet::ColorType::Other); }

		static ColorGroup convertStringToGroup(const string&name);
		ColorGroup getGroup() const {return group;}
		int getHex(int index) const  {return colors[index];	}
		bool operator== (const ColorSet& rhs) {	return getGroup() == rhs.getGroup();}
		// return true if less than, and both of the desired type or Random
		int size() { return colors.size(); }
		ColorSet& operator++() { usage++; return *this; }
		int getUsage() { return usage; }
	protected:
		// these get confused with hex values when not used correctly so protect them
		enum ColorType {
			Fore, Back, Text, Other, Lightest, Darkest
		};
	private:
		void setSetcolors(int c, ...);
		ColorGroup group;
		int usage = 0;// usage count
		vector<int> colors; //hex values of all matching colors
	};

	// global color data
	class AnimiatedColor : public ofxAnimatableOfColor {
	public:
		AnimiatedColor() :ofxAnimatableOfColor() { }
		void draw();
		void update();
		bool paused() { return paused_; }
		bool readFromScript(const Json::Value &data);
		shared_ptr<ColorSet> getColorSet();
		float getAlpha() { return alpha; }//bugbug flush this out, read it in, set it etc
		void setAlpha(float a) { alpha = a; }
		void getNextColors();
		void setColorSet(shared_ptr<ColorSet>p);
		bool useAnimation() { return usingAnimation; }
		void setAnimation(bool b = true) { usingAnimation = true; }
		// avoid long access code
		int getForeground() { return getColorSet()->getForeground(); }
		int getBackground() { return getColorSet()->getBackground(); }
		int getFontColor() { return getColorSet()->getFontColor(); }
		int getLightest() { return getColorSet()->getLightest(); }
		int getDarkest() { return getColorSet()->getDarkest(); }
		int getOther() { return getColorSet()->getOther(); }
		ofFloatColor getFloatObject(int hex) { return ofFloatColor().fromHex(hex, getAlpha()); }
		ofColor getColorObject(int hex) { return ofFloatColor().fromHex(hex, getAlpha()); }

		shared_ptr<ColorSet> colorSet = nullptr; // use a pointer to the global color for dynamic color or set a fixed one
	private:
		bool usingAnimation = false; // force  to set this to make sure its understood
		float alpha = 255;
	};
	
	class ColorList {
	public:
		ColorList();

		//color naming modeled after http://www.creativecolorschemes.com/products/ccs1/rgbColorGuide.shtml

		void update();
		void setup();
		class colordata {
		public:
			forward_list<shared_ptr<ColorSet>> colorlist; // global list of colors
			shared_ptr<AnimiatedColor>currentColor = nullptr;  // color set in use, currentColor never changes but its content does so the pointer can be widely shared
		};

		// call getNext at start up and when ever colors should change
		// do not break colors up or things will not match
		// get next color based on type and usage count
		// example: type==cool gets the next cool type, type=Random gets any next color
		static shared_ptr<ColorSet> getNextColors(ColorSet::ColorGroup group, bool global);
		static shared_ptr<AnimiatedColor> getCurrentColor();

		bool readFromScript(const Json::Value &data);

	protected:
		template<typename T> void removeExpiredItems(forward_list<shared_ptr<T>>&v) {
			v.remove_if(objectLifeTimeManager::OKToRemove);
		}

		shared_ptr<ColorSet> add(const ColorSet::ColorGroup group, int fore, int back, int text, int other, int lightest, int darkest);

	private:
		static shared_ptr<colordata> privateData;// never call directly to assure allocation
		static void setCurrentColorSet(shared_ptr<ColorSet>c);
	};


	
}