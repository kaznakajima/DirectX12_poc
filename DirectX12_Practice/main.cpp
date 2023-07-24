#include <windows.h>
#include <tchar.h>

// DirectX12�ɍŒ���K�v�ȃw�b�_�[�t�@�C��
#include <d3d12.h>
#include <dxgi1_6.h>

// �w�b�_�[�ɍ��킹�����C�u�����������N
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <vector>

// ��{�I�u�W�F�N�g�Q
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

// ID3D12Deveice�I�u�W�F�N�g���쐬����
HRESULT D3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDeveice);

// �t�B�[�`���[���x��(�O���{�ɂ���Ă͑Ή����Ă��Ȃ��\�������邽��)
D3D_FEATURE_LEVEL levels[] = {
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

#ifdef _DEBUG
	#include <iostream>
#endif

#define WINDOW_WIDTH  720
#define WINDOW_HEIGHT 480

using namespace std;

// @brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
// @param format �t�H�[�}�b�g(%d�Ƃ�%f�Ƃ�)
// @param �ϒ��ϐ�
// @remarks ���̊֐��̓f�o�b�O�p�ł��B�f�o�b�O���ɂ������삵�܂���B
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

// �ʓ|�����Ǐ����Ȃ���΂����Ȃ��֐�
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// �E�C���h�E���j�����ꂽ��Ă�
	if ( msg == WM_DESTROY ) {
		PostQuitMessage(0); // OS�ɑ΂��āu�������̃A�v���͏I���v�Ɠ`����B
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

#ifdef _DEBUG
	int main() {
#else
	int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
		//DebugOutputFormatString("Show window test.");
		
		WNDCLASSEX w = {};
		w.cbSize = sizeof(WNDCLASSEX);
		w.lpfnWndProc = (WNDPROC)WindowProcedure; // �R�[���o�b�N�֐��̎w��
		w.lpszClassName = _T("DX12Sample");				// �A�v���P�[�V�����N���X��(�K���ŗǂ�)
		w.hInstance = GetModuleHandle(nullptr);				// �n���h���̎擾

		RegisterClassEx(&w); // �A�v���P�[�V�����N���X(�E�C���h�E�N���X�̎w���OS�ɓ`����)

		RECT wrc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT }; // �E�C���h�E�T�C�Y�����߂�


		D3D_FEATURE_LEVEL featureLevel;

		auto result = CreateDXGIFactory(IID_PPV_ARGS(&_dxgiFactory));

		// �A�_�v�^�[�̗񋓗p
		std::vector<IDXGIAdapter*> adapters;

		// �����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
		IDXGIAdapter* tmpAdapter = nullptr;

		for ( int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i ) {
			adapters.push_back(tmpAdapter);
		}

		for (auto adpt : adapters) {
			DXGI_ADAPTER_DESC adesc = {};
			adpt->GetDesc(&adesc);	// �A�_�v�^�[�̐����I�u�W�F�N�g�擾

			std::wstring strDesc = adesc.Description;

			// �T�������A�_�v�^�[�̖��O���m�F
			if ( strDesc.find(L"NVIDIA") != std::string::npos ) {
				tmpAdapter = adpt;
				break;
			}
		}

		for (auto lv : levels) {

			// �f�o�C�X�̐���
			if ( D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK ) {
				featureLevel = lv;
				break; // ���Y�\�ȃo�[�W���������������烋�[�v��ł��؂�
			}
		}

		// �֐����g���ăE�C���h�E�̃T�C�Y��␳����
		AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

		// �E�C���h�E�T�C�Y�̐���
		HWND hwnd = CreateWindow(
			w.lpszClassName,						// �N���X���w��
			_T("DX12�e�X�g"),						// �^�C�g���o�[�̕���
			WS_OVERLAPPEDWINDOW,		// �^�C�g���o�[�Ƌ��E��������E�C���h�E
			CW_USEDEFAULT,						// �\��x���W��OS�ɂ��C��
			CW_USEDEFAULT,						// �\��y���W��OS�ɂ��C��
			wrc.right - wrc.left,						// �E�C���h�E��
			wrc.bottom - wrc.top,					// �E�C���h�E��
			nullptr,											// �e�E�C���h�E�n���h��
			nullptr,											// ���j���[�n���h��
			w.hInstance,								// �Ăяo���A�v���P�[�V�����n���h��
			nullptr											// �ǉ��p�����[�^�[
		);

		ShowWindow(hwnd, SW_SHOW);

		MSG msg = {};

		while (true) {
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// �p�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�
			if ( msg.message == WM_QUIT || _dev == nullptr ) {
				break;
			}
		}

		// �����N���X�͎g��Ȃ��̂œo�^��������
		UnregisterClass(w.lpszClassName, w.hInstance);
		
		//getchar();

		return 0;
		
	}
