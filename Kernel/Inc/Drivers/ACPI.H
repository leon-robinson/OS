#pragma once

#include <TYPE.H>

typedef struct {
	char signature[8];
	u8 checksum;
	char oem_id[6];
	u8 revision;
	u32 rsdt_address;
} PACKED ACPI_RSDP;

typedef struct {
	char signature[4];
	u32 length;
	u8 revision;
	u8 checksum;
	char oem_id[6];
	char oem_table_id[8];
	u32 oem_revision;
	u32 creator_id;
	u32 creator_revision;
} PACKED ACPI_HEADER;

typedef enum APIC_TYPE {
	APIC_TYPE_LAPIC=0,
	APIC_TYPE_IOAPIC=1,
	APIC_TYPE_INTERRUPT_OVERRIDE=2
};

typedef struct {
	u8 type; // APIC_TYPE
	u8 length;
} PACKED APIC_HEADER;

typedef struct {
	u64 base_address;
	u16 segment;
	u8 start_bus;
	u8 end_bus;
	u32 reserved;
} PACKED MCFG_ENTRY;

typedef struct {
	ACPI_HEADER header;
	u64 reserved;
	MCFG_ENTRY entry;
} PACKED MCFG_HEADER;

typedef struct {
	APIC_HEADER header;
	u8 acpi_processor_id;
	u8 apic_id;
	u32 falgs;
} PACKED APIC_LAPIC;

typedef struct {
	APIC_HEADER header;
	u8 ioapic_id;
	u8 reserved;
	u32 ioapic_address;
	u32 global_system_interrupt_base;
} PACKED APIC_IOAPIC;

typedef struct {
	APIC_HEADER header;
	u8 bus;
	u8 source;
	u32 interrupt;
	u16 flags;
} PACKED APIC_INTERRUPT_OVERRIDE;

typedef struct {
	u8 address_space_id;
	u8 register_bit_width;
	u8 register_bit_offset;
	u8 reserved;
	u64 address;
} PACKED ACPI_HPET_ADDRESS_STRUCT;

typedef struct {
	ACPI_HEADER header;
	u8 hardware_revision_id;
	u8 attribute;
	u16 pci_vendor_id;
	ACPI_HPET_ADDRESS_STRUCT addresses;
	u8 hpet_number;
	u16 minimum_tick;
	u8 page_protection;
} PACKED ACPI_HPET;

typedef struct {
	ACPI_HEADER header;

	u32 firmware_ctrl;
	u32 dsdt;

	u8 reserved;

	u8 preferred_power_management_profile;
	u16 sci_interrupt;
	u32 smi_command_port;
	u8 acpi_enable;
	u8 acpi_disable;
	u8 s4bios_req;
	u8 pstate_control;
	u32 pm1a_event_block;
	u32 pm1b_event_block;
	u32 pm1a_control_block;
	u32 pm1b_control_block;
	u32 pm2_control_block;
	u32 pm_timer_block;
	u32 gpe_0_block;
	u32 gpe_1_block;
	u8 pm1_event_length;
	u8 pm1_control_length;
	u8 pm2_control_length;
	u8 pm_timer_length;
	u8 gpe_0_length;
	u8 gpe_1_length;
	u8 gpe_1_base;
	u8 c_state_control;
	u16 worst_c2_latency;
	u16 worst_c3_latency;
	u16 flush_size;
	u16 flush_stride;
	u8 duty_offset;
	u8 duty_width;
	u8 day_alarm;
	u8 month_alarm;
	u8 century;

	u16 boot_architecture_flags;

	u8 reserved2;
	u32 flags;
} PACKED ACPI_FADT;

typedef struct {
	ACPI_HEADER header;
	u32 lapic_address;
	u32 flags;
} PACKED ACPI_MADT;

// https://github.com/nifanfa/MOOS/blob/be3fed5af51e60c95a4d2f9a195c5aef5ab37636/Kernel/Driver/ACPI.cs
