/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Language.h"
#include <string>

using namespace Acore::ChatCommands;

using GameObjectSpawnId = Variant<Hyperlink<gameobject>, ObjectGuid::LowType>;


class Objscale : public DataMap::Base
{
public:
    Objscale() {}
    Objscale(float scale) : scale(scale) {}
    float scale = -1.0f;
};

class CreatureScale : public AllCreatureScript
{
public:
    CreatureScale() : AllCreatureScript("CreatureScale") {}

    void OnCreatureAddWorld(Creature* creature) override
    {
        std::string query = "SELECT `scale` FROM `creature` WHERE `guid` = " + std::to_string(creature->GetSpawnId()) + ";";
        QueryResult result = WorldDatabase.Query(query);

        if (!result) {}
        else {
            Field* fields = result->Fetch();
            float  scale = fields[0].Get<float>();
            if (scale > 0)
            {
                creature->CustomData.Set("scale", new Objscale(scale));
                creature->SetObjectScale(scale);
            }
        }
    }

    void OnCreatureSaveToDB(Creature* creature) override
    {
        float scale = creature->CustomData.GetDefault<Objscale>("scale")->scale;

        if (scale > 0.0f) {
            std::string query = "UPDATE `creature` SET `scale` = " + std::to_string(scale) + " WHERE guid = " + std::to_string(creature->GetSpawnId());
            WorldDatabase.DirectExecute(query);
        }
    }
};

class GameObjectScale : public AllGameObjectScript
{
public:
    GameObjectScale() : AllGameObjectScript("GameObjectScale") {}

    void OnGameObjectAddWorld(GameObject* gameObject)  override
    {
        std::string query = "SELECT `scale` FROM `gameobject` WHERE `guid` = " + std::to_string(gameObject->GetSpawnId()) + ";";
        QueryResult result = WorldDatabase.Query(query);

        if (!result) {}
        else {
            Field* fields = result->Fetch();
            float  scale = fields[0].Get<float>();
            if (scale > 0.0f)
            {
                gameObject->CustomData.Set("scale", new Objscale(scale));
                gameObject->SetObjectScale(scale);
            }
        }
    }

    void OnGameObjectSaveToDB(GameObject* gameObject)  override
    {
        float scale = gameObject->CustomData.GetDefault<Objscale>("scale")->scale;

        if (scale > 0.0f) {
            std::string query = "UPDATE `gameobject` SET `scale` = " + std::to_string(scale) + " WHERE guid = " + std::to_string(gameObject->GetSpawnId());
            WorldDatabase.DirectExecute(query);
        }
    }
};

class Objscale_Creature_command : public CommandScript
{
public:
    Objscale_Creature_command() : CommandScript("Objscale_Creature_command") {}
    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> ObjscaleCreatureCommandTable = {

            {"set",         SEC_PLAYER, false, &HandleSetCommand, ""},
            {"reset",       SEC_PLAYER, false, &HandleReSetCommand, ""},
            { "",           SEC_PLAYER, false, &HandleBaseCommand,""}
        };

        static std::vector<ChatCommand> ObjscaleCreatureCommandBaseTable = {
            {"npc_scale", SEC_PLAYER, false, nullptr, "", ObjscaleCreatureCommandTable}
        };

        return ObjscaleCreatureCommandBaseTable;
    }

    static bool HandleSetCommand(ChatHandler* handler, float scale)
    {
        if (!scale)
            return false;

        Player* me = handler->GetSession()->GetPlayer();
        if (!me)
            return false;

        Unit* unit = handler->getSelectedUnit();
        if (!unit || unit->GetTypeId() != TYPEID_UNIT)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        Creature* creature = unit->ToCreature();
        if (scale > 0) {
            creature->CustomData.GetDefault<Objscale>("scale")->scale = scale;

            creature->SetObjectScale(scale);
            if (!creature->IsPet())
                creature->SaveToDB();

            me->GetSession()->SendAreaTriggerMessage("Creature scale has been updatet to %f", scale);
            return true;
        }
        else {
            me->GetSession()->SendAreaTriggerMessage("scale has to be greater than 0.");
            return false;
        }
    }
    static bool HandleReSetCommand(ChatHandler* handler)
    {
        Player* me = handler->GetSession()->GetPlayer();
        if (!me)
            return false;

        Unit* unit = handler->getSelectedUnit();
        if (!unit || unit->GetTypeId() != TYPEID_UNIT)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        Creature* creature = unit->ToCreature();

        creature->CustomData.GetDefault<Objscale>("scale")->scale = -1.0f;
        float scale = creature->GetCreatureTemplate()->scale;
        creature->SetObjectScale(scale);
        if (!creature->IsPet())
            creature->SaveToDB();

        me->GetSession()->SendAreaTriggerMessage("Creature scale has been reset to %f", scale);
        return true;

    }
    static bool HandleBaseCommand(ChatHandler* handler, char const* args)
    {

        if (!*args)
            return false;

        Player* me = handler->GetSession()->GetPlayer();
        if (!me)
            return false;

        Unit* unit = handler->getSelectedUnit();
        if (!unit || unit->GetTypeId() != TYPEID_UNIT)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        me->GetSession()->SendAreaTriggerMessage("use set or reset command");
        return true;
    }
};

class Objscale_GameObject_command : public CommandScript
{
public:
    Objscale_GameObject_command() : CommandScript("Objscale_GameObject_command") {}
    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> ObjscaleCreatureCommandTable = {

            {"set", SEC_PLAYER, false, &HandleSetCommand, ""},
            {"reset", SEC_PLAYER, false, &HandleReSetCommand, ""},
            { "",           SEC_PLAYER, false, &HandleBaseCommand,        "" }
        };

        static std::vector<ChatCommand> ObjscaleCreatureCommandBaseTable = {
            {"gob_scale", SEC_PLAYER, false, nullptr, "", ObjscaleCreatureCommandTable}
        };

        return ObjscaleCreatureCommandBaseTable;
    }
    static bool HandleSetCommand(ChatHandler* handler, GameObjectSpawnId guidLow, float scale)
    {
        if (!guidLow)
            return false;

        if (!scale)
            return false;

        Player* me = handler->GetSession()->GetPlayer();
        if (!me)
            return false;


        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object) {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, uint32(guidLow));
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (scale > 0.0f) {
            object->CustomData.GetDefault<Objscale>("scale")->scale = scale;

            object->SetObjectScale(scale);
            object->DestroyForNearbyPlayers();
            object->UpdateObjectVisibility();
            object->SaveToDB();

            me->GetSession()->SendAreaTriggerMessage("Object scale has been updatet to %f", scale);
            return true;
        }
        else {
            me->GetSession()->SendAreaTriggerMessage("scale has to be greater than 0.");
            return false;
        }
    }

    static bool HandleReSetCommand(ChatHandler* handler, GameObjectSpawnId guidLow)
    {
        if (!guidLow)
            return false;

        Player* me = handler->GetSession()->GetPlayer();
        if (!me)
            return false;

        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object) {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, uint32(guidLow));
            handler->SetSentErrorMessage(true);
            return false;
        }

        object->CustomData.GetDefault<Objscale>("scale")->scale = -1.0f;

        float scale = object->GetGOInfo()->size;

        object->SetObjectScale(scale);
        object->DestroyForNearbyPlayers();
        object->UpdateObjectVisibility();
        object->SaveToDB();

        me->GetSession()->SendAreaTriggerMessage("Object scale has been reset to %f", scale);
        return true;
    }

    static bool HandleBaseCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* me = handler->GetSession()->GetPlayer();
        if (!me)
            return false;

        me->GetSession()->SendAreaTriggerMessage("use set or reset command");

        return true;
    }
};

// Add all scripts in one
void AddObjscaleScripts()
{
    new CreatureScale();
    new Objscale_Creature_command();
    new GameObjectScale();
    new Objscale_GameObject_command();
}
