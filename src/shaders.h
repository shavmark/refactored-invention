#pragma once

// shader helpers

namespace Software2552 {
	string zigzag(bool fragment=true); // shader functions
	string basic(bool fragment = true);
	string digits(bool fragment = true);
	string green(bool fragment = true);
	string grid(bool fragment = true);
	string mosaic(bool fragment = true);
	string smooth(bool fragment = true);
	string groovy(bool fragment = true);
	string red(bool fragment = true);
	string greenCircle(bool fragment = true);
	string sea(bool fragment = true);

	Json::Value buildCodeJson(const string& name, const string&fragment, const string&vertex);
	Json::Value buildCodeJson(const string& name);

	class Shader : public ActorRole{
	public:
		bool setup(const Json::Value & val);
		virtual void myDraw();
		void myUpdate();
		static string codeHeader();
	private:
		void start();
		void end();
		int index = -1; // none
		//MAP ofShader to 
		vector<shared_ptr<pair<FrameCounter, shared_ptr<ofShader>>>> shaders;
		bool getShader(const Json::Value &val, shared_ptr<ofShader> shader);
	};
}
