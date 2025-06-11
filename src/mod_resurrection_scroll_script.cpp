/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ResurrectionScroll.h"
#include "Chat.h"
#include "Config.h"
#include "GameTime.h"
#include "Player.h"
#include "ScriptMgr.h"

ResurrectionScroll* ResurrectionScroll::instance()
{
    static ResurrectionScroll instance;
    return &instance;
}

class mod_resurrection_scroll_playerscript : public PlayerScript
{
public:
    mod_resurrection_scroll_playerscript() : PlayerScript("mod_resurrection_scroll_playerscript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_LEVEL_CHANGED
    }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!sResScroll->IsEnabled)
           return;

        if (player->GetLevel() == 1)
            return;

        if (ProcessBonusChecks(player))
            return;

        uint32 validLastLogoutDate = GameTime::GetGameTime().count() - (sResScroll->DaysInactive * DAY);

        if (CharacterDatabase.Query("SELECT 1 FROM characters WHERE guid = {} AND logout_time <= {}", player->GetGUID().GetCounter(), validLastLogoutDate))
        {
            uint32 duration = GameTime::GetGameTime().count() + (sResScroll->Duration * DAY);
            player->SetRestBonus(sObjectMgr->GetXPForLevel(player->GetLevel()));
            player->UpdatePlayerSetting(ModResScrollString, SETTING_RS_ELIGIBLE, 1);
            player->UpdatePlayerSetting(ModResScrollString, SETTING_RS_DISABLE_DATE, duration);
            tm endDate = Acore::Time::TimeBreakdown(duration);
            ChatHandler(player->GetSession()).PSendSysMessage("|cff00ccffYou are eligible for the Scroll of Resurrection program, granting you rested experience until {:%Y-%m-%d %H:%M}.|r", endDate);
        }
    }

    void OnPlayerLevelChanged(Player* player, uint8 /*oldlevel*/) override
    {
        if (!sResScroll->IsEnabled)
            return;

        ProcessBonusChecks(player);
    }

    bool ProcessBonusChecks(Player* player)
    {
        if (player->HasAnyAuras(2000100, 2000101, 2000102))
            return false;

        if (player->GetPlayerSetting(ModResScrollString, SETTING_RS_ELIGIBLE).value)
        {
            uint32 epochEndDate = player->GetPlayerSetting(ModResScrollString, SETTING_RS_DISABLE_DATE).value;
            tm end = Acore::Time::TimeBreakdown(epochEndDate);

            if (GameTime::GetGameTime().count() <= epochEndDate)
            {
                player->UpdatePlayerSetting(ModResScrollString, SETTING_RS_ELIGIBLE, 0);
                ChatHandler(player->GetSession()).PSendSysMessage("|cffff0000You are no longer eligible for the Scroll of Resurrection program. Your bonus ended in: {:%Y-%m-%d %H:%M}.|r", end);
                return true;
            }

            player->SetRestBonus(sObjectMgr->GetXPForLevel(player->GetLevel()));
            ChatHandler(player->GetSession()).PSendSysMessage("|cff00ccffYou are eligible for the Scroll of Resurrection program, granting you rested experience until {:%Y-%m-%d %H:%M}.|r", end);
            return true;
        }

        return false;
    }
};

class mod_resurrection_scroll_worldscript : public WorldScript
{
public:
    mod_resurrection_scroll_worldscript() : WorldScript("mod_resurrection_scroll_worldscript", {
        WORLDHOOK_ON_AFTER_CONFIG_LOAD
    }) { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sResScroll->IsEnabled = sConfigMgr->GetOption<bool>("ModResurrectionScroll.Enable", false);
        sResScroll->DaysInactive = sConfigMgr->GetOption<uint32>("ModResurrectionScroll.DaysInactive", 180);
        sResScroll->Duration = sConfigMgr->GetOption<uint32>("ModResurrectionScroll.Duration", 30);
    }
};

void AddModResurrectionScrollScripts()
{
    new mod_resurrection_scroll_playerscript();
    new mod_resurrection_scroll_worldscript();
}
