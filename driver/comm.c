#include "comm.h"

extern NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(HANDLE, PEPROCESS*);
extern NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS, PVOID, PEPROCESS, PVOID,
                                                SIZE_T, KPROCESSOR_MODE, PSIZE_T);
extern NTKERNELAPI PPEB     PsGetProcessPeb(PEPROCESS);

static NTSTATUS CopyMem(ULONG pid, PVOID src, PVOID dst, SIZE_T size, BOOLEAN write) {
    PEPROCESS target = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)pid, &target);
    if (!NT_SUCCESS(status)) return status;

    SIZE_T done = 0;
    if (write) {
        status = MmCopyVirtualMemory(PsGetCurrentProcess(), src,
                                     target, dst, size, KernelMode, &done);
    } else {
        status = MmCopyVirtualMemory(target, src,
                                     PsGetCurrentProcess(), dst, size, KernelMode, &done);
    }
    ObDereferenceObject(target);
    return status;
}

static NTSTATUS FindPidByName(PCWSTR name, PULONG outPid) {
    *outPid = 0;
    ULONG size = 0;
    ZwQuerySystemInformation(5, NULL, 0, &size);
    if (size == 0) return STATUS_UNSUCCESSFUL;

    PVOID buf = ExAllocatePool2(POOL_FLAG_NON_PAGED, size, 'praL');
    if (!buf) return STATUS_INSUFFICIENT_RESOURCES;

    NTSTATUS st = ZwQuerySystemInformation(5, buf, size, &size);
    if (!NT_SUCCESS(st)) { ExFreePool(buf); return st; }

    PUCHAR p = (PUCHAR)buf;
    UNICODE_STRING want;
    RtlInitUnicodeString(&want, name);
    for (;;) {
        ULONG next = *(ULONG*)p;
        UNICODE_STRING* uname = (UNICODE_STRING*)(p + 0x38);
        HANDLE pid = *(HANDLE*)(p + 0x50);
        if (uname->Buffer && uname->Length) {
            if (RtlEqualUnicodeString(uname, &want, TRUE)) {
                *outPid = (ULONG)(ULONG_PTR)pid;
                break;
            }
        }
        if (!next) break;
        p += next;
    }
    ExFreePool(buf);
    return (*outPid) ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}

static NTSTATUS FindModuleBase(ULONG pid, PCWSTR moduleName, PVOID* outBase, PSIZE_T outSize) {
    PEPROCESS proc = NULL;
    NTSTATUS st = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)pid, &proc);
    if (!NT_SUCCESS(st)) return st;

    KAPC_STATE apc;
    KeStackAttachProcess(proc, &apc);

    PPEB peb = PsGetProcessPeb(proc);
    if (!peb || !peb->Ldr) {
        KeUnstackDetachProcess(&apc);
        ObDereferenceObject(proc);
        return STATUS_NOT_FOUND;
    }

    PLIST_ENTRY head = &peb->Ldr->InMemoryOrderModuleList;
    UNICODE_STRING want;
    RtlInitUnicodeString(&want, moduleName);
    for (PLIST_ENTRY cur = head->Flink; cur != head; cur = cur->Flink) {
        PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(cur, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        if (entry->FullDllName.Buffer &&
            RtlCompareUnicodeString(&entry->BaseDllName, &want, TRUE) == 0) {
            *outBase = entry->DllBase;
            *outSize = entry->SizeOfImage;
            KeUnstackDetachProcess(&apc);
            ObDereferenceObject(proc);
            return STATUS_SUCCESS;
        }
    }

    KeUnstackDetachProcess(&apc);
    ObDereferenceObject(proc);
    return STATUS_NOT_FOUND;
}

NTSTATUS LarpDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION sp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG written = 0;

    switch (sp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_LARP_READ: {
        PLARP_RW r = (PLARP_RW)Irp->AssociatedIrp.SystemBuffer;
        if (sp->Parameters.DeviceIoControl.InputBufferLength < sizeof(LARP_RW)) break;
        PVOID tmp = ExAllocatePool2(POOL_FLAG_NON_PAGED, r->Size, 'praL');
        if (!tmp) { status = STATUS_INSUFFICIENT_RESOURCES; break; }
        status = CopyMem(r->Pid, r->Address, tmp, r->Size, FALSE);
        if (NT_SUCCESS(status)) {
            RtlCopyMemory(r->Buffer, tmp, r->Size);
            written = sizeof(LARP_RW);
        }
        ExFreePool(tmp);
        break;
    }
    case IOCTL_LARP_WRITE: {
        PLARP_RW r = (PLARP_RW)Irp->AssociatedIrp.SystemBuffer;
        if (sp->Parameters.DeviceIoControl.InputBufferLength < sizeof(LARP_RW)) break;
        status = CopyMem(r->Pid, r->Buffer, r->Address, r->Size, TRUE);
        if (NT_SUCCESS(status)) written = sizeof(LARP_RW);
        break;
    }
    case IOCTL_LARP_BASE: {
        PLARP_BASE b = (PLARP_BASE)Irp->AssociatedIrp.SystemBuffer;
        status = FindModuleBase(b->Pid, b->ModuleName, &b->OutBase, &b->OutSize);
        written = sizeof(LARP_BASE);
        break;
    }
    case IOCTL_LARP_PID: {
        PLARP_PID p = (PLARP_PID)Irp->AssociatedIrp.SystemBuffer;
        status = FindPidByName(p->ProcessName, &p->OutPid);
        written = sizeof(LARP_PID);
        break;
    }
    default: break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = written;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}
