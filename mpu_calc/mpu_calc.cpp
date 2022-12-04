/*******************************************************************************
*  COPYRIGHT (C) ALL RIGHTS RESERVED.
* -----------------------------------------------------------------------------
*  This software is licensed to customers pursuant to the terms and conditions
*  of the Software License Agreement contained in the text file software.lic
*  that is distributed along with this software. This software can only be
*  utilized if all terms and conditions of the Software License Agreement are
*  accepted. If there are any questions, concerns, or if the Software License
*  Agreement text file, software.lic, is missing please contact applications for
*  assistance.
*******************************************************************************/

/**
* @file
* @brief
*    convert a memory map to a list of mpu settings
* 
* see device/core/m7/unit_test/memory_map.yaml for an example file.
* 
* see device/core/m7/unit_test/readme_mpu_cal.md for notes.
* 
* google libyaml for tutorials on traverse_yaml_node()
*/

// Include Files
#include <yaml.h>
#include <iostream>
#include "mpu_calculator.h"
#include "configure_mpu.h"
#include "mpu_display.h"
#include "mdx2_cmd_line_options.h"
#include "dx2_gtest_base.h"

uint32_t global_region_number = 0;
mpu_display_t global_display;

static void add_region( uint32_t start_addr, uint32_t size, uint32_t end_addr, uint32_t DisableExec, uint32_t AccessPermission, uint32_t AccessAttributes, std::string comment, std::string attributes )
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = global_region_number;
    if (size != 0)
    {
        end_addr = start_addr+size;
    }
    if (!mpu_calc.build_best_mpu_entries(start_addr,end_addr,DisableExec,AccessPermission,AccessAttributes))
    {
        printf("error building entries for 0x%08x to 0x%08x (region:%d)\n",start_addr,end_addr,global_region_number);
    }

    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        global_display.set(global_region_number,mpu_calc.mpu_table[i].RBAR,mpu_calc.mpu_table[i].RASR,comment);
        global_region_number++;
    }
}

#if 0
static void print_escaped(yaml_char_t * str, size_t length)
{
    int i;
    char c;

    for (i = 0; i < length; i++) {
        c = *(str + i);
        if (c == '\\')
            printf("\\\\");
        else if (c == '\0')
            printf("\\0");
        else if (c == '\b')
            printf("\\b");
        else if (c == '\n')
            printf("\\n");
        else if (c == '\r')
            printf("\\r");
        else if (c == '\t')
            printf("\\t");
        else
            printf("%c", c);
    }
}
#endif

#define DEBUG_PRINT(...)   do { \
    if (MDX2_LOG_IS_WRITABLE(MDX2_LOG_LEVEL_DEBUG)) { \
        printf(__VA_ARGS__); \
    } \
} while(0)

static void printl_utf8(unsigned char *str, size_t length, FILE *stream)
{
    if (MDX2_LOG_IS_WRITABLE(MDX2_LOG_LEVEL_DEBUG))
    {
        fwrite(str, 1, length, stream);
    }
}

static bool token_matches( yaml_node_t *node, const char *token_name )
{
    MDX2_ASSERT(node->type == YAML_SCALAR_NODE,"line: %d",(int)node->start_mark.line);
    uint32_t len = strlen(token_name);
    if (len != node->data.scalar.length)
        return false;
    if (strncmp( (const char *)node->data.scalar.value, token_name, len ) != 0)
        return false;
    return true;
}

static uint32_t token_to_hex( yaml_node_t *node ) UNUSED;
static uint32_t token_to_hex( yaml_node_t *node )
{
    MDX2_ASSERT(node->type == YAML_SCALAR_NODE,"line: %d",(int)node->start_mark.line);
    char buffer[100];
    char *temp;
    uint32_t len = node->data.scalar.length;
    MDX2_ASSERT(len < sizeof(buffer));
    strncpy(buffer,(const char *)node->data.scalar.value,len);
    buffer[len] = 0;
    uint32_t value = strtol( buffer, &temp, 16 );
    if (*temp != 0)
    {
        printf("expected hexadecimal in '%s' at line %d",buffer,(int)node->start_mark.line);
        MDX2_ASSERT(FALSE);
    }
    return value;
}

typedef struct {
    const char *token_name;
    uint32_t value;
} token_value_t;

static uint32_t token_to_from_list( yaml_node_t *node, const char *what, const token_value_t *token_values, uint32_t len )
{
    for (uint32_t i=0;i<len;i++)
    {
        const token_value_t *tv = &token_values[i];
        if (token_matches(node,tv->token_name))
        {
            return tv->value;
        }
    }
    char buffer[100];
    uint32_t token_len = node->data.scalar.length;
    strncpy(buffer,(const char *)node->data.scalar.value,token_len);
    buffer[token_len] = 0;
    printf("unknown %s in '%s' at line %d",what,buffer,(int)node->start_mark.line);
    printf("valid values:\n");
    for (uint32_t i=0;i<len;i++)
    {
        const token_value_t *tv = &token_values[i];
        printf("    %s\n",tv->token_name);
    }
    MDX2_ASSERT(FALSE);
}


static uint32_t token_to_DisableExec( yaml_node_t *node )
{
    const token_value_t token_values[] = {
        {"EXECUTE", 0},
        {"NEVER_EXECUTE", 1},
    };
    return token_to_from_list( node, "DisableExec", token_values, sizeof(token_values)/sizeof(token_values[0]));
}


static uint32_t token_to_AccessPermission( yaml_node_t *node )
{
    const token_value_t token_values[] = {
        {"ARM_MPU_AP_RO", ARM_MPU_AP_RO},
        {"ARM_MPU_AP_NONE", ARM_MPU_AP_NONE},
        {"ARM_MPU_AP_FULL", ARM_MPU_AP_FULL},
    };
    return token_to_from_list( node, "AccessPermission", token_values, sizeof(token_values)/sizeof(token_values[0]));
}


static uint32_t token_to_AccessAttributes( yaml_node_t *node )
{
    const token_value_t token_values[] = {
        {"NO_ACCESS", NO_ACCESS},
        {"STRONGLY_ORDERED", STRONGLY_ORDERED},
        {"DEVICE_SHAREABLE", DEVICE_SHAREABLE},
        {"DEVICE_NON_SHAREABLE", DEVICE_NON_SHAREABLE},
        {"NORMAL_UNCACHED", NORMAL_UNCACHED},
        {"NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE", NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE},
        {"NORMAL_WRITE_BACK_NO_WRITE_ALLOCATE", NORMAL_WRITE_BACK_NO_WRITE_ALLOCATE},
        {"NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE", NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE},
        {"NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE", NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE},
        // repeat the 'normal' memory without 'normal'
        {"UNCACHED", NORMAL_UNCACHED},
        {"WRITE_THROUGH_NO_WRITE_ALLOCATE", NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE},
        {"WRITE_BACK_NO_WRITE_ALLOCATE", NORMAL_WRITE_BACK_NO_WRITE_ALLOCATE},
        {"WRITE_BACK_READ_AND_WRITE_ALLOCATE", NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE},
        {"WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE", NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE},
    };
    return token_to_from_list( node, "AccessAttributes", token_values, sizeof(token_values)/sizeof(token_values[0]));
}


static uint32_t token_to_dec( yaml_node_t *node )
{
    MDX2_ASSERT(node->type == YAML_SCALAR_NODE,"line: %d",(int)node->start_mark.line);
    char buffer[100];
    char *temp;
    uint32_t len = node->data.scalar.length;
    MDX2_ASSERT(len < sizeof(buffer));
    strncpy(buffer,(const char *)node->data.scalar.value,len);
    buffer[len] = 0;
    uint32_t value;
    if (strncmp(buffer,"0x",2) == 0) {
        value = strtol( &buffer[2], &temp, 16 );
        if (*temp != 0)
        {
            printf("expected hex in '%s' at line %d",buffer,(int)node->start_mark.line);
            MDX2_ASSERT(FALSE);
        }
    } else {
        value = strtol( buffer, &temp, 10 );
        if ((strcmp(temp,"G")==0) || (strcmp(temp,"GB") == 0))
        {
            value *= 1024*1024*1024;
        }
        else if ((strcmp(temp,"M")==0) || (strcmp(temp,"MB") == 0))
        {
            value *= 1024*1024;
        }
        else if ((strcmp(temp,"K")==0) || (strcmp(temp,"KB") == 0))
        {
            value *= 1024;
        }
        else if (*temp != 0)
        {
            printf("expected decimal in '%s' at line %d",buffer,(int)node->start_mark.line);
            MDX2_ASSERT(FALSE);
        }
    }
    return value;
}

/// close your eyes,... global variables used to hold values while parsing a 'region'.
// one function per element type style of parsing.
uint32_t global_start_addr;
uint32_t global_size;
uint32_t global_end_addr;
std::string global_comment;
std::string global_attributes;
uint32_t global_DisableExec;
uint32_t global_AccessPermission;
uint32_t global_AccessAttributes;

static void traverse_yaml_node(yaml_document_t *document_p, yaml_node_t *node, uint32_t nesting)
{
    yaml_node_t *next_node_p;
    yaml_node_t *key_node_p;
    yaml_node_t *value_node_p;

    DEBUG_PRINT("%*s",nesting*4,"");
    switch (node->type) {
        case YAML_NO_NODE:
            DEBUG_PRINT("Empty node(%d:%d):\n", (int)node->start_mark.line, (int)node->start_mark.column);
            break;
        case YAML_SCALAR_NODE:
            DEBUG_PRINT("Scalar node(%d:%d):\n", (int)node->start_mark.line, (int)node->start_mark.column);
            DEBUG_PRINT("%*s",nesting*4,"");
            printl_utf8(node->data.scalar.value, node->data.scalar.length, stdout);
            break;
        case YAML_SEQUENCE_NODE:
            DEBUG_PRINT("Sequence node(%d:%d):\n", (int)node->start_mark.line, (int)node->start_mark.column);
            yaml_node_item_t *i_node;
            for (i_node = node->data.sequence.items.start; i_node < node->data.sequence.items.top; i_node++) {
                next_node_p = yaml_document_get_node(document_p, *i_node);
                if (next_node_p)
                    traverse_yaml_node(document_p, next_node_p, nesting+1);
            }
            break;
        case YAML_MAPPING_NODE:
            DEBUG_PRINT("Mapping node(%d:%d):\n", (int)node->start_mark.line, (int)node->start_mark.column);

            yaml_node_pair_t *i_node_p;
            for (i_node_p = node->data.mapping.pairs.start; i_node_p < node->data.mapping.pairs.top; i_node_p++) {
                key_node_p = yaml_document_get_node(document_p, i_node_p->key);
                if (key_node_p) {
                    DEBUG_PRINT("%*s",nesting*4,"");
                    DEBUG_PRINT("Key:");
                    traverse_yaml_node(document_p, key_node_p, nesting+1);
                } else {
                    fputs("Couldn't find next node\n", stderr);
                    exit(1);
                }

                if (token_matches(key_node_p,"region"))
                {
                    // starting a new region
                    global_start_addr = 0;
                    global_end_addr = 0;
                    global_size = 0;
                    global_comment = "";
                    global_attributes = "";
                    global_DisableExec = 1;
                    global_AccessPermission = ARM_MPU_AP_FULL;
                    global_AccessAttributes = 0;
                }

                value_node_p = yaml_document_get_node(document_p, i_node_p->value);
                if (value_node_p) {
                    DEBUG_PRINT("%*s",nesting*4,"");
                    DEBUG_PRINT("Value:");
                    traverse_yaml_node(document_p, value_node_p, nesting+1);
                } else {
                    fputs("Couldn't find next node\n", stderr);
                    exit(1);
                }
                if (key_node_p->type == YAML_SCALAR_NODE)
                {
                    if (token_matches(key_node_p,"start_addr"))
                    {
                        global_start_addr = token_to_dec(value_node_p);
                        DEBUG_PRINT("start_addr = %s (0x%08x)\n",value_node_p->data.scalar.value,global_start_addr);
                    }
                    else if (token_matches(key_node_p,"size"))
                    {
                        global_size = token_to_dec(value_node_p);
                        DEBUG_PRINT("size = %s (0x%08x)\n",value_node_p->data.scalar.value,global_size);
                    }
                    else if (token_matches(key_node_p,"end_addr"))
                    {
                        global_end_addr = token_to_dec(value_node_p);
                        DEBUG_PRINT("end_addr = %s (0x%08x)\n",value_node_p->data.scalar.value,global_end_addr);
                    }
                    else if (token_matches(key_node_p,"DisableExec"))
                    {
                        global_DisableExec = token_to_DisableExec(value_node_p);
                        DEBUG_PRINT("DisableExec = %s (0x%08x)\n",value_node_p->data.scalar.value,global_DisableExec);
                    }
                    else if (token_matches(key_node_p,"AccessPermission"))
                    {
                        global_AccessPermission = token_to_AccessPermission(value_node_p);
                        DEBUG_PRINT("AccessPermission = %s (0x%08x)\n",value_node_p->data.scalar.value,global_AccessPermission);
                    }
                    else if (token_matches(key_node_p,"AccessAttributes"))
                    {
                        global_AccessAttributes = token_to_AccessAttributes(value_node_p);
                        DEBUG_PRINT("AccessAttributes = %s (0x%08x)\n",value_node_p->data.scalar.value,global_AccessAttributes);
                    }
                    else if (token_matches(key_node_p,"comment"))
                    {
                        global_comment = (const char *)value_node_p->data.scalar.value;
                        DEBUG_PRINT("comment = \"%s\"\n",global_comment.c_str());
                    }
                    else if (token_matches(key_node_p,"attributes"))
                    {
                        global_attributes = (const char *)value_node_p->data.scalar.value;
                        DEBUG_PRINT("attributes = \"%s\"\n",global_attributes.c_str());
                    }
                    else if (token_matches(key_node_p,"region"))
                    {
                        // .. finished parsing the region...
                        DEBUG_PRINT("%s: global_start_addr=0x%x global_size=0x%x global_end_addr=0x%x global_comment=%s\n",
                               "region",
                               global_start_addr,
                               global_size,
                               global_end_addr,
                               global_comment.c_str());
                        add_region( global_start_addr, global_size, global_end_addr, global_DisableExec, global_AccessPermission, global_AccessAttributes, global_comment, global_attributes );

                        // add region object...
                    }
                }

            }
            break;
        default:
            fputs("Unknown node type\n", stderr);
            exit(1);
            break;
    }

    DEBUG_PRINT("%*s",nesting*4,"");
    DEBUG_PRINT("END NODE(%d:%d)\n", (int)node->start_mark.line, (int)node->start_mark.column);
}

static void traverse_yaml_document(yaml_document_t *document_p)
{
    DEBUG_PRINT("NEW DOCUMENT");

    traverse_yaml_node(document_p, yaml_document_get_root_node(document_p), 0);

    DEBUG_PRINT("END DOCUMENT");
}

static void read_memory_map_from_file(const char *file_name)
{
    yaml_parser_t parser;
    yaml_document_t document;

    printf("Loading '%s'\n", file_name);

    FILE *file = fopen(file_name, "rb");
    if (file == 0)
    {
        printf("error opening '%s'\n",file_name);
        return;
    }
    assert(file);

    assert(yaml_parser_initialize(&parser));

    yaml_parser_set_input_file(&parser, file);

    int done = 0;
    while (!done)
    {
        if (!yaml_parser_load(&parser, &document)) {

            /* Display a parser error message. */
        
            switch (parser.error)
            {
                case YAML_MEMORY_ERROR:
                    fprintf(stderr, "Memory error: Not enough memory for parsing\n");
                    break;
        
                case YAML_READER_ERROR:
                    if (parser.problem_value != -1) {
                        fprintf(stderr, "Reader error: %s: #%X at %d\n", parser.problem,
                                parser.problem_value, (int)parser.problem_offset);
                    }
                    else {
                        fprintf(stderr, "Reader error: %s at %d\n", parser.problem,
                                (int)parser.problem_offset);
                    }
                    break;
        
                case YAML_SCANNER_ERROR:
                    if (parser.context) {
                        fprintf(stderr, "Scanner error: %s at line %d, column %d\n"
                                "%s at line %d, column %d\n", parser.context,
                                (int)parser.context_mark.line+1, (int)parser.context_mark.column+1,
                                parser.problem, (int)parser.problem_mark.line+1,
                                (int)parser.problem_mark.column+1);
                    }
                    else {
                        fprintf(stderr, "Scanner error: %s at line %d, column %d\n",
                                parser.problem, (int)parser.problem_mark.line+1,
                                (int)parser.problem_mark.column+1);
                    }
                    break;
        
                case YAML_PARSER_ERROR:
                    if (parser.context) {
                        fprintf(stderr, "Parser error: %s at line %d, column %d\n"
                                "%s at line %d, column %d\n", parser.context,
                                (int)parser.context_mark.line+1, (int)parser.context_mark.column+1,
                                parser.problem, (int)parser.problem_mark.line+1,
                                (int)parser.problem_mark.column+1);
                    }
                    else {
                        fprintf(stderr, "Parser error: %s at line %d, column %d\n",
                                parser.problem, (int)parser.problem_mark.line+1,
                                (int)parser.problem_mark.column+1);
                    }
                    break;
        
                case YAML_COMPOSER_ERROR:
                    if (parser.context) {
                        fprintf(stderr, "Composer error: %s at line %d, column %d\n"
                                "%s at line %d, column %d\n", parser.context,
                                (int)parser.context_mark.line+1, (int)parser.context_mark.column+1,
                                parser.problem, (int)parser.problem_mark.line+1,
                                (int)parser.problem_mark.column+1);
                    }
                    else {
                        fprintf(stderr, "Composer error: %s at line %d, column %d\n",
                                parser.problem, (int)parser.problem_mark.line+1,
                                (int)parser.problem_mark.column+1);
                    }
                    break;
        
                default:
                    /* Couldn't happen. */
                    fprintf(stderr, "Internal error\n");
                    break;
            }
            fprintf(stderr, "Failed to load document in %s\n", file_name);
            break;
        }

        done = (!yaml_document_get_root_node(&document));

        if (!done)
            traverse_yaml_document(&document);

        yaml_document_delete(&document);
    }

    yaml_parser_delete(&parser);

    assert(!fclose(file));

}

static StringOption option_memory_map_filename( "memory_map.yaml", "memory_map", "input memory map (yaml)");
static StringOption option_output_filename( "memory_map.h", "output_filename", "output filename (.h)");
static UintOption option_mpu_table_size(16, "mpu_table_size", "mpu table size 1-16");

int main(int argc, const char **argv)
{
    /* parse googletest options */
    testing::InitGoogleTest(&argc, (char **)argv);

    /* parse other options (these options are saved in option_*) */
    CmdLineOptions::GetInstance()->ParseOptions(argc,argv);

    if (option_memory_map_filename.is_set)
    {
        read_memory_map_from_file(option_memory_map_filename.value);
        while (global_region_number < option_mpu_table_size.value)
        {
            global_display.set(global_region_number,ARM_MPU_RBAR(global_region_number,0),0,"unused");
            global_region_number++;
        }
        FILE *f = fopen(option_output_filename.value,"w");
        global_display.display_memory_map(f,"// ");
        global_display.display_entries(f,"    // ");
        fclose(f);
    }
}