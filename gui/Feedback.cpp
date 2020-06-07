#include "Feedback.hpp"
#include "ImageCache.hpp"
#include "MainDisplay.hpp"
#include "main.hpp"

#include "../libs/chesto/src/RootDisplay.hpp"

#ifndef NETWORK_MOCK
#include <curl/curl.h>
#include <curl/easy.h>
#endif

Feedback::Feedback(Package* package)
	: package(package)
	, title((std::string("Leaving feedback for: \"") + package->title + "\"").c_str(), 25*SCREEN_HEIGHT/720)
	, icon(package->getIconUrl().c_str(), []{ return new ImageElement(RAMFS "res/default.png"); })
	, quit("Discard", Y_BUTTON, true, 24*SCREEN_HEIGHT/720)
	, send("Submit", X_BUTTON, true, 24*SCREEN_HEIGHT/720)
	, response("If you need to send more detailed feedback, please email us at fight@fortheusers.org", 20*SCREEN_HEIGHT/720, NULL, NORMAL, 360)
#if defined(__WIIU__)
	, hint("(btw you can press Minus to exit!)", 20*SCREEN_HEIGHT/720, NULL)
#endif
{
	title.position(50*SCREEN_HEIGHT/720, 30*SCREEN_HEIGHT/720);
	super::append(&title);

	icon.position(0, 160*SCREEN_HEIGHT/720);
	icon.alignLeftWith(&title);
#if defined(_3DS) || defined(_3DS_MOCK)
  icon.resize(ICON_SIZE, ICON_SIZE);
#else
	icon.resize(256*SCREEN_HEIGHT/720, ICON_SIZE);
#endif
	super::append(&icon);

	keyboard.inputCallback = std::bind(&Feedback::keyboardInputCallback, this);
	keyboard.x = 200*SCREEN_HEIGHT/720;
	super::append(&keyboard);

	quit.position(470*SCREEN_HEIGHT/720, 340*SCREEN_HEIGHT/720);
	quit.action = std::bind(&Feedback::back, this);
	super::append(&quit);

	send.position(quit.x + quit.width + 25*SCREEN_HEIGHT/720, quit.y);
	send.action = std::bind(&Feedback::submit, this);
	super::append(&send);

	response.position(860*SCREEN_HEIGHT/720, 20*SCREEN_HEIGHT/720);
	super::append(&response);

	feedback.setSize(23*SCREEN_HEIGHT/720);
	feedback.setWrappedWidth(730*SCREEN_HEIGHT/720);
	feedback.position(390*SCREEN_HEIGHT/720, 140*SCREEN_HEIGHT/720);
	super::append(&feedback);

#if defined(__WIIU__)
	hint.position(50*SCREEN_HEIGHT/720, 120*SCREEN_HEIGHT/720);
	super::append(&hint);
#endif
}

void Feedback::render(Element* parent)
{

	// draw a white background, 870 wiz
	CST_Color white = { 0xff, 0xff, 0xff, 0xff };

	  if (parent != NULL) {
    CST_SetDrawColor(parent->renderer, white);
    this->renderer = parent->renderer;
  }

	return super::render(parent);
}

bool Feedback::process(InputEvents* event)
{
	bool ret = super::process(event);

	if (needsRefresh)
	{
		feedback.setText(keyboard.getTextInput());
		feedback.update();
		needsRefresh = false;
	}

	return ret;
}

void Feedback::keyboardInputCallback()
{
	needsRefresh = true;
}

void Feedback::submit()
{
#ifndef NETWORK_MOCK
	CURL* curl;
	CURLcode res;

#if defined(__WIIU__)
	const char* userKey = "wiiu_user";
#else
	const char* userKey = "switch_user";
#endif

	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://switchbru.com/appstore/feedback");
		std::string fields = std::string("name=") + userKey + "&package=" + package->pkg_name + "&message=" + keyboard.getTextInput();
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields.c_str());

		res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
#endif

	// close this window
	this->back();
}

void Feedback::back()
{
	RootDisplay::switchSubscreen(nullptr);
}
