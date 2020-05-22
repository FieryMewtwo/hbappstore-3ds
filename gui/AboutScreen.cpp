#include <sstream>

#include "../libs/get/src/Get.hpp"
#include "../libs/get/src/Utils.hpp"

#include "../libs/chesto/src/Button.hpp"
#include "../libs/chesto/src/RootDisplay.hpp"
#include "../libs/chesto/src/NetImageElement.hpp"


#include "AboutScreen.hpp"
#include "Feedback.hpp"

#define AVATAR_URL "https://avatars.githubusercontent.com/u/"


CST_Color AboutScreen::black = { 0x00, 0x00, 0x00, 0xff };
CST_Color AboutScreen::gray = { 0x50, 0x50, 0x50, 0xff };

AboutScreen::AboutScreen(Get* get)
	: get(get)
	, cancel("Go Back", B_BUTTON, false, (29*SCREEN_WIDTH/1280))
	, feedback("Leave Feedback", A_BUTTON, false, 17) //TODO: implement constant-height buttons in chesto
	, title("Homebrew App Store", (35*SCREEN_HEIGHT/720), &black)
	, subtitle("by fortheusers.org", (25*SCREEN_HEIGHT/720), &black)
	, ftuLogo(AVATAR_URL "40721862", []{
	        return new ImageElement(RAMFS "res/4TU.png");
        })
	, creds("Licensed under the GPLv3 license. This app is free and open source because the users (like you!) deserve it.\nLet's support homebrew and the right to control what software we run on our own devices!",
			(20*SCREEN_HEIGHT/720), &black, false, (SCREEN_WIDTH-40))
{

	// TODO: show current app status somewhere

	// download/update/remove button (2)

	//TextElement* debugvals = new TextElement(("WH"+std::to_string(SCREEN_WIDTH)+","+std::to_string(SCREEN_HEIGHT)).c_str(), 20);
	//debugvals->position(20,20);
	//super::append(debugvals);

	cancel.position(30, 30);
	cancel.action = std::bind(&AboutScreen::back, this);
	super::append(&cancel);

	feedback.position(SCREEN_WIDTH - feedback.width - 30, 30);
	feedback.action = std::bind(&AboutScreen::launchFeedback, this);
	super::append(&feedback);

	if(aspectMode() == WIDE) //left-justify logo with text beside it
	{
		ftuLogo.resize(SCREEN_HEIGHT/5, SCREEN_HEIGHT/5);
		int logoframe_width = ftuLogo.width + 35 + title.width;
		ftuLogo.position(SCREEN_WIDTH/2 - logoframe_width/2, 15);
		int logooffset = (ftuLogo.height * 25)/140; //approx. height of whitespace above/below 4TU lettering
		title.position(SCREEN_WIDTH/2 - logoframe_width/2 + ftuLogo.width + 35, ftuLogo.y + logooffset);
		subtitle.position(SCREEN_WIDTH/2 - logoframe_width/2 + ftuLogo.width + 35, (ftuLogo.y+ftuLogo.height) - (logooffset+subtitle.height));
	}
	else //center logo with text under it
	{
		ftuLogo.resize(SCREEN_HEIGHT/5, SCREEN_HEIGHT/5);
		ftuLogo.position(SCREEN_WIDTH/2 - ftuLogo.width/2, 15);
		title.position(SCREEN_WIDTH/2 - title.width/2, ftuLogo.y + ftuLogo.height*0.75); //hack: there's whitespace in the 4TU logo, set the text inside it
		subtitle.position(SCREEN_WIDTH/2 - subtitle.width/2, title.y + title.height + 5);
	}

	super::append(&ftuLogo);
	super::append(&title);
	super::append(&subtitle);

	creds.position(SCREEN_HEIGHT/7, std::max(ftuLogo.y+ftuLogo.height, subtitle.y+subtitle.height)+5); //approx 100px on 720p
	super::append(&creds);

	// argument order:
	// username, githubId, twitter, github, gitlab, patreon, url, discord, directAvatarURL
	// only first two social points will be used

	credHead("Repo Maintainance and Development", "These are the primary people responsible for actively maintaining and developing the Homebrew App Store. If there's a problem, these are the ones to get in touch with!");
	credit("pwsincd", "20027105", NULL, "pwsincd", NULL, NULL, NULL, "pwsincd#9044");
	credit("VGMoose", "2467473", "vgmoose", "vgmoose");
	credit("rw-r-r_0644", "18355947", "rw_r_r_0644", "rw-r-r-0644");
	credit("crc32", "7893269", "crc32_", "crc-32");
	credit("CompuCat", "12215288", NULL, NULL, "compucat", NULL, "compucat.me");
	credit("Quarky", "8533313", NULL, NULL, "quarktheawesome", NULL, "heyquark.com");
	credit("Whovian9369", "5240754", NULL, NULL, "whovian9369");

	credHead("Library Development and Support", "Without the contributions to open-source libraries and projects by these people, much of the functionality within this program wouldn't be possible.");
	credit("Maschell", "8582508", "maschelldev", "maschell");
	credit("brienj", "17801294", "xhp_creations", "xhp-creations");
	credit("Dimok", "15055714", NULL, "dimok789");
	credit("FIX94", "12349638", NULL, "FIX94", NULL, NULL, NULL, "FIX94#3446");
	credit("Zarklord", "1622280", "zarklore", "zarklord");
	credit("CreeperMario", "15356475", "CreeperMario258", "CreeperMario");
	credit("Ep8Script", "27195853", "ep8script", "ep8script");

	credHead("Interface Development and Design", "In one way or another, everyone in this category provided information regarding core functionality, quality-of-life changes, or the design of the user interface.");
	credit("exelix", "13405476", "exelix11", "exelix11");
	credit("Xortroll", "33005497", NULL, "xortroll", NULL, "xortroll");
	credit("Ave", "584369", NULL, NULL, "a", NULL, "ave.zone", NULL, "https://gitlab.com/uploads/-/system/user/avatar/584369/avatar.png");
	credit("LyfeOnEdge", "26140376", NULL, "lyfeonedge", NULL, NULL, NULL, "Lyfe#1555");
	credit("Román", "57878194", NULL, NULL, NULL, NULL, NULL, "Román#6630");
	credit("Jaames", "9112876", "rakujira", "jaames");
	credit("Jacob", "12831497", NULL, "jacquesCedric");
	credit("iTotalJustice", "47043333", NULL, "iTotalJustice");

	credHead("Toolchain and Environment", "The organizations and people in this category enable Homebrew in general by creating and maintaining a cohesive environment for the community.");
	credit("devkitPro", "7538897", NULL, "devkitPro", NULL, "devkitPro");
	credit("Wintermute", "101194", NULL, "wintermute", NULL, NULL, "devkitPro.org");
	credit("Fincs", "581494", "fincsdev", "fincs");
	credit("yellows8", "585494", "yellows8");
	credit("ReSwitched", "26338222", NULL, "reswitched", NULL, NULL, "reswitched.team");
	credit("exjam", "1302758", NULL, "exjam");
  credit("brett19", "1621627", NULL, "brett19");

	credHead("Homebrew Community Special Thanks", "Awesome people within the community whose work, words, or actions in some way inspired this program to exist in the manner it does.");
	credit("misson20000", "616626", NULL, "misson20000", NULL, NULL, NULL, "misson20000#0752");
	credit("roblabla", "1069318", NULL, "roblabla", NULL, NULL, NULL, "roblabla#8145");
	credit("tomGER", "25822956", "tumGER", "tumGER");
	credit("m4xw", "13141469", "m4xwdev", "m4xw");
	credit("Nikki", "3280345", "NWPlayer123", "NWPlayer123");
	credit("shchmue", "7903403", NULL, "shchmue");
	credit("CTCaer", "3665130", "CTCaer", "CTCaer");
	credit("SciresM", "8676005", "SciresM", "SciresM");
	credit("Shinyquagsire", "1224096", "shinyquagsire", "shinyquagsire23");
	credit("Marionumber1", "775431", "MrMarionumber1");
}

AboutScreen::~AboutScreen()
{
	super::removeAll();
	for (auto& i : creditHeads)
	{
		delete i.text;
		delete i.desc;
	}
	for (auto& i : credits)
	{
		delete i.userLogo;
		delete i.name;
		delete i.social[0].icon;
		delete i.social[0].link;
		delete i.social[1].icon;
		delete i.social[1].link;
	}
}

void AboutScreen::credHead(const char* header, const char* blurb)
{
	if(lastAvi == NULL) lastAvi = elements.back(); //lastAvi is being used as a cheaty way to get the y-position/height of lowest item drawn
	auto head = creditHeads.emplace(creditHeads.end());

	creditCount += (4 - creditCount%4) % 4;
	head->text = new TextElement(header, (30*SCREEN_HEIGHT)/720, &black);
	head->text->position(40, lastAvi->y+lastAvi->height+10);
	super::append(head->text);

	head->desc = new TextElement(blurb, (23*SCREEN_HEIGHT)/720, &gray, false, (SCREEN_WIDTH*15)/16);
	head->desc->position(40, head->text->y+head->text->height+10);
	super::append(head->desc);

	creditCount += creditsPerLine();
}

void AboutScreen::credit(const char* username,
												const char* githubId,
												const char* twitter,
												const char* github,
												const char* gitlab,
												const char* patreon,
												const char* url,
												const char* discord,
												const char* directAvatarUrl)
{
	int X = 40; //credits XY master offset
	int Y = 310;

	int myX = creditCount % creditsPerLine() * 300 + X;
	myY = ((creditCount % creditsPerLine()) == 0) ? elements.back()->y + elements.back()->height+10 : myY; //TODO: rename this to something sensible
	//TODO: This causes issues if last credit on a line has only one social. Will fix as part of converting social cards to their own Element classes.

	auto cred = credits.emplace(credits.end());

	auto avatar = directAvatarUrl ? directAvatarUrl : (std::string(AVATAR_URL) + githubId + "?s=100").c_str();
  #if !defined(__WIIU__) and defined(NETWORK)
	  cred->userLogo = new NetImageElement(directAvatarUrl != NULL ? directAvatarUrl : ((std::string(AVATAR_URL) + githubId + "?s=100").c_str()));
  #else
    cred->userLogo = new ImageElement((std::string(RAMFS "res/pfp_cache/") + githubId).c_str());
  #endif
  cred->userLogo->position(myX, myY);
	cred->userLogo->resize(SCREEN_HEIGHT/7, SCREEN_HEIGHT/7); //approx 100px on 720p
	lastAvi = cred->userLogo;
	super::append(cred->userLogo);

	cred->name = new TextElement(username, (SCREEN_HEIGHT/20)*0.75, &black); //approx 36 px on 720p
	cred->name->position(myX + cred->userLogo->width + 10, myY); //10 pixel margin between pfp and text
	super::append(cred->name);

	int socialCount = 0;

	const char * handles[6] = { twitter, github, gitlab, patreon, url, discord };
	const char * icons[6] = { "twitter", "github", "gitlab", "patreon", "url", "discord" };

	for (int x=0; (x<6) && (socialCount<2); x++) {
		if (handles[x] == NULL) continue;

		cred->social[socialCount].icon = new ImageElement(((std::string(RAMFS "res/") + icons[x]) + ".png").c_str());
		cred->social[socialCount].icon->resize(cred->userLogo->width/5, cred->userLogo->width/5); //approx 20px 720p
		cred->social[socialCount].icon->position(myX + cred->userLogo->width + 10, myY + cred->userLogo->height/2 + socialCount*(cred->social[socialCount].icon->height*1.125));
		super::append(cred->social[socialCount].icon);

		cred->social[socialCount].link = new TextElement(handles[x], cred->social[socialCount].icon->width*0.75, &gray); //0.75 px -> pt conversion, bleugh. TODO: add px text support to chesto
		cred->social[socialCount].link->position(myX + cred->userLogo->width + cred->social[socialCount].icon->width + 20, myY + cred->userLogo->height/2 + socialCount*(cred->social[socialCount].icon->height*1.125));
		super::append(cred->social[socialCount].link);

		socialCount++;
	}

	creditCount++;
}

void AboutScreen::render(Element* parent)
{
	if (this->parent == NULL)
		this->parent = parent;

	// draw a white background
	CST_Rect dimens = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

	CST_Color white = { 0xff, 0xff, 0xff, 0xff };
	CST_SetDrawColor(parent->renderer, white);
	CST_FillRect(parent->renderer, &dimens);
	this->renderer = parent->renderer;

	super::render(parent);
}

bool AboutScreen::process(InputEvents* event)
{
  bool ret = false;
  ret |= ListElement::processUpDown(event);
  return ret || ListElement::process(event);
}

void AboutScreen::back()
{
	RootDisplay::switchSubscreen(nullptr);
}

void AboutScreen::launchFeedback()
{
	// find the package corresponding to us
	for (auto& package : this->get->packages)
	{
		if (package->pkg_name == "appstore")
		{
			RootDisplay::switchSubscreen(new Feedback(package));
			break;
		}
	}
}
