#include <efi.h>
#include <efilib.h>

VOID searchBuffer(IN VOID *Buffer, IN UINT64 BufferLength);
VOID printMemoryMap(IN UINT8 *mapDescBuffer, UINTN mapBytes, IN UINTN descSize);

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
	// Initialize the library
	InitializeLib(image, systab);

	// Get how much space is needed for the memory map
	UINTN mapBytes = 0;
	EFI_STATUS retstat;
	retstat = uefi_call_wrapper(BS->GetMemoryMap, 5, &mapBytes, NULL, NULL, NULL,
			NULL);
	if (retstat == EFI_BUFFER_TOO_SMALL) {
		Print(L"The memory map requires %d bytes\n\r", mapBytes);
	}
	// Convert bytes to 4KiB pages (rounded up), plus a little extra to be safe
	mapBytes += 512;
	UINTN numPages = EFI_SIZE_TO_PAGES(mapBytes);

	// Allocate
	EFI_PHYSICAL_ADDRESS mapDescPages = 0;
	retstat = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages,
			EfiLoaderData, numPages, &mapDescPages);
	if (retstat != EFI_SUCCESS) {
		Print(L"Something went wrong allocating %d pages: %r\r\n", numPages, retstat);
		Print(L"Descs: %x\r\n", mapDescPages);
		goto exit0;
	} else {
		Print(L"Allocation succeeded: 0x%lx\r\n", mapDescPages);
	}

	// Get the memory map
	UINTN descSize;
	UINT8 *mapDescBuffer = (UINT8 *)mapDescPages;
	retstat = uefi_call_wrapper(BS->GetMemoryMap, 5, &mapBytes, mapDescBuffer,
			NULL, &descSize, NULL);
	if (retstat != EFI_SUCCESS) {
		Print(L"BS->GetMemoryMap(...): %r\n\r", retstat);
		goto exit1;
	}

	printMemoryMap(mapDescBuffer, mapBytes, descSize);


exit1:
	uefi_call_wrapper(BS->FreePages, 2, mapDescPages, numPages);
exit0:
	return EFI_SUCCESS;
}

VOID searchBuffer(IN VOID *Buffer, IN UINT64 BufferLength) {
	
}

VOID printMemoryMap(IN UINT8 *mapDescBuffer, UINTN mapBytes, IN UINTN descSize) {
	UINT64 totalPages = 0;
	UINT8 *i;
	Print(L"buffer len: %d, descSize: %ld\r\n\r\n", mapBytes, descSize);
	for (i = mapDescBuffer; i < mapDescBuffer + mapBytes; i += descSize) {
		EFI_MEMORY_DESCRIPTOR *mapDesc = (EFI_MEMORY_DESCRIPTOR *)i;
		switch(mapDesc->Type){
		case EfiReservedMemoryType:
			Print(L"Reserved\t");
			break;
		case EfiLoaderCode:
			Print(L"Loader Code\t");
			break;
		case EfiLoaderData:
			Print(L"Loader Data\t");
			break;
		case EfiBootServicesCode:
			Print(L"Boot Code\t");
			break;
		case EfiBootServicesData:
			Print(L"Boot Data\t");
			break;
		case EfiRuntimeServicesCode:
			Print(L"Runtime Code\t");
			break;
		case EfiRuntimeServicesData:
			Print(L"Runtime Data\t");
			break;
		case EfiConventionalMemory:
			Print(L"Conventional\t");
			break;
		case EfiUnusableMemory:
			Print(L"Unusable\t");
			break;
		case EfiACPIReclaimMemory:
			Print(L"ACPI Reclaim\t");
			break;
		case EfiACPIMemoryNVS:
			Print(L"ACPI NVS\t");
			break;
		case EfiMemoryMappedIO:
			Print(L"Mapped IO\t");
			break;
		case EfiMemoryMappedIOPortSpace:
			Print(L"Mapped IO Ports\t");
			break;
		case EfiPalCode:
			Print(L"CPU Reserved\t");
			break;
		default:
			if (mapDesc->Type > EfiMaxMemoryType && mapDesc->Type <= 0x7FFFFFFF) {
				Print(L"! Illegal\t");
			} else if (mapDesc->Type >= 0x80000000 && mapDesc->Type <= 0xFFFFFFFF) {
				Print(L"! Private\t");
			}
			break;
		}
		UINT64 startAddr = mapDesc->PhysicalStart;
		UINT64 endAddr = mapDesc->NumberOfPages;
		endAddr *= 4096;
		endAddr += startAddr;
		Print(L"0x%lx - 0x%lx\t(%ld)\r\n", startAddr, endAddr, mapDesc->NumberOfPages * 4);
		totalPages += mapDesc->NumberOfPages;
	}
	Print(L"Total pages: %ld\r\n", totalPages);
}
