#include "AppList.hpp"
#include "AboutScreen.hpp"
#include "Keyboard.hpp"
#include "main.hpp"

#include "../libs/get/src/Utils.hpp"

#include "../libs/chesto/src/RootDisplay.hpp"

#include <algorithm>
#include <cstdlib> // std::rand, std::srand
#include <ctime>   // std::time

#if defined(SWITCH)
#include <switch.h>
#endif

const char* AppList::sortingDescriptions[TOTAL_SORTS] = { "by most recent", "by download count", "alphabetically", "by size (descending)", "randomly" };
CST_Color AppList::black = { 0, 0, 0, 0xff };
CST_Color AppList::gray = { 0x50, 0x50, 0x50, 0xff };

AppList::AppList(Get* get, Sidebar* sidebar)
	: get(get)			// the main get instance that contains repo info and stuff
	, sidebar(sidebar)	// the sidebar, which will store the currently selected category info
	, keyboard(this)
	, quitBtn("Quit", SELECT_BUTTON, false, 15*SCREEN_HEIGHT/720)
	, creditsBtn("Credits", X_BUTTON, false, 15*SCREEN_HEIGHT/720)
	, sortBtn("Adjust Sort", Y_BUTTON, false, 15*SCREEN_HEIGHT/720)
	, keyboardBtn("Toggle Keyboard", Y_BUTTON, false, 15*SCREEN_HEIGHT/720)
#if defined(MUSIC)
	, muteBtn(" ", 0, false, 15*SCREEN_HEIGHT/720, 43*SCREEN_HEIGHT/720) //TODO: validate that this is fine on lores, audio is disabled on desktop
	, muteIcon(RAMFS "res/mute.png")
#endif
{
	setDebugName("AppList");
	this->x = sidebar->x+sidebar->width; //400*SCREEN_HEIGHT/720 - 260*SCREEN_HEIGHT/720 * (itemsPerRow - 3); //TODO: where do the magicnums come from? redefine, possibly based on sidebar

	// the offset of how far along scroll'd we are
	this->y = 0;

	width = SCREEN_WIDTH - x;
	height = SCREEN_HEIGHT; //This is only for loading; this is extended to match the actual height of all the AppCards later.

	//Render white background
	backgroundColor={1,1,1};
	hasBackground=true;
	renderBackground();

	// initialize random numbers used for sorting
	std::srand(unsigned(std::time(0)));

	// quit button
	quitBtn.action = quit;

	// additional buttons
	creditsBtn.action = std::bind(&AppList::launchSettings, this);
	sortBtn.action = std::bind(&AppList::cycleSort, this);
#if defined(MUSIC)
	muteBtn.action = std::bind(&AppList::toggleAudio, this);
	muteIcon.resize(32*SCREEN_HEIGHT/720, 32*SCREEN_HEIGHT/720);
#endif

	// search buttons
	keyboardBtn.action = std::bind(&AppList::toggleKeyboard, this);

	// keyboard input callback
	keyboard.inputCallback = std::bind(&AppList::keyboardInputCallback, this);

	// category text
	category.setSize(28*SCREEN_HEIGHT/720);
	category.setColor(black);

	// sort mode text
	sortBlurb.setSize(15*SCREEN_HEIGHT/720);
	sortBlurb.setColor(gray);

#if defined(SWITCH)
  // disable banner icons if we're in applet mode
  // they use up too much memory, and a lot of people only use applet mode
  AppletType at = appletGetAppletType();
  useBannerIcons = (at == AppletType_Application || at == AppletType_SystemApplication);
#endif

	// update current app listing
	update();
}

bool AppList::process(InputEvents* event)//TODO: check
{
	bool ret = false;

	if (event->pressed(ZL_BUTTON) || event->pressed(L_BUTTON))
	{
		itemsPerRow = (itemsPerRow == 3) ? 4 : 3;
		this->x = sidebar->x+sidebar->width; //400*SCREEN_HEIGHT/720 - 260*SCREEN_HEIGHT/720 * (itemsPerRow - 3); //TODO: stupid magicnums, duplicate of code in line 35
		update();
		return true;
	}

	// must be done before keyboard stuff to properly switch modes
	if (event->isTouchDown())
	{
		// remove a highlight if it exists (TODO: same as an above if statement)
		if (this->highlighted >= 0 && this->highlighted < this->elements.size() && this->elements[this->highlighted])
			this->elements[this->highlighted]->elasticCounter = NO_HIGHLIGHT;

		// got a touch, so let's enter touchmode
		this->highlighted = -1;
		this->touchMode = true;
	}

	// if we're showing a keyboard, make sure we're not in its bounds
	// also make sure the children elements exist before trying the keyboard
	// AND we're actually on the search category
	// also if we're not in touchmode, always go in here regardless of any button presses (user can only interact with keyboard)
	bool keyboardIsShowing = sidebar && sidebar->curCategory == 0 && !keyboard.hidden;
	if (keyboardIsShowing && ((event->isTouchDown() && event->touchIn(keyboard.x, keyboard.y, keyboard.width, keyboard.height)) || !touchMode))
	{
		ret |= keyboard.process(event);
		if (event->isKeyDown() && event->held(Y_BUTTON))
			ret |= ListElement::process(event); // continue processing ONLY if they're pressing Y

    if (needsUpdate) update();
    return ret;
	}

	int origHighlight = this->highlighted;

	// process some joycon input events
	if (event->isKeyDown())
	{
		if (keyboardIsShowing)
		{
			// keyboard is showing, but we're pressing buttons, and we're down here, so set touch mode and get out
			touchMode = false;
			if (event->held(Y_BUTTON)) // again, only let a Y through to toggle keyboard (TODO: redo this!)
				ret |= ListElement::process(event);
			return true; // short circuit, should be handled by someone else
		}

		if (event->held(A_BUTTON | B_BUTTON | UP_BUTTON | DOWN_BUTTON | LEFT_BUTTON | RIGHT_BUTTON))
		{
			// if we were in touch mode, draw the cursor in the applist
			// and reset our position
			if (this->touchMode)
			{
				this->touchMode = false;
				this->highlighted = 0;
				this->y = 0;		 // reset scroll TODO: maintain scroll when switching back from touch mode
				event->keyCode = -1; // we already have the cursor where we want it, no further updates
				ret |= true;
			}

			if (event->held(A_BUTTON) && this->highlighted >= 0)
			{
				this->elements[this->highlighted]->action();
				ret |= true;
			}

			// touchmode is false, but our highlight value is negative
			// (do nothing, let sidebar update our highlight value)
			if (this->highlighted < 0) return false;

			// look up whatever is currently chosen as the highlighted position
			// and remove its highlight
			if (this->elements[this->highlighted])
				this->elements[this->highlighted]->elasticCounter = NO_HIGHLIGHT;

			// if we got a LEFT key while on the left most edge already, transfer to categories
			if (this->highlighted % itemsPerRow == 0 && event->held(LEFT_BUTTON))
			{
				this->highlighted = -1;
				this->sidebar->highlighted = this->sidebar->curCategory;
				return true;
			}

			// similarly, prevent a RIGHT from wrapping to the next line
			if (this->highlighted % itemsPerRow == (itemsPerRow - 1) && event->held(RIGHT_BUTTON)) return false;

			// adjust the cursor by 1 for left or right
			this->highlighted += -1 * (event->held(LEFT_BUTTON)) + (event->held(RIGHT_BUTTON));

			// adjust it by itemsPerRow for up and down
			this->highlighted += -1 * itemsPerRow * (event->held(UP_BUTTON)) + itemsPerRow * (event->held(DOWN_BUTTON));

			// don't let the cursor go out of bounds
			if (this->highlighted >= (int)this->elements.size()) this->highlighted = this->elements.size() - 1;

			if (this->highlighted < 0) this->highlighted = 0;
			if (this->highlighted >= (int)this->totalCount) this->highlighted = this->totalCount - 1;
		}
	}

	// always check the currently highlighted piece and try to give it a thick border or adjust the screen
	if (!touchMode && this->elements.size() > this->highlighted && this->highlighted >= 0 && this->elements[this->highlighted])
	{
		// if our highlighted position is large enough, force scroll the screen so that our cursor stays on screen

		Element* curTile = this->elements[this->highlighted];

		// the y-position of the currently highlighted tile, precisely on them screen (accounting for scroll)
		// this means that if it's < 0 or > 720 then it's not visible
		int normalizedY = curTile->y + this->y;

		// if we're out of range above, recenter at the top row
		if (normalizedY < 0)
			this->y = -1 * (curTile->y - marginBetweenCards) + 25;

		// if we're out of range below, recenter at bottom row
		if (normalizedY > SCREEN_HEIGHT - curTile->height)
			this->y = -1 * (curTile->y - 3 * (curTile->height - marginBetweenCards)) - 40;

		// if the card is this close to the top, just set it the list offset to 0 to scroll up to the top
		if (this->highlighted < itemsPerRow)
			this->y = 0;

		if (this->elements[this->highlighted] && this->elements[this->highlighted]->elasticCounter == NO_HIGHLIGHT)
		{
			this->elements[this->highlighted]->elasticCounter = THICK_HIGHLIGHT;
			ret |= true;
		}
	}

	// highlight was modified, we need to redraw
	if (origHighlight != this->highlighted)
		ret |= true;

	ret |= ListElement::process(event);

	if (needsUpdate)
		update();

	return ret;
}

void AppList::render(Element* parent) //TODO: is this even necessary anymore now that backgrounds are handled by chesto?
{
	if (this->parent == NULL) this->parent = parent;
	if (parent != NULL) this->renderer = parent->renderer;

	super::render(parent);
}

bool AppList::sortCompare(const Package* left, const Package* right)
{
	// handle the supported sorting modes
	switch (sortMode)
	{
		case ALPHABETICAL:
			return left->title.compare(right->title) < 0;
		case POPULARITY:
			return left->downloads > right->downloads;
		case SIZE:
			return left->download_size > right->download_size;
		case RECENT:
			break;
		default:
			break;
	}

	// RECENT sort order is the default view, so it puts updates and installed apps first
	auto statusPriority = [](int status)->int
	{
		switch (status)
		{
			case UPDATE:	return 0;
			case INSTALLED:	return 1;
			case LOCAL:		return 2;
			case GET:		return 3;
		}
		return 4;
	};
	int priorityLeft = statusPriority(left->status);
	int priorityRight = statusPriority(right->status);

	if (priorityLeft == priorityRight)
		return left->updated_timestamp > right->updated_timestamp;

	return priorityLeft < priorityRight;
}

void AppList::update()//TODO: check
{
//Initialization

	if (!get) return; //failsafe

#if defined(_3DS) || defined(_3DS_MOCK) //TODO: redefining these in-place is hacky as all hell, this should be init'd properly for 3DS
  itemsPerRow = 3;  // force 3 app cards at time
  this->x = 45; // no sidebar
#endif
	super::removeAll(); // remove elements
	appCards.clear(); // destroy old elements
	std::string curCategoryValue = sidebar->currentCatValue(); // the current category value from the sidebar

//Instantiate topnav text and buttons
	if (curCategoryValue == "_search") {category.setText(std::string("Search: \"") + sidebar->searchQuery + "\"");} // category text
	else category.setText(sidebar->currentCatName());
	category.update();
	category.position(horizontalMargin, 0); //NOTE: y-position was originally 90px 720p, but that seems like too much whitespace imo.
	category.alignBottomWith(&(sidebar->logo)); //y-position is now aligned with logo for visual consistency
	super::append(&category);

	sortBlurb.position(category.x + category.width + SCREEN_HEIGHT/48, 0); // add the search type next to the category in a gray font
	sortBlurb.setText(sortingDescriptions[sortMode]);
	sortBlurb.update();
	sortBlurb.alignBaselineWith(&category);
	super::append(&sortBlurb);

	quitBtn.position(width-quitBtn.width-horizontalMargin, 0); // Quit button
	quitBtn.alignBottomWith(&category);
#if defined(_3DS) || defined(_3DS_MOCK)
  quitBtn.position(SCREEN_WIDTH - quitBtn.width - 5, 20);
#else
	super::append(&quitBtn);
#endif
	// update the view for the current category
	if (curCategoryValue == "_search")
	{
		// add the keyboard
		keyboardBtn.position(quitBtn.x - 20*SCREEN_HEIGHT/720 - keyboardBtn.width, quitBtn.y);
		super::append(&keyboardBtn);
		keyboard.position(372*SCREEN_HEIGHT/720 + (3 - itemsPerRow) * 132*SCREEN_HEIGHT/720, 417*SCREEN_HEIGHT/720); //TODO: position this sanely without the magicnums
		//Keyboard appending deferred until after app list for layering issues
	}
	else
	{
		// add additional buttons
		creditsBtn.position(quitBtn.x - 20*SCREEN_HEIGHT/720 - creditsBtn.width, quitBtn.y);
		super::append(&creditsBtn);
		sortBtn.position(creditsBtn.x - 20*SCREEN_HEIGHT/720 - sortBtn.width, quitBtn.y);
		super::append(&sortBtn);

	#if defined(MUSIC) //TODO: test these with music enabled
		muteBtn.position(sortBtn.x - 20*SCREEN_HEIGHT/720 - muteBtn.width, quitBtn.y);
		super::append(&muteBtn);
		muteIcon.position(sortBtn.x - 20*SCREEN_HEIGHT/720 - muteBtn.width + 5, quitBtn.y + 5);
		super::append(&muteIcon);
	#endif
	}

//Instantiate package grid
	// all packages TODO: move some of this filtering logic into main get library
	// if it's a search, do a search query through get rather than using all packages
	std::vector<Package*> packages = (curCategoryValue == "_search")
		? get->search(sidebar->searchQuery)
		: get->packages;

	if (sortMode == RANDOM) std::shuffle(packages.begin(), packages.end(), randDevice); // sort the packages
	else std::sort(packages.begin(), packages.end(), std::bind(&AppList::sortCompare, this, std::placeholders::_1, std::placeholders::_2));

	for (auto &package : packages) // add AppCards for the packages belonging to the current category
	{
		if (curCategoryValue == "_misc")
		{
			// if we're on misc, filter out packages belonging to some category
			if (std::find(std::begin(sidebar->cat_value), std::end(sidebar->cat_value), package->category) != std::end(sidebar->cat_value))
				continue;
		}
		else if (curCategoryValue != "_all" && curCategoryValue != "_search")
		{
			// if we're in a specific category, filter out package of different categories
			if (curCategoryValue != package->category)
				continue;
		}

		if (curCategoryValue == "_all")
		{
			// hide themes from all
			if (package->category == "theme")
			continue;
		}

		// create and position the AppCard for the package
		appCards.emplace_back(package, this, (width-(2*horizontalMargin)-(itemsPerRow-1)*marginBetweenCards)/itemsPerRow);
		AppCard& card = appCards.back();
		card.index = appCards.size() - 1;
		//card.position(category.x + (card.index % itemsPerRow) * (card.width + 9 / SCALER), category.y+category.height+30*SCREEN_HEIGHT/720 + (card.height + marginBetweenCards) * (card.index / itemsPerRow)); //TODO magicnums
		card.position(category.x + (card.index % itemsPerRow) * (card.width + marginBetweenCards),
		              category.y+category.height+30*SCREEN_HEIGHT/720 + (card.height + marginBetweenCards) * (card.index / itemsPerRow));
		card.update();
		super::append(&card);
		height = std::max(SCREEN_HEIGHT, card.y+card.height+marginBetweenCards);
	}
	totalCount = appCards.size();

	if (curCategoryValue == "_search") super::append(&keyboard); //Layer keyboard over AppCards

	needsUpdate = false;
}

void AppList::reorient()
{
	// remove a highilight if it exists (TODO: extract method, we use this everywehre)
	if (this->highlighted >= 0 && this->highlighted < this->elements.size() && this->elements[this->highlighted])
		this->elements[this->highlighted]->elasticCounter = NO_HIGHLIGHT;
}

void AppList::keyboardInputCallback()
{
	sidebar->searchQuery = keyboard.getTextInput();
	this->y = 0;
	needsUpdate = true;
}

void AppList::cycleSort()
{
	reorient();
	sortMode = (sortMode + 1) % TOTAL_SORTS;
	update();
}

void AppList::toggleAudio()
{
#if defined(MUSIC)
	if (Mix_PausedMusic())
		Mix_ResumeMusic();
	else
		Mix_PauseMusic();
#endif
}

void AppList::toggleKeyboard()
{
	reorient();
	keyboard.hidden = !keyboard.hidden;

	// if it's hidden now, make sure we release our highlight
	if (keyboard.hidden)
	{
		sidebar->highlighted = -1;
		highlighted = 0;
	}

	needsRedraw = true;
}

void AppList::launchSettings()
{
	RootDisplay::switchSubscreen(new AboutScreen(this->get));
}
