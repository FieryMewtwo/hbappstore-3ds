#include "AppCard.hpp"
#include "AppList.hpp"
#include "MainDisplay.hpp"

#define TEXT_SIZE	13*SCREEN_HEIGHT/720 //used to be 13/SCALER

CST_Color AppCard::gray = { 80, 80, 80, 0xff };
CST_Color AppCard::black = { 0, 0, 0, 0xff };

AppCard::AppCard(Package* package, AppList* list)
	: package(package)
	, list(list)
	, icon(package->getIconUrl().c_str(), []{
				return new ImageElement(RAMFS "res/default.png");
			},
			!list
		)
	, version(("v. " + package->version).c_str(), TEXT_SIZE, &gray)
	, status(package->statusString(), TEXT_SIZE, &gray)
	, appname(package->title.c_str(), TEXT_SIZE + 3, &black)
	, author(package->author.c_str(), TEXT_SIZE, &gray)
	, statusicon((RAMFS "res/" + std::string(package->statusString()) + ".png").c_str())
{
	// fixed width+height of one app card
	this->width = 256*SCREEN_HEIGHT/720;  // + 9px margins //TODO: magicnum
	this->height = (150+45)*SCREEN_HEIGHT/720; //used to be 45+ICON_SIZE (PLATFORMSPECIFIC). New calculations based on generic/Switch dimensions. 45px is margin for text

	this->touchable = true;

	// connect the action to the callback for this element, to be invoked when the touch event fires
	this->action = std::bind(&AppCard::displaySubscreen, this);

	// create the layout of the app card (all relative)
#if defined(_3DS) || defined(_3DS_MOCK)
	icon.resize(ICON_SIZE, ICON_SIZE);
  this->width = 85;
#else
  icon.resize(256*SCREEN_HEIGHT/720, 150*SCREEN_HEIGHT/720);
#endif
	statusicon.resize(30*SCREEN_HEIGHT/720, 30*SCREEN_HEIGHT/720);

	super::append(&icon);

#if !defined(_3DS) && !defined(_3DS_MOCK)
	super::append(&version);
	super::append(&status);
#endif

	super::append(&appname);
	super::append(&author);
	super::append(&statusicon);
}

void AppCard::update() // update the position of the elements
{
	icon.position(0, 0);
	statusicon.position(4*SCREEN_HEIGHT/720, icon.height+(height-icon.height-statusicon.height)/2);

	appname.position(0, icon.height+(height-icon.height-appname.height-author.height)/2);
	appname.alignRightWith(&icon, 11*SCREEN_HEIGHT/720); //BUG, TODO: for some reason, alignRightWith(this) gives weird behavior, with text sometimes aligning to the wrong card
	version.position(statusicon.x+statusicon.width+6*SCREEN_HEIGHT/720, 0);
	version.alignBottomWith(&appname);

	author.position(0, appname.y+appname.height);
	author.alignRightWith(&icon, 11*SCREEN_HEIGHT/720);
	status.position(statusicon.x+statusicon.width+6*SCREEN_HEIGHT/720, 0);
	status.alignBottomWith(&author);
}

// Trigger the icon download (if the icon wasn't already cached)
// when the icon is near the visible part of the screen
void AppCard::handleIconLoad()
{
	if (iconFetch)
		return;

	int twoCardsHeight = (this->height + 15) * 2;

	if ((list->y + this->height) < -twoCardsHeight)
		return;
	if ((list->y + this->y) > (SCREEN_HEIGHT + twoCardsHeight))
		return;

	// the icon is either visible or ofscreen within 2 rows,
	// so the download can be started
	icon.fetch();

	iconFetch = true;
}

void AppCard::render(Element* parent)
{
	this->xOff = parent->x;
	this->yOff = parent->y;

	// TODO: don't render this card if it's going to be offscreen anyway according to the parent (AppList)
	CST_Rect rect = { this->xOff + this->x, this->yOff + this->y, this->width, this->height };
  if (CST_isRectOffscreen(&rect))
    return;

	// render all the subelements of this card
	super::render(parent);
}

void AppCard::displaySubscreen()
{
	if (!list)
		return;

	// received a click on this app, add a subscreen under the parent
	AppDetails *appDetails = new AppDetails(this->package, list);

	if (!list->touchMode)
		appDetails->highlighted = 0; // show cursor if we're not in touch mode

	MainDisplay::switchSubscreen(appDetails);
}

bool AppCard::process(InputEvents* event)
{
	if (list)
	{
		handleIconLoad();

		this->xOff = this->list->x;
		this->yOff = this->list->y;
	}

	return super::process(event);
}
