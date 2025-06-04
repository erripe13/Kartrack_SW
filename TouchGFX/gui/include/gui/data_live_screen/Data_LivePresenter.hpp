#ifndef DATA_LIVEPRESENTER_HPP
#define DATA_LIVEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Data_LiveView;

class Data_LivePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Data_LivePresenter(Data_LiveView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~Data_LivePresenter() {}

private:
    Data_LivePresenter();

    Data_LiveView& view;
};

#endif // DATA_LIVEPRESENTER_HPP
