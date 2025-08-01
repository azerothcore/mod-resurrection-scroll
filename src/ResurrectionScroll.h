#ifndef DEF_RESSURRECTIONSCROLL_H
#define DEF_RESSURRECTIONSCROLL_H

#include "Config.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

struct ScrollAccountData
{
    uint32 AccountId;
    uint32 LastLogoutDate;
    uint32 EndDate;
    bool Expired;
};

enum RSSettings
{
    SETTING_RS_ELIGIBLE      = 0,
    SETTING_RS_DISABLE_DATE  = 1
};

const std::string ModResScrollString = "mod_resurrection_scroll";

class ResurrectionScroll
{

private:
    std::unordered_map<uint32, ScrollAccountData> Accounts;

public:
    static ResurrectionScroll* instance();

    bool IsEnabled{ false };
    uint32 DaysInactive{ 180 };
    uint32 Duration{ 30 };

    [[nodiscard]] bool IsAccountLoaded(uint32 accountId) const { return Accounts.find(accountId) != Accounts.end(); }
    void InsertAccountData(ScrollAccountData data);
    [[nodiscard]] ScrollAccountData GetAccountData(uint32 accountId) const
    {
        auto itr = Accounts.find(accountId);
        if (itr != Accounts.end())
            return itr->second;

        return { 0, 0, 0, false };
    }
    void SetExpired(uint32 accountId);
};

#define sResScroll ResurrectionScroll::instance()

#endif
