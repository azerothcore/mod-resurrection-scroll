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

    ScrollAccountData() : AccountId(0), LastLogoutDate(0), EndDate(0), Expired(false) { }

    ScrollAccountData(uint32 accountId, uint32 logout, uint32 end, bool expired = false)
        : AccountId(accountId), LastLogoutDate(logout), EndDate(end), Expired(expired) { }
};

enum RSSettings
{
    SETTING_RS_DISABLE = 0
};

enum ResurrectionScrollStrings : uint32
{
    LANG_MOD_PLAYER_NOT_FOUND      = 1,
    LANG_MOD_BONUS_ENABLED         = 2,
    LANG_MOD_BONUS_DISABLED        = 3,
    LANG_MOD_DISCLAIMER_RESTED_XP  = 4,
    LANG_MOD_DISCLAIMER_NO_REMOVAL = 5,
    LANG_MOD_ADMIN_ENABLED_OTHER   = 6,
    LANG_MOD_ADMIN_DISABLED_OTHER  = 7,
    LANG_MOD_ADMIN_ENABLED_SELF    = 8,
    LANG_MOD_ADMIN_DISABLED_SELF   = 9,
    LANG_MOD_INFO_DAYS_REQUIRED    = 10,
    LANG_MOD_INFO_LAST_LOGIN       = 11,
    LANG_MOD_INFO_NO_LOGIN_HISTORY = 12,
    LANG_MOD_INFO_ELIGIBLE_ON      = 13,
    LANG_MOD_INFO_EXPIRED_ON       = 14,
    LANG_MOD_INFO_ELIGIBLE_AGAIN   = 15,
    LANG_MOD_INFO_EXPIRES_ON       = 16,
    LANG_MOD_BONUS_DISABLED_NOTIFY = 17,
    LANG_MOD_BONUS_ACTIVE_NOTIFY   = 18,
};

const std::string ModResScrollString = "mod_ros";

class ResurrectionScroll
{

private:
    std::unordered_map<uint32, ScrollAccountData> Accounts;
    uint8 MaxAffectedLevel{ 69 };

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
    void LoadAccountData();

    [[nodiscard]] uint8 GetMaxAffectedLevel() const { return MaxAffectedLevel; }
    void SetMaxAffectedLevel(uint8 level) { MaxAffectedLevel = level; }
};

#define sResScroll ResurrectionScroll::instance()

#endif
