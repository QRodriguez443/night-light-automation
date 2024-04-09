#include <windows.h>
#include "NightLightSliderValue.h"
#include "TimeDetection.h"
#include <UIAutomation.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

void PressKey(int keyCode) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode; // virtual key code of the key to press
    input.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &input, sizeof(INPUT));

    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
    SendInput(1, &input, sizeof(INPUT));
}

int createNewWindow() {
    // Create a new console window.
    AllocConsole();

    // Redirect the standard handles to the new console.
    FILE* fp;
    errno_t err = freopen_s(&fp, "CONOUT$", "w", stdout);
    if (err != 0) {
        std::cerr << "Error: Failed to redirect standard output." << std::endl;
        return 1;
    }
    err = freopen_s(&fp, "CONOUT$", "w", stderr);
    if (err != 0) {
        std::cerr << "Error: Failed to redirect standard error." << std::endl;
        return 1;
    }
    err = freopen_s(&fp, "CONIN$", "r", stdin);
    if (err != 0) {
        std::cerr << "Error: Failed to redirect standard input." << std::endl;
        return 1;
    }

    // Set the console output mode to handle Unicode characters.
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
}
////

//Open settings to night light page and get current slider value
Result getSliderValue(double newValue) {
    bool windowHide = false;
    HWND hwnd = NULL;

    // Get the top-level window of the Settings app
    for (int i = 0; i <= 4; i++) {
        hwnd = FindWindowW(L"ApplicationFrameWindow", L"Settings");
        if (!hwnd) {
            // Launch the Settings app to the Night Light page
            ShellExecuteW(NULL, L"open", L"ms-settings:nightlight", NULL, NULL, SW_NORMAL);
            std::cerr << "Error finding Settings app window, resolving..." << std::endl;
        } else {
            // Move the window out of the way!
            SetWindowPos(hwnd, NULL, -500, -500, 10, 10, SW_NORMAL);
            break;
        }
    }

    CoInitialize(NULL);

    IUIAutomation* op;
    CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void**)&op);

    // Get the top-level Settings window Element
    IUIAutomationElement* root = NULL;
    HRESULT hr = op->ElementFromHandle(hwnd, &root);
    if (!root) {
        if (!windowHide) {
            windowHide = FreeConsole() != 0;
            createNewWindow();
        }
        std::cerr << "Error getting IAccessible interface for Settings app window" << std::endl;
    }
    //SECOND-LEVEL WINDOW
    VARIANT otherProp;
    otherProp.vt = VT_BSTR;
    otherProp.bstrVal = SysAllocString(L"Windows.UI.Core.CoreWindow");

    IUIAutomationCondition* newAuto;
    op->CreatePropertyCondition(UIA_ClassNamePropertyId, otherProp, &newAuto);

    IUIAutomationElement* pElement;
    while (true) {
        root->FindFirst(TreeScope_Children, newAuto, &pElement);
        if (!pElement) {
            std::wcout << L"Element not found, retrying..." << std::endl;

            root->FindFirst(TreeScope_Children, newAuto, &pElement);
            if (pElement) {
                break;
            }
        } else {
            break;
        }
    }
    //GROUP ELEMENT
    VARIANT groupProp;
    groupProp.vt = VT_BSTR;
    groupProp.bstrVal = SysAllocString(L"group");

    IUIAutomationCondition* newAuto2;
    op->CreatePropertyCondition(UIA_LocalizedControlTypePropertyId, groupProp, &newAuto2);

    IUIAutomationElement* pElement2;
    while (true) {
        pElement->FindFirst(TreeScope_Children, newAuto2, &pElement2);
        if (!pElement2) {
            std::cerr << "Error finding group element, retrying..." << std::endl;

            pElement->FindFirst(TreeScope_Children, newAuto2, &pElement2);
            if (pElement2) {
                break;
            }
        } else {
            break;
        }
    }
    //PANE ELEMENT
    VARIANT panePropc;
    panePropc.vt = VT_BSTR;
    panePropc.bstrVal = SysAllocString(L"pane");

    IUIAutomationCondition* paneCondition;
    op->CreatePropertyCondition(UIA_LocalizedControlTypePropertyId, panePropc, &paneCondition);

    IUIAutomationElement* pane;
    pElement2->FindFirst(TreeScope_Children, paneCondition, &pane);
    ////

    //STRENGTH SLIDER ELEMENT
    VARIANT sliderProp;
    sliderProp.vt = VT_BSTR;
    sliderProp.bstrVal = SysAllocString(L"slider");

    IUIAutomationCondition* sliderCondition;
    op->CreatePropertyCondition(UIA_LocalizedControlTypePropertyId, sliderProp, &sliderCondition);

    IUIAutomationElement* slider;
    pane->FindFirst(TreeScope_Children, sliderCondition, &slider);

    //MODIFY STRENGTH SLIDER VALUE
    IRangeValueProvider* pValuePattern;
    slider->GetCurrentPattern(UIA_RangeValuePatternId, (IUnknown**)&pValuePattern);

    VARIANT sliderPropc;
    sliderPropc.vt = VT_BSTR;
    slider->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &sliderPropc);
    std::wcout << "Slider element: " << "ValueValueId: " << sliderPropc.bstrVal << std::endl;
    int sliderPropcInt = _wtoi(sliderPropc.bstrVal);

    if (newValue != NULL) {
        pValuePattern->SetValue(newValue); // Change slider value to target value
    }
    Result result;
    result.hwnd = hwnd;
    result.value = (sliderPropcInt < 60) ? 1 : 0;
    result.sliderValue = sliderPropcInt;

    //Clean up
    SysFreeString(sliderPropc.bstrVal);
    pValuePattern->Release();
    slider->Release();
    sliderCondition->Release();
    pane->Release();
    paneCondition->Release();
    pElement2->Release();
    newAuto2->Release();
    pElement->Release();
    newAuto->Release();
    root->Release();
    op->Release();

    return result;
}
////

void changeSliderValue() {
    Point time = timeDetection(); //Get current time of day

    Result value = getSliderValue(NULL);

    //Returned values from getSliderValue()
    HWND returnedHwnd = value.hwnd;
    int sliderPropcInt = value.sliderValue;

    std::wcout << "Slider element: " << "ValueValueId: " << sliderPropcInt << std::endl;

    double newValue = { 0 };
    bool skipNext = false; // Used to skip the code that sets slider value


    for (int i = 0; i <= 5; i++) {
        skipNext = false;
        bool valueSet = false; //Used to determine if newValue is 0
        value = getSliderValue(NULL); // Run the function again to update the slider value

        //Determine what value to set night light
        // 20 == 8pm, if less than or equal to 8:30pm, 
        if (time.hour >= 8 && time.hour <= 19) {
            //Then if night light is not 70, change it to 70
            if (sliderPropcInt != 70) {
                newValue = 70.00;
                valueSet = true;
            }
            else {
                skipNext = true;
            }
        }
        if (time.hour == 20) {
            //Then if night light is not 80, change it to 80
            if (sliderPropcInt != 80) {
                newValue = 80.00;
                valueSet = true;
            }
            else {
                skipNext = true;
            }
        }
        if (time.hour == 21) {
            //Check if night light value is not 90 before changing value
            if (sliderPropcInt != 90) {
                newValue = 90.00;
                valueSet = true;
            }
            else {
                skipNext = true;
            }
        }
        if (time.hour >= 22 || time.hour == 0 && time.hour <= 7) {
            //Check if night light value is not 100 before changing value
            if (sliderPropcInt < 100) {
                newValue = 100.00;
                valueSet = true;
            }
            else {
                skipNext = true;
            }
        }
        if (valueSet) { // Ensure newValue has a value before continuing
            break;
        }
        else {
            std::wcout << newValue << "not setting properly, current value: " << sliderPropcInt << std::endl;
        }
    }
    //Gets the current slider value, then manipulates it
    if (skipNext != true) {
        int newValueToInt = static_cast<int>(newValue); // Convert double to int for logging
        //PressKey(0x09);
        //Sleep(500);
        //PressKey(0x27);
        //Sleep(500);
        
        value = getSliderValue(newValue);
        std::wcout << "Current programmatic slider value: " << sliderPropcInt << ", target value: " << newValueToInt << std::endl;
    }
    SendMessageW(returnedHwnd, WM_CLOSE, NULL, NULL);

    CoUninitialize();
}