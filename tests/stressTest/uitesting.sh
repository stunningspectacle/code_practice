#!/bin/bash
adb shell uiautomator runtest UiTestingExample.jar -c com.intel.vishal.uitestingexample.Launch_Suspend_Resume -e displayOffTime 90 -e displayOnTime 10 -e iterations 3000
