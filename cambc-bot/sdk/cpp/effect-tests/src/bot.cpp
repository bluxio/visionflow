#include <cambc.h>
#include <cstdio>
#include <cstdlib>

void check(bool value)
{
	if (!value)
		std::abort();
}

int main()
{
	if (cambc::is_draft())
	{
		constexpr std::array picks{ cambc::champion_draft::stellar, cambc::champion_draft::willow, cambc::champion_draft::varg };
		for (;;)
		{
			cambc::draft::pick(picks.at(cambc::draft::current_draft(cambc::team::ally).size()));
			cambc::end_turn();
		}
	}
	const auto& self = cambc::self();
	for (const auto& ability : self.abilities())
	{
		if (const auto& info = ability.info())
			std::visit([](const auto& data) { check(data.max_cooldown >= 0); }, *info);
		check(ability.damage_amount() >= 0 && ability.heal_amount() >= 0 && ability.shield_amount() >= 0);
		check(ability.damage_amount(self) == 0);
		check(ability.heal_amount(self) == std::min(ability.heal_amount(), std::max(0.0f, self.max_health() - self.health())));
		check(ability.shield_amount(self) == ability.shield_amount());
		for (const auto& other : cambc::champions())
		{
			if (!other.is_visible() && other.is_alive())
			{
				bool caught = false;
				try
				{
					(void)ability.damage_amount(other);
				}
				catch (const cambc::error& error)
				{
					check(error.code() == cambc::error_code::api_not_in_vision);
					caught = true;
				}
				check(caught);
			}
		}
	}
	const auto buffs = self.buffs();
	for (const auto& buff : buffs)
		std::visit([](const auto& data) { check(data.duration > 0); }, buff);
	if (!buffs.has(cambc::buff_types::stun))
	{
		check(self.restrictions() == cambc::buff_restrictions{});
		bool caught = false;
		try
		{
			(void)buffs.get(cambc::buff_types::stun);
		}
		catch (const std::out_of_range&)
		{
			caught = true;
		}
		check(caught);
	}
	if (self.champion_name() == cambc::champion_type::varg)
	{
		const float shield = self.abilities()[1].shield_amount(self);
		self.abilities()[1].use();
		const auto shield_buffs = self.buffs();
		check(shield_buffs.has(cambc::buff_types::shield));
		check(!self.buffs().is_empty() && self.restrictions() == cambc::buff_restrictions{});
		check(std::get<cambc::buff_shield>(shield_buffs.get(cambc::buff_types::shield)).value == shield);
		check(shield_buffs.get<cambc::buff_shield>().value == shield);
		check(buffs.is_empty());
		bool found = false;
		for (const auto& buff : shield_buffs)
			if (const auto* value = std::get_if<cambc::buff_shield>(&buff))
			{
				check(value->value == shield);
				found = true;
			}
		check(found);
	}
	std::puts("live ability/buff API PASS");
	for (;;)
		cambc::end_turn();
}
