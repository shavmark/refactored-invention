#ifdef _WIN64
// kinect only works on64 bit windows

#include "ofApp.h"
#include "kinect2552.h"
//SetKinectTwoPersonSystemEngagement vs one?
//InputPointerManager.TransformInputPointerCoordinatesToWindowCoordinates
// https://social.msdn.microsoft.com/Forums/en-US/f29a202a-fa27-4cfc-9079-5addad0906e0/how-can-i-map-a-depth-frame-to-camera-space-without-having-a-kinect-on-hand?forum=kinectv2sdk
//file:///C:/Users/mark/Downloads/KinectHIG.2.0.pdf

namespace Software2552 {
	static const double faceRotationIncrementInDegrees = 5.0f;
	
void ExtractFaceRotationInDegrees(const Vector4& pQuaternion, int& pitch, int& yaw, int&roll)	{
	double x = pQuaternion.x;
	double y = pQuaternion.y;
	double z = pQuaternion.z;
	double w = pQuaternion.w;

	// convert face rotation quaternion to Euler angles in degrees		
	double dPitch, dYaw, dRoll;
	dPitch = atan2(2 * (y * z + w * x), w * w - x * x - y * y + z * z) / M_PI * 180.0;
	dYaw = asin(2 * (w * y - x * z)) / M_PI * 180.0;
	dRoll = atan2(2 * (x * y + w * z), w * w + x * x - y * y - z * z) / M_PI * 180.0;

	// clamp rotation values in degrees to a specified range of values to control the refresh rate
	double increment = faceRotationIncrementInDegrees;
	pitch = static_cast<int>(floor((dPitch + increment / 2.0 * (dPitch > 0 ? 1.0 : -1.0)) / increment) * increment);
	yaw = static_cast<int>(floor((dYaw + increment / 2.0 * (dYaw > 0 ? 1.0 : -1.0)) / increment) * increment);
	roll = static_cast<int>(floor((dRoll + increment / 2.0 * (dRoll > 0 ? 1.0 : -1.0)) / increment) * increment);
}
IBodyFrame* getBody(IMultiSourceFrame* frame) {
		IBodyFrame* bodyframe=nullptr;
		if (frame) {
			IBodyFrameReference* pFrameref = NULL;
			frame->get_BodyFrameReference(&pFrameref);
			if (pFrameref) {
				pFrameref->AcquireFrame(&bodyframe);
				pFrameref->Release();
			}
		}
		return bodyframe;
	}
	IBodyIndexFrame* getBodyIndex(IMultiSourceFrame* frame) {
		IBodyIndexFrame* bodyIndexframe = nullptr;
		if (frame) {
			IBodyIndexFrameReference* pFrameref = NULL;
			frame->get_BodyIndexFrameReference(&pFrameref);
			if (pFrameref) {
				pFrameref->AcquireFrame(&bodyIndexframe);
				pFrameref->Release();
			}
		}
		return bodyIndexframe;
	}
	IInfraredFrame* getInfrared(IMultiSourceFrame* frame) {
		IInfraredFrame* infraredframe = nullptr;
		if (frame) {
			IInfraredFrameReference* pFrameref = NULL;
			frame->get_InfraredFrameReference(&pFrameref);
			if (pFrameref) {
				pFrameref->AcquireFrame(&infraredframe);
				pFrameref->Release();
			}
		}
		return infraredframe;
	}
	ILongExposureInfraredFrame* getLongExposureInfrared(IMultiSourceFrame* frame) {
		ILongExposureInfraredFrame* infraredframe = nullptr;
		if (frame) {
			
			ILongExposureInfraredFrameReference* pFrameref = NULL;
			frame->get_LongExposureInfraredFrameReference (&pFrameref);
			if (pFrameref) {
				pFrameref->AcquireFrame(&infraredframe);
				pFrameref->Release();
			}
		}
		return infraredframe;
	}
	IDepthFrame* getDepth(IMultiSourceFrame* frame) {
		IDepthFrame* depthframe = nullptr;
		if (frame) {
			IDepthFrameReference* pFrameref = NULL;
			frame->get_DepthFrameReference(&pFrameref);
			if (pFrameref) {
				pFrameref->AcquireFrame(&depthframe);
				pFrameref->Release();
			}
		}
		return depthframe;
	}
	IColorFrame* getColor(IMultiSourceFrame* frame) {
		IColorFrame* colorframe=nullptr;
		if (frame) {
			IColorFrameReference* pFrameref = NULL;
			frame->get_ColorFrameReference(&pFrameref);
			if (pFrameref) {
				pFrameref->AcquireFrame(&colorframe);
				pFrameref->Release();
			}
		}
		return colorframe;
	}

	KinectDevice::~KinectDevice() {
		if (pSensor) {
			pSensor->Close();
		}
		SafeRelease(pSensor);
		SafeRelease(pCoordinateMapper);
	}
	bool KinectDevice::getIR() {
		if (ir || (getSender() && getSender()->kinectIREnabled())) {
			return true;
		}
		return ir;
	}
	bool KinectDevice::getBodyIndex() {
		if (bi || (getSender() && getSender()->KinectBodyIndexEndabled())) {
			return true;
		}
		return ir;
	}
	bool KinectDevice::getBody() {
		if (body || (getSender() && getSender()->KinectBodyEnabled())) {
			return true;
		}
		return ir;
	}

	//const const int colorwidth = 1920;
	//const const int colorheight = 1080;
	//bugbug send these phase II
	void depth2RGB(KinectDevice *kinect, unsigned short*buffer, float*destrgb, unsigned char*rgbimage) {
		ColorSpacePoint * depth2rgb = new ColorSpacePoint[512 * 424];     // Maps depth pixels to rgb pixels
		if (depth2rgb) {
			HRESULT hResult = kinect->getMapper()->MapDepthFrameToColorSpace(
				512 * 424, buffer,        // Depth frame data and size of depth frame
				512 * 424, depth2rgb); // Output ColorSpacePoint array and size
									   // Write color array for vertices
			float* fdest = (float*)destrgb;
			for (int i = 0; i < 512 * 424; i++) {
				ColorSpacePoint p = depth2rgb[i];
				// Check if color pixel coordinates are in bounds
				if (p.X < 0 || p.Y < 0 || p.X > 1920 || p.Y > 1080) {
					*fdest++ = 0;
					*fdest++ = 0;
					*fdest++ = 0;
				}
				else {
					int idx = (int)p.X + 1920 * (int)p.Y;
					*fdest++ = rgbimage[4 * idx + 0] / 255.;
					*fdest++ = rgbimage[4 * idx + 1] / 255.;
					*fdest++ = rgbimage[4 * idx + 2] / 255.;
				}
				// Don't copy alpha channel
			}
			delete depth2rgb;
		}
	}
	void depth2XYZ(KinectDevice *kinect, unsigned short*buffer, float*destXYZ) {
		CameraSpacePoint * depth2xyz = new CameraSpacePoint[512 * 424];     // Maps depth pixels to rgb pixels
		if (depth2xyz) {
			HRESULT hResult = kinect->getMapper()->MapDepthFrameToCameraSpace(
				512 * 424, buffer,        // Depth frame data and size of depth frame
				512 * 424, depth2xyz); // Output CameraSpacePoint array and size
			for (int i = 0; i <512 * 424; i++) { // map to points (not sure what this is yet)
				destXYZ[3 * i + 0] = depth2xyz[i].X;
				destXYZ[3 * i + 1] = depth2xyz[i].Y;
				destXYZ[3 * i + 2] = depth2xyz[i].Z;
			}
			delete depth2xyz;
		}

	}
	void KinectBody::updateImageBodyIndex(IMultiSourceFrame* frame) {
		IBodyIndexFrame * bodyindex = getBodyIndex(frame);
		if (!bodyindex) {
			return;
		}

		unsigned int bufferSize = 0;
		unsigned char* buffer = nullptr;//bugbug where does this get deleted? when we free bodyindex?
		HRESULT hResult = bodyindex->AccessUnderlyingBuffer(&bufferSize, &buffer);
		if (SUCCEEDED(hResult)) {
			// only send if real data is present
			bool found = false;
			for (int i = 0; i < bufferSize; ++i) {
				if (buffer[i] != 0xff) {
					found = true;//bugbug lots of scans but I am not sure how to find if something is not in a buffer w/o a scan
					break;
				}
			}
			if (found) {
				getKinect()->sendKinectData((const char*)buffer, bufferSize, TCPKinectBodyIndex);
			}
		}
		SafeRelease(bodyindex);
	}
	void KinectBody::updateImageIR(IMultiSourceFrame* frame) {
		IInfraredFrame * ir = getInfrared(frame);
		if (!ir) {
			return;
		}
		UINT bufferSize = 0;
		UINT16 * buffer = nullptr;
		HRESULT hResult = ir->AccessUnderlyingBuffer(&bufferSize, &buffer);
		if (SUCCEEDED(hResult)) {
			getKinect()->sendKinectData((const char*)buffer, bufferSize, TCPKinectIR);
		}

		SafeRelease(ir);
			
	}
	bool KinectBody::getPoint(CameraSpacePoint& position, DepthSpacePoint& depthSpacePoint) {
		HRESULT hResult = getKinect()->depth(1, &position, 1, &depthSpacePoint);
		return !hresultFails(hResult, "depth");
	}
	bool KinectBody::getPoint(CameraSpacePoint& position, ColorSpacePoint& colorSpacePoint) {
		HRESULT hResult = getKinect()->color(1, &position, 1, &colorSpacePoint);
		return !hresultFails(hResult, "color");
	}
	void KinectBody::setHand(Json::Value &data, const TrackingConfidence& confidence, const HandState& state) {

		data["highConfidence"] = confidence == TrackingConfidence::TrackingConfidence_High;

		switch (state) {
		case HandState_Open:
			data["state"] = "open";
			break;
		case HandState_Closed:
			data["state"] = "closed";
			break;
		case HandState_Lasso:
			data["state"] = "lasso";
			break;
		case HandState_Unknown:
			data["state"] = "unknown";
			break;
		}

	}
	void KinectBody::update() {

		if (!getKinect()) {
			return;
		}
		IMultiSourceFrame* frame = NULL;
		HRESULT hResult;
		uint64_t count = ofGetFrameNum();
		if ((count % getKinect()->signonFrequency) == 0) {
			getKinect()->getSender()->sendOsc(getKinect()->getId(), SignOnKinectServerOscAddress);
		}

		hResult = getKinect()->reader->AcquireLatestFrame(&frame); // gets all data in one shot
		if (hresultFails(hResult, "AcquireLatestFrame")) {
			return;
		}
		// IR optional, only sent if client asks for it, not shown locally
		if (getKinect()->getIR() && (count % getKinect()->irThrottle)) {
			updateImageIR(frame);
		}
		if (getKinect()->getBodyIndex() && (count % getKinect()->biThrottle)) {
			updateImageBodyIndex(frame);
		}
		if (!getKinect()->getBody() && (count % getKinect()->bodyThrottle)) {
			SafeRelease(frame);
			return;
		}

		IBodyFrame* bodyframe = getBody(frame);
		if (!bodyframe) {
			SafeRelease(frame);
			return;
		}
		IBody* pBody[BODY_COUNT] = { 0 };

		hResult = bodyframe->GetAndRefreshBodyData(BODY_COUNT, pBody);
		if (!hresultFails(hResult, "GetAndRefreshBodyData")) {
			for (int bodyIndex = 0; bodyIndex < BODY_COUNT; bodyIndex++) {
				// breaks here
				BOOLEAN bTracked = false;

				hResult = pBody[bodyIndex]->get_IsTracked(&bTracked);
				if (SUCCEEDED(hResult) && bTracked) {
					// Set TrackingID to Detect Face
					UINT64 trackingId = _UI64_MAX;
					hResult = pBody[bodyIndex]->get_TrackingId(&trackingId);
					if (hresultFails(hResult, "get_TrackingId")) {
						continue;
					}
					ofxJSONElement data;
					Joint joints[JointType::JointType_Count];
					PointF leanAmount;

					data["body"][bodyIndex]["trackingId"] = trackingId;
					data["body"][bodyIndex]["kinectID"] = getKinect()->getId();
					if (audio) {
						// see if any audio there
						audio->getAudioCorrelation();
						//bugbug can we use the tracking id, and is valid id, here vs creating our own?
						if (audio->getTrackingID() == trackingId) {
							audio->update(data["body"][bodyIndex], trackingId);
						}
					}
					if (faces) {
						// will fire off face data before body data
						setTrackingID(bodyIndex, trackingId);// keep face on track with body
						faces->update(data["body"][bodyIndex], trackingId);//bugbug need to simplfy this but see what happens for now
					}

					// get joints
					hResult = pBody[bodyIndex]->GetJoints(JointType::JointType_Count, joints);
					if (SUCCEEDED(hResult)) {
						pBody[bodyIndex]->get_Lean(&leanAmount);
						data["body"][bodyIndex]["lean"]["x"] = leanAmount.X;//bugbug account for this in drawing code
						data["body"][bodyIndex]["lean"]["y"] = leanAmount.Y;

						for (int i = 0; i < JointType::JointType_Count; ++i) {
							data["body"][bodyIndex]["joint"][i]["trackingState"] = joints[i].TrackingState;
							if (joints[i].TrackingState != TrackingState::TrackingState_NotTracked) {
								ColorSpacePoint colorSpacePoint = { 0 };
								DepthSpacePoint depthSpacePoint = { 0 };
								getPoint(joints[i].Position, colorSpacePoint);
								getPoint(joints[i].Position, depthSpacePoint);
								
								if ((colorSpacePoint.X >= 0) && (colorSpacePoint.X < getColorFrameWidth())
									&& (depthSpacePoint.Y >= 0) && (depthSpacePoint.Y < getDepthFrameHeight())) {
									TrackingConfidence confidence;
									HandState state;
									data["body"][bodyIndex]["joint"][i]["jointType"] = joints[i].JointType;
									float t = round(depthSpacePoint.X);
									data["body"][bodyIndex]["joint"][i]["depth"]["x"] = round(depthSpacePoint.X);  // in x,y per kinect device size
									data["body"][bodyIndex]["joint"][i]["depth"]["y"] = round(depthSpacePoint.Y);
									data["body"][bodyIndex]["joint"][i]["color"]["x"] = round(colorSpacePoint.X); // in x,y per kinect device size
									data["body"][bodyIndex]["joint"][i]["color"]["y"] = round(colorSpacePoint.Y);
									data["body"][bodyIndex]["joint"][i]["cam"]["x"] = round(joints[i].Position.X); // in meters, things like getting closer
									data["body"][bodyIndex]["joint"][i]["cam"]["y"] = round(joints[i].Position.Y);
									data["body"][bodyIndex]["joint"][i]["cam"]["z"] = round(joints[i].Position.Z);

									if (joints[i].JointType == JointType::JointType_HandRight) {
										pBody[bodyIndex]->get_HandRightConfidence(&confidence); // send this so we do not have to decide
										pBody[bodyIndex]->get_HandRightState(&state);
										setHand(data["body"][bodyIndex]["joint"][i]["right"], confidence, state);
									}
									else if (joints[i].JointType == JointType::JointType_HandLeft) {
										pBody[bodyIndex]->get_HandLeftConfidence(&confidence); // send this so we do not have to decide
										pBody[bodyIndex]->get_HandLeftState(&state);
										string s = data.getRawString(false);
										setHand(data["body"][bodyIndex]["joint"][i]["left"], confidence, state);
									}
									else {
										// just the joint gets drawn, its name other than JointType_Head (hand above head)
										// is not super key as we track face/hands separatly 
									}
								}
							}
						}
						string s = data.getRawString(false); // too large for UDP
						getKinect()->sendKinectData((const char*)s.c_str(), s.size(), TCPKinectBody);
					}
				}
			}
		}
		for (int bodyIndex = 0; bodyIndex < BODY_COUNT; bodyIndex++) {
			SafeRelease(pBody[bodyIndex]);
		}
		SafeRelease(bodyframe);
		SafeRelease(frame);
	}

	bool KinectDevice::setup(shared_ptr<Sender>p, shared_ptr<Stage> backStagePassIn, int retries) {
		backStagePass = backStagePassIn;
		sender = p;

		HRESULT hResult = GetDefaultKinectSensor(&pSensor);
		if (hresultFails(hResult, "GetDefaultKinectSensor")) {
			return false;
		}

		hResult = pSensor->Open();
		if (hresultFails(hResult, "IKinectSensor::Open")) {
			return false;
		}
		// we just get body since we are sending data off the box
		// keep color separate https://social.msdn.microsoft.com/Forums/en-US/4672ca22-4ff2-445b-8574-3011ef16a44c/long-exposure-infrared-vs-infrared?forum=kinectv2sdk
		//http://blog.csdn.net/guoming0000/article/details/46392909
		hResult = pSensor->OpenMultiSourceFrameReader(
			 FrameSourceTypes::FrameSourceTypes_Body|	 FrameSourceTypes_BodyIndex | FrameSourceTypes_Infrared,	&reader);
		if (hresultFails(hResult, "OpenMultiSourceFrameReader")) {
			return false;
		}

		hResult = pSensor->get_CoordinateMapper(&pCoordinateMapper);
		if (hresultFails(hResult, "get_CoordinateMapper")) {
			return false;
		}

		// wait for sensor, will wait for ever
		BOOLEAN avail = false;
		do {
			pSensor->get_IsAvailable(&avail);
			if (avail) {
				break;
			}
			else {
				ofLogNotice("KinectDevice::setup") << "waiting for kinect, retries count " << retries;
				ofSleepMillis(1000);
			}
		} while (retries < 0 || --retries > 0); // tries -1 from the start is infinite loop, else count down

		if (!avail) {
			ofLogNotice("Kinect") << "Kinect not found, but life is still good";
			kinectID.clear();// reset
			return false;
		}

		WCHAR id[256] = { 0 };
		hResult = pSensor->get_UniqueKinectId(256, id);
		if (hresultFails(hResult, "get_UniqueKinectId")) {
			return false;
		}
		for (size_t i = 0; i < id[0] != 0; ++i) {
			kinectID += id[i];
		}

		ofLogNotice("Kinect") << "Kinect signed on, life is good";

		return true;
	}
	// send fast as Kinect is waiting
	void  KinectDevice::sendKinectData(const char * bytes, const int numBytes, OurPorts port, int clientID) {
		// new folks need to know about us
		if (getSender() && numBytes > 0) {
			if (numBytes < 1000) {
				ofLogError() << "data size kindof small, maybe use udp " << " " << ofToString(numBytes); // just a hint
			}
			getSender()->sendTCP(bytes, numBytes, port, clientID);
		}

		// show local too if requested

		shared_ptr<ReadTCPPacket> packet;

		switch (port) {
		case TCPKinectIR:
			if (ir && backStagePass) { // ir wanted and there is a stage to send it to 
				shared_ptr<IRImage>p = std::make_shared<IRImage>();
				if (p) {
					p->IRFromTCP((const UINT16 *)bytes, numBytes);
					p->setup();
					backStagePass->addToAnimatable(p);
				}
			}
			break;
		case TCPKinectBody:
			if (body && backStagePass) {
				shared_ptr<Kinect>p = std::make_shared<Kinect>();
				if (p) {
					p->bodyFromTCP(bytes, numBytes);
					p->setup();
					backStagePass->addToAnimatable(p);
				}
			}
			break;
		case TCPKinectBodyIndex:
			if (bi && backStagePass) {
				shared_ptr<BodyIndexImage>p = std::make_shared<BodyIndexImage>();
				if (p) {
					p->bodyIndexFromTCP(bytes, numBytes);
					p->setup();
					backStagePass->addToAnimatable(p);
				}
			}
			break;
		default:
			break;
		}
	}

void KinectFace::cleanup()
{
	// do not call in destructor as pointers are used, call when needed
	SafeRelease(pFaceReader);
	SafeRelease(pFaceSource);
}
KinectFaces::~KinectFaces() {

	for (int i = 0; i < faces.size(); ++i) {
		faces[i]->cleanup();
	}

}
void KinectBody::setTrackingID(int index, UINT64 trackingId) {
	if (faces) {
		faces->setTrackingID(index, trackingId);
	}
}
// get the face readers
void KinectFaces::setup() {
	buildFaces();
}

void KinectFaces::setTrackingID(int index, UINT64 trackingId) { 
	if (faces.size() < index) {
		ofLogError("KinectFaces::setTrackingID") << "not enough faces";
		return;
	}
	faces[index]->getFaceSource()->put_TrackingId(trackingId);
}

// 
void KinectFaces::update(Json::Value &data, UINT64 trackingId)
{
	IFaceFrame* pFaceFrame = nullptr;
	IFaceFrameResult* pFaceResult = nullptr;
	data.clear();
	for (int count = 0; count < BODY_COUNT; count++) {

		HRESULT hResult = faces[count]->getFaceReader()->AcquireLatestFrame(&pFaceFrame); // faces[count].getFaceReader() was pFaceReader[count]
		if (SUCCEEDED(hResult) && pFaceFrame != nullptr) {
			BOOLEAN bFaceTracked = false;
			hResult = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
			if (SUCCEEDED(hResult) && bFaceTracked) {
				hResult = pFaceFrame->get_FaceFrameResult(&pFaceResult);
				if (SUCCEEDED(hResult) && pFaceResult != nullptr) {
					UINT64 id;
					pFaceFrame->get_TrackingId(&id);
					if (id != trackingId) {
						break; // not sure abou this yet
					}
					// check for real data first
					Vector4 faceRotation;
					pFaceResult->get_FaceRotationQuaternion(&faceRotation);
					if (!faceRotation.x && !faceRotation.y && !faceRotation.w && !faceRotation.z) {
						break;// noise
					}
					int pitch=0, yaw = 0, roll = 0;
					ExtractFaceRotationInDegrees(faceRotation, pitch, yaw, roll);
					data["face"]["rotation"]["pitch"] = pitch;
					data["face"]["rotation"]["yaw"] = yaw;
					data["face"]["rotation"]["roll"] = roll;

					data["face"]["trackingId"] = trackingId;
					data["face"]["kinectID"] = getKinect()->getId();


					//bugbug not show how this loops works with the face loop for > 1 person

					PointF facePoint[FacePointType::FacePointType_Count];
					DetectionResult faceProperty[FaceProperty::FaceProperty_Count];
					RectI boundingBox;
					hResult = pFaceResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, facePoint);
					if (hresultFails(hResult, "GetFacePointsInColorSpace")) {
						break;
					}
					pFaceResult->get_FaceBoundingBoxInColorSpace(&boundingBox);
					data["face"]["boundingBox"]["top"] = boundingBox.Top;
					data["face"]["boundingBox"]["left"] = boundingBox.Left;
					data["face"]["boundingBox"]["right"] = boundingBox.Right;
					data["face"]["boundingBox"]["bottom"] = boundingBox.Bottom;

					hResult = pFaceResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faceProperty);
					if (hresultFails(hResult, "GetFaceProperties")) {
						break;
					}
					if (!facePoint[FacePointType_EyeLeft].X && !facePoint[FacePointType_EyeRight].X && !facePoint[FacePointType_Nose].X && !facePoint[FacePointType_MouthCornerLeft].X) {
						// spurious
						break;
					}
					float f;
					data["face"]["eye"]["left"]["x"] = f = round(facePoint[FacePointType_EyeLeft].X);
					data["face"]["eye"]["left"]["y"] = round(facePoint[FacePointType_EyeLeft].Y);

					data["face"]["eye"]["right"]["x"] = round(facePoint[FacePointType_EyeRight].X);
					data["face"]["eye"]["right"]["y"] = round(facePoint[FacePointType_EyeRight].Y);

					data["face"]["nose"]["x"] = round(facePoint[FacePointType_Nose].X);
					data["face"]["nose"]["y"] = round(facePoint[FacePointType_Nose].Y);

					data["face"]["mouth"]["left"]["x"] = round(facePoint[FacePointType_MouthCornerLeft].X);
					data["face"]["mouth"]["left"]["y"] = round(facePoint[FacePointType_MouthCornerLeft].Y);

					data["face"]["mouth"]["right"]["x"] = round(facePoint[FacePointType_MouthCornerRight].X);
					data["face"]["mouth"]["right"]["y"] = round(facePoint[FacePointType_MouthCornerRight].Y);

#define YES_OR_MAYBE(a)(faceProperty[a] == DetectionResult_Yes || faceProperty[a] == DetectionResult_Maybe)
#define NOT_YES(a)(faceProperty[FaceProperty_RightEyeClosed] != DetectionResult_Yes)
					data["face"]["eye"]["right"]["closed"] = NOT_YES(FaceProperty_RightEyeClosed);
					data["face"]["eye"]["left"]["closed"] = NOT_YES(FaceProperty_LeftEyeClosed);
					data["face"]["mouth"]["open"] = YES_OR_MAYBE(FaceProperty_MouthOpen);
					data["face"]["mouth"]["moved"] = YES_OR_MAYBE(FaceFrameFeatures_MouthMoved);
					data["face"]["happy"] = YES_OR_MAYBE(FaceProperty_Happy) || faceProperty[FaceProperty_Happy] == DetectionResult_Unknown; // try hard to be happy

					data["face"]["lookingAway"] = YES_OR_MAYBE(FaceFrameFeatures_LookingAway);
					data["face"]["glasses"] = YES_OR_MAYBE(FaceFrameFeatures_Glasses);
					data["face"]["engaged"] = YES_OR_MAYBE(FaceFrameFeatures_FaceEngagement);
					break;

				}
			}
		}
	}
	SafeRelease(pFaceResult);
	SafeRelease(pFaceFrame);
}
void KinectBody::useFaces(shared_ptr<KinectFaces> facesIn) {
	faces = facesIn; 
	if (faces) {
		faces->setup();
	}
}
void KinectBody::useAudio(shared_ptr<KinectAudio> audioIn) {
	audio = audioIn; 
	if (audio) {
		audio->setup();
	}
}

void KinectFaces::buildFaces() {
	for (int i = 0; i < BODY_COUNT; ++i) {
		shared_ptr<KinectFace> p = make_shared<KinectFace>(getKinect());
		if (p) {
			HRESULT hResult;
			hResult = CreateFaceFrameSource(getKinect()->getSensor(), 0, features, &p->pFaceSource);
			if (hresultFails(hResult, "CreateFaceFrameSource")) {
				// 0x83010001 https://social.msdn.microsoft.com/Forums/en-US/b0d4fb49-5608-49d5-974b-f0044ceac5ca/createfaceframesource-always-returning-error?forum=kinectv2sdk
				return;
			}

			hResult = p->pFaceSource->OpenReader(&p->pFaceReader);
			if (hresultFails(hResult, "face.pFaceSource->OpenReader")) {
				return;
			}

			faces.push_back(p);
		}
	}
}
}
#endif // !WIN32
