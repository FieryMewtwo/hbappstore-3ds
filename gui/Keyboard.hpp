#include "../libs/chesto/src/Element.hpp"
#include "../libs/chesto/src/TextElement.hpp"

//#include <SDL2/SDL_image.h> //TODO: commented out, is this superfluous?
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
	std::string textInput;

	// draw a qwerty keyboard
	static const int rowsCount = 3;
	std::string rows[rowsCount] =
	{
		"Q W E R T Y U I O P",
		"A S D F G H J K L",
		"Z X C V B N M",
	};

	// the currently selected row and index
	int curRow = -1;
	int index = -1;

	// the below variables are stored to be used in processing touch events
	// and rendering the drawings to screen

	// attributes of each key
	int keyWidth = 0;
	int padding = 0;
	int textSize = 0;

	// attributes of delete and backspace keys
	int dPos = 0;
	int dHeight = 0;
	int sPos = 0;
	int dWidth = 0;
	int sWidth = 0;

	// positions of key location offset information
	int kXPad = 0;
	int kXOff = 0;
	int yYOff = 0;
	int kYPad = 0;
	int ySpacing = 0;

	bool touchMode = true;

	AppList* appList = NULL;

	void space();
	void backspace();
	void type(int y, int x);
	void inputChanged();
};
