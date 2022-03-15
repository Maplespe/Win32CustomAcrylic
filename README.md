# Win32CustomAcrylic
Using winrt API to realize custom acrylic synthesis effect

使用WInRT API实现自定义Acrylic合成效果


Windows.UI.Composition - Interop
### Custom
- [x] BlurAmount(模糊度)
- [x] Saturation(饱和度)
- [x] NoiseTexture(噪点纹理)
- [x] NoiseOpacity(噪点透明度)
- [x] TintColor(着色)

## Preview
`blur.BlurAmount(40.f);`


![preview1](https://github.com/Maplespe/Win32CustomAcrylic/blob/main/preview/preview1.png)


`blur.BlurAmount(10.f);`


![preview2](https://github.com/Maplespe/Win32CustomAcrylic/blob/main/preview/preview2.png)


## Effect
The parameters of the effect are defined in `CustomArylic::CreateEffects` in `CustomAcrylic.cpp`


效果参数在 `CustomAcrylic.cpp` 中的 `CustomArylic::CreateEffects` 定义
```cpp
blur.BlurAmount(40.f);
saturation.Saturation(2.f);
opacity.Opacity(0.03f);
```

## Using
Nuget Package:
```xml
<package id="Microsoft.Graphics.Win2D" version="1.0.0.30" targetFramework="native" />
<package id="Microsoft.Windows.CppWinRT" version="2.0.220224.4" targetFramework="native" />
<package id="Microsoft.Windows.SDK.BuildTools" version="10.0.22000.194" targetFramework="native" />
<package id="Microsoft.WindowsAppSDK" version="1.0.0" targetFramework="native" />
```
Environment(开发环境)：
```
Visual Studio 2019
Windows SDK 10.0.18362.0
```

## Tested
Windows 10 1809 17763.737

Windows 10 2004 19041.208

Windows 11 21H2 22000.556

Other versions are not tested, and theoretically support win10 1703 at least

其他版本未测试 理论上应该最低支持Win10 1703版本
## Airspace Issue
acrylic effect is rendered using direct composition, So it always cause [Airspace Issue](https://github.com/dotnet/wpf/issues/152)


Therefore, you can only use DirectX or visual layer to render content


For information about direct2d and composition interoperability, please see [composition-native-interop](https://docs.microsoft.com/en-us/windows/uwp/composition/composition-native-interop)

## 空域问题
Acrylic效果使用composition接口渲染 因此会导致"[空域](https://github.com/dotnet/wpf/issues/152)"问题

因此您只能使用DirectX或者VisualLayer来渲染内容

有关Direct2D和composition的互操作请查看[composition-native-interop](https://docs.microsoft.com/en-us/windows/uwp/composition/composition-native-interop)
