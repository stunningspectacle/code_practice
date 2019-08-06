/*++
   INTEL CONFIDENTIAL
   Copyright (c) 2012 - 2014 Intel Corporation. All Rights Reserved.

   The source code contained or described herein and all documents related
   to the source code ("Material") are owned by Intel Corporation or its
   suppliers or licensors. Title to the Material remains with Intel Corporation
   or its suppliers and licensors. The Material contains trade secrets and
   proprietary and confidential information of Intel or its suppliers and
   licensors. The Material is protected by worldwide copyright and trade secret
   laws and treaty provisions. No part of the Material may be used, copied,
   reproduced, modified, published, uploaded, posted, transmitted, distributed,
   or disclosed in any way without Intel's prior express written permission.

   No license under any patent, copyright, trade secret or other intellectual
   property right is granted to or conferred upon you by disclosure or delivery
   of the Materials, either expressly, by implication, inducement, estoppel or
   otherwise. Any license under such intellectual property rights must be
   express and approved by Intel in writing.
--*/
#ifndef I2C_DEFS_H_
#define I2C_DEFS_H_

// i2c ioctl request types
#define I2C_SPEED_MODE  		          1
#define I2C_FREQ_MODE   		          2
#define I2C_DATA_BUS   			          3
#define I2C_CALIB_BUS			          4
#define I2C_SET_INTERRUPT_TIMEOUT         5

enum{
	// speed mode values
	I2C_SPEED_STD = 0,
	I2C_SPEED_FAST = 1,
	I2C_SPEED_HIGH = 2,
	// freq mode values
	I2C_FREQ_25 = 0,
	I2C_FREQ_50 = 1,
	I2C_FREQ_100 = 2,
	I2C_FREQ_120 = 3,
	I2C_FREQ_40 = 4,
	I2C_FREQ_20 = 5,
	I2C_FREQ_37 = 6
};

enum{
	I2C_READ,
	I2C_WRITE
};

typedef struct i2c_req_struct{
	uint8_t slave_addr;
	uint8_t extra_w_len;
	uint8_t w_len;
	uint8_t r_len;
	IN uint8_t* extra_w_data;
    IN uint8_t* w_data;
	OUT uint8_t* r_data;
	uint16_t* abort_reason;
	int* error;
	struct i2c_req_struct* next;
	uint8_t operation;
	uint8_t flags;
}i2c_req;



#define I2C_FLAG_REPEATED_START_DISABLED    1

#define I2C_RETRIES_COUNT                   3

#endif /* I2C_DEFS_H_ */
