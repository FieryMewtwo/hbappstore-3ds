#include "AppList.hpp"
#include "Sidebar.hpp"

SidebarItem::SidebarItem(int w, int h, const char *imgpath, const char *text, CST_Color *textcolor)
  : icon(imgpath)
	, name(text, (25*SCREEN_HEIGHT)/720, textcolor) //TODO: base this off height instead of SCREEN_HEIGHT
{

  #ifndef RELEASE
	//printf("SidebarItem: h %d, w %d, img %s, text '%s'\n", h, w, imgpath, text);
	//backgroundColor = randomColor(); //red background color
	//hasBackground = true;
	//renderBackground(true);
	#endif

	height = h;
	width = w;

	icon.resize(h/1.4, h/1.4);
	icon.x = w*0.05;
	icon.centerVerticallyIn(this);
	super::append(&icon);

	name.x = icon.x+icon.width+((width-icon.x-icon.width) - name.width)/2;
	name.centerVerticallyIn(this);
	super::append(&name);

	//category[x].icon->position(30, (logo.y+logo.height) + (x * 70) - 5);

	//category[x].name->position(category[x].icon->x+category[x].icon->width+30, category[x].icon->y+(category[x].icon->height - category[x].name->height)/2);
}

SidebarItem::~SidebarItem()
{
	super::removeAll();
}

Sidebar::Sidebar()
	: logo(RAMFS "res/icon.png")
	, title("Homebrew App Store", (22 * SCREEN_HEIGHT)/720)
	, subtitle("GPLv3 License", (18 * SCREEN_HEIGHT)/720)
{
	// a sidebar consists of:
	//		the header: an image (ImageElement) and a logo (TextElement)
	//    a Container of clickable SidebarItems

	// it also can process input if the cursor goes over it, or a touch

	// there's no back color to the sidebar, as the background is already the right color

	// LHS of sidebar is the "minimized" version; width = (SCREEN_HEIGHT/18)+60 (30px on either side of icon)

	width = (365*SCREEN_HEIGHT)/720; //MAGICNUM: copied from 1280x720 manual positioning moosey did
	height = SCREEN_HEIGHT;

	// Instantiate and position logo
	logo.resize(SCREEN_HEIGHT/12, SCREEN_HEIGHT/12); //40px 720p
	logo.position(30, 50); //i'm fine with these magicnum borders -AC
	#ifndef RELEASE //Visually mark dev builds to avoid confusion with official releases
	logo.angle = 180;
	#endif
	super::append(&logo);

	// Instantiate and position title and subtitle
	title.position(logo.x+logo.width+30, logo.y+(logo.height/2)-(title.height+subtitle.height+5)/2);
	super::append(&title);

	#ifndef RELEASE //Visually mark dev builds to avoid confusion with official releases
	subtitle.setText("DEV BUILD");
	subtitle.update();
	#endif
	subtitle.position(title.x, title.y+title.height+5);
	super::append(&subtitle);

	categoryHolder = new Container(COL_LAYOUT, 0);
	categoryHolder->position(0, subtitle.y+subtitle.height+5);
	categoryHolder->width = width;
	//categoryHolder->height = SCREEN_HEIGHT - categoryHolder->y;
	super::append(categoryHolder);

	// for every entry in cat names, create a text element
	// elements 0 through TOTAL_CATS are the sidebar texts (for highlighting)
	for (int x = 0; x < TOTAL_CATS; x++)
	{
		category[x] = new SidebarItem(width, ((SCREEN_HEIGHT - categoryHolder->y)/TOTAL_CATS), (std::string(RAMFS "res/") + cat_value[x] + ".png").c_str(), cat_names[x]);
		categoryHolder->add(category[x]);
	}

	// elasticCounter in this class is used to keep track of which element is being pressed down on in touch mode
	// TODO: elasticCounter belongs to element and should really be renamed (it's for general purpose animations)
	elasticCounter = -1;
}

Sidebar::~Sidebar()
{
	super::removeAll();
	for (int x = 0; x < TOTAL_CATS; x++)
	{
		delete category[x];
		//delete category[x].icon;
		//delete category[x].name;
	}
	if (categoryHolder) delete categoryHolder;
	if (hider) delete hider;
	if (hint) delete hint;
}

void Sidebar::addHints()
{
	// small indicator to switch to advanced view using L
	hider = new ImageElement(RAMFS "res/button-l-outline.png");
	hider->resize(20, 20);  //TODO: magic placement and size
	hider->position(270, 685);
	super::append(hider);

	hint = new TextElement("Hide", 15);
	hint->position(hider->x + hider->width + 5, hider->y);
	super::append(hint);

	showCurrentCategory = true;
}

bool Sidebar::process(InputEvents* event) //Currently disabled; TODO refactor all teh things
{
	//return false; //HACK: disable process() to do button testing
	bool ret = false;
	int origHighlighted = highlighted;

	// process some joycon input events
	if (event->isKeyDown())
	{
		// if highlighted isn't set, don't do anything (applist will set it)
		if (this->highlighted < 0)
			return false;

		// if we got a RIGHT key, send it back to the applist
		if (event->held(RIGHT_BUTTON))
		{
			this->highlighted = -1;
			this->appList->highlighted = 0;
			this->appList->y = 0; // reset scroll TODO: maintain scroll when switching between sidebar and app list
			event->update();
			return true;
		}

		// adjust the cursor by 1 for up or down
		this->highlighted += -1 * (event->held(UP_BUTTON)) + (event->held(DOWN_BUTTON));

		// don't let the cursor go out of bounds
		if (this->highlighted < 0) this->highlighted = 0;
		if (this->highlighted >= TOTAL_CATS) this->highlighted = TOTAL_CATS - 1;
	}

#if !defined(_3DS) && !defined(_3DS_MOCK)
	// saw click down, set dragging state
	if (event->isTouchDown())
	{
		this->dragging = true;
		this->highlighted = -1;

		// go through the categories and see if this touch down was in one of them, to show it highlighted
		for (int i = 0; i < TOTAL_CATS; i++) if (event->touchIn(category[i]->xAbs, category[i]->yAbs, category[i]->width, category[i]->height))
		{
			// touch is over an element of the sidebar, set the elasticCounter
			elasticCounter = i;
			break;
		}

		return true;
	}
#endif

	// detect if a click is on one of the sidebar elements
	// (or we saw the A button be pressed)
	if ((event->isTouchUp() && this->dragging) || (event->isKeyDown() && event->held(A_BUTTON)))
	{
		this->dragging = false;
		int previouslySelected = elasticCounter;
		elasticCounter = -1; // reset highlighted one

		// check if it's one of the text elements
		for (int i = 0; i < TOTAL_CATS; i++)
		{
			//int xc = 0, yc = 150 + x * 70 - 15, width = 400 - 260 * (appList->itemsPerRow - 3) - 35, height = 60; // TODO: extract formula into method (same as AppList x value)
			if ((event->touchIn(category[i]->xAbs, category[i]->yAbs, category[i]->width, category[i]->height) && event->isTouchUp()) || (event->held(A_BUTTON) && this->highlighted == i))
			{
				// if it's a touch up, let's make sure this is the same one we touched down on
				if (event->isTouchUp() && previouslySelected >= 0 && i != previouslySelected)
					return true;

				// saw touchup on a category, adjust active category
				this->curCategory = i;

				// since we updated the active category, we need to update the app listing
				if (this->appList != NULL)
				{
					this->appList->y = 0;
					this->appList->update();
				}
			}
		}

		return true;
	}

	if (origHighlighted != highlighted)
		ret |= true;

	return ret;
}

void Sidebar::render(Element* parent) //TODO: magicnums and direct draw functions lie here
{
#if defined(_3DS) || defined(_3DS_MOCK) //TODO: get rid of this BS and just don't instantiate a Sidebar on 3DS
  // no sidebar on 3ds
  return;
#endif
	// draw the light gray bg behind the active category
	CST_Rect dimens = { 0, 0, 400*SCREEN_HEIGHT/720 - 260*SCREEN_HEIGHT/720 * (appList->itemsPerRow - 3) - 35, 60 }; // TODO: magicnums //TODO: extract this to a method too
	dimens.y = 150 + this->curCategory * 70 - 15;					   // TODO: extract formula into method

#if defined(__WIIU__) //TODO, PLATFORMSPECIFIC: move these into an external platform definition
	CST_Color consoleColor = { 0x3b, 0x3c, 0x4e, 0xFF };
#elif defined(_3DS)
	CST_Color consoleColor = { 0xe4, 0x00, 0x0f, 0xFF };
#else
	CST_Color consoleColor = { 0x67, 0x6a, 0x6d, 0xFF };
#endif

	CST_SetDrawColor(parent->renderer, consoleColor);

	if (this->showCurrentCategory)
		CST_FillRect(parent->renderer, &dimens);

	if (appList && appList->touchMode && this->elasticCounter >= 0)
	{
		CST_Rect dimens2 = { 0, 0, 400, 60 };
		dimens2.y = 150 + this->elasticCounter * 70 - 15; // TODO: extract formula into method
		CST_SetDrawBlend(parent->renderer, true);
		CST_Color highlight = { 0xad, 0xd8, 0xe6, 0x90 };
		CST_SetDrawColor(parent->renderer, highlight); // TODO: matches the DEEP_HIGHLIGHT color
		CST_FillRect(parent->renderer, &dimens2);
	}

	// draw the selected category, if one should be highlighted
	if (this->highlighted >= 0)
	{
		int y = 150 + this->highlighted * 70 - 15;
		//        rectangleRGBA(parent->renderer, 0, y, dimens.w, y + dimens.h, 0xff, 0x00, 0xff, 0xff);

		for (int x = 0; x < 5; x++)
		{
			rectangleRGBA(parent->renderer, dimens.x + x, y + x, dimens.x + dimens.w - x, y + dimens.h - x, 0x66 - x * 10, 0x7c + x * 20, 0x89 + x * 10, 0xFF);
		}
	}

	// render subelements
	super::render(parent);
}

std::string Sidebar::currentCatName()
{
	if (this->curCategory >= 0 && this->curCategory < TOTAL_CATS)
		return std::string(this->cat_names[this->curCategory]);

	return std::string("?");
}

std::string Sidebar::currentCatValue()
{
	if (this->curCategory >= 0 && this->curCategory < TOTAL_CATS)
		return std::string(this->cat_value[this->curCategory]);

	return std::string("?");
}
