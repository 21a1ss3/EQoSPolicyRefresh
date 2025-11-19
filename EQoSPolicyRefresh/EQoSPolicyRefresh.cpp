#include <stdio.h>
#include <Windows.h>
#include <winternl.h>

typedef struct _QOS_NOTIFICATION_PAYLOAD {
    DWORD f1;
    DWORD f2;
    DWORD f3;
    DWORD f4;
    DWORD f5;
} QOS_NOTIFICATION_PAYLOAD;

int main()
{
    const wchar_t* filename = L"\\Device\\eQoS";
    HANDLE file = NULL;
    OBJECT_ATTRIBUTES attributes;
    UNICODE_STRING filenameStruct;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE* _hnd;

    attributes.Length = sizeof(attributes);
    attributes.Attributes = OBJ_CASE_INSENSITIVE;
    attributes.ObjectName = &filenameStruct;
    attributes.RootDirectory = NULL;
    attributes.SecurityDescriptor = NULL;
    attributes.SecurityQualityOfService = NULL;
    filenameStruct.Length = wcslen(filename) * sizeof(wchar_t);
    filenameStruct.MaximumLength = filenameStruct.Length + sizeof(wchar_t);
    filenameStruct.Buffer = (wchar_t*)filename;
    ioStatusBlock.Information = NULL;
    ioStatusBlock.Pointer = NULL;
    ioStatusBlock.Status = 0;

    printf_s("Preparing for submitting an update request\n");
    fopen_s(&_hnd, "QoS_Refetcher_start", "r");

    printf_s("attributes.Length: %d; \n", attributes.Length);

    NTSTATUS ntStatus = NtCreateFile(
        &file,
        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
        &attributes,
        &ioStatusBlock,
        NULL,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        0,
        NULL,
        0
    );

    printf_s("Opening device completed with NTSTATUS 0x%0.8x\n", ntStatus);

    if (!NT_SUCCESS(ntStatus)) {
        fopen_s(&_hnd, "QoS_Refetcher_failed", "r");
        printf_s("Failed to open device, leaving\n");

        return -1;
    }

    printf_s("Sending command to reapply QoS policies");

    QOS_NOTIFICATION_PAYLOAD payload;
    // Purpose or structure is not yet known. Captured by API sniffer
    payload.f1 = 1;
    payload.f2 = 2;
    payload.f3 = 0;
    payload.f4 = 0;
    payload.f5 = 0;

    ntStatus = NtDeviceIoControlFile(
        file,
        NULL,
        NULL,
        NULL,
        &ioStatusBlock,
        0xC07FC000,
        &payload,
        sizeof(QOS_NOTIFICATION_PAYLOAD),
        NULL,
        0
    );

    printf_s("io control completed with NTSTATUS 0x%0.8x\n", ntStatus);
    NtClose(file);

    printf_s("Completed an attempt\n");
    fopen_s(&_hnd, "QoS_Refetcher_stop", "r");

    return 0;
}