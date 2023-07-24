#include <windows.h>
#include <tchar.h>

// DirectX12に最低限必要なヘッダーファイル
#include <d3d12.h>
#include <dxgi1_6.h>

// ヘッダーに合わせたライブラリをリンク
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <vector>

// 基本オブジェクト群
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

// ID3D12Deveiceオブジェクトを作成する
HRESULT D3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDeveice);

// フィーチャーレベル(グラボによっては対応していない可能性があるため)
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

// @brief コンソール画面にフォーマット付き文字列を表示
// @param format フォーマット(%dとか%fとか)
// @param 可変長変数
// @remarks この関数はデバッグ用です。デバッグ時にしか動作しません。
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

// 面倒だけど書かなければいけない関数
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// ウインドウが破棄されたら呼ぶ
	if ( msg == WM_DESTROY ) {
		PostQuitMessage(0); // OSに対して「もうこのアプリは終わる」と伝える。
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
		w.lpfnWndProc = (WNDPROC)WindowProcedure; // コールバック関数の指定
		w.lpszClassName = _T("DX12Sample");				// アプリケーションクラス名(適当で良い)
		w.hInstance = GetModuleHandle(nullptr);				// ハンドルの取得

		RegisterClassEx(&w); // アプリケーションクラス(ウインドウクラスの指定をOSに伝える)

		RECT wrc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT }; // ウインドウサイズを決める


		D3D_FEATURE_LEVEL featureLevel;

		auto result = CreateDXGIFactory(IID_PPV_ARGS(&_dxgiFactory));

		// アダプターの列挙用
		std::vector<IDXGIAdapter*> adapters;

		// ここに特定の名前を持つアダプターオブジェクトが入る
		IDXGIAdapter* tmpAdapter = nullptr;

		for ( int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i ) {
			adapters.push_back(tmpAdapter);
		}

		for (auto adpt : adapters) {
			DXGI_ADAPTER_DESC adesc = {};
			adpt->GetDesc(&adesc);	// アダプターの説明オブジェクト取得

			std::wstring strDesc = adesc.Description;

			// 探したいアダプターの名前を確認
			if ( strDesc.find(L"NVIDIA") != std::string::npos ) {
				tmpAdapter = adpt;
				break;
			}
		}

		for (auto lv : levels) {

			// デバイスの生成
			if ( D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK ) {
				featureLevel = lv;
				break; // 生産可能なバージョンが見つかったらループを打ち切り
			}
		}

		// 関数を使ってウインドウのサイズを補正する
		AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

		// ウインドウサイズの生成
		HWND hwnd = CreateWindow(
			w.lpszClassName,						// クラス名指定
			_T("DX12テスト"),						// タイトルバーの文字
			WS_OVERLAPPEDWINDOW,		// タイトルバーと境界線があるウインドウ
			CW_USEDEFAULT,						// 表示x座標はOSにお任せ
			CW_USEDEFAULT,						// 表示y座標はOSにお任せ
			wrc.right - wrc.left,						// ウインドウ幅
			wrc.bottom - wrc.top,					// ウインドウ高
			nullptr,											// 親ウインドウハンドル
			nullptr,											// メニューハンドル
			w.hInstance,								// 呼び出しアプリケーションハンドル
			nullptr											// 追加パラメーター
		);

		ShowWindow(hwnd, SW_SHOW);

		MSG msg = {};

		while (true) {
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// パプリケーションが終わるときにmessageがWM_QUITになる
			if ( msg.message == WM_QUIT || _dev == nullptr ) {
				break;
			}
		}

		// もうクラスは使わないので登録解除する
		UnregisterClass(w.lpszClassName, w.hInstance);
		
		//getchar();

		return 0;
		
	}
