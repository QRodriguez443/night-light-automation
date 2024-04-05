#pragma once
#ifndef RESULT_H
#define RESULT_H

#include <UIAutomation.h>

struct Result {
    HWND hwnd;
    int value;
    VARIANT sliderPropc;
    IRangeValueProvider* pValuePattern;
    IUIAutomationElement* slider;
};

#endif