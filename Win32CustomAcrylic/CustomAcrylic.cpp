/*
* 自定义Acrylic效果实现
* 
* Author: Maple(mapleshr@icloud.com)
* date 2022-3-14 Create
* 
* license: https://github.com/Maplespe/Win32CustomAcrylic/blob/main/LICENSE
*/
#include "CustomAcrylic.h"
#include "winrt\Windows.Graphics.Effects.h"
#include "winrt\Microsoft.Graphics.Canvas.Effects.h"
#include "winrt\Windows.UI.Composition.h"

#include "resource.h"
#include <shlwapi.h>
#pragma comment (lib, "Shlwapi.lib")

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;
using namespace Windows::Foundation::Numerics;

CustomAcrylic::CustomAcrylic()
{
}

CustomAcrylic::~CustomAcrylic()
{

}

void CustomAcrylic::Initialize(HWND hwnd)
{
	namespace abi = ABI::Windows::System;

	if (m_dispatcherQueueController == nullptr)
	{
		DispatcherQueueOptions options
		{
			sizeof(DispatcherQueueOptions),
			DQTYPE_THREAD_CURRENT,
			DQTAT_COM_ASTA
		};

		Windows::System::DispatcherQueueController controller{ nullptr };
		check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(put_abi(controller))));
		m_dispatcherQueueController = controller;

		m_compositor = Compositor();
		if (m_compositor)
		{
			namespace abi = ABI::Windows::UI::Composition::Desktop;

			//获取互操作接口 绑定到窗口目标 Gets the interop interface bound to the window target
			auto interop = m_compositor.as<abi::ICompositorDesktopInterop>();
			DesktopWindowTarget target{ nullptr };
			check_hresult(interop->CreateDesktopWindowTarget(hwnd, false, reinterpret_cast<abi::IDesktopWindowTarget**>(put_abi(target))));
			m_target = target;

			CreateD2DResource();

			//创建视觉表面 create virtual surface
			auto graphicsDevice2 = m_graphicsDevice.as<ICompositionGraphicsDevice2>();

			m_virtualSurface = graphicsDevice2.CreateVirtualDrawingSurface(
				{ 256, 256 },
				Windows::Graphics::DirectX::DirectXPixelFormat::R16G16B16A16Float,
				Windows::Graphics::DirectX::DirectXAlphaMode::Premultiplied);

			m_surfaceInterop = m_virtualSurface.as<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>();

			ICompositionSurface surface = m_surfaceInterop.as<ICompositionSurface>();

			//创建表面画刷 create surface brush
			m_surfaceBrush = m_compositor.CreateSurfaceBrush(surface);

			//从PE资源加载图像 Load image form PE
			HINSTANCE hDll = GetModuleHandle(NULL);
			HRSRC hRes = FindResourceW(hDll, MAKEINTRESOURCE(IDB_PNG1), L"PNG");
			HGLOBAL hgRes = LoadResource(hDll, hRes);
			DWORD nResSize = SizeofResource(hDll, hRes);
			BYTE* pRes = (BYTE*)LockResource(hgRes);
			IStream* stream = SHCreateMemStream(pRes, nResSize);
			UnlockResource(hgRes);
			FreeResource(hgRes);
			
			//从WIC创建D2D位图 Create D2D bitmap from WIC
			com_ptr<IWICBitmapDecoder> decoder = nullptr;
			check_hresult(m_wicFactory->CreateDecoderFromStream(stream, &GUID_VendorMicrosoft, WICDecodeMetadataCacheOnDemand, decoder.put()));

			stream->Release();

			com_ptr<IWICBitmapFrameDecode> frame = nullptr;
			check_hresult(decoder->GetFrame(0, frame.put()));

			com_ptr<IWICFormatConverter> converter;
			check_hresult(m_wicFactory->CreateFormatConverter(converter.put()));

			com_ptr<IWICPalette> palette = nullptr;
			check_hresult(converter->Initialize(frame.get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, palette.get(),
				0, WICBitmapPaletteTypeCustom));

			com_ptr<IWICBitmap> wicbitmap = nullptr;
			check_hresult(m_wicFactory->CreateBitmapFromSource(converter.get(), WICBitmapCreateCacheOption::WICBitmapNoCache, wicbitmap.put()));

			//D2D位图格式 D2D Bitmap format
			D2D1_PIXEL_FORMAT format;
			format.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
			format.format = DXGI_FORMAT_B8G8R8A8_UNORM;

			D2D1_BITMAP_PROPERTIES1 bitmapProp;
			bitmapProp.dpiX = 96;
			bitmapProp.dpiY = 96;
			bitmapProp.pixelFormat = format;
			bitmapProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
			bitmapProp.colorContext = 0;

			//将纹理绘制到视觉表面 Paint texture to visual surface
			POINT offset = { 0,0 };
			RECT rc = { 0,0,256,256 };
			com_ptr<ID2D1DeviceContext> d2dDeviceContext;
			m_surfaceInterop->BeginDraw(&rc, __uuidof(ID2D1DeviceContext), (void**)d2dDeviceContext.put(), &offset);

			d2dDeviceContext->Clear();

			d2dDeviceContext->CreateBitmapFromWicBitmap(wicbitmap.get(), bitmapProp, m_d2dBitmap.put());

			d2dDeviceContext->DrawBitmap(m_d2dBitmap.get());

			m_surfaceInterop->EndDraw();

			auto root = m_compositor.CreateSpriteVisual();
			root.RelativeSizeAdjustment({ 1.0f, 1.0f });
			m_target.Root(root);
			CreateEffects();
		}
	}
}

void CustomAcrylic::CreateEffects()
{
	auto root = m_target.Root().as<SpriteVisual>();
	if (root)
	{
		using namespace Microsoft::Graphics::Canvas;

		//高斯模糊效果 GaussianBlur
		auto blur = Effects::GaussianBlurEffect();
		blur.Name(L"Blur");
		blur.BorderMode(Effects::EffectBorderMode::Hard);
		blur.BlurAmount(40.f); //40是现代win系统标准的Acrylic模糊度 1709系统的模糊度要低 大概25 
							   //40 is the standard acrylic fuzziness of modern win system. The fuzziness of 1709 system is about 25 lower
		blur.Source(CompositionEffectSourceParameter(L"Backdrop"));

		auto blurBrush = m_compositor.CreateEffectFactory(blur).CreateBrush();
		blurBrush.SetSourceParameter(L"Backdrop", m_compositor.CreateBackdropBrush());

		//饱和度效果 Saturation
		auto saturation = Effects::SaturationEffect();
		saturation.Name(L"Saturation");
		saturation.Saturation(2.f);//饱和度+1倍 Saturation + 1x
		saturation.Source(blur);

		auto saturationBrush = m_compositor.CreateEffectFactory(saturation).CreateBrush();
		saturationBrush.SetSourceParameter(L"Backdrop", blurBrush);

		//平铺效果 Tile effect
		auto border = Effects::BorderEffect();
		border.ExtendX(CanvasEdgeBehavior::Wrap);
		border.ExtendY(CanvasEdgeBehavior::Wrap);
		border.Source(CompositionEffectSourceParameter(L"Noise"));
		
		auto borderBrush = m_compositor.CreateEffectFactory(border).CreateBrush();
		borderBrush.SetSourceParameter(L"Noise", m_surfaceBrush);

		//透明度效果 Opacity
		auto opacity = Effects::OpacityEffect();
		opacity.Name(L"Opacity");
		opacity.Opacity(0.03f);//噪点纹理的透明度是0.03 这是微软wpf源码里定义的
							   //The transparency of the noise texture is 0.03, which is defined in the Microsoft WPF example
		opacity.Source(CompositionEffectSourceParameter(L"Opacity"));

		auto opacityBrush = m_compositor.CreateEffectFactory(opacity).CreateBrush();
		opacityBrush.SetSourceParameter(L"Opacity", borderBrush);

		//合成效果 把噪点纹理和窗口背景组合起来
		//The composite effect combines the noise texture with the window background
		auto blend = Effects::BlendEffect();
		blend.Foreground(CompositionEffectSourceParameter(L"main"));
		blend.Background(CompositionEffectSourceParameter(L"src"));
		blend.Mode(Effects::BlendEffectMode::Multiply);

		auto blendBrush = m_compositor.CreateEffectFactory(blend).CreateBrush();
		blendBrush.SetSourceParameter(L"main", opacityBrush);
		blendBrush.SetSourceParameter(L"src", saturationBrush);

		root.Brush(blendBrush);
	}
}

void CustomAcrylic::CreateD2DResource()
{
	namespace abi = ABI::Windows::UI::Composition;

	D2D1_FACTORY_OPTIONS options {};
	check_hresult(D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		options,
		m_d2dFactory.put()));

	check_hresult(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr, 0,
		D3D11_SDK_VERSION,
		m_d3dDevice.put(),
		nullptr,
		nullptr));

	com_ptr<IDXGIDevice> const dxdevice = m_d3dDevice.as<IDXGIDevice>();
	com_ptr<abi::ICompositorInterop> interopCompositor = m_compositor.as<abi::ICompositorInterop>();

	//从D2D设备创建视觉设备 Create a visual device from a D2D device
	check_hresult(m_d2dFactory->CreateDevice(dxdevice.get(), m_d2dDevice.put()));
	check_hresult(interopCompositor->CreateGraphicsDevice(m_d2dDevice.get(), reinterpret_cast<abi::ICompositionGraphicsDevice**>(put_abi(m_graphicsDevice))));
	check_hresult(CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, __uuidof(m_wicFactory), m_wicFactory.put_void()));
}