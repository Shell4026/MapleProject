#pragma once
#include "Export.h"

#include "Game/ScriptableObject.h"

#include <cstdint>
#include <vector>
namespace sh::editor
{
	class SkillConditionInspector;
}
namespace sh::game
{
	class SkillCondition : public ScriptableObject
	{
		SRPO(SkillCondition)
		friend editor::SkillConditionInspector;
	public:
		enum class Type : uint8_t
		{
			None = 0,
			LandedAfterLastUse = 1,
			JumpAfterLastUse = 2,
			PreviousSkillIn = 3
		};
	public:
		SH_USER_API auto GetConditionType() const -> Type { return static_cast<Type>(type); }
		SH_USER_API auto GetRequiredSkills() const -> const std::vector<uint32_t>& { return requiredSkills; }

		SH_USER_API static auto TypeToString(Type type) -> const char*;
	private:
		PROPERTY(type, core::PropertyOption::invisible)
		uint32_t type = static_cast<uint32_t>(Type::None);
		PROPERTY(requiredSkills)
		std::vector<uint32_t> requiredSkills;
	};
}//namespace

