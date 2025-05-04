/*
    Various helpers for UWP apps, add a new function if you need to interop between a dll and UWP calls
*/

#include "pch.h"
#include "libuwp.h"

#include <algorithm>
#include <future>

#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Graphics.Display.Core.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.AccessCache.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Pickers.Provider.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.ViewManagement.h>

using namespace winrt::Windows;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display::Core;
using namespace Storage;
using namespace Storage::AccessCache;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::ViewManagement;

static int width = 0;
static int height = 0;

static void* winref = nullptr;

void uwp_GetBundlePath(char* buffer)
{
    sprintf_s(buffer, 256, "%s", winrt::to_string(ApplicationModel::Package::Current().InstalledPath()).c_str());
}

void uwp_GetBundleFilePath(char* buffer, const char* filename)
{
    sprintf_s(buffer, 256, "%s\\%s", winrt::to_string(ApplicationModel::Package::Current().InstalledPath()).c_str(), filename);
}

void uwp_PickAFile(char* buffer)
{
    std::promise<std::wstring> promise;
    std::future<std::wstring> future = promise.get_future();

    auto dispatcher = winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher();

    dispatcher.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [p = std::move(promise)]() mutable {
        Pickers::FileOpenPicker picker;
        picker.SuggestedStartLocation(Pickers::PickerLocationId::ComputerFolder);
        picker.FileTypeFilter().Append(L"*");

        auto op = picker.PickSingleFileAsync();

        op.Completed([p = std::move(p)](auto&& asyncInfo, auto status) mutable {
            if (status == winrt::Windows::Foundation::AsyncStatus::Completed)
            {
                auto file = asyncInfo.GetResults();
                if (file)
                {
                    p.set_value(file.Path().c_str());
                    return;
                }
            }
            // Either failed or cancelled
            p.set_value(L"");
        });
    });

    std::wstring result = future.get();
    std::string out = "";

    if (!result.empty()) {
        out = std::string(result.begin(), result.end());
    }

    sprintf_s(buffer, 256, "%s", out.c_str());
}

void uwp_GetScreenSize(int* x, int* y)
{
    if (width == 0) {
        HdmiDisplayInformation hdi = HdmiDisplayInformation::GetForCurrentView();
        width = hdi.GetCurrentDisplayMode().ResolutionWidthInRawPixels();
        height = hdi.GetCurrentDisplayMode().ResolutionHeightInRawPixels();
    }

    *x = width;
    *y = height;
}

// TODO: Should make this a double as that's what RefreshRate() returns
float uwp_GetRefreshRate()
{
    // Limited to 60fps since xbox can't get a 120hz vsync lock in devmode
    return std::min(HdmiDisplayInformation::GetForCurrentView().GetCurrentDisplayMode().RefreshRate(), 60.0);
}

void* uwp_GetWindowReference()
{
    if (winref == nullptr) {
        winref = reinterpret_cast<void*>(winrt::get_abi(CoreWindow::GetForCurrentThread()));
    }

    return winref;
}

void uwp_SetupHDR(bool enabled)
{
    auto hdi = HdmiDisplayInformation::GetForCurrentView();
    auto modes = hdi.GetSupportedDisplayModes();
    for (unsigned i = 0; i < modes.Size(); i++)
    {
        HdmiDisplayMode mode = modes.GetAt(i);

        if (enabled && mode.ColorSpace() == HdmiDisplayColorSpace::BT2020 && mode.RefreshRate() >= 59)
        {
            hdi.RequestSetCurrentDisplayModeAsync(mode, HdmiDisplayHdrOption::Eotf2084);
            return;
        }
        else if (!enabled && mode.ColorSpace() == HdmiDisplayColorSpace::BT709 && mode.RefreshRate() >= 59)
        {
            hdi.RequestSetCurrentDisplayModeAsync(mode, HdmiDisplayHdrOption::EotfSdr);
            return;
        }
    }
}

void uwp_ProcessEvents()
{
    winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
}

// Register gamepad event hooks to method inside of calling program
void uwp_RegisterGamepadCallbacks(void (*callback)(void))
{
    namespace WGI = winrt::Windows::Gaming::Input;

    try
    {
        WGI::Gamepad::GamepadAdded([callback](auto&&, const WGI::Gamepad) {
            callback();
        });

        WGI::Gamepad::GamepadRemoved([callback](auto&&, const WGI::Gamepad) {
            callback();
        });
    }
    catch (winrt::hresult_error)
    {
    }

}