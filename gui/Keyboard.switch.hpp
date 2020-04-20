
#include "../libs/chesto/src/Element.hpp"
#include "../libs/chesto/src/TextElement.hpp"
#include <switch.h>
#pragma once

// bad idea, TODO: Input-able interface for elements that can be typed-in
class Feedback;
class AppList;

class Keyboard : public Element
{
public:
	Keyboard(AppList* appList = NULL);
	~Keyboard();
	void render(Element* parent);
	bool process(InputEvents* event);

	// setup field variables
	void updateSize();

	// get text inputed on the keyboard so far
	const std::string& getTextInput();

	// keyboard input callback
	std::function<void()> inputCallback;

private:
	// text inputed by the keyboard
    bool noApp = false;
	std::string textInput;
    bool hiddenPrev = true;
    SwkbdInline platformKbd;
    u32 trackedLen = -1;
    AppList* appList = NULL;
	void inputChanged();
};
