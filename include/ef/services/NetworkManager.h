#ifndef __EF_SERVICES_NETWORKMANAGER_H__
#define __EF_SERVICES_NETWORKMANAGER_H__

#include <ef/type.h>
#include <ef/vector.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/os.h>

typedef struct org_freedesktop_NetworkManager_Device_StateReason{
	uint32_t StateReason_u0; /**< uu)*/
	uint32_t StateReason_u1; /**< u)*/
}org_freedesktop_NetworkManager_Device_StateReason_s;

typedef struct org_freedesktop_NetworkManager_IP6Config_Addresses{
	uint8_t* vAddresses_y0; /**< yuay)*/
	uint32_t Addresses_u1; /**< uay)*/
	uint8_t* vAddresses_y2; /**< y)*/
}org_freedesktop_NetworkManager_IP6Config_Addresses_s;

typedef struct org_freedesktop_NetworkManager_IP6Config_Routes{
	uint8_t* vRoutes_y0; /**< yuayu)*/
	uint32_t Routes_u1; /**< uayu)*/
	uint8_t* vRoutes_y2; /**< yu)*/
	uint32_t Routes_u3; /**< u)*/
}org_freedesktop_NetworkManager_IP6Config_Routes_s;


/**************************************/
/*** org_freedesktop_NetworkManager ***/
/**************************************/

/** org_freedesktop_NetworkManager_Reload
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_flags u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Reload(sdbus_s* sd, uint32_t in_flags);

/** org_freedesktop_NetworkManager_GetDevices
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vout_devices ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_GetDevices(sdbus_s* sd, char*** vout_devices);

/** org_freedesktop_NetworkManager_GetAllDevices
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vout_devices ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_GetAllDevices(sdbus_s* sd, char*** vout_devices);

/** org_freedesktop_NetworkManager_GetDeviceByIpIface
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_iface s
 * @param out_device o
*/
sdbusMessage_h org_freedesktop_NetworkManager_GetDeviceByIpIface(sdbus_s* sd, char* in_iface, char** out_device);

/** org_freedesktop_NetworkManager_ActivateConnection
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_connection o
 * @param in_device o
 * @param in_specific_object o
 * @param out_active_connection o
*/
sdbusMessage_h org_freedesktop_NetworkManager_ActivateConnection(sdbus_s* sd, char* in_connection, char* in_device, char* in_specific_object, char** out_active_connection);

/** org_freedesktop_NetworkManager_AddAndActivateConnection
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_connection a{sa{sv}}
 * @param in_device o
 * @param in_specific_object o
 * @param out_path o
 * @param out_active_connection o
*/
sdbusMessage_h org_freedesktop_NetworkManager_AddAndActivateConnection(sdbus_s* sd, sdbusKV_s* vin_connection, char* in_device, char* in_specific_object, char** out_path, char** out_active_connection);

/** org_freedesktop_NetworkManager_AddAndActivateConnection2
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_connection a{sa{sv}}
 * @param in_device o
 * @param in_specific_object o
 * @param vin_options a{sv}
 * @param out_path o
 * @param out_active_connection o
 * @param vout_result a{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_AddAndActivateConnection2(sdbus_s* sd, sdbusKV_s* vin_connection, char* in_device, char* in_specific_object, sdbusKV_s* vin_options, char** out_path, char** out_active_connection, sdbusKV_s** vout_result);

/** org_freedesktop_NetworkManager_DeactivateConnection
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_active_connection o
*/
sdbusMessage_h org_freedesktop_NetworkManager_DeactivateConnection(sdbus_s* sd, char* in_active_connection);

/** org_freedesktop_NetworkManager_Sleep
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_sleep b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Sleep(sdbus_s* sd, int in_sleep);

/** org_freedesktop_NetworkManager_Enable
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_enable b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Enable(sdbus_s* sd, int in_enable);

/** org_freedesktop_NetworkManager_GetPermissions
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vout_permissions a{ss}
*/
sdbusMessage_h org_freedesktop_NetworkManager_GetPermissions(sdbus_s* sd, sdbusKV_s** vout_permissions);

/** org_freedesktop_NetworkManager_SetLogging
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_level s
 * @param in_domains s
*/
sdbusMessage_h org_freedesktop_NetworkManager_SetLogging(sdbus_s* sd, char* in_level, char* in_domains);

/** org_freedesktop_NetworkManager_GetLogging
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param out_level s
 * @param out_domains s
*/
sdbusMessage_h org_freedesktop_NetworkManager_GetLogging(sdbus_s* sd, char** out_level, char** out_domains);

/** org_freedesktop_NetworkManager_CheckConnectivity
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param out_connectivity u
*/
sdbusMessage_h org_freedesktop_NetworkManager_CheckConnectivity(sdbus_s* sd, uint32_t* out_connectivity);

/** org_freedesktop_NetworkManager_state
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param out_state u
*/
sdbusMessage_h org_freedesktop_NetworkManager_state(sdbus_s* sd, uint32_t* out_state);

/** org_freedesktop_NetworkManager_CheckpointCreate
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_devices ao
 * @param in_rollback_timeout u
 * @param in_flags u
 * @param out_checkpoint o
*/
sdbusMessage_h org_freedesktop_NetworkManager_CheckpointCreate(sdbus_s* sd, char** vin_devices, uint32_t in_rollback_timeout, uint32_t in_flags, char** out_checkpoint);

/** org_freedesktop_NetworkManager_CheckpointDestroy
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_checkpoint o
*/
sdbusMessage_h org_freedesktop_NetworkManager_CheckpointDestroy(sdbus_s* sd, char* in_checkpoint);

/** org_freedesktop_NetworkManager_CheckpointRollback
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_checkpoint o
 * @param vout_result a{su}
*/
sdbusMessage_h org_freedesktop_NetworkManager_CheckpointRollback(sdbus_s* sd, char* in_checkpoint, sdbusKV_s** vout_result);

/** org_freedesktop_NetworkManager_CheckpointAdjustRollbackTimeout
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_checkpoint o
 * @param in_add_timeout u
*/
sdbusMessage_h org_freedesktop_NetworkManager_CheckpointAdjustRollbackTimeout(sdbus_s* sd, char* in_checkpoint, uint32_t in_add_timeout);

/** property org_freedesktop_NetworkManager_Devices_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Devices ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_Devices_get(sdbus_s* sd, char*** Devices);

/** property org_freedesktop_NetworkManager_AllDevices_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param AllDevices ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_AllDevices_get(sdbus_s* sd, char*** AllDevices);

/** property org_freedesktop_NetworkManager_Checkpoints_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Checkpoints ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_Checkpoints_get(sdbus_s* sd, char*** Checkpoints);

/** property org_freedesktop_NetworkManager_NetworkingEnabled_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param NetworkingEnabled b
*/
sdbusMessage_h org_freedesktop_NetworkManager_NetworkingEnabled_get(sdbus_s* sd, int* NetworkingEnabled);

/** property org_freedesktop_NetworkManager_WirelessEnabled_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param WirelessEnabled b
*/
sdbusMessage_h org_freedesktop_NetworkManager_WirelessEnabled_get(sdbus_s* sd, int* WirelessEnabled);

/** property org_freedesktop_NetworkManager_WirelessEnabled_set
 * @return 0 successfull -1 error
 * @param sd sd-bus object
 * @param WirelessEnabled b
*/
err_t org_freedesktop_NetworkManager_WirelessEnabled_set(sdbus_s* sd, int WirelessEnabled);

/** property org_freedesktop_NetworkManager_WirelessHardwareEnabled_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param WirelessHardwareEnabled b
*/
sdbusMessage_h org_freedesktop_NetworkManager_WirelessHardwareEnabled_get(sdbus_s* sd, int* WirelessHardwareEnabled);

/** property org_freedesktop_NetworkManager_WwanEnabled_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param WwanEnabled b
*/
sdbusMessage_h org_freedesktop_NetworkManager_WwanEnabled_get(sdbus_s* sd, int* WwanEnabled);

/** property org_freedesktop_NetworkManager_WwanEnabled_set
 * @return 0 successfull -1 error
 * @param sd sd-bus object
 * @param WwanEnabled b
*/
err_t org_freedesktop_NetworkManager_WwanEnabled_set(sdbus_s* sd, int WwanEnabled);

/** property org_freedesktop_NetworkManager_WwanHardwareEnabled_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param WwanHardwareEnabled b
*/
sdbusMessage_h org_freedesktop_NetworkManager_WwanHardwareEnabled_get(sdbus_s* sd, int* WwanHardwareEnabled);

/** property org_freedesktop_NetworkManager_WimaxEnabled_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param WimaxEnabled b
*/
sdbusMessage_h org_freedesktop_NetworkManager_WimaxEnabled_get(sdbus_s* sd, int* WimaxEnabled);

/** property org_freedesktop_NetworkManager_WimaxEnabled_set
 * @return 0 successfull -1 error
 * @param sd sd-bus object
 * @param WimaxEnabled b
*/
err_t org_freedesktop_NetworkManager_WimaxEnabled_set(sdbus_s* sd, int WimaxEnabled);

/** property org_freedesktop_NetworkManager_WimaxHardwareEnabled_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param WimaxHardwareEnabled b
*/
sdbusMessage_h org_freedesktop_NetworkManager_WimaxHardwareEnabled_get(sdbus_s* sd, int* WimaxHardwareEnabled);

/** property org_freedesktop_NetworkManager_ActiveConnections_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param ActiveConnections ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_ActiveConnections_get(sdbus_s* sd, char*** ActiveConnections);

/** property org_freedesktop_NetworkManager_PrimaryConnection_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param PrimaryConnection o
*/
sdbusMessage_h org_freedesktop_NetworkManager_PrimaryConnection_get(sdbus_s* sd, char** PrimaryConnection);

/** property org_freedesktop_NetworkManager_PrimaryConnectionType_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param PrimaryConnectionType s
*/
sdbusMessage_h org_freedesktop_NetworkManager_PrimaryConnectionType_get(sdbus_s* sd, char** PrimaryConnectionType);

/** property org_freedesktop_NetworkManager_Metered_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Metered u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Metered_get(sdbus_s* sd, uint32_t* Metered);

/** property org_freedesktop_NetworkManager_ActivatingConnection_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param ActivatingConnection o
*/
sdbusMessage_h org_freedesktop_NetworkManager_ActivatingConnection_get(sdbus_s* sd, char** ActivatingConnection);

/** property org_freedesktop_NetworkManager_Startup_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Startup b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Startup_get(sdbus_s* sd, int* Startup);

/** property org_freedesktop_NetworkManager_Version_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Version s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Version_get(sdbus_s* sd, char** Version);

/** property org_freedesktop_NetworkManager_Capabilities_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Capabilities u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Capabilities_get(sdbus_s* sd, uint32_t* Capabilities);

/** property org_freedesktop_NetworkManager_State_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param State u
*/
sdbusMessage_h org_freedesktop_NetworkManager_State_get(sdbus_s* sd, uint32_t* State);

/** property org_freedesktop_NetworkManager_Connectivity_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Connectivity u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connectivity_get(sdbus_s* sd, uint32_t* Connectivity);

/** property org_freedesktop_NetworkManager_ConnectivityCheckAvailable_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param ConnectivityCheckAvailable b
*/
sdbusMessage_h org_freedesktop_NetworkManager_ConnectivityCheckAvailable_get(sdbus_s* sd, int* ConnectivityCheckAvailable);

/** property org_freedesktop_NetworkManager_ConnectivityCheckEnabled_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param ConnectivityCheckEnabled b
*/
sdbusMessage_h org_freedesktop_NetworkManager_ConnectivityCheckEnabled_get(sdbus_s* sd, int* ConnectivityCheckEnabled);

/** property org_freedesktop_NetworkManager_ConnectivityCheckEnabled_set
 * @return 0 successfull -1 error
 * @param sd sd-bus object
 * @param ConnectivityCheckEnabled b
*/
err_t org_freedesktop_NetworkManager_ConnectivityCheckEnabled_set(sdbus_s* sd, int ConnectivityCheckEnabled);

/** property org_freedesktop_NetworkManager_ConnectivityCheckUri_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param ConnectivityCheckUri s
*/
sdbusMessage_h org_freedesktop_NetworkManager_ConnectivityCheckUri_get(sdbus_s* sd, char** ConnectivityCheckUri);

/** property org_freedesktop_NetworkManager_GlobalDnsConfiguration_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param GlobalDnsConfiguration a{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_GlobalDnsConfiguration_get(sdbus_s* sd, sdbusKV_s** GlobalDnsConfiguration);

/** property org_freedesktop_NetworkManager_GlobalDnsConfiguration_set
 * @return 0 successfull -1 error
 * @param sd sd-bus object
 * @param GlobalDnsConfiguration a{sv}
*/
err_t org_freedesktop_NetworkManager_GlobalDnsConfiguration_set(sdbus_s* sd, sdbusKV_s* GlobalDnsConfiguration);

/*************************************************/
/*** org_freedesktop_NetworkManager_DnsManager ***/
/*************************************************/

/** property org_freedesktop_NetworkManager_DnsManager_Mode_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Mode s
*/
sdbusMessage_h org_freedesktop_NetworkManager_DnsManager_Mode_get(sdbus_s* sd, char** Mode);

/** property org_freedesktop_NetworkManager_DnsManager_RcManager_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param RcManager s
*/
sdbusMessage_h org_freedesktop_NetworkManager_DnsManager_RcManager_get(sdbus_s* sd, char** RcManager);

/** property org_freedesktop_NetworkManager_DnsManager_Configuration_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Configuration aa{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_DnsManager_Configuration_get(sdbus_s* sd, sdbusKV_s*** Configuration);

/************************************************/
/*** org_freedesktop_NetworkManager_IP4Config ***/
/************************************************/

/** property org_freedesktop_NetworkManager_IP4Config_Addresses_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Addresses aau
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Addresses_get(sdbus_s* sd, uint32_t*** Addresses);

/** property org_freedesktop_NetworkManager_IP4Config_AddressData_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param AddressData aa{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_AddressData_get(sdbus_s* sd, sdbusKV_s*** AddressData);

/** property org_freedesktop_NetworkManager_IP4Config_Gateway_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Gateway s
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Gateway_get(sdbus_s* sd, char** Gateway);

/** property org_freedesktop_NetworkManager_IP4Config_Routes_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Routes aau
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Routes_get(sdbus_s* sd, uint32_t*** Routes);

/** property org_freedesktop_NetworkManager_IP4Config_RouteData_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param RouteData aa{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_RouteData_get(sdbus_s* sd, sdbusKV_s*** RouteData);

/** property org_freedesktop_NetworkManager_IP4Config_NameserverData_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param NameserverData aa{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_NameserverData_get(sdbus_s* sd, sdbusKV_s*** NameserverData);

/** property org_freedesktop_NetworkManager_IP4Config_Nameservers_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Nameservers au
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Nameservers_get(sdbus_s* sd, uint32_t** Nameservers);

/** property org_freedesktop_NetworkManager_IP4Config_Domains_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Domains as
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Domains_get(sdbus_s* sd, char*** Domains);

/** property org_freedesktop_NetworkManager_IP4Config_Searches_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Searches as
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Searches_get(sdbus_s* sd, char*** Searches);

/** property org_freedesktop_NetworkManager_IP4Config_DnsOptions_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param DnsOptions as
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_DnsOptions_get(sdbus_s* sd, char*** DnsOptions);

/** property org_freedesktop_NetworkManager_IP4Config_DnsPriority_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param DnsPriority i
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_DnsPriority_get(sdbus_s* sd, int* DnsPriority);

/** property org_freedesktop_NetworkManager_IP4Config_WinsServerData_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param WinsServerData as
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_WinsServerData_get(sdbus_s* sd, char*** WinsServerData);

/** property org_freedesktop_NetworkManager_IP4Config_WinsServers_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param WinsServers au
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_WinsServers_get(sdbus_s* sd, uint32_t** WinsServers);

/********************************************************/
/*** org_freedesktop_NetworkManager_Connection_Active ***/
/********************************************************/

/** property org_freedesktop_NetworkManager_Connection_Active_Connection_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Connection o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Connection_get(sdbus_s* sd, char** Connection);

/** property org_freedesktop_NetworkManager_Connection_Active_SpecificObject_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param SpecificObject o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_SpecificObject_get(sdbus_s* sd, char** SpecificObject);

/** property org_freedesktop_NetworkManager_Connection_Active_Id_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Id s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Id_get(sdbus_s* sd, char** Id);

/** property org_freedesktop_NetworkManager_Connection_Active_Uuid_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Uuid s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Uuid_get(sdbus_s* sd, char** Uuid);

/** property org_freedesktop_NetworkManager_Connection_Active_Type_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Type s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Type_get(sdbus_s* sd, char** Type);

/** property org_freedesktop_NetworkManager_Connection_Active_Devices_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Devices ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Devices_get(sdbus_s* sd, char*** Devices);

/** property org_freedesktop_NetworkManager_Connection_Active_State_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param State u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_State_get(sdbus_s* sd, uint32_t* State);

/** property org_freedesktop_NetworkManager_Connection_Active_StateFlags_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param StateFlags u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_StateFlags_get(sdbus_s* sd, uint32_t* StateFlags);

/** property org_freedesktop_NetworkManager_Connection_Active_Default_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Default b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Default_get(sdbus_s* sd, int* Default);

/** property org_freedesktop_NetworkManager_Connection_Active_Ip4Config_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Ip4Config o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Ip4Config_get(sdbus_s* sd, char** Ip4Config);

/** property org_freedesktop_NetworkManager_Connection_Active_Dhcp4Config_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Dhcp4Config o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Dhcp4Config_get(sdbus_s* sd, char** Dhcp4Config);

/** property org_freedesktop_NetworkManager_Connection_Active_Default6_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Default6 b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Default6_get(sdbus_s* sd, int* Default6);

/** property org_freedesktop_NetworkManager_Connection_Active_Ip6Config_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Ip6Config o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Ip6Config_get(sdbus_s* sd, char** Ip6Config);

/** property org_freedesktop_NetworkManager_Connection_Active_Dhcp6Config_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Dhcp6Config o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Dhcp6Config_get(sdbus_s* sd, char** Dhcp6Config);

/** property org_freedesktop_NetworkManager_Connection_Active_Vpn_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Vpn b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Vpn_get(sdbus_s* sd, int* Vpn);

/** property org_freedesktop_NetworkManager_Connection_Active_Master_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Master o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Master_get(sdbus_s* sd, char** Master);

/***************************************************/
/*** org_freedesktop_NetworkManager_AgentManager ***/
/***************************************************/

/** org_freedesktop_NetworkManager_AgentManager_Register
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_identifier s
*/
sdbusMessage_h org_freedesktop_NetworkManager_AgentManager_Register(sdbus_s* sd, char* in_identifier);

/** org_freedesktop_NetworkManager_AgentManager_RegisterWithCapabilities
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_identifier s
 * @param in_capabilities u
*/
sdbusMessage_h org_freedesktop_NetworkManager_AgentManager_RegisterWithCapabilities(sdbus_s* sd, char* in_identifier, uint32_t in_capabilities);

/** org_freedesktop_NetworkManager_AgentManager_Unregister
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
*/
sdbusMessage_h org_freedesktop_NetworkManager_AgentManager_Unregister(sdbus_s* sd);

/**************************************************/
/*** org_freedesktop_NetworkManager_DHCP4Config ***/
/**************************************************/

/** property org_freedesktop_NetworkManager_DHCP4Config_Options_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Options a{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_DHCP4Config_Options_get(sdbus_s* sd, sdbusKV_s** Options);

/********************************************************/
/*** org_freedesktop_NetworkManager_Device_Statistics ***/
/********************************************************/

/** property org_freedesktop_NetworkManager_Device_Statistics_RefreshRateMs_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param RefreshRateMs u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Statistics_RefreshRateMs_get(sdbus_s* sd, uint32_t* RefreshRateMs);

/** property org_freedesktop_NetworkManager_Device_Statistics_RefreshRateMs_set
 * @return 0 successfull -1 error
 * @param sd sd-bus object
 * @param RefreshRateMs u
*/
err_t org_freedesktop_NetworkManager_Device_Statistics_RefreshRateMs_set(sdbus_s* sd, uint32_t RefreshRateMs);

/** property org_freedesktop_NetworkManager_Device_Statistics_TxBytes_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param TxBytes t
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Statistics_TxBytes_get(sdbus_s* sd, uint64_t* TxBytes);

/** property org_freedesktop_NetworkManager_Device_Statistics_RxBytes_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param RxBytes t
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Statistics_RxBytes_get(sdbus_s* sd, uint64_t* RxBytes);

/*********************************************/
/*** org_freedesktop_NetworkManager_Device ***/
/*********************************************/

/** org_freedesktop_NetworkManager_Device_Reapply
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_connection a{sa{sv}}
 * @param in_version_id t
 * @param in_flags u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Reapply(sdbus_s* sd, sdbusKV_s* vin_connection, uint64_t in_version_id, uint32_t in_flags);

/** org_freedesktop_NetworkManager_Device_GetAppliedConnection
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_flags u
 * @param vout_connection a{sa{sv}}
 * @param out_version_id t
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_GetAppliedConnection(sdbus_s* sd, uint32_t in_flags, sdbusKV_s** vout_connection, uint64_t* out_version_id);

/** org_freedesktop_NetworkManager_Device_Disconnect
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Disconnect(sdbus_s* sd);

/** org_freedesktop_NetworkManager_Device_Delete
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Delete(sdbus_s* sd);

/** property org_freedesktop_NetworkManager_Device_Udi_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Udi s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Udi_get(sdbus_s* sd, char** Udi);

/** property org_freedesktop_NetworkManager_Device_Interface_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Interface s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Interface_get(sdbus_s* sd, char** Interface);

/** property org_freedesktop_NetworkManager_Device_IpInterface_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param IpInterface s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_IpInterface_get(sdbus_s* sd, char** IpInterface);

/** property org_freedesktop_NetworkManager_Device_Driver_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Driver s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Driver_get(sdbus_s* sd, char** Driver);

/** property org_freedesktop_NetworkManager_Device_DriverVersion_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param DriverVersion s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_DriverVersion_get(sdbus_s* sd, char** DriverVersion);

/** property org_freedesktop_NetworkManager_Device_FirmwareVersion_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param FirmwareVersion s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_FirmwareVersion_get(sdbus_s* sd, char** FirmwareVersion);

/** property org_freedesktop_NetworkManager_Device_Capabilities_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Capabilities u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Capabilities_get(sdbus_s* sd, uint32_t* Capabilities);

/** property org_freedesktop_NetworkManager_Device_Ip4Address_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Ip4Address u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Ip4Address_get(sdbus_s* sd, uint32_t* Ip4Address);

/** property org_freedesktop_NetworkManager_Device_State_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param State u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_State_get(sdbus_s* sd, uint32_t* State);

/** property org_freedesktop_NetworkManager_Device_StateReason_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param StateReason (uu)
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_StateReason_get(sdbus_s* sd, org_freedesktop_NetworkManager_Device_StateReason_s** StateReason);

/** property org_freedesktop_NetworkManager_Device_ActiveConnection_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param ActiveConnection o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_ActiveConnection_get(sdbus_s* sd, char** ActiveConnection);

/** property org_freedesktop_NetworkManager_Device_Ip4Config_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Ip4Config o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Ip4Config_get(sdbus_s* sd, char** Ip4Config);

/** property org_freedesktop_NetworkManager_Device_Dhcp4Config_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Dhcp4Config o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Dhcp4Config_get(sdbus_s* sd, char** Dhcp4Config);

/** property org_freedesktop_NetworkManager_Device_Ip6Config_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Ip6Config o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Ip6Config_get(sdbus_s* sd, char** Ip6Config);

/** property org_freedesktop_NetworkManager_Device_Dhcp6Config_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Dhcp6Config o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Dhcp6Config_get(sdbus_s* sd, char** Dhcp6Config);

/** property org_freedesktop_NetworkManager_Device_Managed_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Managed b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Managed_get(sdbus_s* sd, int* Managed);

/** property org_freedesktop_NetworkManager_Device_Managed_set
 * @return 0 successfull -1 error
 * @param sd sd-bus object
 * @param Managed b
*/
err_t org_freedesktop_NetworkManager_Device_Managed_set(sdbus_s* sd, int Managed);

/** property org_freedesktop_NetworkManager_Device_Autoconnect_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Autoconnect b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Autoconnect_get(sdbus_s* sd, int* Autoconnect);

/** property org_freedesktop_NetworkManager_Device_Autoconnect_set
 * @return 0 successfull -1 error
 * @param sd sd-bus object
 * @param Autoconnect b
*/
err_t org_freedesktop_NetworkManager_Device_Autoconnect_set(sdbus_s* sd, int Autoconnect);

/** property org_freedesktop_NetworkManager_Device_FirmwareMissing_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param FirmwareMissing b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_FirmwareMissing_get(sdbus_s* sd, int* FirmwareMissing);

/** property org_freedesktop_NetworkManager_Device_NmPluginMissing_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param NmPluginMissing b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_NmPluginMissing_get(sdbus_s* sd, int* NmPluginMissing);

/** property org_freedesktop_NetworkManager_Device_DeviceType_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param DeviceType u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_DeviceType_get(sdbus_s* sd, uint32_t* DeviceType);

/** property org_freedesktop_NetworkManager_Device_AvailableConnections_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param AvailableConnections ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_AvailableConnections_get(sdbus_s* sd, char*** AvailableConnections);

/** property org_freedesktop_NetworkManager_Device_PhysicalPortId_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param PhysicalPortId s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_PhysicalPortId_get(sdbus_s* sd, char** PhysicalPortId);

/** property org_freedesktop_NetworkManager_Device_Mtu_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Mtu u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Mtu_get(sdbus_s* sd, uint32_t* Mtu);

/** property org_freedesktop_NetworkManager_Device_Metered_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Metered u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Metered_get(sdbus_s* sd, uint32_t* Metered);

/** property org_freedesktop_NetworkManager_Device_LldpNeighbors_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param LldpNeighbors aa{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_LldpNeighbors_get(sdbus_s* sd, sdbusKV_s*** LldpNeighbors);

/** property org_freedesktop_NetworkManager_Device_Real_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Real b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Real_get(sdbus_s* sd, int* Real);

/** property org_freedesktop_NetworkManager_Device_Ip4Connectivity_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Ip4Connectivity u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Ip4Connectivity_get(sdbus_s* sd, uint32_t* Ip4Connectivity);

/** property org_freedesktop_NetworkManager_Device_Ip6Connectivity_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Ip6Connectivity u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Ip6Connectivity_get(sdbus_s* sd, uint32_t* Ip6Connectivity);

/** property org_freedesktop_NetworkManager_Device_InterfaceFlags_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param InterfaceFlags u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_InterfaceFlags_get(sdbus_s* sd, uint32_t* InterfaceFlags);

/** property org_freedesktop_NetworkManager_Device_HwAddress_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param HwAddress s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_HwAddress_get(sdbus_s* sd, char** HwAddress);

/***************************************************/
/*** org_freedesktop_NetworkManager_Device_Wired ***/
/***************************************************/

/** property org_freedesktop_NetworkManager_Device_Wired_HwAddress_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param HwAddress s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wired_HwAddress_get(sdbus_s* sd, char** HwAddress);

/** property org_freedesktop_NetworkManager_Device_Wired_PermHwAddress_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param PermHwAddress s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wired_PermHwAddress_get(sdbus_s* sd, char** PermHwAddress);

/** property org_freedesktop_NetworkManager_Device_Wired_Speed_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Speed u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wired_Speed_get(sdbus_s* sd, uint32_t* Speed);

/** property org_freedesktop_NetworkManager_Device_Wired_S390Subchannels_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param S390Subchannels as
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wired_S390Subchannels_get(sdbus_s* sd, char*** S390Subchannels);

/** property org_freedesktop_NetworkManager_Device_Wired_Carrier_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Carrier b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wired_Carrier_get(sdbus_s* sd, int* Carrier);

/******************************************************/
/*** org_freedesktop_NetworkManager_Device_Wireless ***/
/******************************************************/

/** org_freedesktop_NetworkManager_Device_Wireless_GetAccessPoints
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vout_access_points ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_GetAccessPoints(sdbus_s* sd, char*** vout_access_points);

/** org_freedesktop_NetworkManager_Device_Wireless_GetAllAccessPoints
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vout_access_points ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_GetAllAccessPoints(sdbus_s* sd, char*** vout_access_points);

/** org_freedesktop_NetworkManager_Device_Wireless_RequestScan
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_options a{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_RequestScan(sdbus_s* sd, sdbusKV_s* vin_options);

/** property org_freedesktop_NetworkManager_Device_Wireless_HwAddress_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param HwAddress s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_HwAddress_get(sdbus_s* sd, char** HwAddress);

/** property org_freedesktop_NetworkManager_Device_Wireless_PermHwAddress_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param PermHwAddress s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_PermHwAddress_get(sdbus_s* sd, char** PermHwAddress);

/** property org_freedesktop_NetworkManager_Device_Wireless_Mode_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Mode u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_Mode_get(sdbus_s* sd, uint32_t* Mode);

/** property org_freedesktop_NetworkManager_Device_Wireless_Bitrate_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Bitrate u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_Bitrate_get(sdbus_s* sd, uint32_t* Bitrate);

/** property org_freedesktop_NetworkManager_Device_Wireless_AccessPoints_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param AccessPoints ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_AccessPoints_get(sdbus_s* sd, char*** AccessPoints);

/** property org_freedesktop_NetworkManager_Device_Wireless_ActiveAccessPoint_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param ActiveAccessPoint o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_ActiveAccessPoint_get(sdbus_s* sd, char** ActiveAccessPoint);

/** property org_freedesktop_NetworkManager_Device_Wireless_WirelessCapabilities_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param WirelessCapabilities u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_WirelessCapabilities_get(sdbus_s* sd, uint32_t* WirelessCapabilities);

/** property org_freedesktop_NetworkManager_Device_Wireless_LastScan_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param LastScan x
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_LastScan_get(sdbus_s* sd, int64_t* LastScan);

/*****************************************************/
/*** org_freedesktop_NetworkManager_Device_Generic ***/
/*****************************************************/

/** property org_freedesktop_NetworkManager_Device_Generic_HwAddress_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param HwAddress s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Generic_HwAddress_get(sdbus_s* sd, char** HwAddress);

/** property org_freedesktop_NetworkManager_Device_Generic_TypeDescription_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param TypeDescription s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Device_Generic_TypeDescription_get(sdbus_s* sd, char** TypeDescription);

/**************************************************/
/*** org_freedesktop_NetworkManager_AccessPoint ***/
/**************************************************/

/** property org_freedesktop_NetworkManager_AccessPoint_Flags_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Flags u
*/
sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_Flags_get(sdbus_s* sd, uint32_t* Flags);

/** property org_freedesktop_NetworkManager_AccessPoint_WpaFlags_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param WpaFlags u
*/
sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_WpaFlags_get(sdbus_s* sd, uint32_t* WpaFlags);

/** property org_freedesktop_NetworkManager_AccessPoint_RsnFlags_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param RsnFlags u
*/
sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_RsnFlags_get(sdbus_s* sd, uint32_t* RsnFlags);

/** property org_freedesktop_NetworkManager_AccessPoint_Ssid_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Ssid ay
*/
sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_Ssid_get(sdbus_s* sd, uint8_t** Ssid);

/** property org_freedesktop_NetworkManager_AccessPoint_Frequency_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Frequency u
*/
sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_Frequency_get(sdbus_s* sd, uint32_t* Frequency);

/** property org_freedesktop_NetworkManager_AccessPoint_HwAddress_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param HwAddress s
*/
sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_HwAddress_get(sdbus_s* sd, char** HwAddress);

/** property org_freedesktop_NetworkManager_AccessPoint_Mode_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Mode u
*/
sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_Mode_get(sdbus_s* sd, uint32_t* Mode);

/** property org_freedesktop_NetworkManager_AccessPoint_MaxBitrate_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param MaxBitrate u
*/
sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_MaxBitrate_get(sdbus_s* sd, uint32_t* MaxBitrate);

/** property org_freedesktop_NetworkManager_AccessPoint_Strength_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Strength y
*/
sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_Strength_get(sdbus_s* sd, uint8_t* Strength);

/** property org_freedesktop_NetworkManager_AccessPoint_LastSeen_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param LastSeen i
*/
sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_LastSeen_get(sdbus_s* sd, int* LastSeen);

/************************************************/
/*** org_freedesktop_NetworkManager_IP6Config ***/
/************************************************/

/** property org_freedesktop_NetworkManager_IP6Config_Addresses_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Addresses a(ayuay)
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Addresses_get(sdbus_s* sd, org_freedesktop_NetworkManager_IP6Config_Addresses_s*** Addresses);

/** property org_freedesktop_NetworkManager_IP6Config_AddressData_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param AddressData aa{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_AddressData_get(sdbus_s* sd, sdbusKV_s*** AddressData);

/** property org_freedesktop_NetworkManager_IP6Config_Gateway_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Gateway s
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Gateway_get(sdbus_s* sd, char** Gateway);

/** property org_freedesktop_NetworkManager_IP6Config_Routes_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Routes a(ayuayu)
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Routes_get(sdbus_s* sd, org_freedesktop_NetworkManager_IP6Config_Routes_s*** Routes);

/** property org_freedesktop_NetworkManager_IP6Config_RouteData_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param RouteData aa{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_RouteData_get(sdbus_s* sd, sdbusKV_s*** RouteData);

/** property org_freedesktop_NetworkManager_IP6Config_Nameservers_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Nameservers aay
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Nameservers_get(sdbus_s* sd, uint8_t*** Nameservers);

/** property org_freedesktop_NetworkManager_IP6Config_Domains_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Domains as
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Domains_get(sdbus_s* sd, char*** Domains);

/** property org_freedesktop_NetworkManager_IP6Config_Searches_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Searches as
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Searches_get(sdbus_s* sd, char*** Searches);

/** property org_freedesktop_NetworkManager_IP6Config_DnsOptions_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param DnsOptions as
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_DnsOptions_get(sdbus_s* sd, char*** DnsOptions);

/** property org_freedesktop_NetworkManager_IP6Config_DnsPriority_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param DnsPriority i
*/
sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_DnsPriority_get(sdbus_s* sd, int* DnsPriority);

/***********************************************/
/*** org_freedesktop_NetworkManager_Settings ***/
/***********************************************/

/** org_freedesktop_NetworkManager_Settings_ListConnections
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vout_connections ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_ListConnections(sdbus_s* sd, char*** vout_connections);

/** org_freedesktop_NetworkManager_Settings_GetConnectionByUuid
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_uuid s
 * @param out_connection o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_GetConnectionByUuid(sdbus_s* sd, char* in_uuid, char** out_connection);

/** org_freedesktop_NetworkManager_Settings_AddConnection
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_connection a{sa{sv}}
 * @param out_path o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_AddConnection(sdbus_s* sd, sdbusKV_s* vin_connection, char** out_path);

/** org_freedesktop_NetworkManager_Settings_AddConnectionUnsaved
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_connection a{sa{sv}}
 * @param out_path o
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_AddConnectionUnsaved(sdbus_s* sd, sdbusKV_s* vin_connection, char** out_path);

/** org_freedesktop_NetworkManager_Settings_AddConnection2
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_settings a{sa{sv}}
 * @param in_flags u
 * @param vin_args a{sv}
 * @param out_path o
 * @param vout_result a{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_AddConnection2(sdbus_s* sd, sdbusKV_s* vin_settings, uint32_t in_flags, sdbusKV_s* vin_args, char** out_path, sdbusKV_s** vout_result);

/** org_freedesktop_NetworkManager_Settings_LoadConnections
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_filenames as
 * @param out_status b
 * @param vout_failures as
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_LoadConnections(sdbus_s* sd, char** vin_filenames, int* out_status, char*** vout_failures);

/** org_freedesktop_NetworkManager_Settings_ReloadConnections
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param out_status b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_ReloadConnections(sdbus_s* sd, int* out_status);

/** org_freedesktop_NetworkManager_Settings_SaveHostname
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_hostname s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_SaveHostname(sdbus_s* sd, char* in_hostname);

/** property org_freedesktop_NetworkManager_Settings_Connections_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Connections ao
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connections_get(sdbus_s* sd, char*** Connections);

/** property org_freedesktop_NetworkManager_Settings_Hostname_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Hostname s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Hostname_get(sdbus_s* sd, char** Hostname);

/** property org_freedesktop_NetworkManager_Settings_CanModify_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param CanModify b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_CanModify_get(sdbus_s* sd, int* CanModify);

/**********************************************************/
/*** org_freedesktop_NetworkManager_Settings_Connection ***/
/**********************************************************/

/** org_freedesktop_NetworkManager_Settings_Connection_Update
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_properties a{sa{sv}}
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Update(sdbus_s* sd, sdbusKV_s* vin_properties);

/** org_freedesktop_NetworkManager_Settings_Connection_UpdateUnsaved
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_properties a{sa{sv}}
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_UpdateUnsaved(sdbus_s* sd, sdbusKV_s* vin_properties);

/** org_freedesktop_NetworkManager_Settings_Connection_Delete
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Delete(sdbus_s* sd);

/** org_freedesktop_NetworkManager_Settings_Connection_GetSettings
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vout_settings a{sa{sv}}
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_GetSettings(sdbus_s* sd, sdbusKV_s** vout_settings);

/** org_freedesktop_NetworkManager_Settings_Connection_GetSecrets
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param in_setting_name s
 * @param vout_secrets a{sa{sv}}
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_GetSecrets(sdbus_s* sd, char* in_setting_name, sdbusKV_s** vout_secrets);

/** org_freedesktop_NetworkManager_Settings_Connection_ClearSecrets
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_ClearSecrets(sdbus_s* sd);

/** org_freedesktop_NetworkManager_Settings_Connection_Save
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Save(sdbus_s* sd);

/** org_freedesktop_NetworkManager_Settings_Connection_Update2
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param vin_settings a{sa{sv}}
 * @param in_flags u
 * @param vin_args a{sv}
 * @param vout_result a{sv}
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Update2(sdbus_s* sd, sdbusKV_s* vin_settings, uint32_t in_flags, sdbusKV_s* vin_args, sdbusKV_s** vout_result);

/** property org_freedesktop_NetworkManager_Settings_Connection_Unsaved_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Unsaved b
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Unsaved_get(sdbus_s* sd, int* Unsaved);

/** property org_freedesktop_NetworkManager_Settings_Connection_Flags_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Flags u
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Flags_get(sdbus_s* sd, uint32_t* Flags);

/** property org_freedesktop_NetworkManager_Settings_Connection_Filename_get
 * @return msg or NULL for error, need to sdbus_message_free
 * @param sd sd-bus object
 * @param Filename s
*/
sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Filename_get(sdbus_s* sd, char** Filename);


#endif
