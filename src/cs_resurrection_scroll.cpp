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
            { "restedxp", HandleResScrollRestedXpCommand, SEC_PLAYER, Console::Yes }
        };

        static ChatCommandTable commandTable =
        {
            { "rscroll", scrollTable },
        };

        return commandTable;
    }

    static bool HandleResScrollRestedXpCommand(ChatHandler* handler, Optional<PlayerIdentifier> player, bool enable)
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

        targetPlayer->UpdatePlayerSetting(ModResScrollString, SETTING_RS_DISABLE, enable);

        if (enable)
            targetPlayer->SendSystemMessage("Scroll of Resurrection bonuses enabled. You will now earn rested experience upon leveling up.");
        else
            targetPlayer->SendSystemMessage("Scroll of Resurrection bonuses disabled.");

        // Notify the command issuer
        if (handler->GetSession()->GetPlayer() != targetPlayer)
            handler->PSendSysMessage("Scroll of Resurrection rested bonuses {} for player {} ({}).", enable ? "enabled" : "disabled", targetPlayer->GetName(), targetPlayer->GetGUID().GetCounter());
        else
            handler->PSendSysMessage("Scroll of Resurrection rested bonuses {} for yourself.", enable ? "enabled" : "disabled");

        return true;
    }
};

void AddSC_resurrection_scroll_commandscript()
{
    new resurrection_scroll_commandscript();
}
