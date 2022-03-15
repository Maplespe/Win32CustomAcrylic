/*
* 自定义Acrylic效果定义
*
* Author: Maple(mapleshr@icloud.com)
* date 2022-3-14 Create
*
* license: https://github.com/Maplespe/Win32CustomAcrylic/blob/main/LICENSE
*/
#pragma once
#include <unknwn.h>
#include <winrt\Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>
#include <DispatcherQueue.h>
#include <wincodec.h>
#include <d2d1_3.h>
#include <d3d11_3.h>

using namespace winrt;
using namespace Windows::System;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;

class CustomAcrylic
{
public:
	CustomAcrylic();
	~CustomAcrylic();

	void Initialize(HWND hwnd);

private:
	void CreateEffects();
	void CreateD2DResource();

	//Compositor资源
	Compositor m_compositor = nullptr;
	DesktopWindowTarget m_target = nullptr;
	DispatcherQueueController m_dispatcherQueueController = nullptr;

	com_ptr<ICompositionGraphicsDevice> m_graphicsDevice;
	com_ptr<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop> m_surfaceInterop;
	CompositionVirtualDrawingSurface m_virtualSurface = nullptr;
	CompositionSurfaceBrush m_surfaceBrush = nullptr;

	//D2D资源
	com_ptr<ID2D1Device> m_d2dDevice;
	com_ptr<ID2D1Factory1> m_d2dFactory;
	com_ptr<IWICImagingFactory2> m_wicFactory;
	com_ptr<ID3D11Device> m_d3dDevice;
	com_ptr<ID2D1Bitmap1> m_d2dBitmap;
};