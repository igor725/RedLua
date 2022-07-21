#pragma once

#include "native\types.hpp"

NativeTypeMap NativeTypes {
	{     NTYPE_UNKNOWN, {  NTYPE_UNKNOWN,  0, "UnknownType"}},
	{        NTYPE_VOID, {     NTYPE_VOID,  0, "void"       }},
	{         NTYPE_INT, {     NTYPE_VOID,  4, "int"        }},
	{       NTYPE_FLOAT, {     NTYPE_VOID,  4, "float"      }},
	{        NTYPE_BOOL, {      NTYPE_INT,  4, "BOOL"       }},
	{        NTYPE_CHAR, {     NTYPE_VOID,  1, "char"       }},
	{         NTYPE_ANY, {      NTYPE_INT,  0, "Any"        }},
	{        NTYPE_BLIP, {     NTYPE_VOID,  4, "Blip"       }},
	{         NTYPE_CAM, {     NTYPE_VOID,  4, "Cam"        }},
	{      NTYPE_ENTITY, {     NTYPE_VOID,  4, "Entity"     }},
	{      NTYPE_FIREID, {     NTYPE_VOID,  4, "FireId"     }},
	{        NTYPE_HASH, {      NTYPE_INT,  4, "Hash"       }},
	{    NTYPE_INTERIOR, {     NTYPE_VOID,  4, "Interior"   }},
	{     NTYPE_ITEMSET, {     NTYPE_VOID,  4, "ItemSet"    }},
	{      NTYPE_OBJECT, {   NTYPE_ENTITY,  4, "Object"     }},
	{         NTYPE_PED, {   NTYPE_ENTITY,  4, "Ped"        }},
	{      NTYPE_PICKUP, {     NTYPE_VOID,  4, "Pickup"     }},
	{      NTYPE_PLAYER, {     NTYPE_VOID,  4, "Player"     }},
	{   NTYPE_SCRHANDLE, {     NTYPE_VOID,  4, "ScrHandle"  }},
	{     NTYPE_VECTOR3, {     NTYPE_VOID, 24, "Vector3"    }},
	{     NTYPE_VEHICLE, {   NTYPE_ENTITY,  4, "Vehicle"    }},
	{   NTYPE_ANIMSCENE, {     NTYPE_VOID,  4, "AnimScene"  }},
	{    NTYPE_PERSCHAR, {     NTYPE_VOID,  4, "PersChar"   }},
	{     NTYPE_POPZONE, {     NTYPE_VOID,  4, "PopZone"    }},
	{      NTYPE_PROMPT, {     NTYPE_VOID,  4, "Prompt"     }},
	{     NTYPE_PROPSET, {     NTYPE_VOID,  4, "PropSet"    }},
	{      NTYPE_VOLUME, {     NTYPE_VOID,  4, "Volume"     }}
};
