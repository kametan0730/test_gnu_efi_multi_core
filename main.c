#include <efi.h>
#include <efilib.h>

struct EFI_CPU_PHYSICAL_LOCATION {
    unsigned int Package;
    unsigned int Core;
    unsigned int Thread;
};

struct EFI_PROCESSOR_INFORMATION {
    unsigned long long ProcessorId;
    unsigned int StatusFlag;
    struct EFI_CPU_PHYSICAL_LOCATION Location;
};

struct EFI_MP_SERVICES_PROTOCOL {
    unsigned long long (*GetNumberOfProcessors)(
            struct EFI_MP_SERVICES_PROTOCOL *This,
            unsigned long long *NumberOfProcessors,
            unsigned long long *NumberOfEnabledProcessors);
    unsigned long long (*GetProcessorInfo)(
            struct EFI_MP_SERVICES_PROTOCOL *This,
            unsigned long long ProcessorNumber,
            struct EFI_PROCESSOR_INFORMATION *ProcessorInfoBuffer);
    unsigned long long (*StartupAllAPs)(
            struct EFI_MP_SERVICES_PROTOCOL *This,
            void (*Procedure)(void *ProcedureArgument),
            unsigned char SingleThread,
            void *WaitEvent,
            unsigned long long TimeoutInMicroSeconds,
            void *ProcedureArgument,
            unsigned long long **FailedCpuList);
    unsigned long long (*StartupThisAP)(
            struct EFI_MP_SERVICES_PROTOCOL *This,
            void (*Procedure)(void *ProcedureArgument),
            unsigned long long ProcessorNumber,
            void *WaitEvent,
            unsigned long long TimeoutInMicroseconds,
            void *ProcedureArgument,
            unsigned char *Finished);
    unsigned long long (*SwitchBSP)(
            struct EFI_MP_SERVICES_PROTOCOL *This,
            unsigned long long ProcessorNumber,
            unsigned char EnableOldBSP);
    unsigned long long (*EnableDisableAP)(
            struct EFI_MP_SERVICES_PROTOCOL *This,
            unsigned long long ProcessorNumber,
            unsigned char EnableAP,
            unsigned int *HealthFlag);
    unsigned long long (*WhoAmI)(
            struct EFI_MP_SERVICES_PROTOCOL *This,
            unsigned long long *ProcessorNumber);
};

_Noreturn void halt(){
    while(1){
        __asm__("hlt");
    }
}

struct EFI_MP_SERVICES_PROTOCOL* MSP;

_Noreturn void start_ap(EFI_SYSTEM_TABLE* system_table){
    UINT64 processor_number;
    UINT64 status = MSP->WhoAmI(MSP, &processor_number);
    Print(L"Hello %d core!\n", processor_number);
    //halt();
}

_Noreturn EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table){
    InitializeLib(image_handle, system_table);
    Print(L"Hello loader!!\n");
    EFI_GUID MSP_GUID = {0x3fdda605, 0xa76e, 0x4f46,
                                {0xad, 0x29, 0x12, 0xf4,
                                 0x53, 0x1b, 0x3d, 0x08}};

    EFI_STATUS status;


    status = uefi_call_wrapper(system_table->BootServices->LocateProtocol, 3, &MSP_GUID, NULL, (void **) &MSP);
    if(EFI_ERROR(status)){
        Print(L"Failed to locate mp services: %r\n", status);
        halt();
    }
    UINT64 num_of_processors, num_of_enabled_processors;
    status = uefi_call_wrapper(MSP->GetNumberOfProcessors, 3, MSP, &num_of_processors, &num_of_enabled_processors);
    if(EFI_ERROR(status)){
        Print(L"Failed to get num of processors: %r\n", status);
        halt();
    }
    Print(L"Num of Processors: %d\n", num_of_processors);
    Print(L"Num of Enabled Processors: %d\n", num_of_enabled_processors);

    status = uefi_call_wrapper(MSP->StartupAllAPs, 7, MSP, &start_ap, 0, NULL, 1000000, system_table, NULL);
    Print(L"Returned!\n");
    halt();
}