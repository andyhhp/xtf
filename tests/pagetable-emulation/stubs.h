#ifndef _STUBS_H_
#define _STUBS_H_

unsigned long stub_read(unsigned long va);
unsigned long stub_implicit(unsigned long sel);
unsigned long stub_write(unsigned long va);
unsigned long stub_exec(unsigned long va);

unsigned long stub_force_read(unsigned long va);
unsigned long stub_force_implicit(unsigned long sel);
unsigned long stub_force_write(unsigned long va);
unsigned long stub_force_exec(unsigned long va);

unsigned long stub_read_user(unsigned long va);
unsigned long stub_implicit_user(unsigned long sel);
unsigned long stub_write_user(unsigned long va);
unsigned long stub_exec_user(unsigned long va);

unsigned long stub_force_read_user(unsigned long va);
unsigned long stub_force_implicit_user(unsigned long sel);
unsigned long stub_force_write_user(unsigned long va);
unsigned long stub_force_exec_user(unsigned long va);

/* Returns CPUID.1.D */
uint32_t stub_64bit_probe_paging_mode(void);

#endif
