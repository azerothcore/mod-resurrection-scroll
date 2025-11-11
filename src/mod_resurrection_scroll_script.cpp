/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ResurrectionScroll.h"
#include "Chat.h"
#include "Config.h"
#include "GameTime.h"
#include "Player.h"
#include "ScriptMgr.h"

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

        if (player->GetLevel() >= sResScroll->GetMaxAffectedLevel())
            return;

        ProcessBonusChecks(player);
    }

    void OnPlayerLevelChanged(Player* player, uint8 oldlevel) override
    {
        if (!sResScroll->IsEnabled)
            return;

        if (oldlevel >= sResScroll->GetMaxAffectedLevel())
            return;

        if (player->GetLevel() >= sResScroll->GetMaxAffectedLevel())
            return;

        ProcessBonusChecks(player);
    }

    bool ProcessBonusChecks(Player* player) const
    {
        if (player->HasAnyAuras(2000100, 2000101, 2000102))
            return false;

        uint32 accountId = player->GetSession()->GetAccountId();
        ScrollAccountData const& accountData = sResScroll->GetAccountData(accountId);

        if (accountData.Expired)
            return false;

        uint32 now = GameTime::GetGameTime().count();
        if (accountData.EndDate && accountData.EndDate > now)
        {
            if (player->GetPlayerSetting(ModResScrollString, SETTING_RS_DISABLE).IsEnabled())
            {
                player->SendSystemMessage(
                    "|cffff0000You are eligible for the Scroll of Resurrection bonus, but it has been disabled. You can enable it to receive rewards.|r"
                );
                return false;
            }

            player->SetRestBonus(sObjectMgr->GetXPForLevel(player->GetLevel()));

            tm endTime = Acore::Time::TimeBreakdown(accountData.EndDate);
            ChatHandler(player->GetSession()).PSendSysMessage(
                "|cff00ccffYou are eligible for the Scroll of Resurrection program, granting you rested experience until {:%Y-%m-%d %H:%M}.|r",
                endTime
            );
            return true;
        }

        // Bonus expired — update record
        sResScroll->SetExpired(accountId);
        //ChatHandler(player->GetSession()).PSendSysMessage(
       //     "|cffff0000Your Scroll of Resurrection bonus has expired.|r"
        //);
        return false;
    }
};

class mod_resurrection_scroll_worldscript : public WorldScript
{
public:
    mod_resurrection_scroll_worldscript() : WorldScript("mod_resurrection_scroll_worldscript", {
        WORLDHOOK_ON_AFTER_CONFIG_LOAD
    }) { }

    void OnAfterConfigLoad(bool reload) override
    {
        sResScroll->IsEnabled = sConfigMgr->GetOption<bool>("ModResurrectionScroll.Enable", false);
        sResScroll->DaysInactive = sConfigMgr->GetOption<uint32>("ModResurrectionScroll.DaysInactive", 180);
        sResScroll->Duration = sConfigMgr->GetOption<uint32>("ModResurrectionScroll.Duration", 30);
        sResScroll->SetMaxAffectedLevel(sConfigMgr->GetOption<uint8>("ModResurrectionScroll.MaxAffectedLevel", 70));

        if (!reload)
        {
            sResScroll->LoadAccountData();
        }
    }
};

class mod_resurrection_scroll_accountscript : public AccountScript
{
public:
    mod_resurrection_scroll_accountscript() : AccountScript("mod_resurrection_scroll_accountscript", { ACCOUNTHOOK_ON_ACCOUNT_LOGIN }) {}

    void OnAccountLogin(uint32 accountId) override
    {
        if (!sResScroll->IsEnabled)
            return;

        uint32 now = GameTime::GetGameTime().count();
        uint32 validLastLogoutDate = now - (sResScroll->DaysInactive * DAY);

        // Get the most recent logout time among all characters
        if (QueryResult result = CharacterDatabase.Query(
            "SELECT MAX(logout_time) FROM characters WHERE account = {}", accountId))
        {
            Field* fields = result->Fetch();
            if (!fields->IsNull())
            {
                uint32 lastLogoutTime = fields[0].Get<uint32>();

                // Only reward if last logout was at least N days ago
                if (lastLogoutTime <= validLastLogoutDate)
                {
                    uint32 expiration = now + (sResScroll->Duration * DAY);
                    sResScroll->InsertAccountData(ScrollAccountData(accountId, lastLogoutTime, expiration));
                }
            }
        }
    }
};

void AddModResurrectionScrollScripts()
{
    new mod_resurrection_scroll_playerscript();
    new mod_resurrection_scroll_worldscript();
    new mod_resurrection_scroll_accountscript();
}
