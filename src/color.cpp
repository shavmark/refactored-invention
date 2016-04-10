#include "2552software.h"
#include "animation.h"
#include "model.h"
#include "color.h"
#include <map>
#include <unordered_map>

namespace Software2552 {
	shared_ptr<ColorList::colordata> ColorList::privateData = nullptr; // declare static data

	shared_ptr<ColorSet> parseColor(const Json::Value &data)
	{
		if (data.size() > 0) {
			ColorList list;
			shared_ptr<ColorSet> results = list.read(data); // get latest color set bugbug this will change global color
			if (!results) {
				results = std::make_shared<ColorSet>();
			}
			if (results) {
				results->setup(data);
			}
		}
		else {

		}
		return ColorList::getCurrentColor();
	}
	// true if any alpha enabled
	bool ColorSet::alphaEnbled() {
		for (auto& c : colors) {
			if (c.second->from != 255 && c.second->to != c.second->from) {
				return true;
			}
		}

		return false;
	}
	shared_ptr<AnimiatedColor>ColorSet::getAnimatedColor(ColorType type) {
		ColorMap::iterator itr = colors.find(type);
		if (itr != colors.end())	{
			return itr->second;
		}
		return nullptr;
	}

	ofColor ColorSet::get(ColorType type) {
		ColorMap::iterator itr = colors.find(type);
		if (itr != colors.end()) {
			return itr->second->getCurrentColor();
		}
		return defaultColor;
	}
	void ColorSet::draw() { 
		ColorMap::iterator itr = colors.find(Fore);
		if (itr != colors.end()) {
			itr->second->applyCurrentColor();
		}
	}
	// lets us over ride the built in colors bugbug create a basic color than can then be re-done here as needed
	void ColorSet::setup(const Json::Value &data) {
		//bugbug good to read in to/from/color and other things
		//Fore, Back, Lightest, Darkest, Color1, Color2
		shared_ptr<AnimiatedColor> c = getAnimatedColor(Fore);
		if (c) {
			c->setup(data["fore"]);
		}
		if (c = getAnimatedColor(Back)) {
			c->setup(data["back"]);
		}
		if (c = getAnimatedColor(Back)) {
			c->setup(data["lightest"]);
		}
		if (c = getAnimatedColor(Back)) {
			c->setup(data["color1"]);
		}
		if (c = getAnimatedColor(Back)) {
			c->setup(data["color2"]);
		}
	}
	void ColorSet::update() {
		for (auto& c : colors) {
			c.second->update();
		}
	}
	// set 1 or more colors in the set bugbug function is just an example of var args now
	//void ColorSet::setSetcolors(int c, ...) {
	//	colors.clear();

	//va_list args;
	//va_start(args, c);
	//for (int i = 0; i < c; ++i) {
			// null is not saved
			//	if (va_arg(args, shared_ptr<AnimiatedColor>)) {
				//colors.push_back(va_arg(args, shared_ptr<AnimiatedColor>));
				//}
				//}
				//va_end(args);
				//}
	shared_ptr<ColorSet> ColorList::read(const Json::Value &data) {
		if (data.size() > 0) {
			//bugbug where is this allocated
			if (privateData) {
				string colorGroup;
				READSTRING(colorGroup, data);
				if (colorGroup.size() > 0) {
					privateData->currentColorSet = getNextColors(ColorSet::convertStringToGroup(colorGroup), true);
				}
			}
		}
		return privateData->currentColorSet;
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
		// local helper
		class aColor {
		public:
			aColor(std::unordered_map<char, int> *p) { map = p; }
			int get(char c) { if (map) return (*map)[c]; return 0; }

			ofColor color(char c, int alpha=255) { return ofColor::fromHex(get(c), alpha); }

			shared_ptr<AnimiatedColor> create(char c, int alpha, char c2, int alpha2=255) {
				return create(color('A', 255), color('B', 255));
			}

			shared_ptr<AnimiatedColor> create(const ofColor& c) {
				shared_ptr<AnimiatedColor>ac = std::make_shared<AnimiatedColor>();
				if (ac) {
					ac->setColor(c);
				}
				return ac;
			}
			shared_ptr<AnimiatedColor> create(const ofColor& start, const ofColor& end) {
				shared_ptr<AnimiatedColor>ac = std::make_shared<AnimiatedColor>();
				if (ac) {
					ac->setColor(start);//bugbug get from json
					ac->animateTo(end);//bugbug look at delays and final color, good json data
				}
				return ac;
			}
			// no animation
			shared_ptr<AnimiatedColor> create(char c) {
				shared_ptr<AnimiatedColor>ac = std::make_shared<AnimiatedColor>();
				if (ac) {
					ac->setColor(color(c));//bugbug get from json
				}
				return ac;
			}
			// only alpha changes
			shared_ptr<AnimiatedColor> create(char start, int alpha) {
				shared_ptr<AnimiatedColor>ac = std::make_shared<AnimiatedColor>();
				if (ac) {
					ac->setColor(color(start));
					ac->animateToAlpha(alpha);
				}
				return ac;
			}

			std::unordered_map<char, int> *map;
		};
#define CREATE(a) c.create(a)
		if (group == ColorSet::Modern) {
			aColor c(&privateData->modern);
			shared_ptr<AnimiatedColor>A = c.create('A');
			shared_ptr<AnimiatedColor>C = c.create('C');
			//add the range here with the helper
			addbasic(ColorSet::Modern, c.create('A', 255, 'B'), c.create('C', 100), C, A);

			//jabc, lightest:c, darkest: a, note sharing pointers is ok too, like using the pointer A in more places,
			// but in that case it will changes across colors, something that may be wanted
			shared_ptr<AnimiatedColor>J = c.create('J', 1);
			shared_ptr<AnimiatedColor>B = c.create('B');
			addfull(ColorSet::Modern, J, A, B, CREATE('C'), CREATE('C'), CREATE('A'));

			// jobl a pointer one that will change as others change
			addfull(ColorSet::Modern, J, CREATE('O'), B, CREATE('L'), CREATE('B'), CREATE('O'));
		}
		else if (group == ColorSet::White) {
			aColor c(nullptr);
			addbasic(ColorSet::White, CREATE(ofColor::white));//bugbug add "createColor":"white" kind of thing to json
		}
		else if (group == ColorSet::Orange) {
			aColor c(nullptr);
			addbasic(ColorSet::Orange, c.create(ofColor::orange, ofColor::orangeRed));
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
		if (!ret) {
			return std::make_shared<ColorSet>(); // always return something
		}
		return ret;
	}
	// add basic list
	shared_ptr<ColorSet>  ColorList::addbasic(const ColorSet::ColorGroup group, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor>back, shared_ptr<AnimiatedColor> lightest, shared_ptr<AnimiatedColor> darkest) {
		shared_ptr<ColorSet> s = std::make_shared<ColorSet>(group, fore, back, lightest, darkest);
		if (s) {
			privateData->colorlist.push_front(s);
		}
		return s;
	}
	// add a full color list
	void ColorList::addfull(const ColorSet::ColorGroup group, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor>back,
		shared_ptr<AnimiatedColor> color1, shared_ptr<AnimiatedColor> color2,
		shared_ptr<AnimiatedColor> lightest, shared_ptr<AnimiatedColor> darkest)
	{
		shared_ptr<ColorSet> s = addbasic(group, fore, back, lightest, darkest);
		if (s) {
			s->addColor(ColorSet::Color1, color1);
			s->addColor(ColorSet::Color2, color2);
		}
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
	}
	ColorSet::ColorSet(const ColorGroup groupIn, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor> back, shared_ptr<AnimiatedColor> lightest, shared_ptr<AnimiatedColor> darkest) : objectLifeTimeManager() {
		group = groupIn;
		defaultColor = ofColor::greenYellow;
		if (fore) {
			colors[Fore] = fore; // never set a null
		}
		if (back) {
			colors[Back] = back; // never set a null
		}
		if (lightest) {
			colors[Lightest] = lightest; // never set a null
		}
		if (darkest) {
			colors[Darkest] = darkest; // never set a null
		}
	}
	ofColor ColorSet::getColor1() {
		ColorMap::iterator itr = colors.find(Color1);
		if (itr == colors.end()) {
			ofColor c = getBackground();// start with fore color
			c.setSaturation(c.getSaturation() + 50);// may wrap around?
			c.setBrightness(c.getBrightness() + 20);// may wrap around?
			return c;
		}
		return itr->second->getCurrentColor();
	}
	ofColor ColorSet::getBackground() {
		ColorMap::iterator itr = colors.find(Back);
		if (itr == colors.end()) {
			return get(Fore).getInverted();//only color we can assume is Fore, but maybe we can do more to make it unique
		}
		return itr->second->getCurrentColor();
	}

	ofColor ColorSet::getColor2() {
		ColorMap::iterator itr = colors.find(Color2);
		if (itr == colors.end()) {
			ofColor c = getForeground();// start with fore color
			c.setSaturation(c.getSaturation() + 10);// may wrap around?
			c.setBrightness(c.getBrightness() + 10);// may wrap around?
			return c;
		}
		return itr->second->getCurrentColor();
	}

	ofColor ColorSet::getLightest() {
		// assume data is propery allocated in the colors vector
		ColorMap::iterator itr = colors.find(Lightest);
		if (itr == colors.end()) {
			ofColor c = get(Fore);// start with fore color
			c.setSaturation(c.getSaturation() - 50);// may wrap around?
			c.setBrightness(c.getBrightness() - 20);// may wrap around?
			return c;
		}
		return itr->second->getCurrentColor();
	}
	ofColor ColorSet::getDarkest() {
		ColorMap::iterator itr = colors.find(Darkest);
		if (itr == colors.end()) {
			ofColor c = getBackground();// start with fore color
			c.setSaturation(c.getSaturation() + 50);// may wrap around?
			c.setBrightness(c.getBrightness() + 20);// may wrap around?
			return c;
		}
		return itr->second->getCurrentColor();
	}
	ColorSet::ColorSet() {
		group = Modern;
		defaultColor = ofColor::yellow;
		shared_ptr<AnimiatedColor>c = std::make_shared<AnimiatedColor>();
		if (c) {
			c->setColor(defaultColor);
			colors[Fore] = c;
		}
	};

	ColorSet::ColorSet(const ColorGroup groupIn, shared_ptr<AnimiatedColor> fore) : objectLifeTimeManager() {
		group = groupIn;
		defaultColor = ofColor::greenYellow;//c.invert(); lerp ofColor r = ofColor::red;
									  ///     ofColor b = ofColor::blue;
									  ///     b.lerp(r, 0.5); // now purple
		if (fore) {
			colors[Fore] = fore; // never set a null
		}
	}

	//http://www.creativecolorschemes.com/resources/free-color-schemes/art-deco-color-scheme.shtml
	void ColorList::setup() {
		// assumes static data so color info is shared across app
		if (privateData == nullptr) {
			privateData = std::make_shared<colordata>();
		}
		if (privateData == nullptr) {
			return;
		}
		if (privateData->currentColorSet == nullptr) {
			privateData->currentColorSet = getNextColors(ColorSet::Modern, true);// make sure there is a current color
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