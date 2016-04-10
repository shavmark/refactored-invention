#include "2552software.h"
#include "animation.h"
#include "model.h"
#include "color.h"
#include <map>
#include <unordered_map>

namespace Software2552 {
	shared_ptr<ColorList::colordata> ColorList::privateData = nullptr; // declare static data
																	 // color will animate
	shared_ptr<AnimiatedColor> create(const ofColor& start, const ofColor& end) {
		shared_ptr<AnimiatedColor>ac = std::make_shared<AnimiatedColor>();
		if (ac) {
			ac->setColor(start);//bugbug get from json
			ac->animateTo(end);//bugbug look at delays and final color, good json data
		}
		return ac;
	}
	// no animation
	shared_ptr<AnimiatedColor> create(const ofColor& start) {
		shared_ptr<AnimiatedColor>ac = std::make_shared<AnimiatedColor>();
		if (ac) {
			ac->setColor(start);//bugbug get from json
		}
		return ac;
	}
	// only alpha changes
	shared_ptr<AnimiatedColor> create(const ofColor& start, int alpha) {
		shared_ptr<AnimiatedColor>ac = std::make_shared<AnimiatedColor>();
		if (ac) {
			ac->setColor(start);
			ac->animateToAlpha(alpha);
		}
		return ac;
	}
#define SETACOLOR(a)create(ofColor::fromHex(a))
#define SETACOLORRANGE(a,b,c,d)create(ofColor::fromHex(a, b), ofColor::fromHex(c, d))
#define SETACOLORALPHA(a,b)create(ofColor::fromHex(a),b)

	ofColor& ColorSet::get(int index) {
		if (index < colors.size() && colors[index] != nullptr) {
			return colors[index]->getCurrentColor();
		}
		return defaultColor;
	}
	void ColorSet::draw() { 
		if (colors[Fore]) {
			colors[Fore]->applyCurrentColor();
		}
	}
	void ColorSet::setup(const Json::Value &data) {
		//bugbug good to read in to/from/color and other things
		//for (auto& c : colors) {
			//c->setup();
		//}
	}
	void ColorSet::update() {
		for (auto& c : colors) {
			c->update();
		}
	}
	// set 1 or more colors in the set
	void ColorSet::setSetcolors(int c, ...) {
		colors.clear();

		va_list args;
		va_start(args, c);
		for (int i = 0; i < c; ++i) {
			colors.push_back(va_arg(args, shared_ptr<AnimiatedColor>));
		}
		va_end(args);
	}
	bool ColorList::setup(const Json::Value &data) {
		if (data.size() > 0 && !data["colorAnimation"].empty()) {
			string colorGroupName;
			READSTRING(colorGroupName, data);
			if (colorGroupName.size() > 0) {
				getNextColors(ColorSet::convertStringToGroup(colorGroupName), false);
			}
			//bugbug where is this allocated
			if (privateData) {
				privateData->currentColorSet = parseNoList<ColorSet>(data["colorAnimation"]);
			}
		}
		return true;
	}
	void ColorList::update() {
		if (privateData && privateData->currentColorSet) {
			privateData->currentColorSet->update();
			// remove expired colors
			removeExpiredItems(privateData->colorlist);
			//bugbug call this at the right time
			if (getCurrentColor() && ofRandom(0, 100) > 80) {
				//getNextColors(getCurrentColor()->getGroup(), true); // updates global list
			}
		}
	}
	void ColorList::setCurrentColorSet(shared_ptr<ColorSet>c) {
		if (privateData && c) {
			privateData->currentColorSet = c; //bugbug do need to reset the coloranimation here?
		}
	}
	// load color on demand (save memory at all costs so we can port the heck out of this thing)
	std::forward_list<shared_ptr<ColorSet>>::iterator ColorList::load(ColorSet::ColorGroup group) {
		for (auto& it = privateData->colorlist.begin(); it != privateData->colorlist.end(); ++it) {
			if ((*it)->getGroup() == group) {
				return it; // color already present
			}
		}
		if (group == ColorSet::Modern) {
			shared_ptr<AnimiatedColor>light = create(ofColor::fromHex(privateData->modern['A']));// example

			add(ColorSet::Modern, SETACOLORRANGE(privateData->modern['A'], 255, privateData->modern['B'], 255),
				SETACOLORALPHA(privateData->modern['C'], 100), light, SETACOLOR(privateData->modern['C']));
		}
		else if (group == ColorSet::White) {
			add(ColorSet::White, create(ofColor::white));
		}
		else if (group == ColorSet::Orange) {
			add(ColorSet::Orange, create(ofColor::orange, ofColor::orangeRed));
		}
		return privateData->colorlist.begin();
	}
	// get next color based on type and usage count, this will set the color globally
	// example: type==cool gets the next cool type, type=Random gets any next color
	shared_ptr<ColorSet> ColorList::getNextColors(ColorSet::ColorGroup group, bool global) {
		shared_ptr<ColorSet> ret = nullptr;
		if (getCurrentColor() != nullptr) {
			if (getCurrentColor()->getGroup() != group) {
				// new group, delete current group
				if (global) {
					setCurrentColorSet(nullptr);
				}
			}
		}
		// load group if needed
		std::forward_list<shared_ptr<ColorSet>>::iterator it = load(group);

		// find a match
		for (; it != privateData->colorlist.end(); ++it) {
			if ((*it)->getGroup() == group) {
				if (getCurrentColor() == nullptr || getCurrentColor()->getUsage() >= (*it)->getUsage()) {
					// first time in or a color as less usage than current color
					if (global) {
						setCurrentColorSet(*it);
					}
					ret = *it;
					break;
				}
			}
		}
		return ret;
	}
	void ColorList::add(const ColorSet::ColorGroup group, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor>back, shared_ptr<AnimiatedColor> lightest, shared_ptr<AnimiatedColor> darkest) {
		shared_ptr<ColorSet> s = std::make_shared<ColorSet>(group, fore, back, lightest, darkest);
		privateData->colorlist.push_front(s);
	}


	ColorSet::ColorGroup ColorSet::convertStringToGroup(const string&name) {
		if (name == "Modern") {
			return Modern;
		}
		else if (name == "Smart") {
			return Smart;
		}
		else if (name == "Extreme") {
			return Extreme;
		}
		else if (name == "EarthTone") {
			return EarthTone;
		}
		else if (name == "Orange") {
			return Orange;
		}
		else if (name == "Black") {
			return Black;
		}
		else if (name == "White") {
			return White;
		}
		else if (name == "Blue") {
			return Blue;
		}
		else if (name == "Random") {
			return Random;
		}
		else {
			return Default;
		}
	}

	ColorList::ColorList() {
		// assumes static data so color info is shared across app
		if (privateData == nullptr) {
			privateData = std::make_shared<colordata>();
			setup();
		}
	}
	ColorSet::ColorSet(const ColorGroup groupIn, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor> back, shared_ptr<AnimiatedColor> lightest, shared_ptr<AnimiatedColor> darkest) : objectLifeTimeManager() {
		group = groupIn;
		defaultColor = ofColor::greenYellow;
		colors.push_back(fore);
		colors.push_back(back);
		colors.push_back(lightest);
		colors.push_back(darkest);
	}
	ofColor& ColorSet::getLightest() {
		// assume data is propery allocated in the colors vector
		if (colors[Lightest] == nullptr) {
			return get(Fore);
		}
		return get(Lightest);
	}
	ofColor& ColorSet::getDarkest() {
		// assume data is propery allocated in the colors vector
		if (colors[Darkest] == nullptr) {
			return get(Back); // assume always at least a background
		}
		return get(Darkest);
	}

	ColorSet::ColorSet(const ColorGroup groupIn, shared_ptr<AnimiatedColor> fore) : objectLifeTimeManager() {
		group = groupIn;
		defaultColor = ofColor::greenYellow;//c.invert(); lerp ofColor r = ofColor::red;
									  ///     ofColor b = ofColor::blue;
									  ///     b.lerp(r, 0.5); // now purple
		if (fore) {
			shared_ptr<AnimiatedColor>inverted = std::make_shared<AnimiatedColor>();
			if (inverted) {
				ofColor c = fore->getCurrentColor().getInverted();
				inverted->setColor(c);
			}
			colors.push_back(fore);
			colors.push_back(inverted);
			colors.push_back(fore);
			colors.push_back(inverted);

		}
	}

	//http://www.creativecolorschemes.com/resources/free-color-schemes/art-deco-color-scheme.shtml
	void ColorList::setup() {
		if (privateData == nullptr) {
			return;
		}
		if (privateData->currentColorSet == nullptr) {
			privateData->currentColorSet = std::make_shared<ColorSet>();
			getNextColors(ColorSet::Modern, true);// make sure there is a current color
		}
		
		//bugbug phase II read from json
		// only needs to be setup one time since its static data
		if (privateData->modern.empty()) {
			privateData->modern =
			{ {'A', 0x003F53}, {'B', 0x3994B3}, {'C', 0x64B1D1 }, {'D', 0x00626D }, {'E', 0x079CBA }, {'F', 0x60CDD9 },
			 {'G', 0x003E6B }, {'H', 0x0073A0 }, {'I', 0xBAECE9 }, {'J', 0xD0FC0E }, {'K', 0xFDB075 }, {'L', 0xFFD76E },
			 {'M', 0x4D5E6C }, {'N', 0x858E9C }, {'O', 0xCCD1D5 } };
			privateData->smart = {
			{ 'A',0x312659},{'B', 0x373B81},{ 'C', 0x425096},{ 'D', 0x0D60A5 },{ 'E', 0x297BC6 },{ 'F', 0x3EA0ED },{ 'G', 0x169FAD },
			{ 'H', 0x30C1D8 },{ 'I', 0x7FE3F7 },{ 'J', 0xB01116 },{ 'K', 0xD71920 },{ 'L', 0xFEAB07 },{ 'M', 0xFED341 },{ 'N', 0xFFDA7A },{ 'O', 0xFFEEBC } };

			privateData->extreme = {
				{ 'A',0x023D7B },{ 'B', 0x1B4C8C },{ 'C', 0x448BE3 },{ 'D', 0x025B8F },{ 'E', 0x088BB3 },
				{ 'F',  0x02CAE6 },{ 'G',  0xC61630 },{ 'H',0xFE243E },{ 'I',  0xFE3951 },{ 'J',  0xF03624 },
				{ 'K',  0xF3541B },{ 'L',  0xFE872B },{ 'M',  0x8FD173 },{ 'N',  0xB7B96B },{ 'O',  0xAD985F } };

			privateData->earthtone = {
			{ 'A',0x493829 },{ 'B',  0x816C5B },{ 'C',  0xA9A18C },{ 'D',  0x613318 },{ 'E',  0x855723 },{ 'F',  0xB99C6B },{ 'G',  0x8F3B1B },{ 'H', 0xD57500 },
			{ 'I',  0xDBCA69 },{ 'J',  0x404F24 },{ 'K',  0x668D3C },{ 'L',  0xBDD09F },{ 'M',  0x4E6172 },{ 'N',  0x83929F },{ 'O',  0xA3ADB8} };
			//A C B D A C see the color doc to fill these in. use the 4 colors then pick the lightest and darkest 
		}

		/* bugbug load all these once color is working etc
		// a full color
		add(ColorSet::Modern, modern['A'], modern['C'], modern['B'], modern['D'], modern['A'], modern['C']);

		add(ColorSet::Modern, modern['A'], modern['C'], modern['B'], modern['D'], modern['A'], modern['C']);

		add(ColorSet::White, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff);
		add(ColorSet::Orange, ofColor::orange.getHex(), ofColor::orangeRed.getHex(), ofColor::orange.getHex(), ofColor::orangeRed.getHex(), ofColor::orange.getHex(), ofColor::orangeRed.getHex());

		add(ColorSet::Modern, E, D, ofColor::black.getHex(), ofColor::white.getHex());

		add(ColorSet::Modern, N, M, ofColor::white, ofColor::white);
		add(ColorSet::Modern, G, H, ofColor::black, ofColor::white);
		add(ColorSet::Modern, D, M, ofColor::white, ofColor::white);
		add(ColorSet::Modern, D, B, ofColor::black, ofColor::white);
		add(ColorSet::Modern, J, A, ofColor::white, ofColor::white);
		add(ColorSet::Modern, M, A, ofColor::white, ofColor::white);
		add(ColorSet::Modern, H, N, L, ofColor::white);
		add(ColorSet::Modern, H, N, ofColor::black, ofColor::white);
		add(ColorSet::Modern, O, C, ofColor::black, ofColor::white);
		add(ColorSet::Modern, I, F, ofColor::black, ofColor::white);
		add(ColorSet::Modern, K, N, ofColor::black, ofColor::white);

		setupBasicColors(ColorSet::Smart, smart);
		add(ColorSet::Smart, B, E, ofColor::black, ofColor::white);
		add(ColorSet::Smart, A, G, ofColor::black, ofColor::white);
		add(ColorSet::Smart, F, M, ofColor::black, ofColor::white);
		add(ColorSet::Smart, J, N, ofColor::black, ofColor::white);
		add(ColorSet::Smart, N, D, ofColor::black, ofColor::white);
		add(ColorSet::Smart, H, K, ofColor::black, ofColor::white);
		add(ColorSet::Smart, B, L, ofColor::black, ofColor::white);
		add(ColorSet::Smart, M, A, ofColor::black, ofColor::white);
		add(ColorSet::Smart, B, E, ofColor::blue, ofColor::white);
		add(ColorSet::Smart, O, M, ofColor::black, ofColor::white);
		add(ColorSet::Smart, D, J, ofColor::blue, ofColor::white);
		add(ColorSet::Smart, E, H, ofColor::black, ofColor::white);

		setupBasicColors(ColorSet::Extreme, extreme);
		add(ColorSet::Extreme, B, K, ofColor::white, ofColor::white);
		add(ColorSet::Extreme, E, M, ofColor::black, ofColor::white);
		add(ColorSet::Extreme, G, D, ofColor::black, ofColor::white);
		add(ColorSet::Extreme, D, O, ofColor::black, ofColor::white);
		add(ColorSet::Extreme, F, I, ofColor::black, ofColor::white);
		add(ColorSet::Extreme, H, K, ofColor::black, ofColor::white);
		add(ColorSet::Extreme, L, C, ofColor::black, ofColor::white);

		setupBasicColors(ColorSet::EarthTone, earthtone);
		add(ColorSet::EarthTone, D, B, ofColor::black, ofColor::white);
		add(ColorSet::EarthTone, E, A, ofColor::black, ofColor::white);
		add(ColorSet::EarthTone, J, I, ofColor::black, ofColor::white);
		add(ColorSet::EarthTone, F, N, ofColor::black, ofColor::white);
		add(ColorSet::EarthTone, D, H, ofColor::black, ofColor::white);
		add(ColorSet::EarthTone, H, J, ofColor::black, ofColor::white);
		add(ColorSet::EarthTone, N, J, ofColor::black, ofColor::white);
		add(ColorSet::EarthTone, A, H, ofColor::black, ofColor::white);
		add(ColorSet::EarthTone, I, K, ofColor::black, ofColor::white);
		add(ColorSet::EarthTone, ofColor::green, C, ofColor::black, ofColor::white); // just a demo of using the built in colors

		// setup the built in colors
		add(ColorSet::Blue, ofColor::lightBlue, ofColor::white, ofColor::blue, ofColor::white);
		add(ColorSet::Black, ofColor::black, ofColor::white, ofColor::blue, ofColor::white);
		add(ColorSet::White, ofColor::white, ofColor::black, ofColor::blue, ofColor::white);
		add(ColorSet::RedBlue, ofColor::red, ofColor::lightCoral, ofColor::blue, ofColor::indianRed);
		add(ColorSet::Default, ofColor::red, ofColor::blue, ofColor::white, ofColor::green);
		*/
	}

#if 0
			//http://www.creativecolorschemes.com/resources/free-color-schemes/art-deco-color-scheme.shtml
//bugbug at some point maybe read from json
			ColorSet cs = ColorSet(ColorSet::ArtDeco,
				ofColor(255, 0, 0), ofColor(0, 255, 0), ofColor(0, 0, 255));
			data.push_back(cs);

			ofColor fore, back, text;
			fore.setHsb(200, 100, 40); // just made this up for now
			back.setHsb(100, 100, 50);
			text.setHsb(200, 100, 100);
			cs.set(ColorSet::ArtDeco, fore, back, text);
			data.push_back(cs);
			//			ColorSet cs2 = ColorSet(ColorSet::Warm,
			//			ofColor::aliceBlue, ofColor::crimson, ofColor::antiqueWhite);
			ColorSet cs2 = ColorSet(ColorSet::ArtDeco,
				ofColor(0, 255, 0), ofColor(0, 0, 255), ofColor(255, 255, 255));
			data.push_back(cs2);
	}

#endif // 0
	

}