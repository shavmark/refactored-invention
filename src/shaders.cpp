#include "ofApp.h"
#include "shaders.h"

// define a default set of shaders, other shaders can be added via json files

//generative art

// only GL3 is supported

//http://www.kynd.info/log/
//http://www.flong.com/
//https://github.com/princemio/ofxMIOFlowGLSL/blob/master/src/ofxMioFlowGLSL.cpp
//http://openglbook.com/chapter-0-preface-what-is-opengl.html
//https://thebookofshaders.com/05/
//https://www.asus.com/support/faq/1013017/

namespace Software2552 {
	// return common header code (private)
	string Shader::codeHeader() {
		string s = "#version 150\n"; // correct version?
#ifdef GL_ES
		s += "precision mediump float;\n";
#endif
		s += "uniform vec2 u_mouse;\n"; // share with all
		s += "uniform vec2 u_resolution;\n"; // share with all
		s += "uniform float u_time;\n"; // share with all
		return s;
	}
	// for non file based json, ie internal shaders. bugbug add way to load from file some day too
	Json::Value Shader::buildCodeJson(const string& name, const string&fragment, const string&vertex) {
		Json::Value val;

		val["name"] = name;
		if (fragment.size() > 0) {
			val["fragment"] = fragment;
		}
		if (vertex.size() > 0) {
			val["vertex"] = vertex;
		}
		return val;
	}
	// data driven
	Json::Value Shader::buildCodeJson(const string& name) {
		Json::Value val;

		val["name"] = name;
		if (name == "zigzag") {
			buildCodeJson(name, zigzag(true), zigzag(false));
		}
		return val;
	}
	// return true if shader is loaded
	bool Shader::setup(const Json::Value & val)	{
		string fragment;
		string vertex;

		if (val["name"] == "zigzag") {
			fragment = zigzag(true);
			vertex = zigzag(false);
		}
		else if (val["name"] == "basic") {
			fragment = basic(true);
			vertex = basic(false);
		}
		else if (val["name"] == "digits") {
			fragment = digits(true);
			vertex = digits(false);
		}
		else if (val["name"] == "userdefined"){
			fragment = val["fragment"].asString();
			vertex = val["vertex"].asString(); // bugbug do we need to check size first?
		}

		if (vertex.size() > 0) {
			shader.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
		}
		if (fragment.size() > 0) {
			shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragment);
		}
		if (vertex.size() > 0 || fragment.size() > 0) {
			shader.bindDefaults();
			shader.linkProgram();
		}
		return shader.isLoaded();
	}
	void Shader::start() {
		shader.begin();
		// true for all our shaders (from https://thebookofshaders.com)
		shader.setUniform1f("u_time", ofGetElapsedTimef());
		shader.setUniform2f("u_resolution", ofGetWidth(), ofGetHeight());
		shader.setUniform2f("u_mouse", ((ofApp*)ofGetAppPtr())->mouseX, ((ofApp*)ofGetAppPtr())->mouseY);
	}

	void Shader::myDraw() {
		start();
		ofPushMatrix();
		ofSetColor(ofColor::royalBlue);

		//ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		ofFill();
		ofRect(0, 0, ofGetWidth(), ofGetHeight());
		ofPopMatrix();
		shader.end();
	}

	// for now all shaders start here, but in phase 1b people can add them in via json, once more is understood how things will work in this regard
	// so there is a name in the shader json, but that can become UserDefined bugbug
	string basic(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(

				void main() {
				float r = gl_FragCoord.x / u_resolution.x;
				float g = gl_FragCoord.y / u_resolution.y;
				gl_FragColor = vec4(r, g, 1.0, 1.0);
			}
			);
		}
		else {
			return "";
		}
	}
	// Author @patriciogv - 2015
	// Title: Ikeda Digits
	string digits(bool fragment) {
		if (fragment) {
			//bugbug define json, include a reference tag 
			string frag = Shader::codeHeader();
			frag += STRINGIFY(
			float random(in float x) { return fract(sin(x)*43758.5453); }
			// float random(in vec2 st){ return fract(sin(dot(st.xy ,vec2(12.9898,78.233))) * 43758.5453); }
			float random(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

			float bin(vec2 ipos, float n) {
				float remain = mod(n, 33554432.);
				for (float i = 0.0; i < 25.0; i++) {
					if (floor(i / 3.) == ipos.y && mod(i, 3.) == ipos.x) {
						return step(1.0, mod(remain, 2.));
					}
					remain = ceil(remain / 2.);
				}
				return 0.0;
			}

			float char(vec2 st, float n) {
				st.x = st.x*2. - 0.5;
				st.y = st.y*1.2 - 0.1;

				vec2 grid = vec2(3., 5.);

				vec2 ipos = floor(st*grid);
				vec2 fpos = fract(st*grid);

				n = floor(mod(n, 10.));
				float digit = 0.0;
				if (n < 1.) { digit = 31600.; }
				else if (n < 2.) { digit = 9363.0; }
				else if (n < 3.) { digit = 31184.0; }
				else if (n < 4.) { digit = 31208.0; }
				else if (n < 5.) { digit = 23525.0; }
				else if (n < 6.) { digit = 29672.0; }
				else if (n < 7.) { digit = 29680.0; }
				else if (n < 8.) { digit = 31013.0; }
				else if (n < 9.) { digit = 31728.0; }
				else if (n < 10.) { digit = 31717.0; }
				float pct = bin(ipos, digit);

				vec2 borders = vec2(1.);
				// borders *= step(0.01,fpos.x) * step(0.01,fpos.y);   // inner
				borders *= step(0.0, st)*step(0.0, 1. - st);            // outer

				return step(.5, 1.0 - pct) * borders.x * borders.y;
			}

			void main() {
				vec2 st = gl_FragCoord.st / u_resolution.xy;
				st.x *= u_resolution.x / u_resolution.y;

				float rows = 34.0;
				vec2 ipos = floor(st*rows);
				vec2 fpos = fract(st*rows);

				ipos += vec2(0., floor(u_time*20.*random(ipos.x + 1.)));
				float pct = random(ipos);
				vec3 color = vec3(char(fpos, 100.*pct));
				color = mix(color, vec3(color.r, 0., 0.), step(.99, pct));

				gl_FragColor = vec4(color, 1.0);
			}
			);
			return frag;
		}
		else {
			return "";
		}
	}
	// return json string for Zigzag
	// Author @patriciogv - 2015
	// Title: Zigzag
	string zigzag(bool fragment) {
		if (fragment) {
			//bugbug define json, include a reference tag 
			string frag = Shader::codeHeader();
			frag += STRINGIFY(
				vec2 mirrorTile(vec2 _st, float _zoom) {
				_st *= _zoom;
				if (fract(_st.y * 0.5) > 0.5) {
					_st.x = _st.x + 0.5;
					_st.y = 1.0 - _st.y;
				}
				return fract(_st);
			}

			float fillY(vec2 _st, float _pct, float _antia) {
				return  smoothstep(_pct - _antia, _pct, _st.y);
			}

			void main() {
				vec2 st = gl_FragCoord.xy / u_resolution.xy;
				vec3 color = vec3(0.0);

				st = mirrorTile(st*vec2(1., 2.), 5.);
				float x = st.x*2.;
				float a = floor(1. + sin(x*3.14));
				float b = floor(1. + sin((x + 1.)*3.14));
				float f = fract(x);

				color = vec3(fillY(st, mix(a, b, f), 0.01));

				gl_FragColor = vec4(color, 1.0);
			}
		);

			return frag;
		}
		else {
			return "";
		}
	}


}