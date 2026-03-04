#include "Skill/SkillCondition.h"

namespace sh::game
{
	SH_USER_API auto SkillCondition::TypeToString(Type type) -> const char*
	{
		switch (type)
		{
		case Type::None:
			return "None";
		case Type::LandedAfterLastUse:
			return "LandedAfterLastUse";
		case Type::JumpAfterLastUse:
			return "JumpAfterLastUse";
		case Type::PreviousSkillIn:
			return "PreviousSkillIn";
		}
		return "None";
	}
}//namespace