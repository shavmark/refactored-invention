#pragma once

// shader helpers

namespace Software2552 {
	string zigzag(bool fragment=true); // shader functions
	string basic(bool fragment = true);
	string digits(bool fragment = true);

	class Shader : public ActorRole{
	public:
		bool setup(const Json::Value & val);
		virtual void myDraw();

		static string codeHeader();
		Json::Value buildCodeJson(const string& name, const string&fragment, const string&vertex);
		Json::Value buildCodeJson(const string& name);
	private:
		void start();
		ofShader shader;
	};
}
