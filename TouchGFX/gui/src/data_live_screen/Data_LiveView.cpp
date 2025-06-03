#include <gui/data_live_screen/Data_LiveView.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <cstdio>

Data_LiveView::Data_LiveView()
{
    // Remove unused variable declaration
}

void Data_LiveView::setupScreen()
{
    Data_LiveViewBase::setupScreen();
}

void Data_LiveView::tearDownScreen()
{
    Data_LiveViewBase::tearDownScreen();
}
/*
void Data_LiveView::handleTickEvent()
{
    float vitesseValue = presenter->getVitesse();

    char tempBuffer[10];
    sprintf(tempBuffer, "%.1f", vitesseValue); // @suppress("Float formatting support")
    Unicode::strncpy(vitesseTextBuffer, tempBuffer, 10);

    textArea3.setWildcard(vitesseTextBuffer); // OK car hérité de la ViewBase
    textArea3.invalidate();
}*/

void Data_LiveView::updateValues(float speed)
{
    Unicode::snprintfFloat(speedBuffer, 10, "%.1f", speed);
    vitesse.invalidate();
/*
    Unicode::snprintfFloat(s1Buffer, 10, "%.2f", s1);
    secteur1.invalidate();

    Unicode::snprintfFloat(s2Buffer, 10, "%.2f", s2);
    secteur2.invalidate();

    Unicode::snprintfFloat(s3Buffer, 10, "%.2f", s3);
    secteur3.invalidate();
    */

}

extern "C" void updateDataLiveScreen(float speed)
{
    Data_LiveView *currentScreen =
        static_cast<Data_LiveView*>(touchgfx::Application::getInstance()->getCurrentScreen());

    if (currentScreen)
    {
        currentScreen->updateValues(speed);
    }
}



