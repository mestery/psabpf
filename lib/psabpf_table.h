/*
 * Copyright 2022 Orange
 * Copyright 2022 Warsaw University of Technology
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef P4C_PSABPF_TABLE_H
#define P4C_PSABPF_TABLE_H

typedef struct psabpf_bpf_map_descriptor psabpf_bpf_map_descriptor_t;
int clear_table_cache(psabpf_bpf_map_descriptor_t *map);

void move_action(psabpf_action_t *dst, psabpf_action_t *src);

#endif  /* P4C_PSABPF_TABLE_H */
