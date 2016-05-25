#pragma once

// shader helpers

namespace Software2552 {
	string zigzag(bool fragment=true); // shader function

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
