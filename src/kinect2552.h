
#include <inc\Kinect.Face.h>

//https://github.com/Vangos, http://www.cs.princeton.edu/~edwardz/tutorials/kinect2/kinect0_sdl.html, https://github.com/UnaNancyOwen
//http://www.pointclouds.org/
//https://books.google.com/books?id=CVUpCgAAQBAJ&pg=PA167&lpg=PA167&dq=GetAudioBodyCorrelation&source=bl&ots=UENPsaMG_J&sig=5RHwdiXn4T7gst6lVt9SFvp2ahw&hl=en&sa=X&ved=0ahUKEwjK9umrqL7KAhUIVz4KHVL-BEYQ6AEIPTAG#v=onepage&q=GetAudioBodyCorrelation&f=false
// https://github.com/Microsoft/DirectX-Graphics-Samples

namespace Software2552 {

	bool checkPointer(IUnknown *p, const string&  message, char*file = __FILE__, int line = __LINE__);
	bool CheckHresult(HRESULT hResult, const string& message, char*file, int line);
#define hresultFails(hResult, m) CheckHresult(hResult, m, __FILE__,__LINE__)

	template<class Interface> void SafeRelease(Interface *& pInterfaceToRelease)
	{
		if (pInterfaceToRelease != NULL) {
			pInterfaceToRelease->Release();
			pInterfaceToRelease = NULL;
		}
	}

	class KinectAudioStream : public IStream	{
	public:
		KinectAudioStream(IStream *p32BitAudio) : m_cRef(1), m_p32BitAudio(p32BitAudio), m_SpeechActive(false) {}

		void SetSpeechState(bool state) { m_SpeechActive = state; }

		STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }
		STDMETHODIMP_(ULONG) Release();
		STDMETHODIMP QueryInterface(REFIID riid, void **ppv);

	private:
		STDMETHODIMP Read(void *, ULONG, ULONG *);
		STDMETHODIMP Write(const void *, ULONG, ULONG *) { return E_NOTIMPL; }
		STDMETHODIMP Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER *) { return S_OK; }
		STDMETHODIMP SetSize(ULARGE_INTEGER) { return E_NOTIMPL; }
		STDMETHODIMP CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *) { return E_NOTIMPL; }
		STDMETHODIMP Commit(DWORD) { return E_NOTIMPL; }
		STDMETHODIMP Revert() { return E_NOTIMPL; }
		STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { return E_NOTIMPL; }
		STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { return E_NOTIMPL; }
		STDMETHODIMP Stat(STATSTG *, DWORD) { return E_NOTIMPL; }
		STDMETHODIMP Clone(IStream **) { return E_NOTIMPL; }
		UINT                    m_cRef;
		IStream*                m_p32BitAudio;
		bool                    m_SpeechActive;
	};

	class Kinect2552  {
	public:
		~Kinect2552();

		bool setup(shared_ptr<Sender>p=nullptr);

		IMultiSourceFrame* frame = nullptr;
		IMultiSourceFrameReader* reader = nullptr;   // Kinect data source
		IBodyFrameReference *bodyFrameReference = nullptr;

		IKinectSensor* getSensor() {return pSensor;	}
		ICoordinateMapper* getMapper() {	return pCoordinateMapper;	}

		int getDepthFrameWidth() { return 512; }
		int getDepthFrameHeight() { return 424; }
		int getColorFrameWidth() { return 1920; }
		int getColorFrameHeight() { return 1080; }

		HRESULT depth(UINT cameraPointCount, CameraSpacePoint*csp, UINT depthPointCount, DepthSpacePoint *dsp) { return pCoordinateMapper->MapCameraPointsToDepthSpace(1, csp, 1, dsp); }
		HRESULT color(UINT cameraPointCount, const CameraSpacePoint*csp, UINT depthPointCount, ColorSpacePoint *color) { return pCoordinateMapper->MapCameraPointsToColorSpace(1, csp, 1, color); }

		const string &getId() { return kinectID; }
		// send large binary data over TCP (over 1000 byte)
		void sendKinectData(const char * bytes, const int numBytes, OurPorts port, int clientID = -1);
		// send Json over UDP, fast, small
		void sendUDP(ofxJSON &data, const string& address);
		shared_ptr<Sender> sender() { return router; }
		bool getIR();
		void setIR(bool b) { ir = b; }
		bool getBodyIndex();
		void setBodyIndex(bool b) { bi = b; }
		bool getBody();
		void setBody(bool b) { body = b; }
		int irThrottle = 20; // send every 20th after the first
		int biThrottle = 2;
		int bodyThrottle = 1; 
	private:
		bool ir = false; // get ir
		bool bi = false; // get body index
		bool body = true; // get body
		shared_ptr<Sender> router = nullptr; // optional
		string kinectID;
		IKinectSensor*     pSensor = nullptr;
		ICoordinateMapper* pCoordinateMapper = nullptr;
	};

	class KinectBaseClass {
	public:
		KinectBaseClass(Kinect2552 *pKinectIn) { pKinect = pKinectIn; }
		Kinect2552 *getKinect() { return pKinect; }
	private:
		Kinect2552 *pKinect;
	};


	class KinectFace : public KinectBaseClass {
	public:
		KinectFace(Kinect2552 *pKinect) : KinectBaseClass(pKinect) {}
		void cleanup();

		IFaceFrameReader* getFaceReader() {	return pFaceReader;	}
		IFaceFrameSource* getFaceSource() {	return pFaceSource;	}

		friend class KinectFaces;
		
	protected:
		
		IFaceFrameReader* pFaceReader;
		IFaceFrameSource* pFaceSource;
	};
	// one optional face for every kinect person
	class KinectFaces : public KinectBaseClass {
	public:
		KinectFaces(Kinect2552 *pKinect) : KinectBaseClass(pKinect) {}

		~KinectFaces();

		void setup();
		void update(Json::Value &data, UINT64 trackingId);
		void setTrackingID(int index, UINT64 trackingId); // map to body
		vector<shared_ptr<KinectFace>> faces;
	protected:

		// features are the same for all faces
		DWORD features = FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
			| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
			| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
			| FaceFrameFeatures::FaceFrameFeatures_Happy
			| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
			| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
			| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
			| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
			| FaceFrameFeatures::FaceFrameFeatures_LookingAway
			| FaceFrameFeatures::FaceFrameFeatures_Glasses
			| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;
	private:
		void buildFaces();

	};

	class KinectAudio : public KinectBaseClass {
	public:
		friend class KinectBody;

		KinectAudio(Kinect2552 *pKinect) :KinectBaseClass(pKinect) {};

		~KinectAudio();

		void setup();

		void getAudioCorrelation();
		UINT64 getTrackingID() { return audioTrackingId; }
		IAudioBeamFrameReader* getAudioBeamReader() { return pAudioBeamReader; }
		IAudioSource* getAudioSource() { return pAudioSource; }

	protected:
		bool confident() { return  getConfidence() > 0.5f; }
		float getAngle() { return angle; }
		float getConfidence() { return confidence; }
		void getAudioBeam(Json::Value &data, UINT64 trackingId);
		void getAudioCommands();
		int  getTrackingBodyIndex() { return trackingIndex; }
		virtual void setTrackingID(int index, UINT64 trackingId);
		HRESULT createSpeechRecognizer();
		HRESULT startSpeechRecognition();

	private:
		void update(Json::Value &data, UINT64 trackingId);
		const UINT64 NoTrackingID = _UI64_MAX - 1;
		const UINT64 NoTrackingIndex = -1;

		HRESULT findKinect();
		HRESULT setupSpeachStream();

		IAudioSource* pAudioSource = nullptr;
		IAudioBeamFrameReader* pAudioBeamReader = nullptr;
		IAudioBeamList* pAudioBeamList = nullptr;
		IAudioBeam* pAudioBeam = nullptr;
		IStream* pAudioStream = nullptr;
		ISpStream* pSpeechStream = nullptr;
		ISpRecognizer* pSpeechRecognizer = nullptr;
		ISpRecoContext* pSpeechContext = nullptr;
		ISpRecoGrammar* pSpeechGrammar = nullptr;
		HANDLE hSpeechEvent = INVALID_HANDLE_VALUE;
		KinectAudioStream* audioStream = nullptr;
		UINT64 audioTrackingId= NoTrackingID;
		int trackingIndex= NoTrackingID;
		float angle = 0.0f;
		float confidence = 0.0f;
		UINT32 correlationCount = 0;
	};

	class KinectBody : public KinectBaseClass {
	public:
		KinectBody(Kinect2552 *pKinect) : KinectBaseClass(pKinect) {  }

		void update();
		void useFaces(shared_ptr<KinectFaces> facesIn)  { faces = facesIn; }
		void useAudio(shared_ptr<KinectAudio> audioIn) { audio = audioIn; }

	private:
		int c = 0; // times called count
		void updateImageBodyIndex(IMultiSourceFrame* frame);
		void updateImageIR(IMultiSourceFrame* frame);
		bool getPoint(CameraSpacePoint& position, DepthSpacePoint& depthSpacePoint);
		bool getPoint(CameraSpacePoint& position, ColorSpacePoint& colorSpacePoint);
		void setHand(Json::Value &data, const TrackingConfidence& confidence, const HandState& state);
		// audio id tracks to sound bugbug how does faces do it?
		void setTrackingID(int index, UINT64 trackingId);
		shared_ptr<KinectAudio> audio = nullptr;
		shared_ptr<KinectFaces> faces = nullptr;
	};
}
