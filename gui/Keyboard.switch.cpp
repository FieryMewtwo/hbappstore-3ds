#ifdef SWITCH
#include "Keyboard.switch.hpp"
#include "AppList.hpp"
#include "Feedback.hpp"

std::string lastText;
u32 lastLen = 0;

void texChanged(const char* str, SwkbdChangedStringArg* arg) {
	lastText = str;
	lastLen = arg->stringLen;
}

Keyboard::Keyboard(AppList* appList)
	: appList(appList)
{
	if (appList == NULL) {
		noApp = true;
	}
}
#include <iostream>
#include <sstream>
void _errOnFail(Result res, int line) {
	if (R_FAILED(res)){
	ErrorApplicationConfig con;
	std::stringstream msg;
	msg << "Error on line: " << line << "; 0x" << std::hex << res;
	errorApplicationCreate(&con, msg.str().c_str(), NULL);
	errorApplicationShow(&con);
	exit(0);
	}
}

#define errOnFail(result) _errOnFail(result, __LINE__)

void Keyboard::render(Element* parent)
{
	if (!hidden && hiddenPrev) {
		hiddenPrev = false;
		errOnFail(swkbdInlineCreate(&platformKbd));
		swkbdInlineSetChangedStringCallback(&platformKbd, texChanged);
		errOnFail(swkbdInlineLaunchForLibraryApplet(&platformKbd, SwkbdInlineMode_AppletDisplay, 0));
		SwkbdAppearArg arg;
		swkbdInlineMakeAppearArg(&arg, SwkbdType_Normal);
		swkbdInlineAppearArgSetOkButtonText(&arg, "Done");
		arg.dicFlag = 1;
		arg.returnButtonFlag = 0;
		swkbdInlineAppear(&platformKbd, &arg);
	}else if (hidden && !hiddenPrev) {
		hiddenPrev = true;
		errOnFail(swkbdInlineClose(&platformKbd));
	}

	if (!hidden) {
		SwkbdState st;
		
		errOnFail(swkbdInlineUpdate(&platformKbd, &st));
		if (st == SwkbdState_Submitted) {
			if(!noApp) {
				appList->toggleKeyboard();
			}else{
				hidden = true;
			}
			return;
		}
		textInput = lastText;
		if (lastLen != trackedLen) {
			trackedLen = lastLen;
			inputChanged();
		}
	}

}

bool Keyboard::process(InputEvents* event)
{
	return true;
}

const std::string& Keyboard::getTextInput()
{
	return textInput;
}

void Keyboard::inputChanged()
{
	if (inputCallback)
		inputCallback();
}

Keyboard::~Keyboard()
{
	swkbdInlineClose(&platformKbd);
	for (auto &i : this->elements)
		delete i;
	super::removeAll();
}
#endif