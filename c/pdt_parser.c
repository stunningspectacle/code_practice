#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define PDT_VERSION 5
#define PDT_FILENAME "INTC_pdt"
#define BDT_CUSTOMER_DATA_MAX_LEN (1024)
#define  PDT_VALID_DESC_FORMAT 0
#define ISH_HID_MAX_STATIC_DEVICES 6

enum PDT_ENTRY_TYPE {
	PDT_ENTRY_TYPE_BOARD_INFO,
	PDT_ENTRY_TYPE_SENSORS_INFO,
};

enum BDT_ELEMENT_TYPE {
	BDT_ELEMENT_TYPE_I2C,
	BDT_ELEMENT_TYPE_HOST_INTERFACE,
	BDT_ELEMENT_TYPE_FORM_FACTOR,
	BDT_ELEMENT_TYPE_PDT_DESCRIPTION,
	BDT_ELEMENT_TYPE_CUSTOMER_DATA,   
	BDT_ELEMENT_TYPE_I3C,
	BDT_ELEMENT_TYPE_PLAT_ID,
};

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#pragma pack(1)
struct _pdt_entry_t {
	uint8_t	type;
	uint16_t	length; // not including this field
	uint8_t data[];
}; 

typedef struct _pdt_entry_t pdt_entry_t;

struct _pdt_header_info_t {
	uint8_t  version;
	uint8_t  sku_id; // not used for ISH2
	uint16_t total_size; // including the fields above
};
typedef struct _pdt_header_info_t  pdt_header_info_t;

typedef struct {
	uint16_t		hcnt;
	uint16_t		lcnt;
	uint16_t		sda_hold;
} i2c_bus_data_t;

struct _i2c_bus_info_t {
	uint8_t			bus_id;
	i2c_bus_data_t	std_speed;
	i2c_bus_data_t	fast_speed;
	i2c_bus_data_t	high_speed;
};
typedef struct _i2c_bus_info_t i2c_bus_info_t;

struct _i2c_bdt_data_t {
	uint8_t			num_of_buses; // number of I2C buses
	i2c_bus_info_t 	bus[];
};
typedef struct _i2c_bdt_data_t i2c_bdt_data_t;

struct _i3c_bus_info_t {
	uint8_t     bus_id;
	uint32_t    i3c_od_scl;
	uint32_t    i3c_pp_scl;
	uint32_t    i2c_fm_scl;
	uint32_t    i2c_fmp_scl;
	uint32_t    i2c_ss_scl;
};
typedef struct _i3c_bus_info_t i3c_bus_info_t;

struct _i3c_bdt_data_t {
	uint8_t			num_of_buses; // number of I3C buses
	i3c_bus_info_t 	bus[];
};
typedef struct _i3c_bdt_data_t i3c_bdt_data_t;

#define SUPPORTED_PDT_DESCRIPTION_FORMAT 0
#define MAX_PDT_DESCRIPTION_LEN  255
struct _pdt_description_t {
	uint8_t             format;
	uint8_t             str_len;
	uint8_t             str[0]; //string withouth 0 terminator
};
typedef struct _pdt_description_t pdt_description_t;

struct _bdt_element_t {
	uint8_t	 	type;
	uint16_t	length; // including this field
	uint8_t  	data[];	// specific element data
};
typedef struct _bdt_element_t bdt_element_t;


struct _bdt_info_t {
	uint8_t	 	num_bdt_element_ts;
	uint8_t bdt_element_t[];
};
typedef struct _bdt_info_t bdt_info_t;

typedef struct{
	uint16_t		hcnt;
	uint16_t		lcnt;
}i2c_clibration_time;

typedef struct{
	i2c_clibration_time		std;
	i2c_clibration_time		fast;
}i2c_clibration_response;

typedef struct _sc_sensor_luid {
	uint16_t sensor_type;
	uint16_t vendor_id;
	uint16_t sensor_sub_type;
	uint8_t instance_id;
	uint8_t luid_flags;
} sc_sensor_luid;

typedef struct _sc_device_info {
	uint16_t hdt_len;
	uint16_t product_id;
	uint16_t vendor_id;
	uint16_t host_id_mask;
	uint16_t reserved;
	uint8_t flags;
	uint8_t n_of_sensors;
	sc_sensor_luid sensors[];
} sc_device_info;

typedef struct _sc_hdt_info {
	uint8_t n_of_devices;
	uint8_t reserved;
	sc_device_info device[];
} sc_hdt_info;
#pragma pack(0)

char pdt_buf[4096];

void parse_board_info(pdt_entry_t *pdt_entry)
{
	int offset = 0;
	bdt_info_t *bdt_info = (bdt_info_t *)pdt_entry->data;
	bdt_element_t *bdt;
	uint32_t plat_id;
	int len = 0;
	i2c_bdt_data_t *i2c_bdt;
	i3c_bdt_data_t *i3c_bdt;
	pdt_description_t *pdt_desc;

	printf("board_info: num_bdt(%d)\n", bdt_info->num_bdt_element_ts);
	offset += sizeof(bdt_info_t);
	while (offset < pdt_entry->length) {
		bdt = (bdt_element_t *)(pdt_entry->data + offset);
		switch (bdt->type) {
		case BDT_ELEMENT_TYPE_PLAT_ID:
			plat_id = *(uint32_t *)bdt->data;
			printf("BDT_ELEMENT_TYPE_PLAT_ID: len(%d), ID(%d)\n",
					bdt->length, plat_id);
			break;
		case BDT_ELEMENT_TYPE_I2C:
			i2c_bdt = (i2c_bdt_data_t *)bdt->data;
			printf("BDT_ELEMENT_TYPE_I2C: len(%d), num_of_buses:%d\n",
					bdt->length, i2c_bdt->num_of_buses);
			break;
		case BDT_ELEMENT_TYPE_I3C:
			i3c_bdt = (i3c_bdt_data_t *)bdt->data;
			printf("BDT_ELEMENT_TYPE_I3C: len(%d), num_of_buses:%d\n",
					bdt->length, i3c_bdt->num_of_buses);
			break;
		case BDT_ELEMENT_TYPE_HOST_INTERFACE:
			printf("BDT_ELEMENT_TYPE_HOST_INTERFACE, len(%d)\n",
					bdt->length);
			break;
		case BDT_ELEMENT_TYPE_FORM_FACTOR:
			printf("BDT_ELEMENT_TYPE_FORM_FACTOR, len(%d)\n",
					bdt->length);
			break;
		case BDT_ELEMENT_TYPE_PDT_DESCRIPTION:
			printf("BDT_ELEMENT_TYPE_PDT_DESCRIPTION, len(%d)\n",
					bdt->length);
			pdt_desc = (pdt_description_t *)bdt->data;
			len = pdt_desc->str_len + sizeof(pdt_description_t);
			if (len != bdt->length ||
				(pdt_desc->format != PDT_VALID_DESC_FORMAT)) {
				printf("Error in PDT description\n");
			} else {
				char tmp[pdt_desc->str_len + 1];

				memcpy(tmp, pdt_desc->str, pdt_desc->str_len);
				tmp[pdt_desc->str_len] = 0;
				printf("PDT description: %s\n", tmp);
			}
			break;
		case BDT_ELEMENT_TYPE_CUSTOMER_DATA:
			printf("BDT_ELEMENT_TYPE_CUSTOMER_DATA, len(%d)\n",
					bdt->length);
			if (!bdt->length ||
				(bdt->length > BDT_CUSTOMER_DATA_MAX_LEN)) {
				printf("Wrong PDT customer data, size %d\n",
						bdt->length);
			} else {
				printf("custom data: length(%d)\n",
						bdt->length);
			}
			break;
		default:
			printf("Unexpected BDT type(%d), len(%d)\n",
					bdt->type, bdt->length);
			break;
		}

		offset += sizeof(bdt_element_t) + bdt->length;
	}
	printf("%s: offset = %d\n\n", __func__, offset);
}

void parse_sensor_info(pdt_entry_t *pdt_entry)
{
	int i = 0;
	sc_hdt_info *hdt_info;
	sc_device_info *info;
	int hdt_len = 0;
	int sdt_len = 0;

	printf("sensor info length: %d, sizeof(sc_device_info)(%ld), sizeof(sc_hdt_info)(%ld)\n",
			pdt_entry->length, sizeof(sc_device_info), sizeof(sc_hdt_info));
	hdt_info = (sc_hdt_info *)pdt_entry->data;
	printf("hdt info: n_of_devices(%d)\n", hdt_info->n_of_devices);

	while (i < hdt_info->n_of_devices) {
		info = (sc_device_info *)((char *)hdt_info->device + hdt_len);
		printf("device %d: hdt_len(%d), product_id(0x%x), vendor_id(0x%x), host_id_mask(0x%x), "
				"flags(0x%x), n_of_sensors(%d)\n",
				i,
				info->hdt_len, info->product_id,
				info->vendor_id, info->host_id_mask,
				info->flags, info->n_of_sensors);
		hdt_len += info->hdt_len;
		i++;
	}
	hdt_len += sizeof(sc_hdt_info);
	sdt_len = pdt_entry->length - hdt_len;

	printf("total hdt_len(%d), sdt_len(%d), pdt_entry->length(%d)\n",
			hdt_len, sdt_len, pdt_entry->length);
}

int main(int argc, char *argv[])
{
	int fd, len;
	int offset = 0; 

	if (argc != 2) {
		printf("usage: %s pdt.bin\n", argv[0]);
		return -1;
	}

	fd = open(argv[1], O_RDONLY); 
	if (fd < 0) {
		perror("error:");
		return -1;
	}

	len = read(fd, pdt_buf, sizeof(pdt_buf));
	printf("pdt len: %d\n", len);

	pdt_header_info_t *header;

	header = (pdt_header_info_t *)pdt_buf;
	printf("header: version(%d), sk_id(%d), total_size(%d)\n\n",
			header->version, header->sku_id, header->total_size);
	offset += sizeof(*header);

	pdt_entry_t *pdt_entry;
	while (offset < header->total_size) {
		pdt_entry = (pdt_entry_t *)(pdt_buf + offset);
		if (pdt_entry->type == PDT_ENTRY_TYPE_SENSORS_INFO) {
			printf("PDT_ENTRY_TYPE_SENSORS_INFO: length(%d)\n",
					pdt_entry->length);
			parse_sensor_info(pdt_entry);
		} else if (pdt_entry->type == PDT_ENTRY_TYPE_BOARD_INFO) { 
			printf("PDT_ENTRY_TYPE_BOARD_INFO: length(%d)\n",
					pdt_entry->length);
			parse_board_info(pdt_entry);
		} else {
			printf("Wrong pdt entry: type(%d), length(%d)\n",
					pdt_entry->type, pdt_entry->length);
		}
		offset += sizeof(pdt_entry_t) + pdt_entry->length;
	}
	printf("offset = %d\n", offset);

	return 0;
}

