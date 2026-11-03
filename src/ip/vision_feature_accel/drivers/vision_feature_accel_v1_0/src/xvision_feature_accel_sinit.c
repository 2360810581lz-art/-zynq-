// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.1 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xvision_feature_accel.h"

extern XVision_feature_accel_Config XVision_feature_accel_ConfigTable[];

XVision_feature_accel_Config *XVision_feature_accel_LookupConfig(u16 DeviceId) {
	XVision_feature_accel_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XVISION_FEATURE_ACCEL_NUM_INSTANCES; Index++) {
		if (XVision_feature_accel_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XVision_feature_accel_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XVision_feature_accel_Initialize(XVision_feature_accel *InstancePtr, u16 DeviceId) {
	XVision_feature_accel_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XVision_feature_accel_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XVision_feature_accel_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

