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
			Modern, Smart, Orange, Extreme, EarthTone, BuiltIn, Default, Black, White, Blue, RedBlue, Random, lastcolor//only modern so far, ArtDeco, Warm, Cool, Stark, Pastel, LightValue, DarkValue, MediumValue, Random
		};
		void update();
		void setup(const Json::Value &data);
		void draw();
		//bugbug color set may need 4 or more colors once we do more with graphics
		// something like fore/back/text/other[n], not sure, or maybe we
		// just use multiple ColorSets, find out more as we continue on
		ColorSet(const ColorGroup groupIn, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor>back, shared_ptr<AnimiatedColor> lightest = nullptr, shared_ptr<AnimiatedColor> darkest=nullptr);
		ColorSet(const ColorGroup groupIn, shared_ptr<AnimiatedColor> basecolor);
		ColorSet() {};
		// will return default if requested item is not in the set
		ofColor& getForeground() { return get(Fore); }
		ofColor& getBackground() { return get(Back); }
		ofColor& getLightest();
		ofColor& getDarkest();
		ofColor& getCustom(int index) { return get(lastcolor+index); } //bugbug support later, defaults for now
		static ColorGroup convertStringToGroup(const string&name);
		ColorGroup getGroup() const {return group;}
		ofColor& get(int index);
		bool operator== (const ColorSet& rhs) {	return getGroup() == rhs.getGroup();}
		// return true if less than, and both of the desired type or Random
		int size() { return colors.size(); }
		ColorSet& operator++() { usage++; return *this; }
		int getUsage() { return usage; }
	protected:
		// these get confused with hex values when not used correctly so protect them
		enum ColorType {
			Fore, Back, Lightest, Darkest
		};
	private:
		ofColor defaultColor;
		void setSetcolors(int c, ...);
		ColorGroup group;
		int usage = 0;// usage count
		vector<shared_ptr<AnimiatedColor>> colors; //hex values of all matching colors
	};

	class ColorList {
	public:
		ColorList();

		//color naming modeled after http://www.creativecolorschemes.com/products/ccs1/rgbColorGuide.shtml

		void update();
		void setup();
		bool setup(const Json::Value &data);
		void getNextColors();

		class colordata {
		public:
			forward_list<shared_ptr<ColorSet>> colorlist; // global list of colors
			shared_ptr<ColorSet>currentColorSet = nullptr;  // color set in use, currentColor never changes but its content does so the pointer can be widely shared
			std::unordered_map<char, int> modern;
			std::unordered_map<char, int> smart;
			std::unordered_map<char, int> extreme;
			std::unordered_map<char, int> earthtone;

		};

		// call getNext at start up and when ever colors should change
		// do not break colors up or things will not match
		// get next color based on type and usage count
		// example: type==cool gets the next cool type, type=Random gets any next color
		static shared_ptr<ColorSet> getNextColors(ColorSet::ColorGroup group, bool global);
		static shared_ptr<ColorSet> getCurrentColor();

	protected:
		template<typename T> void removeExpiredItems(forward_list<shared_ptr<T>>&v) {
			v.remove_if(objectLifeTimeManager::OKToRemove);
		}

	private:
		static void add(const ColorSet::ColorGroup groupIn, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor>back = nullptr, shared_ptr<AnimiatedColor> lightest = nullptr, shared_ptr<AnimiatedColor> darkest = nullptr);
		static std::forward_list<shared_ptr<ColorSet>>::iterator load(ColorSet::ColorGroup group);

		static shared_ptr<colordata> privateData;// never call directly to assure allocation
		static void setCurrentColorSet(shared_ptr<ColorSet>c);
	};

	
}