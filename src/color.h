#pragma once
#include "2552software.h"
#include "animation.h"
#include <forward_list>
#include <unordered_map>


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
		friend class ColorList;
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
		ColorSet();
		ColorSet(const ColorGroup, shared_ptr<AnimiatedColor> basecolor);
		ColorSet(const ColorGroup, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor>back, shared_ptr<AnimiatedColor> lightest = nullptr, shared_ptr<AnimiatedColor> darkest = nullptr);

		// colors can be set or generated from the Fore color (the only color required)
		ofColor& getForeground() { return get(Fore); }
		ofColor getBackground();
		ofColor getLightest();
		ofColor getDarkest();
		ofColor getColor1();
		ofColor getColor2();
		bool alphaEnbled(); // true if any alpha enabled
		bool operator== (const ColorSet& rhs) {	return getGroup() == rhs.getGroup();}
		// return true if less than, and both of the desired type or Random

	private:
		enum ColorType {
			Fore, Back, Lightest, Darkest, Color1, Color2
		};
		typedef std::unordered_map<ColorType, shared_ptr<AnimiatedColor>>ColorMap;
		shared_ptr<AnimiatedColor>getAnimatedColor(ColorType type);
		ColorGroup getGroup() const { return group; }
		void addColor(ColorType type, shared_ptr<AnimiatedColor> c) { colors[type] = c; }
		static ColorGroup convertStringToGroup(const string&name);
		int size() { return colors.size(); }
		ofColor get(ColorType type);
		// these get confused with hex values when not used correctly so protect them

		ofColor defaultColor;
		//void setSetcolors(int c, ...);
		ColorGroup group;
		ColorMap colors;
	};
	
	shared_ptr<ColorSet> parseColor(const Json::Value &data);

	class ColorList {
	public:
		ColorList();

		//color naming modeled after http://www.creativecolorschemes.com/products/ccs1/rgbColorGuide.shtml

		void update();
		void setup();
		static shared_ptr<ColorSet> read(const Json::Value &data);
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
		static shared_ptr<ColorSet> addbasic(const ColorSet::ColorGroup, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor>back = nullptr, shared_ptr<AnimiatedColor> lightest = nullptr, shared_ptr<AnimiatedColor> darkest = nullptr);
		static void addfull(const ColorSet::ColorGroup, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor>back, 
			shared_ptr<AnimiatedColor> color1, shared_ptr<AnimiatedColor> color2,
		shared_ptr<AnimiatedColor> lightest, shared_ptr<AnimiatedColor> darkest);
		static std::forward_list<shared_ptr<ColorSet>>::iterator load(ColorSet::ColorGroup group);

		static shared_ptr<colordata> privateData;// never call directly to assure allocation
		static void setCurrentColorSet(shared_ptr<ColorSet>c);
	};

	
}