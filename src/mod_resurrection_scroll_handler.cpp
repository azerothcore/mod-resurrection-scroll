/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Chat.h"
#include "GameTime.h"
#include "ScriptMgr.h"
#include "Smartstone.h"
#include "Player.h"
#include "ResurrectionScroll.h"

ResurrectionScroll* ResurrectionScroll::instance()
{
    static ResurrectionScroll instance;
    return &instance;
}

void ResurrectionScroll::InsertAccountData(ScrollAccountData data)
{
    Accounts[data.AccountId] = data;

    CharacterDatabase.Query("REPLACE INTO mod_ress_scroll_accounts (AccountId, EndDate) VALUES (?, ?)", data.AccountId, data.EndDate);
}

void ResurrectionScroll::SetExpired(uint32 accountId)
{
    auto itr = Accounts.find(accountId);
    if (itr != Accounts.end())
    {
        itr->second.Expired = true;
        CharacterDatabase.Query("UPDATE mod_ress_scroll_accounts SET Expired = 1 WHERE AccountId = ?", accountId);
    }
}
