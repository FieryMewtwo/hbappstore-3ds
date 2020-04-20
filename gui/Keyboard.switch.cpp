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

void Keyboard::render(Element* parent)
{
	if (!hidden && hiddenPrev) {
		hiddenPrev = false;
		swkbdInlineCreate(&platformKbd);
		swkbdInlineSetChangedStringCallback(&platformKbd, texChanged);
		swkbdInlineLaunchForLibraryApplet(&platformKbd, SwkbdInlineMode_AppletDisplay, 0);
		SwkbdAppearArg arg;
		swkbdInlineMakeAppearArg(&arg, SwkbdType_Normal);
		swkbdInlineAppearArgSetOkButtonText(&arg, "Done");
		arg.dicFlag = 1;
		arg.returnButtonFlag = 0;
		swkbdInlineAppear(&platformKbd, &arg);
	}else if (hidden && !hiddenPrev) {
		hiddenPrev = true;
		lastText = "";
		lastLen = -1;
		swkbdInlineClose(&platformKbd);
	}

	if (!hidden) {
		SwkbdState st;
		
		swkbdInlineUpdate(&platformKbd, &st);
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
	lastText = "";
	lastLen = -1;
	for (auto &i : this->elements)
		delete i;
	super::removeAll();
}
#endif