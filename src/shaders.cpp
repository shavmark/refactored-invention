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
		else if (val["name"] == "green") {
			fragment = green(true);
			vertex = green(false);
		}
		else if (val["name"] == "grid") {
			fragment = grid(true);
			vertex = grid(false);
		}
		else if (val["name"] == "mosaic") {
			fragment = mosaic(true);
			vertex = mosaic(false);
		}
		else if (val["name"] == "smooth") {
			fragment = smooth(true);
			vertex = smooth(false);
		}
		else if (val["name"] == "groovy") {
			fragment = groovy(true);
			vertex = groovy(false);
		}
		else if (val["name"] == "red") {
			fragment = red(true);
			vertex = red(false);
		}
		else if (val["name"] == "userdefined"){
			if (val["fragment"].asString().size() > 0) {
				fragment = Shader::codeHeader() + val["fragment"].asString();
			}
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
		//bugbug add kinect stuff, voice stuff go beyond mouse
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
	//http://glslsandbox.com/e#32867.0
	string groovy(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(
			void main(void)
			{

				vec2 uv = (gl_FragCoord.xy / u_resolution.xy)*4.0;

				vec2 uv0 = uv;
				float i0 = 1.0;
				float i1 = 1.0;
				float i2 = 1.0;
				float i4 = 0.0;
				for (int s = 0; s < 7; s++)
				{
					vec2 r;
					r = vec2(cos(uv.y*i0 - i4 + u_time / i1), sin(uv.x*i0 - i4 + u_time / i1)) / i2;
					r += vec2(-r.y, r.x)*0.3;
					uv.xy += r;

					i0 *= 1.93;
					i1 *= 1.15;
					i2 *= 1.7;
					i4 += 0.05 + 0.1*u_time*i1;
				}
				float r = sin(uv.x - u_time)*0.5 + 0.5;
				float b = sin(uv.y + u_time)*0.5 + 0.5;
				float g = sin((sqrt(uv.x*uv.x + uv.y*uv.y) + u_time))*0.5 + 0.5;
				vec3 c = vec3(r, g, b);
				gl_FragColor = vec4(c, 1.0);

				gl_FragColor *= vec4(c - 0.5*sqrt(uv.x*uv.x + uv.y*uv.y), 1.0);
				gl_FragColor *= vec4(c - 1.0*sqrt(uv.x*uv.x + uv.y*uv.y), 1.0);
			}
			);
		}
		else {
			return "";
		}
	}
	// Author unknown but from http://glslsandbox.com/e#32842.2
	// Title: smooth
	string smooth(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(
			void main(void)
			{
				vec2 uv = (gl_FragCoord.xy / u_resolution.xy)*4.0;

				vec2 uv0 = uv;
				float i0 = 1.2;
				float i1 = 0.95;
				float i2 = 1.5;
				vec2 i4 = vec2(0.0, 0.0);
				for (int s = 0; s < 4; s++)
				{
					vec2 r;
					r = vec2(cos(uv.y*i0 - i4.y + u_time / i1), sin(uv.x*i0 + i4.x + u_time / i1)) / i2;
					r += vec2(-r.y, r.x)*0.2;
					uv.xy += r;

					i0 *= 1.93;
					i1 *= 1.25;
					i2 *= 1.7;
					i4 += r.xy*1.0 + 0.5*u_time*i1;
				}
				float r = sin(uv.x - u_time)*0.5 + 0.5;
				float b = sin(uv.y + u_time)*0.5 + 0.5;
				float g = sin((sqrt(uv.x*uv.x + uv.y*uv.y) + u_time))*0.5 + 0.5;
				vec3 c = vec3(r, g, b);
				gl_FragColor = vec4(c, 1.0);
			}
			);
		}
		else {
			return "";
		}

	}
	string shadertemplate(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(
				hi
			);
			return frag;
		}
		else {
			return "";// return vector
		}

	}
	// http://glslsandbox.com/e#32902.0
	string red(bool fragment) {
		if (fragment) {
			string frag = "#extension GL_OES_standard_derivatives : enable\n";
			frag += Shader::codeHeader();
			return frag + STRINGIFY(
				void main(void) {
				vec2 uv = (02.0 * gl_FragCoord.xy / u_resolution.xy - 1.0) * vec2(u_resolution.x / u_resolution.y, 9.50);

				float a = 71.90*atan(uv.y / uv.x);
				uv /= 0.5 + 0.202 * sin(115.0 * a - u_time * 16.0);

				float f = 0.40 + 0.2 * sin(u_time * 04786.14);
				float d = (abs(length(uv) - f) * 1.0);

				gl_FragColor += vec4(09.3 / d, 0.62 / d, 0.22 / d, 1);
				}
			);
		}
		else {
			return "";
		}

	}
	// Author @patriciogv - 2015
	// Title: Mosaic
	string mosaic(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(

			float random(vec2 st) {
				return fract(sin(dot(st.xy,
					vec2(12.9898, 78.233)))*
					43758.5453123);
			}

			void main() {
				vec2 st = gl_FragCoord.xy / u_resolution.xy;

				st *= 10.0; // Scale the coordinate system by 10
				vec2 ipos = floor(st);  // get the integer coords
				vec2 fpos = fract(st);  // get the fractional coords

										// Assign a random value based on the integer coord
				vec3 color = vec3(random(ipos));

				// Uncomment to see the subdivided grid
				color = vec3(fpos,0.0);

				gl_FragColor = vec4(color, 1.0);
			}
			);
		}
		else {
			return "";
		}
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
		}
		else {
			return "";
		}
	}
	// Author @patriciogv - 2015
	// http://patriciogonzalezvivo.com
	string green(bool fragment) {
		if (fragment) {

			string frag = Shader::codeHeader();
			frag += STRINGIFY(
				float plot(vec2 _st, float _pct) {
				return  smoothstep(_pct - 0.01, _pct, _st.y) -
					smoothstep(_pct, _pct + 0.01, _st.y);
			}

			float random(in float _x) {
				return fract(sin(_x)*43758.5453);
			}

			void main() {
				vec2 st = gl_FragCoord.xy / u_resolution.xy;
				st.x *= u_resolution.x / u_resolution.y;
				vec3 color = vec3(0.0);

				float y = random(st.x*0.001 + u_time);

				// color = vec3(y);
				float pct = plot(st, y);
				color = (1.0 - pct)*color + pct*vec3(0.0, 1.0, 0.0);

				gl_FragColor = vec4(color, 1.0);
			}
			);
		}
		else {
			return "";
		}
	}
	// Author @patriciogv - 2015
	// Title: Ikeda Numered Grid
	string grid(bool fragment) {
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

				float grid(vec2 st, float res) {
					vec2 grid = fract(st*res);
					return 1. - (step(res, grid.x) * step(res, grid.y));
				}

				float box(in vec2 st, in vec2 size) {
					size = vec2(0.5) - size*0.5;
					vec2 uv = smoothstep(size,
						size + vec2(0.001),
						st);
					uv *= smoothstep(size,
						size + vec2(0.001),
						vec2(1.0) - st);
					return uv.x*uv.y;
				}

				float cross(in vec2 st, vec2 size) {
					return  clamp(box(st, vec2(size.x*0.5, size.y*0.125)) +
						box(st, vec2(size.y*0.125, size.x*0.5)), 0., 1.);
				}

				void main() {
					vec2 st = gl_FragCoord.st / u_resolution.xy;
					st.x *= u_resolution.x / u_resolution.y;

					vec3 color = vec3(0.0);

					// Grid
					vec2 grid_st = st*300.;
					color += vec3(0.5, 0., 0.)*grid(grid_st, 0.01);
					color += vec3(0.2, 0., 0.)*grid(grid_st, 0.02);
					color += vec3(0.2)*grid(grid_st, 0.1);

					// Crosses
					vec2 crosses_st = st + .5;
					crosses_st *= 3.;
					vec2 crosses_st_f = fract(crosses_st);
					color *= 1. - cross(crosses_st_f, vec2(.3, .3));
					color += vec3(.9)*cross(crosses_st_f, vec2(.2, .2));

					// Digits
					vec2 digits_st = mod(st*60., 20.);
					vec2 digits_st_i = floor(digits_st);
					if (digits_st_i.y == 1. &&
						digits_st_i.x > 0. && digits_st_i.x < 6.) {
						vec2 digits_st_f = fract(digits_st);
						float pct = random(digits_st_i + floor(crosses_st) + floor(u_time*20.));
						color += vec3(char(digits_st_f, 100.*pct));
					}
					else if (digits_st_i.y == 2. &&
						digits_st_i.x > 0. && digits_st_i.x < 8.) {
						vec2 digits_st_f = fract(digits_st);
						float pct = random(digits_st_i + floor(crosses_st) + floor(u_time*20.));
						color += vec3(char(digits_st_f, 100.*pct));
					}
					gl_FragColor = vec4(color, 1.0);
				}
			);
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
		}
		else {
			return "";
		}
	}


}