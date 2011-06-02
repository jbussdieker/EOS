#define KNOWN_BASECLASS_COUNT 18
static char *class_codes[] = {
	"Early PCI",
	"Mass Storage Controller",
	"Network Adapter",
	"Display Adapter",
	"Multimedia Device",
	"Memory",
	"Bridge Device",
	"Simple Communications",
	"Base System Peripheral",
	"Inupt Device",
	"Docking Station",
	"Processor",
	"Serial Bus",
	"Wireless",
	"Intelligent I/O",
	"Satellite Communication",
	"Encryption/Decryption",
	"Data Acquisition/Signal Processing",
};

char *sub_class[KNOWN_BASECLASS_COUNT][255] = {
	{ "SCSI", "IDE", "Floppy", "IPI", "RAID", "ATA w/ DMA"},
	{ "Ethernet", "Token Ring"},
	{ "VGA", "XGA", "3D"},
	{ "Video", "Audio", "Phone"},
	{ "RAM", "Flash"},
	{ "Host", "ISA", "EISA", "MCA", "PCI-to-PCI", "PCMCIA", "NuBus", "CardBus", "RACEway", "Semi-transparent PCI-to-PCI"},
	{ "Generic XT-compatible serial controller" },
	{ "PIC", "DMA", "Timer", "RTC", "PCI Hot-Plug" },
	{ "Keyboard", "Pen", "Mouse", "Scanner", "Gameport" },
	{ "Generic" },
	{ "386", "486", "Pentium" },
	{ "IEEE 1394", "ACCESS", "SSA", "Universal", "Fibre Channel", "SMBus"},
};

typedef struct pci_list_t
{
	unsigned short id;
	char *name;
} pci_list_t;

static pci_list_t pci_list[] =
{
	{.id=0x8086, .name="Intel"},
		{.id=0x1237, .name="PCI & Memory"},
		{.id=0x7000, .name="PIIX3 PCI-to-ISA Bridge"},
		{.id=0x7010, .name="PIIX3 IDE Interface"},
		{.id=0x7190, .name="440BX/ZX AGPset Host"},
		{.id=0x7191, .name="440BX/ZX AGPset PCI-to-PCI"},
		{.id=0x7110, .name="PIIX4/4E/4M ISBridgeA"},
		{.id=0x7111, .name="PIIX4/4E/4M IDE"},
		{.id=0x7112, .name="PIIX4/4E/4M USB Interface"},
		{.id=0x7113, .name="PIIX4/4E/4M Power Management"},
		{.id=0xFFFF, .name=".end"},

	{.id=0x15AD, .name="VMWare"},
		{.id=0x0405, .name="VMware SVGA"},
		{.id=0xFFFF, .name=".end"},

	{.id=0x10DE, .name="nVidia"},
		{.id=0x05E2, .name="GTX 260"},
		{.id=0xFFFF, .name=".end"},

	{.id=0x10EC, .name="Realtek"},
		{.id=0x8168, .name="RTL8168"},
		{.id=0xFFFF, .name=".end"},

	{.id=0x197B, .name="JMicron"},
		{.id=0x2368, .name="JMB368"},
		{.id=0xFFFF, .name=".end"},

	{.id=0x104B, .name="Buslogic"},
		{.id=0x1040, .name="BT958 SCSI"},
		{.id=0xFFFF, .name=".end"},

	{.id=0x1022, .name="AMD"},
		{.id=0x2000, .name="PCnet LANCE"},
		{.id=0xFFFF, .name=".end"},

	{.id=0xFFFF, .name=".end"},
};

static char *get_vendor_string(unsigned short vendor)
{
	int i = 0;
	while (1)
	{
		if (pci_list[i].id == 0xFFFF)
			return NULL;
		else if (pci_list[i].id == vendor)
			return pci_list[i].name;
		while (1)
			if (pci_list[i].id == 0xFFFF)
				break;
			else
				i++;
		i++;
	}
}

static char *get_device_string(unsigned short vendor, unsigned short device)
{
	int i = 0;
	while (1)
	{
		if (pci_list[i].id == 0xFFFF)
			return 0;
		else if (pci_list[i].id == vendor)
		{
			while (1)
				if (pci_list[i].id == 0xFFFF)
					return NULL;
				else if (pci_list[i].id == device)
					return pci_list[i].name;
				else
					i++;
		}
		while (1)
			if (pci_list[i].id == 0xFFFF)
				break;
			else
				i++;
		i++;
	}
}
