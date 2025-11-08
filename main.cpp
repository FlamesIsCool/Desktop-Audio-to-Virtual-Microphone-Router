#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "winmm.lib")

#define REFTIMES_PER_SEC 10000000 // 100ns units for reference time

void CheckHR(HRESULT hr, const std::string& msg) {
    if (FAILED(hr)) {
        std::cerr << "Error: " << msg << " (0x" << std::hex << hr << ")" << std::endl;
        exit(1);
    }
}

std::wstring GetDeviceName(IMMDevice* pDevice) {
    LPWSTR wstrID = nullptr;
    HRESULT hr = pDevice->GetId(&wstrID);
    if (FAILED(hr)) {
        return L"Unknown";
    }

    IPropertyStore* pProps = nullptr;
    hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    if (FAILED(hr)) {
        CoTaskMemFree(wstrID);
        return L"Unknown";
    }

    PROPVARIANT varName;
    PropVariantInit(&varName);
    hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
    pProps->Release();
    CoTaskMemFree(wstrID);

    std::wstring name = (varName.vt == VT_LPWSTR) ? varName.pwszVal : L"Unknown";
    PropVariantClear(&varName);
    return name;
}

IMMDevice* FindVirtualCable(IMMDeviceEnumerator* pEnumerator, EDataFlow dataFlow) {
    IMMDeviceCollection* pCollection = nullptr;
    HRESULT hr = pEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &pCollection);
    if (FAILED(hr)) {
        std::cerr << "Failed to enumerate audio endpoints (0x" << std::hex << hr << ")" << std::endl;
        return nullptr;
    }

    UINT count;
    pCollection->GetCount(&count);
    IMMDevice* pDevice = nullptr;

    for (UINT i = 0; i < count; i++) {
        IMMDevice* pTempDevice = nullptr;
        pCollection->Item(i, &pTempDevice);
        if (pTempDevice) {
            std::wstring name = GetDeviceName(pTempDevice);
            std::wcout << L"Found device: " << name << std::endl;
            if (name.find(L"VB-Audio") != std::wstring::npos || name.find(L"CABLE") != std::wstring::npos) {
                pDevice = pTempDevice;
                break;
            }
            pTempDevice->Release();
        }
    }
    pCollection->Release();
    return pDevice;
}

int main() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    CheckHR(hr, "Failed to initialize COM");

    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    CheckHR(hr, "Failed to create device enumerator");

    IMMDevice* pRenderDevice = nullptr;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pRenderDevice);
    CheckHR(hr, "Failed to get default render device");
    std::wcout << L"Using render device: " << GetDeviceName(pRenderDevice) << std::endl;

    IMMDevice* pVirtualCableDevice = FindVirtualCable(pEnumerator, eRender);
    if (!pVirtualCableDevice) {
        std::cerr << "Error: VB-Audio Virtual Cable (render) not found!" << std::endl;
        pEnumerator->Release();
        CoUninitialize();
        return 1;
    }
    std::wcout << L"Using virtual cable device: " << GetDeviceName(pVirtualCableDevice) << std::endl;

    IAudioClient* pAudioClient = nullptr;
    hr = pRenderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pAudioClient);
    CheckHR(hr, "Failed to activate render audio client");

    WAVEFORMATEX* pwfx = nullptr;
    hr = pAudioClient->GetMixFormat(&pwfx);
    CheckHR(hr, "Failed to get mix format");
    std::cout << "Mix format: " << pwfx->nSamplesPerSec << " Hz, " << pwfx->nChannels << " channels, "
        << pwfx->wBitsPerSample << " bits" << std::endl;

    IAudioClient* pVirtualCableAudioClient = nullptr;
    hr = pVirtualCableDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pVirtualCableAudioClient);
    CheckHR(hr, "Failed to activate virtual cable audio client");

    AUDCLNT_SHAREMODE shareMode = AUDCLNT_SHAREMODE_SHARED;
    hr = pVirtualCableAudioClient->IsFormatSupported(shareMode, pwfx, nullptr);
    if (hr == AUDCLNT_E_UNSUPPORTED_FORMAT) {
        std::cerr << "Warning: Virtual cable does not support render format. Trying 44100 Hz, 16-bit, stereo..." << std::endl;
        CoTaskMemFree(pwfx);
        pwfx = nullptr;

        pwfx = (WAVEFORMATEX*)CoTaskMemAlloc(sizeof(WAVEFORMATEX));
        if (!pwfx) {
            std::cerr << "Failed to allocate memory for fallback format" << std::endl;
            pVirtualCableAudioClient->Release();
            pAudioClient->Release();
            pRenderDevice->Release();
            pVirtualCableDevice->Release();
            pEnumerator->Release();
            CoUninitialize();
            return 1;
        }
        ZeroMemory(pwfx, sizeof(WAVEFORMATEX));
        pwfx->wFormatTag = WAVE_FORMAT_PCM;
        pwfx->nChannels = 2;
        pwfx->nSamplesPerSec = 44100;
        pwfx->wBitsPerSample = 16;
        pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
        pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

        hr = pVirtualCableAudioClient->IsFormatSupported(shareMode, pwfx, nullptr);
        CheckHR(hr, "Virtual cable does not support fallback format");
    }

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,
        REFTIMES_PER_SEC, 0, pwfx, nullptr);
    CheckHR(hr, "Failed to initialize render audio client");

    hr = pVirtualCableAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, REFTIMES_PER_SEC, 0, pwfx, nullptr);
    CheckHR(hr, "Failed to initialize virtual cable audio client");

    IAudioCaptureClient* pCaptureClient = nullptr;
    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    CheckHR(hr, "Failed to get capture client");

    IAudioRenderClient* pRenderClient = nullptr;
    hr = pVirtualCableAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&pRenderClient);
    CheckHR(hr, "Failed to get render client");

    hr = pAudioClient->Start();
    CheckHR(hr, "Failed to start render audio client");
    hr = pVirtualCableAudioClient->Start();
    CheckHR(hr, "Failed to start virtual cable audio client");

    std::cout << "Capturing and routing audio to virtual cable. Press Ctrl+C to stop..." << std::endl;

    while (true) {
        UINT32 packetLength = 0;
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            std::cerr << "Failed to get packet size (0x" << std::hex << hr << ")" << std::endl;
            break;
        }

        while (packetLength != 0) {
            BYTE* pData;
            UINT32 numFramesAvailable;
            DWORD flags;
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
            if (FAILED(hr)) {
                std::cerr << "Failed to get capture buffer (0x" << std::hex << hr << ")" << std::endl;
                break;
            }

            BYTE* pRenderData;
            hr = pRenderClient->GetBuffer(numFramesAvailable, &pRenderData);
            if (FAILED(hr)) {
                std::cerr << "Failed to get render buffer (0x" << std::hex << hr << ")" << std::endl;
                pCaptureClient->ReleaseBuffer(numFramesAvailable);
                break;
            }

            memcpy(pRenderData, pData, numFramesAvailable * pwfx->nBlockAlign);

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr)) {
                std::cerr << "Failed to release capture buffer (0x" << std::hex << hr << ")" << std::endl;
                break;
            }
            hr = pRenderClient->ReleaseBuffer(numFramesAvailable, 0);
            if (FAILED(hr)) {
                std::cerr << "Failed to release render buffer (0x" << std::hex << hr << ")" << std::endl;
                break;
            }

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) {
                std::cerr << "Failed to get next packet size (0x" << std::hex << hr << ")" << std::endl;
                break;
            }
        }

        Sleep(10);
    }

    pAudioClient->Stop();
    pVirtualCableAudioClient->Stop();
    pCaptureClient->Release();
    pRenderClient->Release();
    pAudioClient->Release();
    pVirtualCableAudioClient->Release();
    pRenderDevice->Release();
    pVirtualCableDevice->Release();
    pEnumerator->Release();
    if (pwfx) CoTaskMemFree(pwfx);
    CoUninitialize();

    return 0;
}
