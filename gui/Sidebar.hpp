#include "../libs/chesto/src/ImageElement.hpp"
#include "../libs/chesto/src/ListElement.hpp"
#include "../libs/chesto/src/TextElement.hpp"
#include "../libs/chesto/src/Container.hpp"

class AppList;

#define TOTAL_CATS 8
#pragma once

class SidebarItem : public Element ///An Element representing a sidebar listing.
{
public:
	SidebarItem(int w, int h, const char *imgpath, const char *text, CST_Color *textcolor=0);
	~SidebarItem();
	ImageElement icon;
	TextElement name;
};

class Sidebar : public ListElement
{
public:
	Sidebar();
	~Sidebar();

	std::string currentCatName();
	std::string currentCatValue();

  void addHints();

	std::string searchQuery = "";

	AppList* appList = NULL;

	void render(Element* parent);
	bool process(InputEvents* event);

  bool showCurrentCategory = false;

	// the currently selected category index
	int curCategory = 1; // 1 is all apps

	// list of human-readable category titles and short names from the json

#if defined(__WIIU__) //TODO, PLATFORMSPECIFIC: move these into an external platform definition
	const char* cat_names[TOTAL_CATS] = { "Search", "All Apps", "Games", "Emulators", "Tools", "Advanced", "Concepts", "Misc" };
	const char* cat_value[TOTAL_CATS] = { "_search", "_all", "game", "emu", "tool", "advanced", "concept", "_misc" };
#else
	const char* cat_names[TOTAL_CATS] = { "Search", "All Apps", "Games", "Emulators", "Tools", "Advanced", "Themes", "Misc" };
	const char* cat_value[TOTAL_CATS] = { "_search", "_all", "game", "emu", "tool", "advanced", "theme", "_misc" };
#endif

	ImageElement logo;

private:
	Container* categoryHolder;
	SidebarItem* category[TOTAL_CATS];

	TextElement title;
	TextElement subtitle;

	ImageElement* hider = nullptr; ///Button to minify sidebar
	TextElement* hint = nullptr; ///Hint text for hider
};
