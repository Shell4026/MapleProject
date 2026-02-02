#include "UI/Button.h"

#include "Game/Input.h"
namespace sh::game
{
    Button::Button(GameObject& owner) :
        UIRect(owner)
    {
    }
    SH_USER_API void Button::OnHover()
    {
        if (Input::GetMouseReleased(Input::MouseType::Left))
        {
            onClick.Notify(this);
        }
    }
}//namespace