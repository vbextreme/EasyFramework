#include <ef/services/NetworkManager.h>

/**************************************/
/*** org_freedesktop_NetworkManager ***/
/**************************************/

sdbusMessage_h org_freedesktop_NetworkManager_Reload(sdbus_s* sd, uint32_t in_flags){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "Reload",
		"u",
		in_flags
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_GetDevices(sdbus_s* sd, char*** vout_devices){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "GetDevices",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "ao",
		vout_devices)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_GetAllDevices(sdbus_s* sd, char*** vout_devices){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "GetAllDevices",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "ao",
		vout_devices)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_GetDeviceByIpIface(sdbus_s* sd, char* in_iface, char** out_device){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "GetDeviceByIpIface",
		"s",
		in_iface
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "o",
		out_device)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_ActivateConnection(sdbus_s* sd, char* in_connection, char* in_device, char* in_specific_object, char** out_active_connection){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "ActivateConnection",
		"ooo",
		in_connection,
		in_device,
		in_specific_object
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "o",
		out_active_connection)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AddAndActivateConnection(sdbus_s* sd, sdbusKV_s* vin_connection, char* in_device, char* in_specific_object, char** out_path, char** out_active_connection){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "AddAndActivateConnection",
		"a{sa{sv}}oo",
		vin_connection,
		in_device,
		in_specific_object
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "o",
		out_path)
	){
		sdbus_message_free(msg);
		return NULL;
	}
	if( sdbus_message_read(msg, "o",
		out_active_connection)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AddAndActivateConnection2(sdbus_s* sd, sdbusKV_s* vin_connection, char* in_device, char* in_specific_object, sdbusKV_s* vin_options, char** out_path, char** out_active_connection, sdbusKV_s** vout_result){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "AddAndActivateConnection2",
		"a{sa{sv}}ooa{sv}",
		vin_connection,
		in_device,
		in_specific_object,
		vin_options
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "o",
		out_path)
	){
		sdbus_message_free(msg);
		return NULL;
	}
	if( sdbus_message_read(msg, "o",
		out_active_connection)
	){
		sdbus_message_free(msg);
		return NULL;
	}
	if( sdbus_message_read(msg, "a{sv}",
		vout_result)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_DeactivateConnection(sdbus_s* sd, char* in_active_connection){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "DeactivateConnection",
		"o",
		in_active_connection
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Sleep(sdbus_s* sd, int in_sleep){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "Sleep",
		"b",
		in_sleep
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Enable(sdbus_s* sd, int in_enable){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "Enable",
		"b",
		in_enable
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_GetPermissions(sdbus_s* sd, sdbusKV_s** vout_permissions){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "GetPermissions",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "a{ss}",
		vout_permissions)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_SetLogging(sdbus_s* sd, char* in_level, char* in_domains){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "SetLogging",
		"ss",
		in_level,
		in_domains
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_GetLogging(sdbus_s* sd, char** out_level, char** out_domains){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "GetLogging",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "s",
		out_level)
	){
		sdbus_message_free(msg);
		return NULL;
	}
	if( sdbus_message_read(msg, "s",
		out_domains)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_CheckConnectivity(sdbus_s* sd, uint32_t* out_connectivity){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "CheckConnectivity",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "u",
		out_connectivity)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_state(sdbus_s* sd, uint32_t* out_state){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "state",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "u",
		out_state)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_CheckpointCreate(sdbus_s* sd, char** vin_devices, uint32_t in_rollback_timeout, uint32_t in_flags, char** out_checkpoint){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "CheckpointCreate",
		"aouu",
		vin_devices,
		in_rollback_timeout,
		in_flags
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "o",
		out_checkpoint)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_CheckpointDestroy(sdbus_s* sd, char* in_checkpoint){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "CheckpointDestroy",
		"o",
		in_checkpoint
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_CheckpointRollback(sdbus_s* sd, char* in_checkpoint, sdbusKV_s** vout_result){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "CheckpointRollback",
		"o",
		in_checkpoint
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "a{su}",
		vout_result)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_CheckpointAdjustRollbackTimeout(sdbus_s* sd, char* in_checkpoint, uint32_t in_add_timeout){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "CheckpointAdjustRollbackTimeout",
		"ou",
		in_checkpoint,
		in_add_timeout
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Devices_get(sdbus_s* sd, char*** Devices){
	sdbusVariant_s tmpDevices;
	tmpDevices.type = "ao";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "Devices",
		&tmpDevices
	);
	if( !msg ){
		return NULL;
	}

	*Devices = tmpDevices.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AllDevices_get(sdbus_s* sd, char*** AllDevices){
	sdbusVariant_s tmpAllDevices;
	tmpAllDevices.type = "ao";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "AllDevices",
		&tmpAllDevices
	);
	if( !msg ){
		return NULL;
	}

	*AllDevices = tmpAllDevices.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Checkpoints_get(sdbus_s* sd, char*** Checkpoints){
	sdbusVariant_s tmpCheckpoints;
	tmpCheckpoints.type = "ao";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "Checkpoints",
		&tmpCheckpoints
	);
	if( !msg ){
		return NULL;
	}

	*Checkpoints = tmpCheckpoints.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_NetworkingEnabled_get(sdbus_s* sd, int* NetworkingEnabled){
	sdbusVariant_s tmpNetworkingEnabled;
	tmpNetworkingEnabled.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "NetworkingEnabled",
		&tmpNetworkingEnabled
	);
	if( !msg ){
		return NULL;
	}

	*NetworkingEnabled = tmpNetworkingEnabled.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_WirelessEnabled_get(sdbus_s* sd, int* WirelessEnabled){
	sdbusVariant_s tmpWirelessEnabled;
	tmpWirelessEnabled.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "WirelessEnabled",
		&tmpWirelessEnabled
	);
	if( !msg ){
		return NULL;
	}

	*WirelessEnabled = tmpWirelessEnabled.var.b;

	return msg;
};

err_t org_freedesktop_NetworkManager_WirelessEnabled_set(sdbus_s* sd, int WirelessEnabled){
	sdbusVariant_s tmpWirelessEnabled;
	tmpWirelessEnabled.type = "b";
	tmpWirelessEnabled.var.b = WirelessEnabled;
	if( sdbus_property_set(
			sd,
			"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager", "WirelessEnabled", 
			&tmpWirelessEnabled
		)
	){
		return -1;
	}

	return 0;
};

sdbusMessage_h org_freedesktop_NetworkManager_WirelessHardwareEnabled_get(sdbus_s* sd, int* WirelessHardwareEnabled){
	sdbusVariant_s tmpWirelessHardwareEnabled;
	tmpWirelessHardwareEnabled.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "WirelessHardwareEnabled",
		&tmpWirelessHardwareEnabled
	);
	if( !msg ){
		return NULL;
	}

	*WirelessHardwareEnabled = tmpWirelessHardwareEnabled.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_WwanEnabled_get(sdbus_s* sd, int* WwanEnabled){
	sdbusVariant_s tmpWwanEnabled;
	tmpWwanEnabled.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "WwanEnabled",
		&tmpWwanEnabled
	);
	if( !msg ){
		return NULL;
	}

	*WwanEnabled = tmpWwanEnabled.var.b;

	return msg;
};

err_t org_freedesktop_NetworkManager_WwanEnabled_set(sdbus_s* sd, int WwanEnabled){
	sdbusVariant_s tmpWwanEnabled;
	tmpWwanEnabled.type = "b";
	tmpWwanEnabled.var.b = WwanEnabled;
	if( sdbus_property_set(
			sd,
			"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager", "WwanEnabled", 
			&tmpWwanEnabled
		)
	){
		return -1;
	}

	return 0;
};

sdbusMessage_h org_freedesktop_NetworkManager_WwanHardwareEnabled_get(sdbus_s* sd, int* WwanHardwareEnabled){
	sdbusVariant_s tmpWwanHardwareEnabled;
	tmpWwanHardwareEnabled.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "WwanHardwareEnabled",
		&tmpWwanHardwareEnabled
	);
	if( !msg ){
		return NULL;
	}

	*WwanHardwareEnabled = tmpWwanHardwareEnabled.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_WimaxEnabled_get(sdbus_s* sd, int* WimaxEnabled){
	sdbusVariant_s tmpWimaxEnabled;
	tmpWimaxEnabled.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "WimaxEnabled",
		&tmpWimaxEnabled
	);
	if( !msg ){
		return NULL;
	}

	*WimaxEnabled = tmpWimaxEnabled.var.b;

	return msg;
};

err_t org_freedesktop_NetworkManager_WimaxEnabled_set(sdbus_s* sd, int WimaxEnabled){
	sdbusVariant_s tmpWimaxEnabled;
	tmpWimaxEnabled.type = "b";
	tmpWimaxEnabled.var.b = WimaxEnabled;
	if( sdbus_property_set(
			sd,
			"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager", "WimaxEnabled", 
			&tmpWimaxEnabled
		)
	){
		return -1;
	}

	return 0;
};

sdbusMessage_h org_freedesktop_NetworkManager_WimaxHardwareEnabled_get(sdbus_s* sd, int* WimaxHardwareEnabled){
	sdbusVariant_s tmpWimaxHardwareEnabled;
	tmpWimaxHardwareEnabled.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "WimaxHardwareEnabled",
		&tmpWimaxHardwareEnabled
	);
	if( !msg ){
		return NULL;
	}

	*WimaxHardwareEnabled = tmpWimaxHardwareEnabled.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_ActiveConnections_get(sdbus_s* sd, char*** ActiveConnections){
	sdbusVariant_s tmpActiveConnections;
	tmpActiveConnections.type = "ao";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "ActiveConnections",
		&tmpActiveConnections
	);
	if( !msg ){
		return NULL;
	}

	*ActiveConnections = tmpActiveConnections.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_PrimaryConnection_get(sdbus_s* sd, char** PrimaryConnection){
	sdbusVariant_s tmpPrimaryConnection;
	tmpPrimaryConnection.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "PrimaryConnection",
		&tmpPrimaryConnection
	);
	if( !msg ){
		return NULL;
	}

	*PrimaryConnection = tmpPrimaryConnection.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_PrimaryConnectionType_get(sdbus_s* sd, char** PrimaryConnectionType){
	sdbusVariant_s tmpPrimaryConnectionType;
	tmpPrimaryConnectionType.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "PrimaryConnectionType",
		&tmpPrimaryConnectionType
	);
	if( !msg ){
		return NULL;
	}

	*PrimaryConnectionType = tmpPrimaryConnectionType.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Metered_get(sdbus_s* sd, uint32_t* Metered){
	sdbusVariant_s tmpMetered;
	tmpMetered.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "Metered",
		&tmpMetered
	);
	if( !msg ){
		return NULL;
	}

	*Metered = tmpMetered.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_ActivatingConnection_get(sdbus_s* sd, char** ActivatingConnection){
	sdbusVariant_s tmpActivatingConnection;
	tmpActivatingConnection.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "ActivatingConnection",
		&tmpActivatingConnection
	);
	if( !msg ){
		return NULL;
	}

	*ActivatingConnection = tmpActivatingConnection.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Startup_get(sdbus_s* sd, int* Startup){
	sdbusVariant_s tmpStartup;
	tmpStartup.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "Startup",
		&tmpStartup
	);
	if( !msg ){
		return NULL;
	}

	*Startup = tmpStartup.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Version_get(sdbus_s* sd, char** Version){
	sdbusVariant_s tmpVersion;
	tmpVersion.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "Version",
		&tmpVersion
	);
	if( !msg ){
		return NULL;
	}

	*Version = tmpVersion.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Capabilities_get(sdbus_s* sd, uint32_t* Capabilities){
	sdbusVariant_s tmpCapabilities;
	tmpCapabilities.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "Capabilities",
		&tmpCapabilities
	);
	if( !msg ){
		return NULL;
	}

	*Capabilities = tmpCapabilities.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_State_get(sdbus_s* sd, uint32_t* State){
	sdbusVariant_s tmpState;
	tmpState.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "State",
		&tmpState
	);
	if( !msg ){
		return NULL;
	}

	*State = tmpState.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connectivity_get(sdbus_s* sd, uint32_t* Connectivity){
	sdbusVariant_s tmpConnectivity;
	tmpConnectivity.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "Connectivity",
		&tmpConnectivity
	);
	if( !msg ){
		return NULL;
	}

	*Connectivity = tmpConnectivity.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_ConnectivityCheckAvailable_get(sdbus_s* sd, int* ConnectivityCheckAvailable){
	sdbusVariant_s tmpConnectivityCheckAvailable;
	tmpConnectivityCheckAvailable.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "ConnectivityCheckAvailable",
		&tmpConnectivityCheckAvailable
	);
	if( !msg ){
		return NULL;
	}

	*ConnectivityCheckAvailable = tmpConnectivityCheckAvailable.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_ConnectivityCheckEnabled_get(sdbus_s* sd, int* ConnectivityCheckEnabled){
	sdbusVariant_s tmpConnectivityCheckEnabled;
	tmpConnectivityCheckEnabled.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "ConnectivityCheckEnabled",
		&tmpConnectivityCheckEnabled
	);
	if( !msg ){
		return NULL;
	}

	*ConnectivityCheckEnabled = tmpConnectivityCheckEnabled.var.b;

	return msg;
};

err_t org_freedesktop_NetworkManager_ConnectivityCheckEnabled_set(sdbus_s* sd, int ConnectivityCheckEnabled){
	sdbusVariant_s tmpConnectivityCheckEnabled;
	tmpConnectivityCheckEnabled.type = "b";
	tmpConnectivityCheckEnabled.var.b = ConnectivityCheckEnabled;
	if( sdbus_property_set(
			sd,
			"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager", "ConnectivityCheckEnabled", 
			&tmpConnectivityCheckEnabled
		)
	){
		return -1;
	}

	return 0;
};

sdbusMessage_h org_freedesktop_NetworkManager_ConnectivityCheckUri_get(sdbus_s* sd, char** ConnectivityCheckUri){
	sdbusVariant_s tmpConnectivityCheckUri;
	tmpConnectivityCheckUri.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "ConnectivityCheckUri",
		&tmpConnectivityCheckUri
	);
	if( !msg ){
		return NULL;
	}

	*ConnectivityCheckUri = tmpConnectivityCheckUri.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_GlobalDnsConfiguration_get(sdbus_s* sd, sdbusKV_s** GlobalDnsConfiguration){
	sdbusVariant_s tmpGlobalDnsConfiguration;
	tmpGlobalDnsConfiguration.type = "a{sv}";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "GlobalDnsConfiguration",
		&tmpGlobalDnsConfiguration
	);
	if( !msg ){
		return NULL;
	}

	*GlobalDnsConfiguration = tmpGlobalDnsConfiguration.var.v;
	return msg;
};

err_t org_freedesktop_NetworkManager_GlobalDnsConfiguration_set(sdbus_s* sd, sdbusKV_s* GlobalDnsConfiguration){
	sdbusVariant_s tmpGlobalDnsConfiguration;
	tmpGlobalDnsConfiguration.type = "a{sv}";
	tmpGlobalDnsConfiguration.var.v = GlobalDnsConfiguration;
	if( sdbus_property_set(
			sd,
			"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager", "GlobalDnsConfiguration", 
			&tmpGlobalDnsConfiguration
		)
	){
		return -1;
	}

	return 0;
};

/*************************************************/
/*** org_freedesktop_NetworkManager_DnsManager ***/
/*************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_DnsManager_Mode_get(sdbus_s* sd, char** Mode){
	sdbusVariant_s tmpMode;
	tmpMode.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.DnsManager", "Mode",
		&tmpMode
	);
	if( !msg ){
		return NULL;
	}

	*Mode = tmpMode.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_DnsManager_RcManager_get(sdbus_s* sd, char** RcManager){
	sdbusVariant_s tmpRcManager;
	tmpRcManager.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.DnsManager", "RcManager",
		&tmpRcManager
	);
	if( !msg ){
		return NULL;
	}

	*RcManager = tmpRcManager.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_DnsManager_Configuration_get(sdbus_s* sd, sdbusKV_s*** Configuration){
	sdbusVariant_s tmpConfiguration;
	tmpConfiguration.type = "aa{sv}";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.DnsManager", "Configuration",
		&tmpConfiguration
	);
	if( !msg ){
		return NULL;
	}

	*Configuration = tmpConfiguration.var.v;
	return msg;
};

/************************************************/
/*** org_freedesktop_NetworkManager_IP4Config ***/
/************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Addresses_get(sdbus_s* sd, uint32_t*** Addresses){
	sdbusVariant_s tmpAddresses;
	tmpAddresses.type = "aau";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "Addresses",
		&tmpAddresses
	);
	if( !msg ){
		return NULL;
	}

	*Addresses = tmpAddresses.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_AddressData_get(sdbus_s* sd, sdbusKV_s*** AddressData){
	sdbusVariant_s tmpAddressData;
	tmpAddressData.type = "aa{sv}";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "AddressData",
		&tmpAddressData
	);
	if( !msg ){
		return NULL;
	}

	*AddressData = tmpAddressData.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Gateway_get(sdbus_s* sd, char** Gateway){
	sdbusVariant_s tmpGateway;
	tmpGateway.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "Gateway",
		&tmpGateway
	);
	if( !msg ){
		return NULL;
	}

	*Gateway = tmpGateway.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Routes_get(sdbus_s* sd, uint32_t*** Routes){
	sdbusVariant_s tmpRoutes;
	tmpRoutes.type = "aau";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "Routes",
		&tmpRoutes
	);
	if( !msg ){
		return NULL;
	}

	*Routes = tmpRoutes.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_RouteData_get(sdbus_s* sd, sdbusKV_s*** RouteData){
	sdbusVariant_s tmpRouteData;
	tmpRouteData.type = "aa{sv}";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "RouteData",
		&tmpRouteData
	);
	if( !msg ){
		return NULL;
	}

	*RouteData = tmpRouteData.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_NameserverData_get(sdbus_s* sd, sdbusKV_s*** NameserverData){
	sdbusVariant_s tmpNameserverData;
	tmpNameserverData.type = "aa{sv}";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "NameserverData",
		&tmpNameserverData
	);
	if( !msg ){
		return NULL;
	}

	*NameserverData = tmpNameserverData.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Nameservers_get(sdbus_s* sd, uint32_t** Nameservers){
	sdbusVariant_s tmpNameservers;
	tmpNameservers.type = "au";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "Nameservers",
		&tmpNameservers
	);
	if( !msg ){
		return NULL;
	}

	*Nameservers = tmpNameservers.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Domains_get(sdbus_s* sd, char*** Domains){
	sdbusVariant_s tmpDomains;
	tmpDomains.type = "as";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "Domains",
		&tmpDomains
	);
	if( !msg ){
		return NULL;
	}

	*Domains = tmpDomains.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_Searches_get(sdbus_s* sd, char*** Searches){
	sdbusVariant_s tmpSearches;
	tmpSearches.type = "as";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "Searches",
		&tmpSearches
	);
	if( !msg ){
		return NULL;
	}

	*Searches = tmpSearches.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_DnsOptions_get(sdbus_s* sd, char*** DnsOptions){
	sdbusVariant_s tmpDnsOptions;
	tmpDnsOptions.type = "as";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "DnsOptions",
		&tmpDnsOptions
	);
	if( !msg ){
		return NULL;
	}

	*DnsOptions = tmpDnsOptions.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_DnsPriority_get(sdbus_s* sd, int* DnsPriority){
	sdbusVariant_s tmpDnsPriority;
	tmpDnsPriority.type = "i";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "DnsPriority",
		&tmpDnsPriority
	);
	if( !msg ){
		return NULL;
	}

	*DnsPriority = tmpDnsPriority.var.i;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_WinsServerData_get(sdbus_s* sd, char*** WinsServerData){
	sdbusVariant_s tmpWinsServerData;
	tmpWinsServerData.type = "as";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "WinsServerData",
		&tmpWinsServerData
	);
	if( !msg ){
		return NULL;
	}

	*WinsServerData = tmpWinsServerData.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP4Config_WinsServers_get(sdbus_s* sd, uint32_t** WinsServers){
	sdbusVariant_s tmpWinsServers;
	tmpWinsServers.type = "au";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP4Config", "WinsServers",
		&tmpWinsServers
	);
	if( !msg ){
		return NULL;
	}

	*WinsServers = tmpWinsServers.var.v;
	return msg;
};

/********************************************************/
/*** org_freedesktop_NetworkManager_Connection_Active ***/
/********************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Connection_get(sdbus_s* sd, char** Connection){
	sdbusVariant_s tmpConnection;
	tmpConnection.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Connection",
		&tmpConnection
	);
	if( !msg ){
		return NULL;
	}

	*Connection = tmpConnection.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_SpecificObject_get(sdbus_s* sd, char** SpecificObject){
	sdbusVariant_s tmpSpecificObject;
	tmpSpecificObject.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "SpecificObject",
		&tmpSpecificObject
	);
	if( !msg ){
		return NULL;
	}

	*SpecificObject = tmpSpecificObject.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Id_get(sdbus_s* sd, char** Id){
	sdbusVariant_s tmpId;
	tmpId.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Id",
		&tmpId
	);
	if( !msg ){
		return NULL;
	}

	*Id = tmpId.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Uuid_get(sdbus_s* sd, char** Uuid){
	sdbusVariant_s tmpUuid;
	tmpUuid.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Uuid",
		&tmpUuid
	);
	if( !msg ){
		return NULL;
	}

	*Uuid = tmpUuid.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Type_get(sdbus_s* sd, char** Type){
	sdbusVariant_s tmpType;
	tmpType.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Type",
		&tmpType
	);
	if( !msg ){
		return NULL;
	}

	*Type = tmpType.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Devices_get(sdbus_s* sd, char*** Devices){
	sdbusVariant_s tmpDevices;
	tmpDevices.type = "ao";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Devices",
		&tmpDevices
	);
	if( !msg ){
		return NULL;
	}

	*Devices = tmpDevices.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_State_get(sdbus_s* sd, uint32_t* State){
	sdbusVariant_s tmpState;
	tmpState.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "State",
		&tmpState
	);
	if( !msg ){
		return NULL;
	}

	*State = tmpState.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_StateFlags_get(sdbus_s* sd, uint32_t* StateFlags){
	sdbusVariant_s tmpStateFlags;
	tmpStateFlags.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "StateFlags",
		&tmpStateFlags
	);
	if( !msg ){
		return NULL;
	}

	*StateFlags = tmpStateFlags.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Default_get(sdbus_s* sd, int* Default){
	sdbusVariant_s tmpDefault;
	tmpDefault.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Default",
		&tmpDefault
	);
	if( !msg ){
		return NULL;
	}

	*Default = tmpDefault.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Ip4Config_get(sdbus_s* sd, char** Ip4Config){
	sdbusVariant_s tmpIp4Config;
	tmpIp4Config.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Ip4Config",
		&tmpIp4Config
	);
	if( !msg ){
		return NULL;
	}

	*Ip4Config = tmpIp4Config.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Dhcp4Config_get(sdbus_s* sd, char** Dhcp4Config){
	sdbusVariant_s tmpDhcp4Config;
	tmpDhcp4Config.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Dhcp4Config",
		&tmpDhcp4Config
	);
	if( !msg ){
		return NULL;
	}

	*Dhcp4Config = tmpDhcp4Config.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Default6_get(sdbus_s* sd, int* Default6){
	sdbusVariant_s tmpDefault6;
	tmpDefault6.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Default6",
		&tmpDefault6
	);
	if( !msg ){
		return NULL;
	}

	*Default6 = tmpDefault6.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Ip6Config_get(sdbus_s* sd, char** Ip6Config){
	sdbusVariant_s tmpIp6Config;
	tmpIp6Config.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Ip6Config",
		&tmpIp6Config
	);
	if( !msg ){
		return NULL;
	}

	*Ip6Config = tmpIp6Config.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Dhcp6Config_get(sdbus_s* sd, char** Dhcp6Config){
	sdbusVariant_s tmpDhcp6Config;
	tmpDhcp6Config.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Dhcp6Config",
		&tmpDhcp6Config
	);
	if( !msg ){
		return NULL;
	}

	*Dhcp6Config = tmpDhcp6Config.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Vpn_get(sdbus_s* sd, int* Vpn){
	sdbusVariant_s tmpVpn;
	tmpVpn.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Vpn",
		&tmpVpn
	);
	if( !msg ){
		return NULL;
	}

	*Vpn = tmpVpn.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Connection_Active_Master_get(sdbus_s* sd, char** Master){
	sdbusVariant_s tmpMaster;
	tmpMaster.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Connection.Active", "Master",
		&tmpMaster
	);
	if( !msg ){
		return NULL;
	}

	*Master = tmpMaster.var.o;

	return msg;
};

/***************************************************/
/*** org_freedesktop_NetworkManager_AgentManager ***/
/***************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_AgentManager_Register(sdbus_s* sd, char* in_identifier){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AgentManager", "Register",
		"s",
		in_identifier
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AgentManager_RegisterWithCapabilities(sdbus_s* sd, char* in_identifier, uint32_t in_capabilities){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AgentManager", "RegisterWithCapabilities",
		"su",
		in_identifier,
		in_capabilities
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AgentManager_Unregister(sdbus_s* sd){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AgentManager", "Unregister",
		NULL
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

/**************************************************/
/*** org_freedesktop_NetworkManager_DHCP4Config ***/
/**************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_DHCP4Config_Options_get(sdbus_s* sd, sdbusKV_s** Options){
	sdbusVariant_s tmpOptions;
	tmpOptions.type = "a{sv}";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.DHCP4Config", "Options",
		&tmpOptions
	);
	if( !msg ){
		return NULL;
	}

	*Options = tmpOptions.var.v;
	return msg;
};

/********************************************************/
/*** org_freedesktop_NetworkManager_Device_Statistics ***/
/********************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_Device_Statistics_RefreshRateMs_get(sdbus_s* sd, uint32_t* RefreshRateMs){
	sdbusVariant_s tmpRefreshRateMs;
	tmpRefreshRateMs.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Statistics", "RefreshRateMs",
		&tmpRefreshRateMs
	);
	if( !msg ){
		return NULL;
	}

	*RefreshRateMs = tmpRefreshRateMs.var.u;

	return msg;
};

err_t org_freedesktop_NetworkManager_Device_Statistics_RefreshRateMs_set(sdbus_s* sd, uint32_t RefreshRateMs){
	sdbusVariant_s tmpRefreshRateMs;
	tmpRefreshRateMs.type = "u";
	tmpRefreshRateMs.var.u = RefreshRateMs;
	if( sdbus_property_set(
			sd,
			"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager.Device.Statistics", "RefreshRateMs", 
			&tmpRefreshRateMs
		)
	){
		return -1;
	}

	return 0;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Statistics_TxBytes_get(sdbus_s* sd, uint64_t* TxBytes){
	sdbusVariant_s tmpTxBytes;
	tmpTxBytes.type = "t";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Statistics", "TxBytes",
		&tmpTxBytes
	);
	if( !msg ){
		return NULL;
	}

	*TxBytes = tmpTxBytes.var.t;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Statistics_RxBytes_get(sdbus_s* sd, uint64_t* RxBytes){
	sdbusVariant_s tmpRxBytes;
	tmpRxBytes.type = "t";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Statistics", "RxBytes",
		&tmpRxBytes
	);
	if( !msg ){
		return NULL;
	}

	*RxBytes = tmpRxBytes.var.t;

	return msg;
};

/*********************************************/
/*** org_freedesktop_NetworkManager_Device ***/
/*********************************************/

sdbusMessage_h org_freedesktop_NetworkManager_Device_Reapply(sdbus_s* sd, sdbusKV_s* vin_connection, uint64_t in_version_id, uint32_t in_flags){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Reapply",
		"a{sa{sv}}tu",
		vin_connection,
		in_version_id,
		in_flags
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_GetAppliedConnection(sdbus_s* sd, uint32_t in_flags, sdbusKV_s** vout_connection, uint64_t* out_version_id){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "GetAppliedConnection",
		"u",
		in_flags
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "a{sa{sv}}",
		vout_connection)
	){
		sdbus_message_free(msg);
		return NULL;
	}
	if( sdbus_message_read(msg, "t",
		out_version_id)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Disconnect(sdbus_s* sd){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Disconnect",
		NULL
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Delete(sdbus_s* sd){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Delete",
		NULL
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Udi_get(sdbus_s* sd, char** Udi){
	sdbusVariant_s tmpUdi;
	tmpUdi.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Udi",
		&tmpUdi
	);
	if( !msg ){
		return NULL;
	}

	*Udi = tmpUdi.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Interface_get(sdbus_s* sd, char** Interface){
	sdbusVariant_s tmpInterface;
	tmpInterface.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Interface",
		&tmpInterface
	);
	if( !msg ){
		return NULL;
	}

	*Interface = tmpInterface.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_IpInterface_get(sdbus_s* sd, char** IpInterface){
	sdbusVariant_s tmpIpInterface;
	tmpIpInterface.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "IpInterface",
		&tmpIpInterface
	);
	if( !msg ){
		return NULL;
	}

	*IpInterface = tmpIpInterface.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Driver_get(sdbus_s* sd, char** Driver){
	sdbusVariant_s tmpDriver;
	tmpDriver.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Driver",
		&tmpDriver
	);
	if( !msg ){
		return NULL;
	}

	*Driver = tmpDriver.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_DriverVersion_get(sdbus_s* sd, char** DriverVersion){
	sdbusVariant_s tmpDriverVersion;
	tmpDriverVersion.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "DriverVersion",
		&tmpDriverVersion
	);
	if( !msg ){
		return NULL;
	}

	*DriverVersion = tmpDriverVersion.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_FirmwareVersion_get(sdbus_s* sd, char** FirmwareVersion){
	sdbusVariant_s tmpFirmwareVersion;
	tmpFirmwareVersion.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "FirmwareVersion",
		&tmpFirmwareVersion
	);
	if( !msg ){
		return NULL;
	}

	*FirmwareVersion = tmpFirmwareVersion.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Capabilities_get(sdbus_s* sd, uint32_t* Capabilities){
	sdbusVariant_s tmpCapabilities;
	tmpCapabilities.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Capabilities",
		&tmpCapabilities
	);
	if( !msg ){
		return NULL;
	}

	*Capabilities = tmpCapabilities.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Ip4Address_get(sdbus_s* sd, uint32_t* Ip4Address){
	sdbusVariant_s tmpIp4Address;
	tmpIp4Address.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Ip4Address",
		&tmpIp4Address
	);
	if( !msg ){
		return NULL;
	}

	*Ip4Address = tmpIp4Address.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_State_get(sdbus_s* sd, uint32_t* State){
	sdbusVariant_s tmpState;
	tmpState.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "State",
		&tmpState
	);
	if( !msg ){
		return NULL;
	}

	*State = tmpState.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_StateReason_get(sdbus_s* sd, org_freedesktop_NetworkManager_Device_StateReason_s** StateReason){
	sdbusVariant_s tmpStateReason;
	tmpStateReason.type = "(uu)";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "StateReason",
		&tmpStateReason
	);
	if( !msg ){
		return NULL;
	}

	*StateReason = tmpStateReason.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_ActiveConnection_get(sdbus_s* sd, char** ActiveConnection){
	sdbusVariant_s tmpActiveConnection;
	tmpActiveConnection.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "ActiveConnection",
		&tmpActiveConnection
	);
	if( !msg ){
		return NULL;
	}

	*ActiveConnection = tmpActiveConnection.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Ip4Config_get(sdbus_s* sd, char** Ip4Config){
	sdbusVariant_s tmpIp4Config;
	tmpIp4Config.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Ip4Config",
		&tmpIp4Config
	);
	if( !msg ){
		return NULL;
	}

	*Ip4Config = tmpIp4Config.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Dhcp4Config_get(sdbus_s* sd, char** Dhcp4Config){
	sdbusVariant_s tmpDhcp4Config;
	tmpDhcp4Config.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Dhcp4Config",
		&tmpDhcp4Config
	);
	if( !msg ){
		return NULL;
	}

	*Dhcp4Config = tmpDhcp4Config.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Ip6Config_get(sdbus_s* sd, char** Ip6Config){
	sdbusVariant_s tmpIp6Config;
	tmpIp6Config.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Ip6Config",
		&tmpIp6Config
	);
	if( !msg ){
		return NULL;
	}

	*Ip6Config = tmpIp6Config.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Dhcp6Config_get(sdbus_s* sd, char** Dhcp6Config){
	sdbusVariant_s tmpDhcp6Config;
	tmpDhcp6Config.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Dhcp6Config",
		&tmpDhcp6Config
	);
	if( !msg ){
		return NULL;
	}

	*Dhcp6Config = tmpDhcp6Config.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Managed_get(sdbus_s* sd, int* Managed){
	sdbusVariant_s tmpManaged;
	tmpManaged.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Managed",
		&tmpManaged
	);
	if( !msg ){
		return NULL;
	}

	*Managed = tmpManaged.var.b;

	return msg;
};

err_t org_freedesktop_NetworkManager_Device_Managed_set(sdbus_s* sd, int Managed){
	sdbusVariant_s tmpManaged;
	tmpManaged.type = "b";
	tmpManaged.var.b = Managed;
	if( sdbus_property_set(
			sd,
			"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager.Device", "Managed", 
			&tmpManaged
		)
	){
		return -1;
	}

	return 0;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Autoconnect_get(sdbus_s* sd, int* Autoconnect){
	sdbusVariant_s tmpAutoconnect;
	tmpAutoconnect.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Autoconnect",
		&tmpAutoconnect
	);
	if( !msg ){
		return NULL;
	}

	*Autoconnect = tmpAutoconnect.var.b;

	return msg;
};

err_t org_freedesktop_NetworkManager_Device_Autoconnect_set(sdbus_s* sd, int Autoconnect){
	sdbusVariant_s tmpAutoconnect;
	tmpAutoconnect.type = "b";
	tmpAutoconnect.var.b = Autoconnect;
	if( sdbus_property_set(
			sd,
			"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager.Device", "Autoconnect", 
			&tmpAutoconnect
		)
	){
		return -1;
	}

	return 0;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_FirmwareMissing_get(sdbus_s* sd, int* FirmwareMissing){
	sdbusVariant_s tmpFirmwareMissing;
	tmpFirmwareMissing.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "FirmwareMissing",
		&tmpFirmwareMissing
	);
	if( !msg ){
		return NULL;
	}

	*FirmwareMissing = tmpFirmwareMissing.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_NmPluginMissing_get(sdbus_s* sd, int* NmPluginMissing){
	sdbusVariant_s tmpNmPluginMissing;
	tmpNmPluginMissing.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "NmPluginMissing",
		&tmpNmPluginMissing
	);
	if( !msg ){
		return NULL;
	}

	*NmPluginMissing = tmpNmPluginMissing.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_DeviceType_get(sdbus_s* sd, uint32_t* DeviceType){
	sdbusVariant_s tmpDeviceType;
	tmpDeviceType.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "DeviceType",
		&tmpDeviceType
	);
	if( !msg ){
		return NULL;
	}

	*DeviceType = tmpDeviceType.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_AvailableConnections_get(sdbus_s* sd, char*** AvailableConnections){
	sdbusVariant_s tmpAvailableConnections;
	tmpAvailableConnections.type = "ao";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "AvailableConnections",
		&tmpAvailableConnections
	);
	if( !msg ){
		return NULL;
	}

	*AvailableConnections = tmpAvailableConnections.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_PhysicalPortId_get(sdbus_s* sd, char** PhysicalPortId){
	sdbusVariant_s tmpPhysicalPortId;
	tmpPhysicalPortId.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "PhysicalPortId",
		&tmpPhysicalPortId
	);
	if( !msg ){
		return NULL;
	}

	*PhysicalPortId = tmpPhysicalPortId.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Mtu_get(sdbus_s* sd, uint32_t* Mtu){
	sdbusVariant_s tmpMtu;
	tmpMtu.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Mtu",
		&tmpMtu
	);
	if( !msg ){
		return NULL;
	}

	*Mtu = tmpMtu.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Metered_get(sdbus_s* sd, uint32_t* Metered){
	sdbusVariant_s tmpMetered;
	tmpMetered.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Metered",
		&tmpMetered
	);
	if( !msg ){
		return NULL;
	}

	*Metered = tmpMetered.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_LldpNeighbors_get(sdbus_s* sd, sdbusKV_s*** LldpNeighbors){
	sdbusVariant_s tmpLldpNeighbors;
	tmpLldpNeighbors.type = "aa{sv}";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "LldpNeighbors",
		&tmpLldpNeighbors
	);
	if( !msg ){
		return NULL;
	}

	*LldpNeighbors = tmpLldpNeighbors.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Real_get(sdbus_s* sd, int* Real){
	sdbusVariant_s tmpReal;
	tmpReal.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Real",
		&tmpReal
	);
	if( !msg ){
		return NULL;
	}

	*Real = tmpReal.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Ip4Connectivity_get(sdbus_s* sd, uint32_t* Ip4Connectivity){
	sdbusVariant_s tmpIp4Connectivity;
	tmpIp4Connectivity.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Ip4Connectivity",
		&tmpIp4Connectivity
	);
	if( !msg ){
		return NULL;
	}

	*Ip4Connectivity = tmpIp4Connectivity.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Ip6Connectivity_get(sdbus_s* sd, uint32_t* Ip6Connectivity){
	sdbusVariant_s tmpIp6Connectivity;
	tmpIp6Connectivity.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "Ip6Connectivity",
		&tmpIp6Connectivity
	);
	if( !msg ){
		return NULL;
	}

	*Ip6Connectivity = tmpIp6Connectivity.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_InterfaceFlags_get(sdbus_s* sd, uint32_t* InterfaceFlags){
	sdbusVariant_s tmpInterfaceFlags;
	tmpInterfaceFlags.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "InterfaceFlags",
		&tmpInterfaceFlags
	);
	if( !msg ){
		return NULL;
	}

	*InterfaceFlags = tmpInterfaceFlags.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_HwAddress_get(sdbus_s* sd, char** HwAddress){
	sdbusVariant_s tmpHwAddress;
	tmpHwAddress.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device", "HwAddress",
		&tmpHwAddress
	);
	if( !msg ){
		return NULL;
	}

	*HwAddress = tmpHwAddress.var.s;

	return msg;
};

/***************************************************/
/*** org_freedesktop_NetworkManager_Device_Wired ***/
/***************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wired_HwAddress_get(sdbus_s* sd, char** HwAddress){
	sdbusVariant_s tmpHwAddress;
	tmpHwAddress.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wired", "HwAddress",
		&tmpHwAddress
	);
	if( !msg ){
		return NULL;
	}

	*HwAddress = tmpHwAddress.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wired_PermHwAddress_get(sdbus_s* sd, char** PermHwAddress){
	sdbusVariant_s tmpPermHwAddress;
	tmpPermHwAddress.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wired", "PermHwAddress",
		&tmpPermHwAddress
	);
	if( !msg ){
		return NULL;
	}

	*PermHwAddress = tmpPermHwAddress.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wired_Speed_get(sdbus_s* sd, uint32_t* Speed){
	sdbusVariant_s tmpSpeed;
	tmpSpeed.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wired", "Speed",
		&tmpSpeed
	);
	if( !msg ){
		return NULL;
	}

	*Speed = tmpSpeed.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wired_S390Subchannels_get(sdbus_s* sd, char*** S390Subchannels){
	sdbusVariant_s tmpS390Subchannels;
	tmpS390Subchannels.type = "as";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wired", "S390Subchannels",
		&tmpS390Subchannels
	);
	if( !msg ){
		return NULL;
	}

	*S390Subchannels = tmpS390Subchannels.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wired_Carrier_get(sdbus_s* sd, int* Carrier){
	sdbusVariant_s tmpCarrier;
	tmpCarrier.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wired", "Carrier",
		&tmpCarrier
	);
	if( !msg ){
		return NULL;
	}

	*Carrier = tmpCarrier.var.b;

	return msg;
};

/******************************************************/
/*** org_freedesktop_NetworkManager_Device_Wireless ***/
/******************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_GetAccessPoints(sdbus_s* sd, char*** vout_access_points){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "GetAccessPoints",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "ao",
		vout_access_points)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_GetAllAccessPoints(sdbus_s* sd, char*** vout_access_points){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "GetAllAccessPoints",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "ao",
		vout_access_points)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_RequestScan(sdbus_s* sd, sdbusKV_s* vin_options){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "RequestScan",
		"a{sv}",
		vin_options
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_HwAddress_get(sdbus_s* sd, char** HwAddress){
	sdbusVariant_s tmpHwAddress;
	tmpHwAddress.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "HwAddress",
		&tmpHwAddress
	);
	if( !msg ){
		return NULL;
	}

	*HwAddress = tmpHwAddress.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_PermHwAddress_get(sdbus_s* sd, char** PermHwAddress){
	sdbusVariant_s tmpPermHwAddress;
	tmpPermHwAddress.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "PermHwAddress",
		&tmpPermHwAddress
	);
	if( !msg ){
		return NULL;
	}

	*PermHwAddress = tmpPermHwAddress.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_Mode_get(sdbus_s* sd, uint32_t* Mode){
	sdbusVariant_s tmpMode;
	tmpMode.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "Mode",
		&tmpMode
	);
	if( !msg ){
		return NULL;
	}

	*Mode = tmpMode.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_Bitrate_get(sdbus_s* sd, uint32_t* Bitrate){
	sdbusVariant_s tmpBitrate;
	tmpBitrate.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "Bitrate",
		&tmpBitrate
	);
	if( !msg ){
		return NULL;
	}

	*Bitrate = tmpBitrate.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_AccessPoints_get(sdbus_s* sd, char*** AccessPoints){
	sdbusVariant_s tmpAccessPoints;
	tmpAccessPoints.type = "ao";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "AccessPoints",
		&tmpAccessPoints
	);
	if( !msg ){
		return NULL;
	}

	*AccessPoints = tmpAccessPoints.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_ActiveAccessPoint_get(sdbus_s* sd, char** ActiveAccessPoint){
	sdbusVariant_s tmpActiveAccessPoint;
	tmpActiveAccessPoint.type = "o";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "ActiveAccessPoint",
		&tmpActiveAccessPoint
	);
	if( !msg ){
		return NULL;
	}

	*ActiveAccessPoint = tmpActiveAccessPoint.var.o;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_WirelessCapabilities_get(sdbus_s* sd, uint32_t* WirelessCapabilities){
	sdbusVariant_s tmpWirelessCapabilities;
	tmpWirelessCapabilities.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "WirelessCapabilities",
		&tmpWirelessCapabilities
	);
	if( !msg ){
		return NULL;
	}

	*WirelessCapabilities = tmpWirelessCapabilities.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Wireless_LastScan_get(sdbus_s* sd, int64_t* LastScan){
	sdbusVariant_s tmpLastScan;
	tmpLastScan.type = "x";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Wireless", "LastScan",
		&tmpLastScan
	);
	if( !msg ){
		return NULL;
	}

	*LastScan = tmpLastScan.var.x;

	return msg;
};

/*****************************************************/
/*** org_freedesktop_NetworkManager_Device_Generic ***/
/*****************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_Device_Generic_HwAddress_get(sdbus_s* sd, char** HwAddress){
	sdbusVariant_s tmpHwAddress;
	tmpHwAddress.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Generic", "HwAddress",
		&tmpHwAddress
	);
	if( !msg ){
		return NULL;
	}

	*HwAddress = tmpHwAddress.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Device_Generic_TypeDescription_get(sdbus_s* sd, char** TypeDescription){
	sdbusVariant_s tmpTypeDescription;
	tmpTypeDescription.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Device.Generic", "TypeDescription",
		&tmpTypeDescription
	);
	if( !msg ){
		return NULL;
	}

	*TypeDescription = tmpTypeDescription.var.s;

	return msg;
};

/**************************************************/
/*** org_freedesktop_NetworkManager_AccessPoint ***/
/**************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_Flags_get(sdbus_s* sd, uint32_t* Flags){
	sdbusVariant_s tmpFlags;
	tmpFlags.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AccessPoint", "Flags",
		&tmpFlags
	);
	if( !msg ){
		return NULL;
	}

	*Flags = tmpFlags.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_WpaFlags_get(sdbus_s* sd, uint32_t* WpaFlags){
	sdbusVariant_s tmpWpaFlags;
	tmpWpaFlags.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AccessPoint", "WpaFlags",
		&tmpWpaFlags
	);
	if( !msg ){
		return NULL;
	}

	*WpaFlags = tmpWpaFlags.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_RsnFlags_get(sdbus_s* sd, uint32_t* RsnFlags){
	sdbusVariant_s tmpRsnFlags;
	tmpRsnFlags.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AccessPoint", "RsnFlags",
		&tmpRsnFlags
	);
	if( !msg ){
		return NULL;
	}

	*RsnFlags = tmpRsnFlags.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_Ssid_get(sdbus_s* sd, uint8_t** Ssid){
	sdbusVariant_s tmpSsid;
	tmpSsid.type = "ay";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AccessPoint", "Ssid",
		&tmpSsid
	);
	if( !msg ){
		return NULL;
	}

	*Ssid = tmpSsid.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_Frequency_get(sdbus_s* sd, uint32_t* Frequency){
	sdbusVariant_s tmpFrequency;
	tmpFrequency.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AccessPoint", "Frequency",
		&tmpFrequency
	);
	if( !msg ){
		return NULL;
	}

	*Frequency = tmpFrequency.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_HwAddress_get(sdbus_s* sd, char** HwAddress){
	sdbusVariant_s tmpHwAddress;
	tmpHwAddress.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AccessPoint", "HwAddress",
		&tmpHwAddress
	);
	if( !msg ){
		return NULL;
	}

	*HwAddress = tmpHwAddress.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_Mode_get(sdbus_s* sd, uint32_t* Mode){
	sdbusVariant_s tmpMode;
	tmpMode.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AccessPoint", "Mode",
		&tmpMode
	);
	if( !msg ){
		return NULL;
	}

	*Mode = tmpMode.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_MaxBitrate_get(sdbus_s* sd, uint32_t* MaxBitrate){
	sdbusVariant_s tmpMaxBitrate;
	tmpMaxBitrate.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AccessPoint", "MaxBitrate",
		&tmpMaxBitrate
	);
	if( !msg ){
		return NULL;
	}

	*MaxBitrate = tmpMaxBitrate.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_Strength_get(sdbus_s* sd, uint8_t* Strength){
	sdbusVariant_s tmpStrength;
	tmpStrength.type = "y";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AccessPoint", "Strength",
		&tmpStrength
	);
	if( !msg ){
		return NULL;
	}

	*Strength = tmpStrength.var.y;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_AccessPoint_LastSeen_get(sdbus_s* sd, int* LastSeen){
	sdbusVariant_s tmpLastSeen;
	tmpLastSeen.type = "i";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.AccessPoint", "LastSeen",
		&tmpLastSeen
	);
	if( !msg ){
		return NULL;
	}

	*LastSeen = tmpLastSeen.var.i;

	return msg;
};

/************************************************/
/*** org_freedesktop_NetworkManager_IP6Config ***/
/************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Addresses_get(sdbus_s* sd, org_freedesktop_NetworkManager_IP6Config_Addresses_s*** Addresses){
	sdbusVariant_s tmpAddresses;
	tmpAddresses.type = "a(ayuay)";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP6Config", "Addresses",
		&tmpAddresses
	);
	if( !msg ){
		return NULL;
	}

	*Addresses = tmpAddresses.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_AddressData_get(sdbus_s* sd, sdbusKV_s*** AddressData){
	sdbusVariant_s tmpAddressData;
	tmpAddressData.type = "aa{sv}";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP6Config", "AddressData",
		&tmpAddressData
	);
	if( !msg ){
		return NULL;
	}

	*AddressData = tmpAddressData.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Gateway_get(sdbus_s* sd, char** Gateway){
	sdbusVariant_s tmpGateway;
	tmpGateway.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP6Config", "Gateway",
		&tmpGateway
	);
	if( !msg ){
		return NULL;
	}

	*Gateway = tmpGateway.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Routes_get(sdbus_s* sd, org_freedesktop_NetworkManager_IP6Config_Routes_s*** Routes){
	sdbusVariant_s tmpRoutes;
	tmpRoutes.type = "a(ayuayu)";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP6Config", "Routes",
		&tmpRoutes
	);
	if( !msg ){
		return NULL;
	}

	*Routes = tmpRoutes.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_RouteData_get(sdbus_s* sd, sdbusKV_s*** RouteData){
	sdbusVariant_s tmpRouteData;
	tmpRouteData.type = "aa{sv}";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP6Config", "RouteData",
		&tmpRouteData
	);
	if( !msg ){
		return NULL;
	}

	*RouteData = tmpRouteData.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Nameservers_get(sdbus_s* sd, uint8_t*** Nameservers){
	sdbusVariant_s tmpNameservers;
	tmpNameservers.type = "aay";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP6Config", "Nameservers",
		&tmpNameservers
	);
	if( !msg ){
		return NULL;
	}

	*Nameservers = tmpNameservers.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Domains_get(sdbus_s* sd, char*** Domains){
	sdbusVariant_s tmpDomains;
	tmpDomains.type = "as";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP6Config", "Domains",
		&tmpDomains
	);
	if( !msg ){
		return NULL;
	}

	*Domains = tmpDomains.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_Searches_get(sdbus_s* sd, char*** Searches){
	sdbusVariant_s tmpSearches;
	tmpSearches.type = "as";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP6Config", "Searches",
		&tmpSearches
	);
	if( !msg ){
		return NULL;
	}

	*Searches = tmpSearches.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_DnsOptions_get(sdbus_s* sd, char*** DnsOptions){
	sdbusVariant_s tmpDnsOptions;
	tmpDnsOptions.type = "as";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP6Config", "DnsOptions",
		&tmpDnsOptions
	);
	if( !msg ){
		return NULL;
	}

	*DnsOptions = tmpDnsOptions.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_IP6Config_DnsPriority_get(sdbus_s* sd, int* DnsPriority){
	sdbusVariant_s tmpDnsPriority;
	tmpDnsPriority.type = "i";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.IP6Config", "DnsPriority",
		&tmpDnsPriority
	);
	if( !msg ){
		return NULL;
	}

	*DnsPriority = tmpDnsPriority.var.i;

	return msg;
};

/***********************************************/
/*** org_freedesktop_NetworkManager_Settings ***/
/***********************************************/

sdbusMessage_h org_freedesktop_NetworkManager_Settings_ListConnections(sdbus_s* sd, char*** vout_connections){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "ListConnections",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "ao",
		vout_connections)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_GetConnectionByUuid(sdbus_s* sd, char* in_uuid, char** out_connection){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "GetConnectionByUuid",
		"s",
		in_uuid
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "o",
		out_connection)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_AddConnection(sdbus_s* sd, sdbusKV_s* vin_connection, char** out_path){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "AddConnection",
		"a{sa{sv}}",
		vin_connection
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "o",
		out_path)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_AddConnectionUnsaved(sdbus_s* sd, sdbusKV_s* vin_connection, char** out_path){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "AddConnectionUnsaved",
		"a{sa{sv}}",
		vin_connection
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "o",
		out_path)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_AddConnection2(sdbus_s* sd, sdbusKV_s* vin_settings, uint32_t in_flags, sdbusKV_s* vin_args, char** out_path, sdbusKV_s** vout_result){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "AddConnection2",
		"a{sa{sv}}ua{sv}",
		vin_settings,
		in_flags,
		vin_args
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "o",
		out_path)
	){
		sdbus_message_free(msg);
		return NULL;
	}
	if( sdbus_message_read(msg, "a{sv}",
		vout_result)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_LoadConnections(sdbus_s* sd, char** vin_filenames, int* out_status, char*** vout_failures){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "LoadConnections",
		"as",
		vin_filenames
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "b",
		out_status)
	){
		sdbus_message_free(msg);
		return NULL;
	}
	if( sdbus_message_read(msg, "as",
		vout_failures)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_ReloadConnections(sdbus_s* sd, int* out_status){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "ReloadConnections",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "b",
		out_status)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_SaveHostname(sdbus_s* sd, char* in_hostname){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "SaveHostname",
		"s",
		in_hostname
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connections_get(sdbus_s* sd, char*** Connections){
	sdbusVariant_s tmpConnections;
	tmpConnections.type = "ao";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "Connections",
		&tmpConnections
	);
	if( !msg ){
		return NULL;
	}

	*Connections = tmpConnections.var.v;
	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Hostname_get(sdbus_s* sd, char** Hostname){
	sdbusVariant_s tmpHostname;
	tmpHostname.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "Hostname",
		&tmpHostname
	);
	if( !msg ){
		return NULL;
	}

	*Hostname = tmpHostname.var.s;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_CanModify_get(sdbus_s* sd, int* CanModify){
	sdbusVariant_s tmpCanModify;
	tmpCanModify.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings", "CanModify",
		&tmpCanModify
	);
	if( !msg ){
		return NULL;
	}

	*CanModify = tmpCanModify.var.b;

	return msg;
};

/**********************************************************/
/*** org_freedesktop_NetworkManager_Settings_Connection ***/
/**********************************************************/

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Update(sdbus_s* sd, sdbusKV_s* vin_properties){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "Update",
		"a{sa{sv}}",
		vin_properties
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_UpdateUnsaved(sdbus_s* sd, sdbusKV_s* vin_properties){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "UpdateUnsaved",
		"a{sa{sv}}",
		vin_properties
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Delete(sdbus_s* sd){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "Delete",
		NULL
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_GetSettings(sdbus_s* sd, sdbusKV_s** vout_settings){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "GetSettings",
		NULL
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "a{sa{sv}}",
		vout_settings)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_GetSecrets(sdbus_s* sd, char* in_setting_name, sdbusKV_s** vout_secrets){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "GetSecrets",
		"s",
		in_setting_name
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "a{sa{sv}}",
		vout_secrets)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_ClearSecrets(sdbus_s* sd){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "ClearSecrets",
		NULL
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Save(sdbus_s* sd){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "Save",
		NULL
	);
	if( !msg ){
		return NULL;
	}


	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Update2(sdbus_s* sd, sdbusKV_s* vin_settings, uint32_t in_flags, sdbusKV_s* vin_args, sdbusKV_s** vout_result){
	sdbusMessage_h msg = sdbus_method(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "Update2",
		"a{sa{sv}}ua{sv}",
		vin_settings,
		in_flags,
		vin_args
	);
	if( !msg ){
		return NULL;
	}

	if( sdbus_message_read(msg, "a{sv}",
		vout_result)
	){
		sdbus_message_free(msg);
		return NULL;
	}

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Unsaved_get(sdbus_s* sd, int* Unsaved){
	sdbusVariant_s tmpUnsaved;
	tmpUnsaved.type = "b";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "Unsaved",
		&tmpUnsaved
	);
	if( !msg ){
		return NULL;
	}

	*Unsaved = tmpUnsaved.var.b;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Flags_get(sdbus_s* sd, uint32_t* Flags){
	sdbusVariant_s tmpFlags;
	tmpFlags.type = "u";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "Flags",
		&tmpFlags
	);
	if( !msg ){
		return NULL;
	}

	*Flags = tmpFlags.var.u;

	return msg;
};

sdbusMessage_h org_freedesktop_NetworkManager_Settings_Connection_Filename_get(sdbus_s* sd, char** Filename){
	sdbusVariant_s tmpFilename;
	tmpFilename.type = "s";
	sdbusMessage_h msg = sdbus_property_get(
		sd,
		"org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager.Settings.Connection", "Filename",
		&tmpFilename
	);
	if( !msg ){
		return NULL;
	}

	*Filename = tmpFilename.var.s;

	return msg;
};

