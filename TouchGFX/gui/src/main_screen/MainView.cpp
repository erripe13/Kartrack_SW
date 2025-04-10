#include <gui/main_screen/MainView.hpp>

MainView::MainView()
{
    selectedIndex = 0;
}

void MainView::setupScreen()
{
    MainViewBase::setupScreen();

    // Position initiale du sélecteur
    //Selector.setXY(Session_Start.getX() - 164, Session_Start.getY() - 141); // Ajuste selon ton design
    Selector.setXY( 164, 141); // Ajuste selon ton design
    Selector.setVisible(true);
}

void MainView::tearDownScreen()
{
    MainViewBase::tearDownScreen();
}

void MainView::updateSelectorPosition()
{
    switch (selectedIndex)
    {
        case 0:
        	//Selector.moveTo(Session_Start.getX() - 164, Session_Start.getY() - 141);
            Selector.moveTo(164, 141);
            break;
        case 1:
            Selector.moveTo(164,171);  // même offset Y pour garder aligné
            break;
    }
    Selector.invalidate();
}

void MainView::UPClicked()
{
    if (selectedIndex == 0)
        selectedIndex = MENU_COUNT - 1;
    else
        selectedIndex--;
    updateSelectorPosition();
}

void MainView::DOWNClicked()
{
    selectedIndex = (selectedIndex + 1) % MENU_COUNT;
    updateSelectorPosition();
}

void MainView::VALIDEClicked()
{
    switch (selectedIndex)
    {
        case 0:
            application().gotoSession_StartScreenNoTransition();
            break;

        case 1:
        	application().gotoData_LiveScreenNoTransition();  // ✅ une fois que tu auras activé la navigation vers Data_Live
            break;
    }
}
