#ifndef SESSION_STARTVIEW_HPP
#define SESSION_STARTVIEW_HPP

#include <gui_generated/session_start_screen/Session_StartViewBase.hpp>
#include <gui/session_start_screen/Session_StartPresenter.hpp>

class Session_StartView : public Session_StartViewBase
{
public:
    Session_StartView();
    virtual ~Session_StartView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // SESSION_STARTVIEW_HPP
