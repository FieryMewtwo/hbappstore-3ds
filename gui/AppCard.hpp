#pragma once

#include "../libs/get/src/Package.hpp"

#include "../libs/chesto/src/RootDisplay.hpp"
#include "../libs/chesto/src/ImageElement.hpp"
#include "../libs/chesto/src/NetImageElement.hpp"
#include "../libs/chesto/src/TextElement.hpp"

class AppList;

class AppCard : public Element
{
public:
	AppCard(Package* package, AppList* list = nullptr, int w = 256*SCREEN_HEIGHT/720); //, int h = (150+45)*SCREEN_HEIGHT/720); //Defaults to the original hardcoded dimensions
	void update();
	bool process(InputEvents* event);
	void render(Element* parent);
	void displaySubscreen();
	void handleIconLoad();

	Package* package;
	AppList* list;
	bool iconFetch = false;

	// the number of which package this is in the list
	int index;

private:
	static CST_Color gray, black;

	// app icon
	NetImageElement icon;
	// version
	TextElement version;
	// status string
	TextElement status;
	// app name
	TextElement appname;
	// author
	TextElement author;
	// download status icon
	ImageElement statusicon;
};
