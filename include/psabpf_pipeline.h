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

#include "psabpf.h"

bool psabpf_pipeline_exists(psabpf_context_t *ctx);
/* This function should load BPF program and initialize default maps (call map initializer program) */
int psabpf_pipeline_load(psabpf_context_t *ctx, const char *file);
int psabpf_pipeline_unload(psabpf_context_t *ctx);
int psabpf_pipeline_add_port(psabpf_context_t *ctx, const char *interface);
int psabpf_pipeline_del_port(psabpf_context_t *ctx, const char *interface);
