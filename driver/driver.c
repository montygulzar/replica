#include <ntddk.h>
#include "comm.h"

#define DEVICE_NAME     L"\\Device\\Larp"
#define SYMLINK_NAME    L"\\DosDevices\\Larp"

PDEVICE_OBJECT g_DeviceObject = NULL;

NTSTATUS LarpCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

VOID LarpUnload(PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);
    UNICODE_STRING symlink = RTL_CONSTANT_STRING(SYMLINK_NAME);
    IoDeleteSymbolicLink(&symlink);
    if (g_DeviceObject) IoDeleteDevice(g_DeviceObject);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    UNICODE_STRING devName = RTL_CONSTANT_STRING(DEVICE_NAME);
    UNICODE_STRING symlink = RTL_CONSTANT_STRING(SYMLINK_NAME);

    NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName,
                                     FILE_DEVICE_UNKNOWN, 0, FALSE,
                                     &g_DeviceObject);
    if (!NT_SUCCESS(status)) return status;

    status = IoCreateSymbolicLink(&symlink, &devName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(g_DeviceObject);
        return status;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = LarpCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = LarpCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = LarpDeviceControl;
    DriverObject->DriverUnload = LarpUnload;

    g_DeviceObject->Flags |= DO_BUFFERED_IO;
    g_DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    return STATUS_SUCCESS;
}
