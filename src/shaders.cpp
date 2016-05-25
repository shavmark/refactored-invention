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
		s += "#define PI 3.14159265359\n"; // share with all

#ifdef GL_ES
		s += "precision mediump float;\n";
#endif
		return s + STRINGIFY(
		uniform vec2 u_resolution;
		uniform vec2 u_mouse;
		uniform float u_time
			);
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

	bool Shader::setup(const Json::Value & val)	{
		ofShader shader;
		string fragment = val["fragment"].asString();
		string vertex = val["vertex"].asString(); // bugbug do we need to check size first?

		if (vertex.size() > 0) {
			shader.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
		}
		if (fragment.size() > 0) {
			shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragment);
		}
		shader.bindDefaults();
		shader.linkProgram();
		return true;
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
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		ofRect(0, 0, ofGetWidth(), ofGetHeight());
		ofPopMatrix();
		shader.end();
	}

	// for now all shaders start here, but in phase 1b people can add them in via json, once more is understood how things will work in this regard
	// so there is a name in the shader json, but that can become UserDefined bugbug

	// return json string for Zigzag
	// Author @patriciogv - 2015
	// Title: Zigzag
	string zigzag(bool fragment) {
		if (fragment) {
			//bugbug define json, include a reference tag 
			string frag = Shader::codeHeader();
			frag += STRINGIFY(
				vec2 mirrorTile(vec2 st, float zoom) {
				st *= zoom;
				if (fract(st.y * 0.5) > 0.5) {
					st.x = st.x + 0.5;
					st.y = 1.0 - st.y;
				}
				return fract(st);
			}

			float fillY(vec2 st, float pct, float antia) {
				return  smoothstep(pct - antia, pct, st.y);
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