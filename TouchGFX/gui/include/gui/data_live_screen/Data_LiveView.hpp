#ifndef DATA_LIVEVIEW_HPP
#define DATA_LIVEVIEW_HPP

#include <gui_generated/data_live_screen/Data_LiveViewBase.hpp>
#include <gui/data_live_screen/Data_LivePresenter.hpp>

class Data_LiveView : public Data_LiveViewBase
{
public:
    Data_LiveView();
    virtual ~Data_LiveView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // DATA_LIVEVIEW_HPP
