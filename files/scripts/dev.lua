function DoGodmode()
	game.game:GetActivePlayer():SetDemigod(true)
	game.game:GetActivePlayer():EnableInjury(false)
	game.game:GetActivePlayer().invulnerability = true
end

function giveGuns()
	local weapons = {
		{ ID = 2, ammo = 42 },   -- Model_12_Revolver
		{ ID = 3, ammo = 60 },   -- Mauser_C96
		{ ID = 4, ammo = 56 },   -- Colt_M1911A1
		{ ID = 5, ammo = 92 },   -- Colt_M1911_Special
		{ ID = 6, ammo = 42 },   -- Model_19_Revolver
		{ ID = 8, ammo = 56 },   -- Remington_Model_870_Field_gun
		{ ID = 9, ammo = 120 },  -- M3_Grease_Gun
		{ ID = 10, ammo = 128 }, -- MP40
		{ ID = 11, ammo = 200 }, -- Thompson_1928
		{ ID = 12, ammo = 120 }, -- M1A1_Thompson
		{ ID = 13, ammo = 120 }, -- Beretta_Model_38A
		{ ID = 15, ammo = 40 },  -- M1_Garand
		{ ID = 17, ammo = 35 },  -- Kar98k
		{ ID = 20, ammo = 6 },   -- MK2_Frag_Grenade
		{ ID = 21, ammo = 6 }    -- Molotov_Cocktail
	}
	
	local player = game.game:GetActivePlayer()
	for _, weapon in pairs(weapons) do
		player:InventoryAddWeapon(weapon.ID, weapon.ammo)
		player:InventoryReload(weapon.ID, weapon.ammo) --Praise the lord, and pass the ammunition.
	end
end

function FixCar()
	local veh = game.game:GetActivePlayer():GetOwner()
	if veh then
		local level = veh:GetActualTuningTable()
		if level < 3 then
			veh:SetActualTuningTable(3)
		end
		local damage = veh:GetDamage()
		ai.police:ClearKnownCars()
		if damage > 0 then
			local carSpeed = veh:GetSpeedFloat()
			veh:RepairAndClear()
			if carSpeed > 1 then
				veh:SetSpeed(carSpeed)
			end
		end
		local dirt = veh:GetDirty()
		if (dirt > 0) then
			veh:SetDirty(0)
		end
		local motorDamage = veh:GetMotorDamage()
		if motorDamage > 0 then
			veh:SetMotorDamage(0)
		end
	end
end

function AddMoneyToPlayer()
	game.game:GetActivePlayer():InventoryAddMoney(100*1000000)
end

RepairCar = [[DelayBuffer:Insert(FixCar, nil, 100, 1, false)]] --We must use DelayBuffer, or else the user could execute too many scripts and crash the game.
allWeapons = [[DelayBuffer:Insert(giveGuns, nil, 100, 1, false)]] --We must use DelayBuffer, or else the user could execute too many scripts and crash the game.
godMode = [[DelayBuffer:Insert(DoGodmode, nil, 100, 1, false)]] --We must use DelayBuffer, or else the user could execute too many scripts and crash the game.
addMoney = [[DelayBuffer:Insert(AddMoneyToPlayer, nil, 100, 1, false)]] --We must use DelayBuffer, or else the user could execute too many scripts and crash the game.

--For a list of functions and how to use them, please see this github documentation by NobleFalcon:
--https://github.com/NobleFalcon/mafia2-scripts?tab=readme-ov-file#core-objects


unbindKey("p")
bindKey("p", godMode)
unbindKey("o")
bindKey("o", allWeapons)
unbindKey("i")
bindKey("i", addMoney)

unbindKey("VK_MULTIPLY")
bindKey("VK_MULTIPLY", RepairCar)