#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "modbus.h"

#include "stdint.h"

/* The goal of this program is to check all major functions of
   libmodbus:
   - write_coil
   - read_bits
   - write_coils
   - write_register
   - read_registers
   - write_registers
   - read_registers

   All these functions are called with random values on a address
   range defined by the following defines.
*/
#define SERVER_ID       1

/* At each loop, the program works in the range ADDRESS_START to
 * ADDRESS_END then ADDRESS_START + 1 to ADDRESS_END and so on.
 */

int main(int argc, char* argv[]) {
	modbus_t* ctx;
	int rc;
	int addr = 0;
	int nb = 5;
	int i;
	uint8_t* tab_rq_bits;
	uint8_t* tab_rp_bits;
	uint16_t* tab_rq_registers;
	uint16_t* tab_rp_registers;

	ctx = modbus_new_rtu("COM2:", 9600, 'N', 8, 1);
	modbus_set_slave(ctx, SERVER_ID);

	modbus_set_debug(ctx, TRUE);

	if (modbus_connect(ctx) == -1) {
		fprintf(stderr, "Connection failed: %s\n",
			modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	// Coils test - start pump 1
	tab_rq_bits = (uint8_t*)malloc(nb * sizeof(uint8_t));
	memset(tab_rq_bits, 0, nb * sizeof(uint8_t));

	tab_rp_bits = (uint8_t*)malloc(nb * sizeof(uint8_t));
	memset(tab_rp_bits, 0, nb * sizeof(uint8_t));

	/* WRITE BIT */
	tab_rq_bits[0] = 1;
	rc = modbus_write_bit(ctx, addr, tab_rq_bits[0]);
	if (rc != 1) {
		printf("ERROR modbus_write_bit (%d)\n", rc);
		printf("Address = %d, value = %d\n", addr, tab_rq_bits[0]);
	}
	else {
		rc = modbus_read_bits(ctx, addr, 1, tab_rp_bits);
		if (rc != 1 || tab_rq_bits[0] != tab_rp_bits[0]) {
			printf("ERROR modbus_read_bits single (%d)\n", rc);
			printf("address = %d\n", addr);
		}
	}

	/* MULTIPLE BITS */
	tab_rq_bits[0] = 0;
	tab_rq_bits[1] = 1;
	tab_rq_bits[2] = 1;
	tab_rq_bits[3] = 1;
	tab_rq_bits[4] = 1;
	rc = modbus_write_bits(ctx, addr, nb, tab_rq_bits);
	if (rc != nb) {
		printf("ERROR modbus_write_bits (%d)\n", rc);
		printf("Address = %d, nb = %d\n", addr, nb);
	}
	else {
		rc = modbus_read_bits(ctx, addr, nb, tab_rp_bits);
		if (rc != nb) {
			printf("ERROR modbus_read_bits\n");
			printf("Address = %d, nb = %d\n", addr, nb);
		}
		else {
			for (i = 0; i < nb; i++) {
				if (tab_rp_bits[i] != tab_rq_bits[i]) {
					printf("ERROR modbus_read_bits\n");
					printf("Address = %d, value %d (0x%X) != %d (0x%X)\n",
						addr, tab_rq_bits[i], tab_rq_bits[i],
						tab_rp_bits[i], tab_rp_bits[i]);
				}
			}
		}
	}

	// Registers test - pressure real value pump 1
	tab_rq_registers = (uint16_t*)malloc(nb * sizeof(uint16_t));
	memset(tab_rq_registers, 0, nb * sizeof(uint16_t));

	tab_rp_registers = (uint16_t*)malloc(nb * sizeof(uint16_t));
	memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

	/* SINGLE REGISTER */
	tab_rq_registers[0] = 271;
	rc = modbus_write_register(ctx, addr, tab_rq_registers[0]);
	if (rc != 1) {
		printf("ERROR modbus_write_register (%d)\n", rc);
		printf("Address = %d, value = %d (0x%X)\n",
			addr, tab_rq_registers[0], tab_rq_registers[0]);
	}
	else {
		rc = modbus_read_registers(ctx, addr, 1, tab_rp_registers);
		if (rc != 1) {
			printf("ERROR modbus_read_registers single (%d)\n", rc);
			printf("Address = %d\n", addr);
		}
		else {
			if (tab_rq_registers[0] != tab_rp_registers[0]) {
				printf("ERROR modbus_read_registers single\n");
				printf("Address = %d, value = %d (0x%X) != %d (0x%X)\n",
					addr, tab_rq_registers[0], tab_rq_registers[0],
					tab_rp_registers[0], tab_rp_registers[0]);
			}
		}
	}

	/* MULTIPLE REGISTERS */
	tab_rq_registers[0] = 42;
	tab_rq_registers[1] = 271;
	tab_rq_registers[2] = 314;
	tab_rq_registers[3] = 161;
	tab_rq_registers[4] = 69;
	rc = modbus_write_registers(ctx, addr, nb, tab_rq_registers);
	if (rc != nb) {
		printf("ERROR modbus_write_registers (%d)\n", rc);
		printf("Address = %d, nb = %d\n", addr, nb);
	}
	else {
		rc = modbus_read_registers(ctx, addr, nb, tab_rp_registers);
		if (rc != nb) {
			printf("ERROR modbus_read_registers (%d)\n", rc);
			printf("Address = %d, nb = %d\n", addr, nb);
		}
		else {
			for (i = 0; i < nb; i++) {
				if (tab_rq_registers[i] != tab_rp_registers[i]) {
					printf("ERROR modbus_read_registers\n");
					printf("Address = %d, value %d (0x%X) != %d (0x%X)\n",
						addr, tab_rq_registers[i], tab_rq_registers[i],
						tab_rp_registers[i], tab_rp_registers[i]);
				}
			}
		}
	}

	/* Free the memory */
	free(tab_rq_bits);
	free(tab_rp_bits);
	free(tab_rq_registers);
	free(tab_rp_registers);

	/* Close the connection */
	modbus_close(ctx);
	modbus_free(ctx);

	return 0;
}