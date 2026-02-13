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

    static bool HandleResScrollRestedXpCommand(ChatHandler* handler, Optional<PlayerIdentifier> player, bool disable)
    {
        if (handler->GetSession() && AccountMgr::IsPlayerAccount(handler->GetSession()->GetSecurity()))
            player = PlayerIdentifier::FromSelf(handler);

        if (!player)
            player = PlayerIdentifier::FromTargetOrSelf(handler);

        Player* targetPlayer = player ? player->GetConnectedPlayer() : nullptr;
        if (!targetPlayer)
        {
            handler->SendErrorMessage("Player not found or not online.");
            return false;
        }

        targetPlayer->UpdatePlayerSetting(ModResScrollString, SETTING_RS_DISABLE, disable);

        if (!disable)
            targetPlayer->SendSystemMessage("Scroll of Resurrection bonuses enabled. You will now earn rested experience upon leveling up.");
        else
        {
            targetPlayer->SendSystemMessage("Scroll of Resurrection bonuses disabled. You will no longer earn rested experience upon leveling.");
            targetPlayer->SendSystemMessage("DISCLAIMER: Rested XP is a game mechanic earned while resting in capital cities or inns. You will still be granted rested XP gained by regular means.");
            targetPlayer->SendSystemMessage("Rested XP you already gained through Scroll of Resurrection or other means will NOT be removed!");
        }

        // Notify the command issuer
        if (!handler->GetSession() || handler->GetSession()->GetPlayer() != targetPlayer)
            handler->PSendSysMessage("Scroll of Resurrection rested bonuses {} for player {} ({}).", disable ? "disabled" : "enabled", targetPlayer->GetName(), targetPlayer->GetGUID().GetCounter());
        else
            handler->PSendSysMessage("Scroll of Resurrection rested bonuses {} for yourself.", disable ? "disabled" : "enabled");

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

        handler->PSendSysMessage("Days inactive required: {}.", sResScroll->DaysInactive);

        if (lastLogout)
        {
            tm logoutTime = Acore::Time::TimeBreakdown(lastLogout);
            handler->PSendSysMessage("Account {} (ID: {}) last logged in: {:%Y-%m-%d %H:%M}.", accountName, accountId, logoutTime);
        }
        else
            handler->PSendSysMessage("Account {} (ID: {}) has no login history.", accountName, accountId);

        if (!sResScroll->IsAccountLoaded(accountId))
        {
            if (lastLogout)
            {
                uint32 eligibleDate = lastLogout + (sResScroll->DaysInactive * DAY);
                tm eligibleTime = Acore::Time::TimeBreakdown(eligibleDate);
                handler->PSendSysMessage("Eligible for Scroll of Resurrection on: {:%Y-%m-%d %H:%M}.", eligibleTime);
            }
            return true;
        }

        ScrollAccountData const& data = sResScroll->GetAccountData(accountId);
        tm endTime = Acore::Time::TimeBreakdown(data.EndDate);

        if (data.Expired || data.EndDate <= GameTime::GetGameTime().count())
        {
            handler->PSendSysMessage("Scroll bonus expired on: {:%Y-%m-%d %H:%M}.", endTime);
            if (lastLogout)
            {
                uint32 eligibleDate = lastLogout + (sResScroll->DaysInactive * DAY);
                tm eligibleTime = Acore::Time::TimeBreakdown(eligibleDate);
                handler->PSendSysMessage("Eligible again on: {:%Y-%m-%d %H:%M}.", eligibleTime);
            }
            return true;
        }

        handler->PSendSysMessage("Scroll bonus expires on: {:%Y-%m-%d %H:%M}.", endTime);
        return true;
    }
};

void AddSC_resurrection_scroll_commandscript()
{
    new resurrection_scroll_commandscript();
}
