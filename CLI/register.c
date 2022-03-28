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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <jansson.h>

#include "register.h"
#include <psabpf.h>

static int parse_dst_register(int *argc, char ***argv, const char **register_name,
                             psabpf_context_t *psabpf_ctx, psabpf_register_context_t *ctx)
{
    if (*argc < 1) {
        fprintf(stderr, "too few parameters\n");
        return EINVAL;
    }

    if (is_keyword(**argv, "id")) {
        NEXT_ARGP_RET();
        fprintf(stderr, "id: register access not supported\n");
        return ENOTSUP;
    } else if (is_keyword(**argv, "name")) {
        NEXT_ARGP_RET();
        fprintf(stderr, "name: register access not supported yet\n");
        return ENOTSUP;
    } else {
        if (register_name != NULL)
            *register_name = **argv;
        int error_code = psabpf_register_ctx_name(psabpf_ctx, ctx, **argv);
        if (error_code != NO_ERROR)
            return error_code;
    }

    NEXT_ARGP();
    return NO_ERROR;
}

static int parse_register_key(int *argc, char ***argv, psabpf_register_entry_t *entry)
{
    if (!is_keyword(**argv, "key"))
        return NO_ERROR; /* key is optional */
    NEXT_ARGP_RET();

    bool has_any_key = false;
    while (*argc > 0) {
        if (has_any_key) {
            if (is_keyword(**argv, "value"))
                return NO_ERROR;
        }

        int err = translate_data_to_bytes(**argv, entry, CTX_REGISTER_KEY);
        if (err != NO_ERROR)
            return err;

        has_any_key = true;
        NEXT_ARGP();
    }

    return NO_ERROR;
}

static int build_struct_json(json_t *parent, psabpf_register_context_t *ctx, psabpf_register_entry_t *entry)
{
    psabpf_struct_field_t *field;
    while ((field = psabpf_register_get_next_field(ctx, entry)) != NULL) {
        /* To build flat structure of output JSON just remove this and next conditional
         * statement. In other words, preserve only condition and instructions below it:
         *      if (psabpf_digest_get_field_type(field) != DIGEST_FIELD_TYPE_DATA) continue; */
        if (psabpf_struct_get_field_type(field) == PSABPF_STRUCT_FIELD_TYPE_STRUCT_START) {
            json_t *sub_struct = json_object();
            if (sub_struct == NULL) {
                fprintf(stderr, "failed to prepare message sub-object JSON\n");
                return ENOMEM;
            }
            if (json_object_set(parent, psabpf_struct_get_field_name(field), sub_struct)) {
                fprintf(stderr, "failed to add message sub-object JSON\n");
                json_decref(sub_struct);
                return EPERM;
            }

            int ret = build_struct_json(sub_struct, ctx, entry);
            json_decref(sub_struct);
            if (ret != NO_ERROR)
                return ret;

            continue;
        }

        if (psabpf_struct_get_field_type(field) == PSABPF_STRUCT_FIELD_TYPE_STRUCT_END)
            return NO_ERROR;

        if (psabpf_struct_get_field_type(field) != PSABPF_STRUCT_FIELD_TYPE_DATA)
            continue;

        const char *encoded_data = convert_bin_data_to_hexstr(psabpf_struct_get_field_data(field),
                                                              psabpf_struct_get_field_data_len(field));
        if (encoded_data == NULL) {
            fprintf(stderr, "not enough memory\n");
            return ENOMEM;
        }
        const char *field_name = psabpf_struct_get_field_name(field);
        if (field_name == NULL)
            field_name = "";
        json_object_set_new(parent, field_name, json_string(encoded_data));
        free((void *) encoded_data);
    }

    return NO_ERROR;
}

int do_register_get(int argc, char **argv)
{
    int ret = EINVAL;
    const char *register_name = NULL;
    psabpf_context_t psabpf_ctx;
    psabpf_register_context_t ctx;
    psabpf_register_entry_t entry;

    psabpf_context_init(&psabpf_ctx);
    psabpf_register_ctx_init(&ctx);
    psabpf_register_entry_init(&entry);

    if (parse_pipeline_id(&argc, &argv, &psabpf_ctx) != NO_ERROR)
        goto clean_up;

    if (parse_dst_register(&argc, &argv, &register_name, &psabpf_ctx, &ctx) != NO_ERROR)
        goto clean_up;

    if (parse_register_key(&argc, &argv, &entry) != NO_ERROR)
        goto clean_up;

    if (argc > 0) {
        fprintf(stderr, "%s: unused argument\n", *argv);
        goto clean_up;
    }

    ret = psabpf_register_get(&ctx, &entry);

    json_t *root = json_object();
    json_t *extern_type = json_object();
    json_t *instance_name = json_object();
    if (root == NULL || extern_type == NULL || instance_name == NULL) {
        fprintf(stderr, "failed to prepare JSON\n");
        goto clean_up;
    }

    if (json_object_set(extern_type, register_name, instance_name)) {
        fprintf(stderr, "failed to add JSON instance %s\n", register_name);
        goto clean_up;
    }
    json_object_set(root, "Register", extern_type);

    json_t *json_entry = json_object();
    if (json_entry == NULL) {
        fprintf(stderr, "failed to prepare register in JSON\n");
        goto clean_up;
    }
    ret = build_struct_json(json_entry, &ctx, &entry);
    json_object_set(instance_name, "value", json_entry);

    json_dumpf(root, stdout, JSON_INDENT(4) | JSON_ENSURE_ASCII);

clean_up:
    psabpf_register_entry_free(&entry);
    psabpf_register_ctx_free(&ctx);
    psabpf_context_free(&psabpf_ctx);

    return ret;
}

int do_register_set(int argc, char **argv) {
    // TODO implement
    return NO_ERROR;
}

int do_register_reset(int argc, char **argv) {
    // TODO implement
    return NO_ERROR;
}

int do_register_help(int argc, char **argv)
{
    (void) argc; (void) argv;
    fprintf(stderr,
            "Usage: %1$s register get pipe ID REGISTER key DATA\n"
            "Unimplemented commands:\n"
            "       %1$s register set pipe ID REGISTER key DATA value REGISTER_VALUE\n"
            "       %1$s register reset pipe ID REGISTER key DATA\n"
            "\n"
            "       REGISTER := { id REGISTER_ID | name REGISTER | REGISTER_FILE }\n"
            "       REGISTER_VALUE := { DATA }\n"
            "",
            program_name);

    return NO_ERROR;
}
