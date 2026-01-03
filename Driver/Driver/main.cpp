#include <ntddk.h>
#include <string.h>
#include <ntstrsafe.h>

#define SIOCTL_TYPE 40000
#define IOCTL_HI \
    CTL_CODE( SIOCTL_TYPE, 0x900, METHOD_BUFFERED , FILE_ANY_ACCESS  )
#define IOCTL_SUBSCRIBE_LOG \
    CTL_CODE( SIOCTL_TYPE, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS  )
#define IOCTL_ADD_RULE \
    CTL_CODE( SIOCTL_TYPE, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS  )
#define IOCTL_TOGGLE_PROT \
    CTL_CODE( SIOCTL_TYPE, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS  )

#define MAX_SIZE 100

bool active_prot = false;

template <typename T>
class array_class
{
private:
    T cache[MAX_SIZE] = {};
    int array_size = 0;

public:
    bool push(const T value)
    {
        if (array_size < MAX_SIZE)
        {
            cache[array_size] = value;
            array_size += 1;
            return true;
        }
        return false;
    }

    void clear()
    {
        for (int i = 0; i < array_size; i++)
        {
            cache[i] = T();
        }
        array_size = 0;
    }

    T get(int index)
    {
        if (index >= 0 && index < array_size)
        {
            return cache[index];
        }
        return T();
    }

    bool erase(int index)
    {
        if (index >= 0 && index < array_size)
        {
            for (int i = index; i < array_size - 1; i++)
            {
                cache[i] = cache[i + 1];
            }
            cache[array_size - 1] = T();
            array_size -= 1;
            return true;
        }
        return false;
    }

    int size()
    {
        return array_size;
    }
};

array_class<const wchar_t*> cmds;

PIRP g_PendingIrp = NULL;
FAST_MUTEX g_IrpMutex;
KSPIN_LOCK g_IrpSpinLock;
BOOLEAN g_SpinLockInitialized = FALSE;

VOID
PrintChars(
    _In_reads_(CountChars) PCHAR BufferAddress,
    _In_ size_t CountChars
)
{
    PAGED_CODE();

    if (CountChars) {

        while (CountChars--) {

            if (*BufferAddress > 31
                && *BufferAddress != 127) {

                KdPrint(("%c", *BufferAddress));

            }
            else {

                KdPrint(("."));

            }
            BufferAddress++;
        }
        KdPrint(("\n"));
    }
    return;
}


VOID SendLogToUsermode(PWCHAR LogMessage)
{
    KIRQL oldIrql;
    PIRP pendingIrp = NULL;

    if (!g_SpinLockInitialized) {
        return;
    }

    KeAcquireSpinLock(&g_IrpSpinLock, &oldIrql);

    if (g_PendingIrp != NULL) {
        pendingIrp = g_PendingIrp;
        g_PendingIrp = NULL;
    }

    KeReleaseSpinLock(&g_IrpSpinLock, oldIrql);

    if (pendingIrp != NULL) {
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(pendingIrp);
        ULONG outBufLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
        PVOID outBuf = pendingIrp->AssociatedIrp.SystemBuffer;

        SIZE_T messageLength = wcslen(LogMessage) * sizeof(WCHAR);
        SIZE_T bytesToCopy = min(messageLength, outBufLength);

        if (bytesToCopy > 0) {
            RtlCopyMemory(outBuf, LogMessage, bytesToCopy);
            pendingIrp->IoStatus.Information = bytesToCopy;
            pendingIrp->IoStatus.Status = STATUS_SUCCESS;
        }
        else {
            pendingIrp->IoStatus.Information = 0;
            pendingIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        }

        IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
    }
}

PCWSTR GetProcessName(PCUNICODE_STRING ImagePath)
{
    PCWSTR name = ImagePath->Buffer;
    for (USHORT i = 0; i < ImagePath->Length / sizeof(WCHAR); i++)
    {
        if (ImagePath->Buffer[i] == L'\\')
            name = &ImagePath->Buffer[i + 1];
    }
    return name;
}


BOOLEAN MatchRule(
    PCWSTR rule,
    PCUNICODE_STRING ImageFileName,
    PCWSTR CommandLine
)
{
    WCHAR proc[32] = { 0 };
    WCHAR arg[128] = { 0 };

    // Find colon
    PCWSTR colon = wcschr(rule, L':');
    if (!colon)
        return FALSE;

    // Copy process name
    size_t procLen = colon - rule;
    if (procLen >= ARRAYSIZE(proc))
        return FALSE;

    RtlCopyMemory(proc, rule, procLen * sizeof(WCHAR));
    proc[procLen] = L'\0';

    // Copy argument
    RtlStringCchCopyW(arg, ARRAYSIZE(arg), colon + 1);

    // Compare process name
    PCWSTR imageName = GetProcessName(ImageFileName);

    if (_wcsicmp(imageName, proc) != 0 &&
        _wcsicmp(imageName, wcscat(proc, L".exe")) != 0)
        return FALSE;

    // Match argument in command line
    if (wcsstr(CommandLine, arg))
        return TRUE;

    return FALSE;
}


VOID
CreateProcessNotifyRoutine(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId,
    _In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
    if ((CreateInfo != NULL) && (active_prot))
    {
        DbgPrintEx(
            DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL,
            "Process %p (ID 0x%p) created, creator %Ix:%Ix\n"
            "    command line %wZ\n"
            "    file name %wZ (FileOpenNameAvailable: %d)\n",
            Process,
            (PVOID)ProcessId,
            (ULONG_PTR)CreateInfo->CreatingThreadId.UniqueProcess,
            (ULONG_PTR)CreateInfo->CreatingThreadId.UniqueThread,
            CreateInfo->CommandLine,
            CreateInfo->ImageFileName,
            CreateInfo->FileOpenNameAvailable
        );

        WCHAR logBuffer[512];
        UNICODE_STRING cmdLineCopy;

        if (CreateInfo->CommandLine && CreateInfo->CommandLine->Buffer) {
            cmdLineCopy.Length = 0;
            cmdLineCopy.MaximumLength = sizeof(logBuffer) - 100;
            cmdLineCopy.Buffer = logBuffer + 50;

            RtlCopyUnicodeString(&cmdLineCopy, CreateInfo->CommandLine);

            wchar_t* asd = cmdLineCopy.Buffer;

            for (int i = 0; i < cmds.size(); i++) {
                if (MatchRule(cmds.get(i), CreateInfo->ImageFileName, asd)) {
                    DbgPrintEx(
                        DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL,
                        "PWSH CMD\n"
                    );

                    RtlStringCchPrintfW(logBuffer, 512, L"BLOCKED %wZ", CreateInfo->CommandLine);
                    SendLogToUsermode(logBuffer);

                    CreateInfo->CreationStatus = STATUS_ACCESS_DENIED;
                }/*
                else {
                    RtlStringCchPrintfW(logBuffer, 512, L"Process: %wZ", CreateInfo->ImageFileName);
                    SendLogToUsermode(logBuffer);
                }*/
            }
        }

    }
}

VOID CancelPendingIrp(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(DeviceObject);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    KeAcquireSpinLock(&g_IrpSpinLock, &oldIrql);

    if (g_PendingIrp == Irp) {
        g_PendingIrp = NULL;
    }

    KeReleaseSpinLock(&g_IrpSpinLock, oldIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

NTSTATUS
SioctlCreateClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
)
{
    PIO_STACK_LOCATION irpSp;

    UNREFERENCED_PARAMETER(DeviceObject);

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (irpSp->MajorFunction == IRP_MJ_CLOSE && g_SpinLockInitialized) {
        KIRQL oldIrql;
        PIRP pendingIrp = NULL;

        KeAcquireSpinLock(&g_IrpSpinLock, &oldIrql);

        if (g_PendingIrp != NULL) {
            pendingIrp = g_PendingIrp;
            g_PendingIrp = NULL;
        }

        KeReleaseSpinLock(&g_IrpSpinLock, oldIrql);

        // Cancel the pending IRP outside the lock
        if (pendingIrp != NULL) {
            if (IoSetCancelRoutine(pendingIrp, NULL) != NULL) {
                pendingIrp->IoStatus.Status = STATUS_CANCELLED;
                pendingIrp->IoStatus.Information = 0;
                IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
            }
        }
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
SioctlDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
)

{
    PIO_STACK_LOCATION  irpSp;// Pointer to current stack location
    NTSTATUS            ntStatus = STATUS_SUCCESS;// Assume success
    ULONG               inBufLength; // Input buffer length
    ULONG               outBufLength; // Output buffer length
    PCHAR               inBuf, outBuf; // pointer to Input and output buffer
    PCHAR               data = "Device Driver is Working Correctly !!!";
    size_t              datalen = strlen(data) + 1;//Length of data including null
    KIRQL               oldIrql; // NEW

    UNREFERENCED_PARAMETER(DeviceObject);

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    inBufLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    outBufLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

    //
    // Determine which I/O control code was specified.
    //

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
    {

    case IOCTL_HI:

        if (!inBufLength || !outBufLength)
        {
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }

        KdPrint(("Called IOCTL_SIOCTL_METHOD_BUFFERED\n"));

        inBuf = (PCHAR)Irp->AssociatedIrp.SystemBuffer;
        outBuf = (PCHAR)Irp->AssociatedIrp.SystemBuffer;

        KdPrint(("\tData from User :"));

        PrintChars(inBuf, inBufLength);


        RtlCopyBytes(outBuf, data, outBufLength);

        //KdPrint(("\tData to User : "));
        //PrintChars(outBuf, datalen);

        Irp->IoStatus.Information = (outBufLength < datalen ? outBufLength : datalen);

        break;

    case IOCTL_ADD_RULE: {
        if (!inBufLength || !outBufLength)
        {
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }
        KdPrint(("Called IOCTL_ADD_RULE\n"));
        inBuf = (PCHAR)Irp->AssociatedIrp.SystemBuffer;
        outBuf = (PCHAR)Irp->AssociatedIrp.SystemBuffer;

        ANSI_STRING ansiStr;
        RtlInitAnsiString(&ansiStr, inBuf);

        UNICODE_STRING unicodeStr;
        ntStatus = RtlAnsiStringToUnicodeString(&unicodeStr, &ansiStr, TRUE);

        if (NT_SUCCESS(ntStatus))
        {
            bool found = false;
            for (int i = 0; i < cmds.size(); i++) {
                if (NULL != wcsstr(unicodeStr.Buffer, cmds.get(i))) {
                    found = true;
                    KdPrint(("Found\n"));
                    break;
                }
            }

            if (!found)
                cmds.push(unicodeStr.Buffer);
        }

        PCHAR data2 = "Success!";
        size_t datalen2 = strlen(data2) + 1;
        RtlCopyBytes(outBuf, data2, outBufLength);
        Irp->IoStatus.Information = (outBufLength < datalen2 ? outBufLength : datalen2);
        break;
    }

    case IOCTL_TOGGLE_PROT: {
        if (!inBufLength || !outBufLength)
        {
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }
        KdPrint(("Called IOCTL_TOGGLE_PROT\n"));
        inBuf = (PCHAR)Irp->AssociatedIrp.SystemBuffer;
        outBuf = (PCHAR)Irp->AssociatedIrp.SystemBuffer;

        ANSI_STRING ansiStr;
        RtlInitAnsiString(&ansiStr, inBuf);

        UNICODE_STRING unicodeStr;
        ntStatus = RtlAnsiStringToUnicodeString(&unicodeStr, &ansiStr, TRUE);

        if (NT_SUCCESS(ntStatus))
        {
            wchar_t* asd = L"START";
            if (NULL != wcsstr(unicodeStr.Buffer, asd)) {
                active_prot = true;
            }
            else {
                active_prot = false;
            }
        }

        PCHAR data2 = "Success!";
        size_t datalen2 = strlen(data2) + 1;
        RtlCopyBytes(outBuf, data2, outBufLength);
        Irp->IoStatus.Information = (outBufLength < datalen2 ? outBufLength : datalen2);
        break;
    }
    case IOCTL_SUBSCRIBE_LOG:
        KdPrint(("Called IOCTL_SUBSCRIBE_LOG\n"));

        if (!outBufLength) {
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }

        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, CancelPendingIrp);

        KeAcquireSpinLock(&g_IrpSpinLock, &oldIrql);

        if (g_PendingIrp != NULL) {
            PIRP oldIrp = g_PendingIrp;
            g_PendingIrp = NULL;
            KeReleaseSpinLock(&g_IrpSpinLock, oldIrql);

            oldIrp->IoStatus.Status = STATUS_CANCELLED;
            oldIrp->IoStatus.Information = 0;
            IoCompleteRequest(oldIrp, IO_NO_INCREMENT);

            KeAcquireSpinLock(&g_IrpSpinLock, &oldIrql);
        }

        g_PendingIrp = Irp;
        KeReleaseSpinLock(&g_IrpSpinLock, oldIrql);

        return STATUS_PENDING;

    default:

        //
        // The specified I/O control code is unrecognized by this driver.
        //

        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        KdPrint(("ERROR: unrecognized IOCTL %x\n",
            irpSp->Parameters.DeviceIoControl.IoControlCode));
        break;
    }

    //
    // Finish the I/O operation by simply completing the packet and returning
    // the same status as in the packet itself.
    //

    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return ntStatus;
}

VOID
SioctlUnloadDriver(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;
    UNICODE_STRING uniWin32NameString;
    KIRQL oldIrql; // NEW

    PAGED_CODE();

    //
    // Create counted string version of our Win32 device name.
    //

    RtlInitUnicodeString(&uniWin32NameString, L"\\DosDevices\\LoLScanDRV");


    //
    // Delete the link from our device name to a name in the Win32 namespace.
    //

    IoDeleteSymbolicLink(&uniWin32NameString);

    if (deviceObject != NULL)
    {
        IoDeleteDevice(deviceObject);
    }

    PsSetCreateProcessNotifyRoutineEx(
        CreateProcessNotifyRoutine,
        TRUE
    );

    if (g_SpinLockInitialized) {
        KeAcquireSpinLock(&g_IrpSpinLock, &oldIrql);
        if (g_PendingIrp != NULL) {
            PIRP irp = g_PendingIrp;
            g_PendingIrp = NULL;
            KeReleaseSpinLock(&g_IrpSpinLock, oldIrql);

            irp->IoStatus.Status = STATUS_CANCELLED;
            irp->IoStatus.Information = 0;
            IoCompleteRequest(irp, IO_NO_INCREMENT);
        }
        else {
            KeReleaseSpinLock(&g_IrpSpinLock, oldIrql);
        }
    }

    UNREFERENCED_PARAMETER(DriverObject);
    KdPrint(("Driver unloaded\n"));


}

NTSTATUS DriverEntryCPP(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    //DriverObject->DriverUnload = DriverUnload;

    KdPrint(("Driver loaded\n"));

    NTSTATUS        ntStatus;
    UNICODE_STRING  ntUnicodeString;    // NT Device Name "\Device\SIOCTL"
    UNICODE_STRING  ntWin32NameString;    // Win32 Name "\DosDevices\IoctlTest"
    PDEVICE_OBJECT  deviceObject = NULL;    // ptr to device object

    KeInitializeSpinLock(&g_IrpSpinLock);
    g_SpinLockInitialized = TRUE;

    RtlInitUnicodeString(&ntUnicodeString, L"\\Device\\LoLScanDRV");

    ntStatus = IoCreateDevice(
        DriverObject,                   // Our Driver Object
        0,                              // We don't use a device extension
        &ntUnicodeString,               // Device name "\Device\SIOCTL"
        FILE_DEVICE_UNKNOWN,            // Device type
        FILE_DEVICE_SECURE_OPEN,     // Device characteristics
        FALSE,                          // Not an exclusive device
        &deviceObject);                // Returned ptr to Device Object

    if (!NT_SUCCESS(ntStatus))
    {
        KdPrint(("Couldn't create the device object\n"));
        return ntStatus;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = SioctlCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = SioctlCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SioctlDeviceControl;
    DriverObject->DriverUnload = SioctlUnloadDriver;

    //
    // Initialize a Unicode String containing the Win32 name
    // for our device.
    //

    RtlInitUnicodeString(&ntWin32NameString, L"\\DosDevices\\LoLScanDRV");

    //
    // Create a symbolic link between our device name  and the Win32 name
    //

    ntStatus = IoCreateSymbolicLink(
        &ntWin32NameString, &ntUnicodeString);

    if (!NT_SUCCESS(ntStatus))
    {
        //
        // Delete everything that this routine has allocated.
        //
        KdPrint(("Couldn't create symbolic link\n"));
        IoDeleteDevice(deviceObject);
    }





    // =============================== CALLBACK FOR PROCESS ===============================

    NTSTATUS Status = PsSetCreateProcessNotifyRoutineEx(
        CreateProcessNotifyRoutine,
        FALSE
    );

    if (!NT_SUCCESS(Status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DriverEntry: PsSetCreateProcessNotifyRoutineEx(2) returned 0x%x\n", Status);

        return STATUS_UNSUCCESSFUL;

    }


    return STATUS_SUCCESS;
}

extern "C"
{
    DRIVER_INITIALIZE DriverEntry;
    _Use_decl_annotations_
        NTSTATUS
        DriverEntry(
            struct _DRIVER_OBJECT* DriverObject,
            PUNICODE_STRING  RegistryPath
        )
    {
        DriverEntryCPP(DriverObject, RegistryPath);
        return STATUS_SUCCESS;
    }
}
