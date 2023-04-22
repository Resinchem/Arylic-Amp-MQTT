#include <display-panel.h>

// The current page number.
int pageNumber = 0;
// Max page number (zero based)
int maxPageNumber = 3;

// Last touched page
DisplayPanel* lastTouchedPanel = NULL;

// The display/lcd we are working with. Defined in initializePanels().
esphome::display::DisplayBuffer* lcd = NULL;

// For sprintf calls.
char buffer[25];

#define WIDTH 320
#define HEIGHT 240
// Convert percentage width or height (0-100) to pixels
// for easier panel layout.
#define PW(PCT_WIDTH) (PCT_WIDTH * 0.01 * WIDTH)
#define PH(PCT_HEIGHT) (PCT_HEIGHT * 0.01 * HEIGHT)

// The Panels used by this app. Params are (X, Y, W, H)
//Page 0 - system settings
DisplayPanel panelStandby(PW(5), PH(4), PW(65), PH(28));
DisplayPanel panelBootTFT(PW(5), PH(36), PW(65), PH(28));
DisplayPanel panelBootAmp(PW(5), PH(68), PW(65), PH(28));

// Create 3 x 3 grid of equal panels for each favorites page
//Page 1
DisplayPanel page1_01(0 * PW(25), 0 * PH(33), PW(25), PH(33));
DisplayPanel page1_02(1 * PW(25), 0 * PH(33), PW(25), PH(33));
DisplayPanel page1_03(2 * PW(25), 0 * PH(33), PW(25), PH(33));

DisplayPanel page1_04(0 * PW(25), 1 * PH(33), PW(25), PH(33));
DisplayPanel page1_05(1 * PW(25), 1 * PH(33), PW(25), PH(33));
DisplayPanel page1_06(2 * PW(25), 1 * PH(33), PW(25), PH(33));

DisplayPanel page1_07(0 * PW(25), 2 * PH(33), PW(25), PH(33));
DisplayPanel page1_08(1 * PW(25), 2 * PH(33), PW(25), PH(33));
DisplayPanel page1_09(2 * PW(25), 2 * PH(33), PW(25), PH(33));

//Page 2
DisplayPanel page2_01(0 * PW(25), 0 * PH(33), PW(25), PH(33));
DisplayPanel page2_02(1 * PW(25), 0 * PH(33), PW(25), PH(33));
DisplayPanel page2_03(2 * PW(25), 0 * PH(33), PW(25), PH(33));

DisplayPanel page2_04(0 * PW(25), 1 * PH(33), PW(25), PH(33));
DisplayPanel page2_05(1 * PW(25), 1 * PH(33), PW(25), PH(33));
DisplayPanel page2_06(2 * PW(25), 1 * PH(33), PW(25), PH(33));

DisplayPanel page2_07(0 * PW(25), 2 * PH(33), PW(25), PH(33));
DisplayPanel page2_08(1 * PW(25), 2 * PH(33), PW(25), PH(33));
DisplayPanel page2_09(2 * PW(25), 2 * PH(33), PW(25), PH(33));

//Page 3
DisplayPanel page3_01(0 * PW(25), 0 * PH(33), PW(25), PH(33));
DisplayPanel page3_02(1 * PW(25), 0 * PH(33), PW(25), PH(33));
DisplayPanel page3_03(2 * PW(25), 0 * PH(33), PW(25), PH(33));

DisplayPanel page3_04(0 * PW(25), 1 * PH(33), PW(25), PH(33));
DisplayPanel page3_05(1 * PW(25), 1 * PH(33), PW(25), PH(33));
DisplayPanel page3_06(2 * PW(25), 1 * PH(33), PW(25), PH(33));

DisplayPanel page3_07(0 * PW(25), 2 * PH(33), PW(25), PH(33));
DisplayPanel page3_08(1 * PW(25), 2 * PH(33), PW(25), PH(33));
DisplayPanel page3_09(2 * PW(25), 2 * PH(33), PW(25), PH(33));


// Menu buttons (right most column)
DisplayPanel panelHome(3 * PW(25), 0 * PH(33), PW(25), PH(33));
DisplayPanel panelUp(3 * PW(25), 1 * PH(33), PW(25), PH(33));
DisplayPanel panelDown(3 * PW(25), 2 * PH(33), PW(25), PH(33));

DisplayPanel flashPanel(PW(5), PH(20), PW(90), PH(50));

//Setup pages with panels

std::vector<std::vector<DisplayPanel*>> pages = {
    {  //Page 0 - System
        &panelStandby,
        &panelBootTFT,
        &panelBootAmp,
        &panelHome,
        &panelUp,
        &panelDown
    },
    {
        // Page 1.
        &page1_01,
        &page1_02,
        &page1_03,
        &page1_04,
        &page1_05,
        &page1_06,
        &page1_07,
        &page1_08,
        &page1_09,
        &panelHome,
        &panelUp,
        &panelDown
    },
    {
        // Page 2.
        &page2_01,
        &page2_02,
        &page2_03,
        &page2_04,
        &page2_05,
        &page2_06,
        &page2_07,
        &page2_08,
        &page2_09,
        &panelHome,
        &panelUp,
        &panelDown
    },
    {
        // Page 3.
        &page3_01,
        &page3_02,
        &page3_03,
        &page3_04,
        &page3_05,
        &page3_06,
        &page3_07,
        &page3_08,
        &page3_09,
        &panelHome,
        &panelUp,
        &panelDown
    }
};
// icons for menu buttons
std::vector<std::string> panelHomeText = { "\U000F02DC" };
std::vector<std::string> panelUpText = { "\U000F0062" };
std::vector<std::string> panelDownText = { "\U000F004A" };
std::vector<std::string> blankText = {};

// One time, initialize the Panels
void initializePanels(esphome::display::DisplayBuffer &display) {
    lcd = &display;
    //Page 0 - System
    panelStandby.font = font_button_label;
    panelStandby.color = color_green;
    panelStandby.textColor = Color::BLACK;
    panelStandby.touchable = true;
    panelStandby.text = {"STANDBY"};
    panelStandby.name = "Standby";
    panelStandby.tag = "script.sirius_power_down_amp";
    
    panelBootTFT.font = font_button_label;
    panelBootTFT.color = color_blue;
    panelBootTFT.textColor = Color::WHITE;
    panelBootTFT.touchable = true;
    panelBootTFT.text = {"Reboot TFT"};
    panelBootTFT.name = "BootTFT";
    panelBootTFT.tag = "script.sirius_reboot_tft";
    
    panelBootAmp.font = font_button_label;
    panelBootAmp.color = color_red;
    panelBootAmp.textColor = Color::WHITE;
    panelBootAmp.touchable = true;
    panelBootAmp.text = {"Reboot Amp"};
    panelBootAmp.name = "BootAmp";
    panelBootAmp.tag = "script.sirius_reboot_amp";
    
    //Page 1
    page1_01.font = font_button_label;
    page1_01.color = color_red;
    page1_01.textColor = Color::WHITE;
    page1_01.touchable = true;
    page1_01.text = {"Clssc", "Rwnd"};
    page1_01.name = "Classic Rewind";
    page1_01.tag = "script.sirius_classicrewind";
    
    page1_02.font = font_button_label;
    page1_02.color = color_green;
    page1_02.textColor = Color::BLACK;
    page1_02.touchable = true;
    page1_02.text = {"Clssc", "Rock"};
    page1_02.name = "Classic Rock";
    page1_02.tag = "script.sirius_classicrock";
 
    page1_03.font = font_button_label;
    page1_03.color = color_blue;
    page1_03.textColor = Color::WHITE;
    page1_03.touchable = true;
    page1_03.text = {"Clssc", "Vinyl"};
    page1_03.name = "Classic Vinyl";
    page1_03.tag = "script.sirius_classicvinyl";

    page1_04.font = font_button_label;
    page1_04.color = color_cyan;
    page1_04.textColor = Color::BLACK;
    page1_04.touchable = true;
    page1_04.text = {"70s", "on", "7"};
    page1_04.name = "70s on 7";
    page1_04.tag = "script.sirius_70son7";

    page1_05.font = font_button_label;
    page1_05.color = color_magenta;
    page1_05.textColor = Color::WHITE;
    page1_05.touchable = true;
    page1_05.text = {"80s", "on", "8"};
    page1_05.name = "80s on 8";
    page1_05.tag = "script.sirius_80son8";

    page1_06.font = font_button_label;
    page1_06.color = color_red;
    page1_06.textColor = Color::WHITE;
    page1_06.touchable = true;
    page1_06.text = {"70/80s", "Pop"};
    page1_06.name = "70s/80s Pop";
    page1_06.tag = "script.sirius_7080pop";

    page1_07.font = font_button_label;
    page1_07.color = color_red;
    page1_07.textColor = Color::WHITE;
    page1_07.touchable = true;
    page1_07.text = {"Red", "White", "Booze"};
    page1_07.name = "Red White Booze";
    page1_07.tag = "script.sirius_redwhitebooze";

    page1_08.font = font_button_label;
    page1_08.color = color_green;
    page1_08.textColor = Color::BLACK;
    page1_08.touchable = true;
    page1_08.text = {"Road", "House"};
    page1_08.name = "Willies Roadhouse";
    page1_08.tag = "script.sirius_roadhouse";

    page1_09.font = font_button_label;
    page1_09.color = color_blue;
    page1_09.textColor = Color::WHITE;
    page1_09.touchable = true;
    page1_09.text = {"Road", "Trip"};
    page1_09.name = "Rdtrip Radio";
    page1_09.tag = "script.sirius_roadtrip";

    //Page 2
    page2_01.font = font_button_label;
    page2_01.color = color_green;
    page2_01.textColor = Color::BLACK;
    page2_01.touchable = true;
    page2_01.text = {"Hair", "Nation"};
    page2_01.name = "Hair Nation";
    page2_01.tag = "script.sirius_hairnation";
    
    page2_02.font = font_button_label;
    page2_02.color = color_blue;
    page2_02.textColor = Color::WHITE;
    page2_02.touchable = true;
    page2_02.text = {"Bone", "Yard"};
    page2_02.name = "Ozzys Boneyard";
    page2_02.tag = "script.sirius_boneyard";

    page2_03.font = font_button_label;
    page2_03.color = color_red;
    page2_03.textColor = Color::WHITE;
    page2_03.touchable = true;
    page2_03.text = {"Oldies", "Party"};
    page2_03.name = "Oldies Party";
    page2_03.tag = "script.sirius_oldiesparty";

    page2_04.font = font_button_label;
    page2_04.color = color_magenta;
    page2_04.textColor = Color::WHITE;
    page2_04.touchable = true;
    page2_04.text = {"Outlaw"};
    page2_04.name = "Outlaw Cntry";
    page2_04.tag = "script.sirius_outlaw";

    page2_05.font = font_button_label;
    page2_05.color = color_red;
    page2_05.textColor = Color::WHITE;
    page2_05.touchable = true;
    page2_05.text = {"Prime", "Cntry"};
    page2_05.name = "Prime Country";
    page2_05.tag = "script.sirius_primecountry";

    page2_06.font = font_button_label;
    page2_06.color = color_cyan;
    page2_06.textColor = Color::BLACK;
    page2_06.touchable = true;
    page2_06.text = {"The", "Highwy"};
    page2_06.name = "The Highway";
    page2_06.tag = "script.sirius_highway";

    page2_07.font = font_button_label;
    page2_07.color = color_green;
    page2_07.textColor = Color::BLACK;
    page2_07.touchable = true;
    page2_07.text = {"Girl", "Cntry"};
    page2_07.name = "Girl Country";
    page2_07.tag = "script.sirius_womencountry";

    page2_08.font = font_button_label;
    page2_08.color = color_blue;
    page2_08.textColor = Color::WHITE;
    page2_08.touchable = true;
    page2_08.text = {"Elvis", "Radio"};
    page2_08.name = "Elvis Radio";
    page2_08.tag = "script.sirius_elvis";

    page2_09.font = font_button_label;
    page2_09.color = color_red;
    page2_09.textColor = Color::WHITE;
    page2_09.touchable = true;
    page2_09.text = {"Blue", "Grass"};
    page2_09.name = "Bluegrass Jnct";
    page2_09.tag = "script.sirius_bluegrass";

    //Page 3
    page3_01.font = font_button_label;
    page3_01.color = color_red;
    page3_01.textColor = Color::WHITE;
    page3_01.touchable = true;
    page3_01.text = {"Mrgta", "ville"};
    page3_01.name = "Margarita";
    page3_01.tag = "script.sirius_margaritaville";
    
    page3_02.font = font_button_label;
    page3_02.color = color_green;
    page3_02.textColor = Color::BLACK;
    page3_02.touchable = true;
    page3_02.text = {"Sintra"};
    page3_02.name = "Sinatra";
    page3_02.tag = "script.sirius_sinatra";

    page3_03.font = font_button_label;
    page3_03.color = color_blue;
    page3_03.textColor = Color::WHITE;
    page3_03.touchable = true;
    page3_03.text = {"Studio", "54"};
    page3_03.name = "Studio 54";
    page3_03.tag = "script.sirius_studio54";

    page3_04.font = font_button_label;
    page3_04.color = color_cyan;
    page3_04.textColor = Color::BLACK;
    page3_04.touchable = true;
    page3_04.text = {"50s", "on", "5"};
    page3_04.name = "50s on 5";
    page3_04.tag = "script.sirius_50son5";

    page3_05.font = font_button_label;
    page3_05.color = color_magenta;
    page3_05.textColor = Color::WHITE;
    page3_05.touchable = true;
    page3_05.text = {"60s", "on", "6"};
    page3_05.name = "60s on 6";
    page3_05.tag = "script.sirius_60son6";

    page3_06.font = font_button_label;
    page3_06.color = color_red;
    page3_06.textColor = Color::WHITE;
    page3_06.touchable = true;
    page3_06.text = {"Road", "Dog"};
    page3_06.name = "Road Dog Trkn";
    page3_06.tag = "script.sirius_roaddog";

    page3_07.font = font_button_label;
    page3_07.color = color_red;
    page3_07.textColor = Color::WHITE;
    page3_07.touchable = true;
    page3_07.text = {"Fox", "Sports"};
    page3_07.name = "Fox Sports";
    page3_07.tag = "script.sirius_foxsports";

    page3_08.font = font_button_label;
    page3_08.color = color_green;
    page3_08.textColor = Color::BLACK;
    page3_08.touchable = true;
    page3_08.text = {"CBS", "Sports"};
    page3_08.name = "CBS Sports";
    page3_08.tag = "script.sirius_cbssports";

    page3_09.font = font_button_label;
    page3_09.color = color_blue;
    page3_09.textColor = Color::WHITE;
    page3_09.touchable = true;
    page3_09.text = {"NBC", "Sports"};
    page3_09.name = "NBC Sports";
    page3_09.tag = "script.sirius_nbcsports";

    // Menu
    panelHome.font = icon_font_45;
    panelHome.color = color_yellow;
    panelHome.textColor = Color::BLACK;
    panelHome.touchable = true;
    panelHome.text = panelHomeText;
    panelHome.name = "Home";

    panelUp.font = icon_font_45;
    panelUp.color = color_yellow;
    panelUp.textColor = Color::BLACK;
    panelUp.touchable = true;
    panelUp.text = panelUpText;
    panelUp.name = "Prev Page";
    
    panelDown.font = icon_font_45;
    panelDown.color = color_yellow;
    panelDown.textColor = Color::BLACK;
    panelDown.touchable = true;
    panelDown.text = panelDownText;
    panelDown.name = "Next Page";

    flashPanel.font = font_flash;
    flashPanel.color = Color::BLACK;
    flashPanel.textColor = Color::WHITE;
    flashPanel.touchable = false;
    flashPanel.drawPanelOutline = true;
    flashPanel.enabled = false;


    // Backlight initial level
    backlight->set_level(brightness->value());

    // Fill the screen the first time to have BLACK in any gaps in Panels.
    display.fill(Color::BLACK);
}

// The time until which to display flash
esphome::time::ESPTime flashUntil;

// Enable the Flash message with some specific text
void enableFlash(std::vector<std::string> flashText) {
    flashUntil = esptime->now();
    flashUntil.increment_second();
    flashPanel.enabled = true;
    flashPanel.text = flashText;
}

void updatePanelStates() {
    //
    // Update configuration for Panels that might have changed.
    //
    auto now = esptime->now();

    if (flashPanel.enabled && now > flashUntil) {
        flashPanel.enabled = false;
    }
    
}

// Draw all of the panels
void drawPanels() {
    // drawAllPanels is generally preferred
    DisplayPanel::drawAllPanels(*lcd, pages[pageNumber]);
    
    
    // But draw flashPanel separately so it over-draws
    // what is below it.
    flashPanel.draw(*lcd);
}

// See if one of the enabled, touchable panels on the
// current page has been touched.
// lastTouchedPanel will be set to a pointer to the
// touched panel (or NULL of no panel was found for the coordinates).
boolean isPanelTouched(int tpX, int tpY) {
    lastTouchedPanel = DisplayPanel::touchedPanel(pages[pageNumber], tpX, tpY);
    return lastTouchedPanel != NULL;
}