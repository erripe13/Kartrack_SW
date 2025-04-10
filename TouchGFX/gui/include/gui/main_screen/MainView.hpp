#ifndef MAINVIEW_HPP
#define MAINVIEW_HPP

#include <gui_generated/main_screen/MainViewBase.hpp>
#include <gui/main_screen/MainPresenter.hpp>

class MainView : public MainViewBase
{
private:
    uint8_t selectedIndex;
    static const uint8_t MENU_COUNT = 2;

    void updateSelectorPosition();

public:
    MainView();
    virtual ~MainView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void UPClicked();
    void DOWNClicked();
    void VALIDEClicked();
};

#endif // MAINVIEW_HPP
