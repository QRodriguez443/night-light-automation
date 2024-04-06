#define UNICODE
#include "NightLightSliderValue.h"
#include "TimeDetection.h"
#include <UIAutomation.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

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
Result getSliderValue() {
    bool windowHide = false;

    // Launch the Settings app to the Night Light page
    ShellExecuteW(NULL, L"open", L"ms-settings:nightlight", NULL, NULL, SW_HIDE);

    // Get the top-level window of the Settings app
    HWND hwnd = FindWindowW(L"ApplicationFrameWindow", L"Settings");
    if (!hwnd) {
        if (!windowHide) {
            windowHide = FreeConsole() != 0;
            createNewWindow();
        }
        std::cerr << "Error finding Settings app window" << std::endl;
    }
    // Move the window out of the way!
    SetWindowPos(hwnd, NULL, -500, -500, 10, 10, SWP_HIDEWINDOW);
    SetWindowPos(hwnd, NULL, -500, -500, 10, 10, SWP_HIDEWINDOW);

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
    root->FindFirst(TreeScope_Children, newAuto, &pElement);
    if (!pElement) {
        std::wcout << L"Element not found, retrying..." << std::endl;

        while (true) {
            root->FindFirst(TreeScope_Children, newAuto, &pElement);
            if (pElement) {
                break;
            }
        }
    }
    //GROUP ELEMENT
    VARIANT groupProp;
    groupProp.vt = VT_BSTR;
    groupProp.bstrVal = SysAllocString(L"group");

    IUIAutomationCondition* newAuto2;
    op->CreatePropertyCondition(UIA_LocalizedControlTypePropertyId, groupProp, &newAuto2);

    IUIAutomationElement* pElement2;
    pElement->FindFirst(TreeScope_Children, newAuto2, &pElement2);
    if (!pElement2) {
        std::cerr << "Error finding group element, retrying..." << std::endl;

        while (true) {
            root->FindFirst(TreeScope_Children, newAuto, &pElement);
            if (pElement) {
                break;
            }
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

    Result result;
    result.hwnd = hwnd;
    result.value = (sliderPropcInt < 60) ? 1 : 0;
    result.pValuePattern = pValuePattern;
    result.slider = slider;

    //Clean up
    sliderCondition->Release();
    pane->Release();
    paneCondition->Release();
    pElement2->Release();
    newAuto2->Release();
    pElement->Release();
    newAuto->Release();
    root->Release();
    op->Release();
    SysFreeString(sliderPropc.bstrVal);

    return result;
}
////

void changeSliderValue() {
    Point time = timeDetection(); //Get current time of day

    Result value = getSliderValue();

    //Returned values from getSliderValue()
    HWND returnedHwnd = value.hwnd;
    IRangeValueProvider* pValuePattern = value.pValuePattern;
    IUIAutomationElement* slider = value.slider;

    VARIANT sliderPropc;
    sliderPropc.vt = VT_BSTR;
    slider->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &sliderPropc);
    std::wcout << "Slider element: " << "ValueValueId: " << sliderPropc.bstrVal << std::endl;
    int sliderPropcPrevious = _wtoi(sliderPropc.bstrVal);

    double newValue = { 0 };
    bool skipNext = false; //Used to skip the code that sets slider value
    int sliderPropcInt = _wtoi(sliderPropc.bstrVal);

    for (int i = 0; i <= 8; i++) {
        slider->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &sliderPropc);
        sliderPropcInt = _wtoi(sliderPropc.bstrVal); //In case the loop iterates more than once
        skipNext = false;
        bool valueSet = false; //Used to determine if newValue is 0

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
            if (i == 3) {
                skipNext = true;
                std::wcout << L"No reason to attempt change, slider value already correct..." << std::endl;
            }
        }
    }
    //Gets the current slider value, then manipulates it
    if (skipNext != true) {
        int newValueToInt = static_cast<int>(newValue);
        pValuePattern->SetValue(newValue);

        SysFreeString(sliderPropc.bstrVal);

        /*The excess iterations ensure the current property retrieved is the updated programmatic
        value and not the previous value, as there is a short delay when changing the value.*/
        for (int i = 0; i <= 40; i++) {
            VARIANT sliderPropc;
            sliderPropc.vt = VT_BSTR;
            slider->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &sliderPropc);
            std::wcout << "Current programmatic slider value: " << sliderPropc.bstrVal << ", target value: " << newValueToInt << std::endl;
            int sliderPropcInt = _wtoi(sliderPropc.bstrVal);

            //If the programmatic value is equal to the target value after i = 30, then update is successful
            if (i == 35) {
                if (sliderPropcInt == newValueToInt) {
                    slider->GetCurrentPattern(UIA_RangeValuePatternId, (IUnknown**)&pValuePattern);
                    pValuePattern->SetValue(newValue); // Double-set to ensure screen updates UI
                    break;
                }
                else { 
                    std::wcout << L"Wtf! The update failed..." << std::endl; 
                }
            }
            else if (i == 40) { //In case loop is unbreakable
                break;
            }
        }
    }

    SendMessageW(returnedHwnd, WM_CLOSE, 0, 0);

    //Clean up
    slider->Release();
    pValuePattern->Release();
    SysFreeString(sliderPropc.bstrVal);

    CoUninitialize();
}