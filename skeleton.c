#include <efi.h>
#include <efilib.h>
#include <stdio.h>

#define EFI_DEBUG

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {

	SIMPLE_TEXT_OUTPUT_INTERFACE *conout;
	conout = systab->ConOut;
	//uefi_call_wrapper(conout->OutputString, 2, conout, L"Hello World\n\r");

	InitializeLib(image, systab);

	Print(L"Hello again (%d)\n\r", 2);

	// EFI_GET_MEMORY_MAP, EFI_ALLOCATE_PAGES, EFI_FREE_PAGES
	UINTN mapSize = 0;
	EFI_STATUS retstat;
	retstat = uefi_call_wrapper(BS->GetMemoryMap, 5, &mapSize, NULL, NULL, NULL,
			NULL);

	if (retstat == EFI_BUFFER_TOO_SMALL) {
		Print(L"The memory map requires %d bytes\n\r", mapSize);
	}


	return EFI_SUCCESS;
}
