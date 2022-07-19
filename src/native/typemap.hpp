#pragma once

#include "native\types.hpp"

NativeTypeMap NativeTypes {
	{     NTYPE_UNKNOWN, {  NTYPE_UNKNOWN, false,  0, "UnknownType"}},
	{        NTYPE_VOID, {     NTYPE_VOID, false,  0, "void"       }},
	{         NTYPE_INT, {     NTYPE_VOID, false,  4, "int"        }},
	{      NTYPE_INTPTR, {     NTYPE_VOID,  true,  8, "int*"       }},
	{       NTYPE_FLOAT, {     NTYPE_VOID, false,  4, "float"      }},
	{    NTYPE_FLOATPTR, {     NTYPE_VOID,  true,  8, "float*"     }},
	{        NTYPE_BOOL, {     NTYPE_VOID, false,  4, "BOOL"       }},
	{     NTYPE_BOOLPTR, {     NTYPE_VOID,  true,  8, "BOOL*"      }},
	{     NTYPE_CHARPTR, {     NTYPE_VOID,  true,  8, "char*"      }},
	{      NTYPE_STRING, {     NTYPE_VOID,  true,  8, "const char*"}},
	{         NTYPE_ANY, {     NTYPE_VOID, false,  0, "Any"        }},
	{      NTYPE_ANYPTR, {     NTYPE_VOID,  true,  8, "Any*"       }},
	{        NTYPE_BLIP, {     NTYPE_VOID, false,  4, "Blip"       }},
	{     NTYPE_BLIPPTR, {     NTYPE_VOID,  true,  8, "Blip*"      }},
	{         NTYPE_CAM, {     NTYPE_VOID, false,  4, "Cam"        }},
	{      NTYPE_CAMPTR, {     NTYPE_VOID,  true,  8, "Cam*"       }},
	{      NTYPE_ENTITY, {     NTYPE_VOID, false,  4, "Entity"     }},
	{   NTYPE_ENTITYPTR, {     NTYPE_VOID,  true,  8, "Entity*"    }},
	{      NTYPE_FIREID, {     NTYPE_VOID, false,  4, "FireId"     }},
	{   NTYPE_FIREIDPTR, {     NTYPE_VOID,  true,  8, "FireId*"    }},
	{        NTYPE_HASH, {      NTYPE_INT, false,  4, "Hash"       }},
	{     NTYPE_HASHPTR, {   NTYPE_INTPTR,  true,  8, "Hash*"      }},
	{    NTYPE_INTERIOR, {     NTYPE_VOID, false,  4, "Interior"   }},
	{ NTYPE_INTERIORPTR, {     NTYPE_VOID,  true,  8, "Interior*"  }},
	{     NTYPE_ITEMSET, {     NTYPE_VOID, false,  4, "ItemSet"    }},
	{  NTYPE_ITEMSETPTR, {     NTYPE_VOID,  true,  8, "ItemSet*"   }},
	{      NTYPE_OBJECT, {   NTYPE_ENTITY, false,  4, "Object"     }},
	{   NTYPE_OBJECTPTR, {NTYPE_ENTITYPTR,  true,  8, "Object*"    }},
	{         NTYPE_PED, {   NTYPE_ENTITY, false,  4, "Ped"        }},
	{      NTYPE_PEDPTR, {NTYPE_ENTITYPTR,  true,  8, "Ped*"       }},
	{      NTYPE_PICKUP, {     NTYPE_VOID, false,  4, "Pickup"     }},
	{   NTYPE_PICKUPPTR, {     NTYPE_VOID,  true,  8, "Pickup*"    }},
	{      NTYPE_PLAYER, {     NTYPE_VOID, false,  4, "Player"     }},
	{   NTYPE_PLAYERPTR, {     NTYPE_VOID,  true,  8, "Player*"    }},
	{   NTYPE_SCRHANDLE, {     NTYPE_VOID, false,  4, "ScrHandle"  }},
	{NTYPE_SCRHANDLEPTR, {     NTYPE_VOID,  true,  8, "ScrHandle*" }},
	{     NTYPE_VECTOR3, {     NTYPE_VOID, false, 24, "Vector3"    }},
	{  NTYPE_VECTOR3PTR, {     NTYPE_VOID,  true,  8, "Vector3*"   }},
	{     NTYPE_VEHICLE, {   NTYPE_ENTITY, false,  4, "Vehicle"    }},
	{  NTYPE_VEHICLEPTR, {NTYPE_ENTITYPTR,  true,  8, "Vehicle*"   }},
	{   NTYPE_ANIMSCENE, {     NTYPE_VOID, false,  4, "AnimScene"  }},
	{NTYPE_ANIMSCENEPTR, {     NTYPE_VOID,  true,  8, "AnimScene*" }},
	{    NTYPE_PERSCHAR, {     NTYPE_VOID, false,  4, "PersChar"   }},
	{ NTYPE_PERSCHARPTR, {     NTYPE_VOID,  true,  8, "PersChar*"  }},
	{     NTYPE_POPZONE, {     NTYPE_VOID, false,  4, "PopZone"    }},
	{  NTYPE_POPZONEPTR, {     NTYPE_VOID,  true,  8, "PopZone*"   }},
	{      NTYPE_PROMPT, {     NTYPE_VOID, false,  4, "Prompt"     }},
	{   NTYPE_PROMPTPTR, {     NTYPE_VOID,  true,  8, "Prompt*"    }},
	{     NTYPE_PROPSET, {     NTYPE_VOID, false,  4, "PropSet"    }},
	{  NTYPE_PROPSETPTR, {     NTYPE_VOID,  true,  8, "PropSet*"   }},
	{      NTYPE_VOLUME, {     NTYPE_VOID, false,  4, "Volume"     }},
	{   NTYPE_VOLUMEPTR, {     NTYPE_VOID,  true,  8, "Volume*"    }}
};
