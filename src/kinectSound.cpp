#ifdef _WIN64
// kinect only works on64 bit windows

#include "ofApp.h"
#include "kinect2552.h"
namespace Software2552 {
	STDMETHODIMP_(ULONG) KinectAudioStream::Release() {
		UINT ref = InterlockedDecrement(&m_cRef);
		if (ref == 0) {
			delete this;
		}
		return ref;
	}
	STDMETHODIMP KinectAudioStream::QueryInterface(REFIID riid, void **ppv) {
		if (riid == IID_IUnknown) {
			AddRef();
			*ppv = (IUnknown*)this;
			return S_OK;
		}
		else if (riid == IID_IStream) {
			AddRef();
			*ppv = (IStream*)this;
			return S_OK;
		}
		else {
			return E_NOINTERFACE;
		}
	}

	/////////////////////////////////////////////
	// IStream methods
	STDMETHODIMP KinectAudioStream::Read(void *pBuffer, ULONG cbBuffer, ULONG *pcbRead) {
		if (pcbRead == NULL || cbBuffer == NULL) {
			return E_INVALIDARG;
		}

		HRESULT hr = S_OK;

		// 32bit -> 16bit conversion support
		INT16* p16Buffer = (INT16*)pBuffer;
		int factor = sizeof(float) / sizeof(INT16);

		// 32 bit read support
		float* p32Buffer = new float[cbBuffer / factor];
		byte* index = (byte*)p32Buffer;
		ULONG bytesRead = 0;
		ULONG bytesRemaining = cbBuffer * factor;

		// Speech reads at high frequency - this slows down the process
		int sleepDuration = 50;

		// Speech Service isn't tolerant of partial reads
		while (bytesRemaining > 0) {
			// Stop returning Audio data if Speech isn't active
			if (!m_SpeechActive) {
				*pcbRead = 0;
				hr = S_FALSE;
				goto exit;
			}

			hr = m_p32BitAudio->Read(index, bytesRemaining, &bytesRead);
			index += bytesRead;
			bytesRemaining -= bytesRead;

			// All Audio buffers drained - wait for buffers to fill
			if (bytesRemaining != 0) {
				Sleep(sleepDuration);
			}
		}

		// Convert float value [-1,1] to int16 [SHRT_MIN, SHRT_MAX] and copy to output butter
		for (UINT i = 0; i < cbBuffer / factor; i++) {
			float sample = p32Buffer[i];

			// Make sure it is in the range [-1, +1]
			if (sample > 1.0f) {
				sample = 1.0f;
			}
			else if (sample < -1.0f) {
				sample = -1.0f;
			}

			// Scale float to the range (SHRT_MIN, SHRT_MAX] and then
			// convert to 16-bit signed with proper rounding
			float sampleScaled = sample * (float)SHRT_MAX;
			p16Buffer[i] = (sampleScaled > 0.f) ? (INT16)(sampleScaled + 0.5f) : (INT16)(sampleScaled - 0.5f);
		}

		*pcbRead = cbBuffer;

	exit:
		delete p32Buffer;
		return hr;
	}
	void KinectAudio::getAudioCommands() {
		unsigned long waitObject = WaitForSingleObject(hSpeechEvent, 0);
		if (waitObject == WAIT_TIMEOUT) {
			ofLogVerbose("getAudioCommands") << "signaled";
		}
		else if (waitObject == WAIT_OBJECT_0) {
			ofLogNotice("getAudioCommands") << "nonsignaled";
			// Retrieved Event
			const float confidenceThreshold = 0.3f;
			SPEVENT eventStatus;
			//eventStatus.eEventId = SPEI_UNDEFINED;
			unsigned long eventFetch = 0;
			pSpeechContext->GetEvents(1, &eventStatus, &eventFetch);
			while (eventFetch > 0) {
				switch (eventStatus.eEventId) {
					// Speech Recognition Events
					//   SPEI_HYPOTHESIS  : Estimate
					//   SPEI_RECOGNITION : Recognition
				case SPEI_HYPOTHESIS:
				case SPEI_RECOGNITION:
					if (eventStatus.elParamType == SPET_LPARAM_IS_OBJECT) {
						// Retrieved Phrase
						ISpRecoResult* pRecoResult = reinterpret_cast<ISpRecoResult*>(eventStatus.lParam);
						SPPHRASE* pPhrase = nullptr;
						HRESULT hResult = pRecoResult->GetPhrase(&pPhrase);
						if (!hresultFails(hResult, "GetPhrase")) {
							if ((pPhrase->pProperties != nullptr) && (pPhrase->pProperties->pFirstChild != nullptr)) {

								// Compared with the Phrase Tag in the grammar file
								const SPPHRASEPROPERTY* pSemantic = pPhrase->pProperties->pFirstChild;
								switch (pSemantic->Confidence) {
								case SP_LOW_CONFIDENCE:
									break;
								case SP_NORMAL_CONFIDENCE:
									break;
								case SP_HIGH_CONFIDENCE:
									break;
								}

								if (pSemantic->SREngineConfidence > confidenceThreshold) {
									ofxJSONElement data;
									data["trackingId"] = audioTrackingId;
									data["confidence"] = pSemantic->Confidence;//if not enough hit turn down the gain bugbug
									data["value"] = pSemantic->pszValue;
									data["kinectID"] = getKinect()->getId();
									getKinect()->sendUDP(data, "kinect/audioCommand");
								}
							}
							CoTaskMemFree(pPhrase);
						}
					}
				}
				pSpeechContext->GetEvents(1, &eventStatus, &eventFetch);
			}
		}

	}
	//http://www.buildinsider.net/small/kinectv2cpp/07

	KinectAudio::~KinectAudio() {
		if (pSpeechRecognizer != nullptr) {
			pSpeechRecognizer->SetRecoState(SPRST_INACTIVE_WITH_PURGE);
		}
		SafeRelease(pAudioSource);
		SafeRelease(pAudioBeamReader);
		SafeRelease(pAudioBeamList);
		SafeRelease(pAudioBeam);
		SafeRelease(pAudioStream);
		SafeRelease(pSpeechStream);
		SafeRelease(pSpeechRecognizer);
		SafeRelease(pSpeechContext);
		SafeRelease(pSpeechGrammar);
		if (hSpeechEvent != INVALID_HANDLE_VALUE) {
			CloseHandle(hSpeechEvent);
		}
		CoUninitialize();
		if (audioStream != nullptr) {
			audioStream->SetSpeechState(false);
			delete audioStream;
			audioStream = nullptr;
		}

	}

	void KinectAudio::setup() {

		HRESULT hResult = getKinect()->getSensor()->get_AudioSource(&pAudioSource);
		if (hresultFails(hResult, "get_AudioSource")) {
			return;
		}

		hResult = pAudioSource->OpenReader(&pAudioBeamReader);
		if (hresultFails(hResult, "IAudioSource::OpenReader")) {
			return;
		}

		hResult = pAudioSource->get_AudioBeams(&pAudioBeamList);
		if (hresultFails(hResult, "IAudioSource::get_AudioBeams")) {
			return;
		}

		hResult = pAudioBeamList->OpenAudioBeam(0, &pAudioBeam);
		if (hresultFails(hResult, "pAudioBeamList->OpenAudioBeam")) {
			return;
		}

		hResult = pAudioBeam->OpenInputStream(&pAudioStream);
		if (hresultFails(hResult, "IAudioSource::OpenInputStream")) {
			return;
		}

		audioStream = new KinectAudioStream(pAudioStream);

		hResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (hresultFails(hResult, "CoInitializeEx")) {
			return;
		}

		if (FAILED(setupSpeachStream())) {
			return;
		}
		if (FAILED(createSpeechRecognizer())) {
			return;
		}
		if (FAILED(findKinect())) {
			return;
		}
		if (FAILED(startSpeechRecognition())) {
			return;
		}

	}

	HRESULT KinectAudio::setupSpeachStream() {

		HRESULT hResult = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&pSpeechStream);
		if (hresultFails(hResult, "CoCreateInstance( CLSID_SpStream )")) {
			return hResult;
		}

		WORD AudioFormat = WAVE_FORMAT_PCM;
		WORD AudioChannels = 1;
		DWORD AudioSamplesPerSecond = 16000;
		DWORD AudioAverageBytesPerSecond = 32000;
		WORD AudioBlockAlign = 2;
		WORD AudioBitsPerSample = 16;

		WAVEFORMATEX waveFormat = { AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0 };

		audioStream->SetSpeechState(true);
		hResult = pSpeechStream->SetBaseStream(audioStream, SPDFID_WaveFormatEx, &waveFormat);
		if (hresultFails(hResult, "ISpStream::SetBaseStream")) {
			return hResult;
		}

		return hResult;

	}
	HRESULT KinectAudio::findKinect() {

		ISpObjectTokenCategory* pTokenCategory = nullptr;
		HRESULT hResult = CoCreateInstance(CLSID_SpObjectTokenCategory, nullptr, CLSCTX_ALL, __uuidof(ISpObjectTokenCategory), reinterpret_cast<void**>(&pTokenCategory));
		if (hresultFails(hResult, "CoCreateInstance")) {
			return hResult;
		}

		hResult = pTokenCategory->SetId(SPCAT_RECOGNIZERS, false);
		if (hresultFails(hResult, "ISpObjectTokenCategory::SetId()")) {
			SafeRelease(pTokenCategory);
			return hResult;
		}

		IEnumSpObjectTokens* pEnumTokens = nullptr;
		hResult = CoCreateInstance(CLSID_SpMMAudioEnum, nullptr, CLSCTX_ALL, __uuidof(IEnumSpObjectTokens), reinterpret_cast<void**>(&pEnumTokens));
		if (hresultFails(hResult, "CoCreateInstance( CLSID_SpMMAudioEnum )")) {
			SafeRelease(pTokenCategory);
			return hResult;
		}

		// Find Best Token
		const wchar_t* pVendorPreferred = L"VendorPreferred";
		const unsigned long lengthVendorPreferred = static_cast<unsigned long>(wcslen(pVendorPreferred));
		unsigned long length;
		ULongAdd(lengthVendorPreferred, 1, &length);
		wchar_t* pAttribsVendorPreferred = new wchar_t[length];
		StringCchCopyW(pAttribsVendorPreferred, length, pVendorPreferred);

		hResult = pTokenCategory->EnumTokens(L"Language=409;Kinect=True", pAttribsVendorPreferred, &pEnumTokens); //  English "Language=409;Kinect=True"
		if (hresultFails(hResult, "pTokenCategory->EnumTokens")) {
			SafeRelease(pTokenCategory);
			return hResult;
		}

		SafeRelease(pTokenCategory);
		delete[] pAttribsVendorPreferred;

		ISpObjectToken* pEngineToken = nullptr;
		hResult = pEnumTokens->Next(1, &pEngineToken, nullptr);
		if (hresultFails(hResult, "ISpObjectToken Next")) {
			SafeRelease(pTokenCategory);
			return hResult;
		}
		if (hResult == S_FALSE) {
			//note this but continus things will still work, not sure it matters with the new sdk
			ofLogError("KinectAudio::findKinect") << "Kinect not found";
		}
		SafeRelease(pEnumTokens);
		SafeRelease(pTokenCategory);

		// Set Speech Recognizer
		hResult = pSpeechRecognizer->SetRecognizer(pEngineToken);
		if (hresultFails(hResult, "SetRecognizer")) {
			return hResult;
		}
		SafeRelease(pEngineToken);

		hResult = pSpeechRecognizer->CreateRecoContext(&pSpeechContext);
		if (hresultFails(hResult, "CreateRecoContext")) {
			return hResult;
		}

		hResult = pSpeechRecognizer->SetPropertyNum(L"AdaptationOn", 0);
		if (hresultFails(hResult, "SetPropertyNum")) {
			return hResult;
		}

		return hResult;

	}
	HRESULT KinectAudio::createSpeechRecognizer()
	{
		// Create Speech Recognizer Instance
		HRESULT hResult = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&pSpeechRecognizer);
		if (hresultFails(hResult, "CLSID_SpInprocRecognizer")) {
			return hResult;
		}

		// Set Input Stream
		hResult = pSpeechRecognizer->SetInput(pSpeechStream, TRUE);
		if (hresultFails(hResult, "pSpeechRecognizer->SetInput")) {
			return hResult;
		}

		return hResult;
	}
	HRESULT KinectAudio::startSpeechRecognition()
	{
		HRESULT hResult = pSpeechContext->CreateGrammar(1, &pSpeechGrammar);
		if (hresultFails(hResult, "CreateGrammar")) {
			return hResult;
		}

		hResult = pSpeechGrammar->LoadCmdFromFile(L"data\\grammar.grxml", SPLO_STATIC); // http://www.w3.org/TR/speech-grammar/ (UTF-8/CRLF)
		if (hresultFails(hResult, "LoadCmdFromFile")) {
			return hResult;
		}

		// Specify that all top level rules in grammar are now active
		pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

		// Specify that engine should always be reading audio
		pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

		// Specify that we're only interested in receiving recognition events
		pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

		// Ensure that engine is recognizing speech and not in paused state
		hResult = pSpeechContext->Resume(0);
		if (SUCCEEDED(hResult)) {
			hSpeechEvent = pSpeechContext->GetNotifyEventHandle();
		}
		hSpeechEvent = pSpeechContext->GetNotifyEventHandle();

		return hResult;
	}
	void  KinectAudio::setTrackingID(int index, UINT64 trackingId) {

		if (trackingId == audioTrackingId) {
			trackingIndex = index;
		}
	}
	// only call if audio matches a body via tracking Id
	void KinectAudio::update(Json::Value &data, UINT64 trackingId) {
		getAudioBeam(data, trackingId);
		getAudioCommands();
	}

	void KinectAudio::getAudioCorrelation() {
		correlationCount = 0;
		trackingIndex = NoTrackingIndex;
		audioTrackingId = NoTrackingID;

		IAudioBeamFrameList* pAudioBeamList = nullptr;
		HRESULT hResult = getAudioBeamReader()->AcquireLatestBeamFrames(&pAudioBeamList);
		if (!hresultFails(hResult, "getAudioCorrelation AcquireLatestBeamFrames")) {

			IAudioBeamFrame* pAudioBeamFrame = nullptr;
			hResult = pAudioBeamList->OpenAudioBeamFrame(0, &pAudioBeamFrame);
			if (!hresultFails(hResult, "OpenAudioBeamFrame")) {
				IAudioBeamSubFrame* pAudioBeamSubFrame = nullptr;
				hResult = pAudioBeamFrame->GetSubFrame(0, &pAudioBeamSubFrame);
				if (SUCCEEDED(hResult)) {
					hResult = pAudioBeamSubFrame->get_AudioBodyCorrelationCount(&correlationCount);
					if (SUCCEEDED(hResult) && (correlationCount != 0)) {
						IAudioBodyCorrelation* pAudioBodyCorrelation = nullptr;
						hResult = pAudioBeamSubFrame->GetAudioBodyCorrelation(0, &pAudioBodyCorrelation);

						if (SUCCEEDED(hResult)) {
							pAudioBodyCorrelation->get_BodyTrackingId(&audioTrackingId);
							SafeRelease(pAudioBodyCorrelation);
						}
					}
					SafeRelease(pAudioBeamSubFrame);
				}
				SafeRelease(pAudioBeamFrame);
			}
			SafeRelease(pAudioBeamList);
		}

	}

	// AudioBeam Frame https://masteringof.wordpress.com/examples/sounds/ https://masteringof.wordpress.com/projects-based-on-book/
	void KinectAudio::getAudioBeam(Json::Value &data, UINT64 trackingId) {

		IAudioBeamFrameList* pAudioBeamList = nullptr;
		HRESULT hResult = getAudioBeamReader()->AcquireLatestBeamFrames(&pAudioBeamList);
		if (!hresultFails(hResult, "getAudioBeam AcquireLatestBeamFrames")) {
			//bugbug add error handling maybe other clean up
			UINT beamCount = 0;
			hResult = pAudioBeamList->get_BeamCount(&beamCount);
			// Only one audio beam is currently supported, but write the code in case this changes
			for (int beam = 0; beam < beamCount; ++beam) {
				angle = 0.0f;
				confidence = 0.0f;
				IAudioBeamFrame* pAudioBeamFrame = nullptr;
				hResult = pAudioBeamList->OpenAudioBeamFrame(beam, &pAudioBeamFrame);

				if (!hresultFails(hResult, "OpenAudioBeamFrame")) {
					// Get Beam Angle and Confidence
					IAudioBeam* pAudioBeam = nullptr;
					hResult = pAudioBeamFrame->get_AudioBeam(&pAudioBeam);
					if (!hresultFails(hResult, "get_AudioBeam")) {
						pAudioBeam->get_BeamAngle(&angle); // radian [-0.872665f, 0.872665f]
						pAudioBeam->get_BeamAngleConfidence(&confidence); // confidence [0.0f, 1.0f]
						ofxJSONElement data;
						data["beam"][beam]["angle"] = angle;
						data["beam"][beam]["confidence"] = confidence;
						data["beam"][beam]["kinectID"] = getKinect()->getId();
						data["beam"][beam]["trackingId"] = audioTrackingId; // matches a body
						SafeRelease(pAudioBeam);
					}
					SafeRelease(pAudioBeamFrame);
				}
			}
			SafeRelease(pAudioBeamList);
		}

	}
}
#endif