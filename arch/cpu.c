////////////////////////////////////////////////////////////////////////////////
// $Id: cpu.c,v 1.6 2010/12/23 09:22:01 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <assert.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
#define CPU_FEAT_FPU	0x01
#define CPU_FEAT_VME	0x02
#define CPU_FEAT_DE		0x04
#define CPU_FEAT_PSE	0x08
#define CPU_FEAT_TSC	0x10
#define CPU_FEAT_MSR	0x20
#define CPU_FEAT_PAE	0x40
#define CPU_FEAT_MCE	0x80
#define CPU_FEAT_CX8	0x100
#define CPU_FEAT_APIC	0x200
#define CPU_FEAT_SEP	0x400
//
#define CPU_FEAT_MTRR	0x1000
#define CPU_FEAT_PGE	0x2000
#define CPU_FEAT_MCA	0x4000
#define CPU_FEAT_CMOV	0x8000
#define CPU_FEAT_PAT	0x10000
#define CPU_FEAT_PSE36  0x20000
#define CPU_FEAT_PSN    0x40000
#define CPU_FEAT_CLFL   0x80000
//
#define CPU_FEAT_DTES   0x200000
#define CPU_FEAT_TACPI  0x400000
#define CPU_FEAT_MMX    0x800000
#define CPU_FEAT_FXSR   0x1000000
#define CPU_FEAT_SSE    0x2000000
#define CPU_FEAT_SSE2   0x4000000
#define CPU_FEAT_SS     0x8000000
#define CPU_FEAT_HT     0x10000000
#define CPU_FEAT_TM1    0x20000000
#define CPU_FEAT_IA64   0x40000000
#define CPU_FEAT_PBE    0x80000000

#define IA32_PAT			0x277

#define IA32_MTRR_PHYSBASE0	0x200
#define IA32_MTRR_PHYSMASK0 0x201
#define IA32_MTRR_PHYSBASE1	0x202
#define IA32_MTRR_PHYSMASK1 0x203
#define IA32_MTRR_PHYSBASE2	0x204
#define IA32_MTRR_PHYSMASK2 0x205
#define IA32_MTRR_PHYSBASE3	0x206
#define IA32_MTRR_PHYSMASK3 0x207
#define IA32_MTRR_PHYSBASE4	0x208
#define IA32_MTRR_PHYSMASK4 0x209
#define IA32_MTRR_PHYSBASE5	0x20A
#define IA32_MTRR_PHYSMASK5 0x20B
#define IA32_MTRR_PHYSBASE6	0x20C
#define IA32_MTRR_PHYSMASK6 0x20D
#define IA32_MTRR_PHYSBASE7	0x20E
#define IA32_MTRR_PHYSMASK7 0x20F

#define PAT_UC			0x00
#define PAT_WC			0x01
#define PAT_WT			0x04
#define PAT_WP			0x05
#define PAT_WB			0x06
#define PAT_UC_			0x07

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////
typedef union {
	struct {
		unsigned long	max_eax;
		char	vendorid[12];
	} eax_0;
	
	struct {
		unsigned long	stepping	: 4;
		unsigned long	model		: 4;
		unsigned long	family		: 4;
		unsigned long	type		: 2;
		unsigned long	reserved_0	: 2;
		unsigned long   model_ext   : 4;
		unsigned long   family_ext  : 8;
		unsigned long 	reserved_1  : 4;
		
		unsigned long	reserved_2;
		unsigned long	features; 
		unsigned long	reserved_3;
	} eax_1;
	
	struct {
		unsigned char	call_num;
		unsigned char	cache_descriptors[15];
	} eax_2;

	struct {
		unsigned long	reserved[2];
		unsigned long	serial_number_high;
		unsigned long	serial_number_low;
	} eax_3;

	char		as_chars[16];

	struct {
		unsigned long	eax;
		unsigned long	ebx;
		unsigned long	edx;
		unsigned long	ecx;
	} regs; 
} cpuid_info;

typedef struct cpu_list_t
{
	int family;
	int model;
	int stepping;
	int type;
	char *name;
} cpu_list_t;

typedef struct pa_t
{
	unsigned char value : 4;
	unsigned char reserved : 4;
} pa_t;

typedef union
{
	pa_t pats[8];
	struct
	{
		unsigned long hi;
		unsigned long low;
	} raw;
} pat_msr_t;

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////
static cpu_list_t cpu_list[] =
{
	{.stepping=0, .model=2, .family=15, .type=0, .name="Intel Pentium 4 CPU"},
	{.stepping=8, .model=7, .family=6, .type=0, .name="Intel Pentium Core 2 Extreme CPU"},
	{.stepping=10, .model=7, .family=6, .type=0, .name="Intel Pentium Dual-Core CPU"},
	{.stepping=0, .model=0, .family=0, .type=0, .name=".end"},
};

////////////////////////////////////////////////////////////////////////////////
// Loacl Functions
////////////////////////////////////////////////////////////////////////////////
static void cpu_read_cpuid(unsigned long eax, cpuid_info *cpuid)
{
	__asm__("cpuid" : "=a" (cpuid->regs.eax), "=b" (cpuid->regs.ebx),"=d" (cpuid->regs.edx), "=c" (cpuid->regs.ecx) : "a" (eax));
}

static void cpu_read_msr(unsigned long msr, unsigned long *lo, unsigned long *hi)
{
   asm volatile("rdmsr":"=a"(*lo),"=d"(*hi):"c"(msr));
}

static void cpu_write_msr(unsigned long msr, unsigned long lo, unsigned long hi)
{
   asm volatile("wrmsr"::"a"(lo),"d"(hi),"c"(msr));
}


////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
/// Read CPU information
void cpu_init()
{
	kprintf(1, "cpu_init()\n", 0);
	cpuname(kinfo.cpuname);
	kprintf(2, "found CPU: %s\n", kinfo.cpuname);
	return;
	
	char cpu_magic[13];
	
	// EAX = 0 (Get Vendor ID)
	cpuid_info cpuid;
	cpu_read_cpuid(0, &cpuid);
	sprintf(cpu_magic, "%12s", cpuid.eax_0.vendorid, 0);
	cpu_magic[12] = 0;

	// EAX = 1 (Get Features and Flags)
	assert(cpuid.eax_0.max_eax >= 1);
	cpu_read_cpuid(1, &cpuid);
	
	int i = 0;
	while (1)
	{
		if ((cpu_list[i].stepping == 0) && (cpu_list[i].model == 0) && (cpu_list[i].family == 0) && (cpu_list[i].type == 0))
		{
			sprintf(kinfo.cpuname, "%s stepping: %d model: %d family: %d", cpu_magic, cpuid.eax_1.stepping, cpuid.eax_1.model, cpuid.eax_1.family);
			break;
		}
		if ((cpu_list[i].stepping == cpuid.eax_1.stepping) && 
			(cpu_list[i].model == cpuid.eax_1.model) && 
			(cpu_list[i].family == cpuid.eax_1.family) && 
			(cpu_list[i].type == cpuid.eax_1.type))
		{
			//sprintf(kinfo.cpuname, "%s", cpu_list[i].name);
			break;
		}
		i++;
	}
	
	kprintf(1, "cpu_init() %s\n", kinfo.cpuname);
}

void cpu_debug_pat()
{
	cpuid_info cpuid;
	cpu_read_cpuid(1, &cpuid);

	if (cpuid.eax_1.features & CPU_FEAT_PAT)
	{
		pat_msr_t patmsr;
		cpu_read_msr(IA32_PAT, &patmsr.raw.hi, &patmsr.raw.low);
		
		printf("IA32_PAT:\n");
		
		int i;
		for (i = 0; i < 8; i++)
		{
			printf(" PA%d: ", i);
			if (patmsr.pats[i].value == PAT_UC)
				printf("UC");
			if (patmsr.pats[i].value == PAT_WC)
				printf("WC");
			if (patmsr.pats[i].value == PAT_WT)
				printf("WT");
			if (patmsr.pats[i].value == PAT_WP)
				printf("WP");
			if (patmsr.pats[i].value == PAT_WB)
				printf("WB");
			if (patmsr.pats[i].value == PAT_UC_)
				printf("UC-");
			printf("\n");
		}
	}
}

void cpu_debug()
{
	printf("CPU Information\n");
	cpuid_info cpuid;
	cpu_read_cpuid(1, &cpuid);
	printf(" Family: %d Model: %d Stepping: %d\n", cpuid.eax_1.family, cpuid.eax_1.model, cpuid.eax_1.stepping);
	printf(" Ext. Family: %d Ext. Model: %d Revision: %d\n", cpuid.eax_1.family_ext, cpuid.eax_1.model_ext, cpuid.eax_1.type);
	printf(" Features: \n");
	printf(" ");
	if (cpuid.eax_1.features & CPU_FEAT_PBE)
		printf(" PBE ");
	if (cpuid.eax_1.features & CPU_FEAT_IA64)
		printf(" IA64 ");
	if (cpuid.eax_1.features & CPU_FEAT_TM1)
		printf(" TM1 ");
	if (cpuid.eax_1.features & CPU_FEAT_HT)
		printf(" HT ");
	if (cpuid.eax_1.features & CPU_FEAT_SS)
		printf(" SS ");
	if (cpuid.eax_1.features & CPU_FEAT_SSE2)
		printf(" SSE2 ");
	if (cpuid.eax_1.features & CPU_FEAT_SSE)
		printf(" SSE ");
	if (cpuid.eax_1.features & CPU_FEAT_FXSR)
		printf(" FXSR ");
	if (cpuid.eax_1.features & CPU_FEAT_MMX)
		printf(" MMX ");
	if (cpuid.eax_1.features & CPU_FEAT_TACPI)
		printf(" TACPI ");
	if (cpuid.eax_1.features & CPU_FEAT_DTES)
		printf(" DTES ");
	if (cpuid.eax_1.features & CPU_FEAT_CLFL)
		printf(" CLFL ");
	if (cpuid.eax_1.features & CPU_FEAT_PSN)
		printf(" PSN ");
	if (cpuid.eax_1.features & CPU_FEAT_PSE36)
		printf(" PSE36 ");
	if (cpuid.eax_1.features & CPU_FEAT_PAT)
		printf(" PAT ");
	printf("\n ");
	if (cpuid.eax_1.features & CPU_FEAT_CMOV)
		printf(" CMOV ");
	if (cpuid.eax_1.features & CPU_FEAT_MCA)
		printf(" MCA ");
	if (cpuid.eax_1.features & CPU_FEAT_PGE)
		printf(" PGE ");
	if (cpuid.eax_1.features & CPU_FEAT_MTRR)
		printf(" MTRR ");
	if (cpuid.eax_1.features & CPU_FEAT_SEP)
		printf(" SEP ");
	if (cpuid.eax_1.features & CPU_FEAT_APIC)
		printf(" APIC ");
	if (cpuid.eax_1.features & CPU_FEAT_CX8)
		printf(" CX8 ");
	if (cpuid.eax_1.features & CPU_FEAT_MCE)
		printf(" MCE ");
	if (cpuid.eax_1.features & CPU_FEAT_PAE)
		printf(" PAE ");
	if (cpuid.eax_1.features & CPU_FEAT_MSR)
		printf(" MSR ");
	if (cpuid.eax_1.features & CPU_FEAT_TSC)
		printf(" TSC ");
	if (cpuid.eax_1.features & CPU_FEAT_PSE)
		printf(" PSE ");
	if (cpuid.eax_1.features & CPU_FEAT_DE)
		printf(" DE ");
	if (cpuid.eax_1.features & CPU_FEAT_VME)
		printf(" VME ");
	if (cpuid.eax_1.features & CPU_FEAT_FPU)
		printf(" FPU ");
	printf("\n");
}
