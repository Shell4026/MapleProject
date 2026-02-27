#pragma once
#include "Export.h"

#include "Game/ScriptableObject.h"

#include <string>
#include <vector>
namespace sh::game
{
	class Skill;
	class Job : public ScriptableObject
	{
		SRPO(Job)
	public:
		SH_USER_API auto GetId() const -> uint32_t { return id; }
		SH_USER_API auto GetName() const -> const std::string& { return name; }
		SH_USER_API auto GetSkills() const -> const std::vector<Skill*>& { return skills; }
	private:
		PROPERTY(id)
		uint32_t id = 0;
		PROPERTY(name)
		std::string name = "Job";
		PROPERTY(skills, core::PropertyOption::sobjPtr)
		std::vector<Skill*> skills;
	};
}//namespace