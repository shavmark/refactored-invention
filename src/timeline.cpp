#include "timeline.h"
#include "model.h"
namespace Software2552 {

	Timeline::Timeline(){
		//bugbug should we get this from data too at some point? these items cannot be set for each object ofColor colorOne(255, 0, 0);
		// bugbug a timeline json?

	}
	// read one act and save it to the list of acts (ie a story)
	bool Timeline::readScript(const string& path) {

		//return playlist.read(path);
		return true;
	}
	void Timeline::start() {
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->setup();
			playlist.getCurrent()->start();
		}
	}
	void Timeline::resume() {
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->resume();
		}
				
	}
	void Timeline::pause() {
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->pause();
		}
	}
#if 0 
	if ((x >= 0) && (x < getKinect()->getFrameWidth()) && (y >= 0) && (y < getKinect()->getFrameHeight())) {
		if (leftHandState == HandState::HandState_Open) {
			ofDrawCircle(x, y, 30);
		}
		else if (leftHandState == HandState::HandState_Closed) {
			ofDrawCircle(x, y, 5);
		}
		else if (leftHandState == HandState::HandState_Lasso) {
			ofDrawCircle(x, y, 15);
		}
	}
	if (!drawface) {
		if ((joints[type].JointType == JointType::JointType_Head) |
			(joints[type].JointType == JointType::JointType_Neck)) {
			continue;// assume face is drawn elsewhere
		}
}
	if ((x >= 0) && (x < getKinect()->getFrameWidth()) && (y >= 0) && (y < getKinect()->getFrameHeight())) {
		ofDrawCircle(x, y, 10);
	}
	if (faceProperty[FaceProperty_LeftEyeClosed] != DetectionResult_Yes) {
		ofDrawCircle(leftEye().X - 15, leftEye().Y, 10);
	}
	if (faceProperty[FaceProperty_RightEyeClosed] != DetectionResult_Yes) {
		ofDrawCircle(rightEye().X + 15, rightEye().Y, 10);
	}
	ofDrawCircle(nose().X, nose().Y, 5);
	if (faceProperty[FaceProperty_Happy] == DetectionResult_Yes || faceProperty[FaceProperty_Happy] == DetectionResult_Maybe || faceProperty[FaceProperty_Happy] == DetectionResult_Unknown) {
		// smile as much as possible
		ofDrawCurve(mouthCornerLeft().X - 70, mouthCornerLeft().Y - 70, mouthCornerLeft().X, mouthCornerRight().Y + 30, mouthCornerRight().X, mouthCornerRight().Y + 30, mouthCornerRight().X + 70, mouthCornerRight().Y - 70);
	}
	else {
		float height;
		float offset = 0;
		if (faceProperty[FaceProperty_MouthOpen] == DetectionResult_Yes || faceProperty[FaceProperty_MouthOpen] == DetectionResult_Maybe) {
			height = 60.0;
			offset = height / 2;
		}
		else {
			height = 5.0;
			offset = 10;
		}
		if (mouthCornerRight().X > 0) {
			//points2String();
		}
		float width = abs(mouthCornerRight().X - mouthCornerLeft().X);
		ofDrawEllipse(mouthCornerLeft().X - 5, mouthCornerLeft().Y + offset, width + 5, height);
	}
	}
void KinectFaces::ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll)
{
	double x = pQuaternion->x;
	double y = pQuaternion->y;
	double z = pQuaternion->z;
	double w = pQuaternion->w;

	// convert face rotation quaternion to Euler angles in degrees
	*pPitch = static_cast<int>(std::atan2(2 * (y * z + w * x), w * w - x * x - y * y + z * z) / M_PI * 180.0f);
	*pYaw = static_cast<int>(std::asin(2 * (w * y - x * z)) / M_PI * 180.0f);
	*pRoll = static_cast<int>(std::atan2(2 * (x * y + w * z), w * w + x * x - y * y - z * z) / M_PI * 180.0f);
}


#endif

	void Timeline::setup() {
		//ofSeedRandom(); // turn of to debug if needed
		
		ofSetVerticalSync(false);
		ofSetFrameRate(frameRate);
		colorlist.setup();
		stage.setup();
		read.setup();
		//write.setup();
		ofxJSON data;
		data.open("json4.json");// use json editor vs. coding it up
		//write.send(data, "graphics");
		return;

		mesh.setup();
	}
	// keep this super fast
	void Timeline::update() { 
		//kinect/joints kinect/face kinect/audio kinect/body kinect/audioCommand
		shared_ptr<ofxJSON> joints = read.get("kinect/joints");
		string s;
		if (joints) {
			s = joints->getRawString();
			int type = (*joints)["jointType"].asInt();
			x = (*joints)["depth"]["x"].asFloat();
			y = (*joints)["depth"]["y"].asFloat();
			x2 = (*joints)["color"]["x"].asFloat();
			y2 = (*joints)["color"]["y"].asFloat();
			x3 = (*joints)["cam"]["x"].asFloat();
			y3 = (*joints)["cam"]["y"].asFloat();
			z3 = (*joints)["cam"]["z"].asFloat();

		}
		shared_ptr<ofxJSON> face = read.get("kinect/face");
		if (face) {
			s = face->getRawString();
		}
		shared_ptr<ofxJSON> body = read.get("kinect/body");
		if (body) {
			s = body->getRawString();
		}
		shared_ptr<ofxJSON> audio = read.get("kinect/audio");
		if (audio) {
			s = audio->getRawString();
		}
		shared_ptr<ofxJSON> audioCommand = read.get("kinect/audioCommand");
		if (audioCommand) {
			s = audioCommand->getRawString();
		}

		//stage.updateData(read.get("graphics")); // data can come from files, http/s, osc ++
		stage.update();
		return;

		playlist.removeExpiredItems();
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->update();
		}
		else {
			ofExit();
		}
		colorlist.update();
	};

	// keep as fast as possible
	void Timeline::draw() {
		stage.draw();
		//mesh.draw();
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->draw();
		}
	};

}