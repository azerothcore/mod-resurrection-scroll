/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Chat.h"
#include "GameTime.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "ResurrectionScroll.h"

ResurrectionScroll* ResurrectionScroll::instance()
{
    static ResurrectionScroll instance;
    return &instance;
}

void ResurrectionScroll::LoadAccountData()
{
    LOG_INFO("sql.sql", "Loading Resurrection Scroll account data...");
    if (QueryResult result = CharacterDatabase.Query("SELECT AccountId, EndDate, Expired FROM mod_ress_scroll_accounts"))
    {
        do
        {
            Field* fields = result->Fetch();
            ScrollAccountData data;
            data.AccountId = fields[0].Get<uint32>();
            data.EndDate = fields[1].Get<uint32>();
            data.Expired = fields[2].Get<bool>();
            Accounts[data.AccountId] = data;
        } while (result->NextRow());
    }
}

void ResurrectionScroll::InsertAccountData(ScrollAccountData data)
{
    Accounts[data.AccountId] = data;
    CharacterDatabase.Execute("INSERT IGNORE INTO mod_ress_scroll_accounts (AccountId, EndDate) VALUES ({}, {})", data.AccountId, data.EndDate);
}

void ResurrectionScroll::SetExpired(uint32 accountId)
{
    CharacterDatabase.DirectExecute("UPDATE mod_ress_scroll_accounts SET Expired = 1 WHERE AccountId = {}", accountId);

    if (auto itr = Accounts.find(accountId); itr != Accounts.end())
        itr->second.Expired = true;
}
