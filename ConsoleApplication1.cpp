#define OEMRESOURCE
#include <windows.h>
#include <iostream>
#include <shellapi.h>
#include <chrono>
#include <time.h>
#include "Result.h"
#include "NightLightSliderValue.h"
#include "TimeDetection.h"


int main() {
    bool firstLoop = true;
    int timeToSleep = 5000; //5 seconds
    bool windowHide = false;

    while (true) {
        int conditionIsMet = 0;

        //TIMER DISPLAY
        if (timeToSleep >= 1000 && timeToSleep <= 59000) {
            std::wcout << L"Waiting " << timeToSleep / 1000 << " seconds..." << std::endl;
        }
        else if (timeToSleep >= 60000 && timeToSleep <= 3540000) {
            std::wcout << L"Waiting " << timeToSleep / 60000 << " minutes..." << std::endl;
        }
        else if (timeToSleep == 3600000){
            std::wcout << L"Waiting " << timeToSleep / 3600000 << " hour..." << std::endl;
        }
        else {
            std::wcout << L"Waiting " << timeToSleep / 3600000 << " hours..." << std::endl;
        }
        ////
        Sleep(timeToSleep);

        //If cmd is not hidden and is not the first loop, hide it
        if (!windowHide && !firstLoop) {
            windowHide = FreeConsole() != 0;
        }
        //Set sleep time to 5 mins after first iteration
        if (firstLoop) {
            firstLoop = false;
            timeToSleep = 300000; //5 minutes
        }

        //Wait until specific time is reached
        Point time = timeDetection(); //Get current time of day

        if (time.hour >= 8 && time.hour <= 16) {
            std::wcout << L"Condition met, executing function..." << std::endl;
            if (timeToSleep != 7200000) {
                timeToSleep = 7200000; // 2 hours
            }
            conditionIsMet = 1;
            changeSliderValue();
        }
        if (time.hour >= 17 && time.hour <= 19) {
            std::wcout << L"Condition met, executing function..." << std::endl;
            if (timeToSleep != 3600000) {
                timeToSleep = 3600000; // 1 hour
            }
            conditionIsMet = 1;
            changeSliderValue();
        }
        if (time.hour == 20) {
            std::wcout << L"Condition met, executing function..." << std::endl;
            if (timeToSleep != 1200000) {
                timeToSleep = 1200000; // 20 minutes
            }
            conditionIsMet = 1;
            changeSliderValue();
        }
        if (time.hour == 21) {
            std::wcout << L"Condition met, executing function..." << std::endl;
            if (timeToSleep != 1200000) {
                timeToSleep = 1200000; // 20 minutes
            }
            conditionIsMet = 1;
            changeSliderValue();
        }
        if (time.hour >= 22 || time.hour == 0 && time.hour <= 7) {
            std::wcout << L"Condition met, executing function..." << std::endl;
            if (timeToSleep != 3600000) {
                timeToSleep = 3600000; // 1 hour
            }
            conditionIsMet = 1;
            changeSliderValue();
            }

        if (conditionIsMet == 0) { // Skip this block if any of the above statements were executed
            Result value = getSliderValue();
            int result = value.value;
            HWND returnedHwnd = value.hwnd;
            timeToSleep = 300000; //5 minutes

            // If value of slider is less than the minimum: 60
            if (result == 1) {
                std::wcout << L"Last resort condition met, executing function..." << std::endl;
                changeSliderValue();
            }
            else {
                SendMessageW(returnedHwnd, WM_CLOSE, 0, 0);
                std::wcout << L"No reason to attempt change, restarting iteration..." << std::endl;
            }
        }
    }
}
