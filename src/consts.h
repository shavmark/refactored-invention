#pragma once
namespace Software2552 {
	// kinect stuff 
	static float getDepthFrameWidth() { return 512.0f; }
	static float getDepthFrameHeight() { return 424.0f; }
	static float getIRFrameWidth() { return 512.0f; }
	static float getIRFrameHeight() { return 424.0f; }
	static float getColorFrameWidth() { return 1920.0f; }
	static float getColorFrameHeight() { return 1080.0f; }

	static float ratioColorToScreenX() { return (float)ofGetScreenWidth() / getColorFrameWidth(); }
	static float ratioColorToScreenY() { return (float)ofGetScreenHeight() / getColorFrameHeight(); }

	static float ratioDepthToScreenX() { return (float)ofGetScreenWidth() / getDepthFrameWidth(); }
	static float ratioDepthToScreenY() { return (float)ofGetScreenHeight() / getDepthFrameHeight(); }

	static float ratioColorToDepthX() { return (float)getDepthFrameWidth() / getColorFrameWidth(); }
	static float ratioColorToDepthY() { return (float)getDepthFrameHeight() / getColorFrameHeight(); }

	enum DataType : char {
		TCPID = 't',	//116
		BodyIndexID = 'x',//120
		IrID = 'i',		//105
		BodyID = 'b',	// 98
		JsonID = 'j',	//106
		Pixels = 'p',		//12
		UnknownID = 'k'
	};
}
