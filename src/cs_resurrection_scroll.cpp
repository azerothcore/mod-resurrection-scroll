/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "AccountMgr.h"
#include "Chat.h"
#include "GameTime.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ResurrectionScroll.h"

using namespace Acore::ChatCommands;

class resurrection_scroll_commandscript : public CommandScript
{
public:
    resurrection_scroll_commandscript() : CommandScript("resurrection_scroll_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable scrollTable =
        {
            { "disable", HandleResScrollRestedXpCommand, SEC_PLAYER, Console::Yes },
            { "info",    HandleResScrollInfoCommand,     SEC_PLAYER, Console::Yes }
        };

        static ChatCommandTable commandTable =
        {
            { "rscroll", scrollTable },
        };

        return commandTable;
    }

    static bool HandleResScrollRestedXpCommand(ChatHandler* handler, Optional<PlayerIdentifier> player, Optional<bool> disable)
    {
        if (handler->GetSession() && AccountMgr::IsPlayerAccount(handler->GetSession()->GetSecurity()))
            player = PlayerIdentifier::FromSelf(handler);

        if (!player)
            player = PlayerIdentifier::FromTargetOrSelf(handler);

        Player* targetPlayer = player ? player->GetConnectedPlayer() : nullptr;
        if (!targetPlayer)
        {
            handler->PSendModuleSysMessage(ModResScrollString, LANG_MOD_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        bool newDisabled = disable.has_value()
            ? *disable
            : !targetPlayer->GetPlayerSetting(ModResScrollString, SETTING_RS_DISABLE).IsEnabled();

        targetPlayer->UpdatePlayerSetting(ModResScrollString, SETTING_RS_DISABLE, newDisabled ? 1 : 0);

        ChatHandler targetHandler(targetPlayer->GetSession());
        if (!newDisabled)
            targetHandler.PSendModuleSysMessage(ModResScrollString, LANG_MOD_BONUS_ENABLED);
        else
        {
            targetHandler.PSendModuleSysMessage(ModResScrollString, LANG_MOD_BONUS_DISABLED);
            targetHandler.PSendModuleSysMessage(ModResScrollString, LANG_MOD_DISCLAIMER_RESTED_XP);
            targetHandler.PSendModuleSysMessage(ModResScrollString, LANG_MOD_DISCLAIMER_NO_REMOVAL);
        }

        Player* issuer = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (issuer != targetPlayer)
            handler->PSendModuleSysMessage(ModResScrollString,
                newDisabled ? LANG_MOD_ADMIN_DISABLED_OTHER : LANG_MOD_ADMIN_ENABLED_OTHER,
                targetPlayer->GetName(), targetPlayer->GetGUID().GetCounter());
        else
            handler->PSendModuleSysMessage(ModResScrollString,
                newDisabled ? LANG_MOD_ADMIN_DISABLED_SELF : LANG_MOD_ADMIN_ENABLED_SELF);

        return true;
    }

    static bool HandleResScrollInfoCommand(ChatHandler* handler, Optional<AccountIdentifier> account)
    {
        uint32 accountId = 0;

        if (handler->GetSession() && AccountMgr::IsPlayerAccount(handler->GetSession()->GetSecurity()))
        {
            // Players can only view their own info
            accountId = handler->GetSession()->GetAccountId();
        }
        else if (account)
        {
            accountId = account->GetID();
        }
        else if (handler->GetSession())
        {
            accountId = handler->GetSession()->GetAccountId();
        }

        std::string accountName;
        AccountMgr::GetName(accountId, accountName);

        // Get last logout time for the account
        uint32 lastLogout = 0;
        if (QueryResult result = CharacterDatabase.Query(
            "SELECT MAX(logout_time) FROM characters WHERE account = {}", accountId))
        {
            Field* fields = result->Fetch();
            if (!fields->IsNull())
                lastLogout = fields[0].Get<uint32>();
        }

        handler->PSendModuleSysMessage(ModResScrollString, LANG_MOD_INFO_DAYS_REQUIRED, sResScroll->DaysInactive);

        if (lastLogout)
        {
            tm logoutTime = Acore::Time::TimeBreakdown(lastLogout);
            handler->PSendModuleSysMessage(ModResScrollString, LANG_MOD_INFO_LAST_LOGIN, accountName, accountId, logoutTime);
        }
        else
            handler->PSendModuleSysMessage(ModResScrollString, LANG_MOD_INFO_NO_LOGIN_HISTORY, accountName, accountId);

        if (!sResScroll->IsAccountLoaded(accountId))
        {
            if (lastLogout)
            {
                uint32 eligibleDate = lastLogout + (sResScroll->DaysInactive * DAY);
                tm eligibleTime = Acore::Time::TimeBreakdown(eligibleDate);
                handler->PSendModuleSysMessage(ModResScrollString, LANG_MOD_INFO_ELIGIBLE_ON, eligibleTime);
            }
            return true;
        }

        ScrollAccountData const& data = sResScroll->GetAccountData(accountId);
        tm endTime = Acore::Time::TimeBreakdown(data.EndDate);

        if (data.Expired || data.EndDate <= GameTime::GetGameTime().count())
        {
            handler->PSendModuleSysMessage(ModResScrollString, LANG_MOD_INFO_EXPIRED_ON, endTime);
            if (lastLogout)
            {
                uint32 eligibleDate = lastLogout + (sResScroll->DaysInactive * DAY);
                tm eligibleTime = Acore::Time::TimeBreakdown(eligibleDate);
                handler->PSendModuleSysMessage(ModResScrollString, LANG_MOD_INFO_ELIGIBLE_AGAIN, eligibleTime);
            }
            return true;
        }

        handler->PSendModuleSysMessage(ModResScrollString, LANG_MOD_INFO_EXPIRES_ON, endTime);
        return true;
    }
};

void AddSC_resurrection_scroll_commandscript()
{
    new resurrection_scroll_commandscript();
}
