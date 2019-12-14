#ifndef APPDETAILS_H_
#define APPDETAILS_H_

#include "../libs/get/src/Get.hpp"
#include "../libs/get/src/Package.hpp"

#include "../libs/chesto/src/Button.hpp"
#include "../libs/chesto/src/ListElement.hpp"
#include "../libs/chesto/src/ProgressBar.hpp"
#include "../libs/chesto/src/TextElement.hpp"
#include "../libs/chesto/src/NetImageElement.hpp"

class AppList;


class AppDetailsContent : public ListElement
{
public:
	AppDetailsContent(Package *package);
	bool process(InputEvents* event);
	void render(Element* parent);

	Button reportIssue;
	Button moreByAuthor;

	SDL_Color gray = { 0x50, 0x50, 0x50, 0xff };
	SDL_Color black = { 0x00, 0x00, 0x00, 0xff };

private:
	// banner/text constants
	const int MARGIN = 60;
	const int BANNER_X = MARGIN + 5;
	const int BANNER_Y = 140;

	TextElement title;
	NetImageElement banner;
	TextElement title2;
	TextElement details;
	TextElement changelog;
};


class AppDetails : public Element
{
public:
	AppDetails(Package* package, AppList* appList);
	~AppDetails();

	std::string getPackageDetails(Package* package);
	const char *getAction(Package* package);
	bool process(InputEvents* event);
	void render(Element* parent);
	bool launchFile(char* path, char* context);
	bool themeInstall(char* installerPath);
	void getSupported();

	SDL_Color red = { 0xFF, 0x00, 0x00, 0xff };
	SDL_Color gray = { 0x50, 0x50, 0x50, 0xff };
	SDL_Color black = { 0x00, 0x00, 0x00, 0xff };
	SDL_Color white = { 0xFF, 0xFF, 0xFF, 0xff };

	bool operating = false;
	Package* package;
	Get* get;
	AppList* appList;
	ProgressBar* pbar = NULL;
	int highlighted = -1;

	bool canLaunch = false;

	// the callback method to update the currently displayed pop up (and variables it needs)
	static int updateCurrentlyDisplayedPopup(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
	static int lastFrameTime;

	void proceed();
	void back();
	void launch();

	void moreByAuthor();
	void leaveFeedback();

	void preInstallHook();
	void postInstallHook();

	ProgressBar downloadProgress;

private:
	Button download;
	Button cancel;
#if defined(SWITCH)
	Button* start = nullptr;
	TextElement* errorText = nullptr;
#endif
	TextElement details;
	AppDetailsContent content;
	TextElement downloadStatus;
};


#endif
