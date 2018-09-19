/*
 * Copyright (C) 2013, 2014 Intel Mobile Communications GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __ION_HAL_H
#define __ION_HAL_H


int ion_get_param(int fdion, int bufid, unsigned int *phys, size_t *len);

int ion_alloc_secure(int fdion, size_t *len, unsigned int *phys);
int ion_free_secure(int fdion, size_t len, unsigned int phys);


#endif /* __ION_HAL_H */
