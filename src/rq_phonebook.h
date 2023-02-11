#pragma once
#include "rq_common.h"
#include <stdlib.h>
#include <stdio.h>
#include "rq_term.h"
#include "rq_levenshtein.h"

#define INITIAL_LEN_OF_CONTACTS 64

typedef struct {
    uint  id;
    char *name;
    char *contact_info;
} Contact;

typedef struct {
    uint     length;
    uint     allocated;
    Contact *contacts;
} PhoneBook;

void pb_add_contact(PhoneBook *pb, uint id, char *name, char *contact_info) {
    if (pb->allocated == pb->length) {
        pb->allocated *= 2;
        pb->contacts = (Contact *) realloc(pb->contacts, pb->allocated * sizeof(Contact));
        if (pb->contacts == NULL) {
            panic("Failed to reallocate memory", RQ_ERROR_NOT_ENOUGH_MEMORY);
        }
    }
    uint curr = pb->length++;
    pb->contacts[curr].id = id;
    pb->contacts[curr].name = name;
    pb->contacts[curr].contact_info = contact_info; 
}

void pb_read_contacts(PhoneBook *pb, FILE *f) {
    int id;
    while (fread(&id, sizeof(int), 1, f)) {
        int len_of_name;
        uint ret = fread(&len_of_name, sizeof(int), 1, f);
        if (ret == 0) panic("EOF found before being able to read length of name", RQ_ERROR_FILE_READING);

        // we allocate +1, that way there is an extra zero at the end, 
        // meaning this will be used as a regular c string
        char *name = (char *) calloc(len_of_name + 1, sizeof(char));
        if (name == NULL) {
            panic("Failed to allocate enough memory", RQ_ERROR_NOT_ENOUGH_MEMORY);
        }
        ret = fread(name, sizeof(char), len_of_name, f);
        if (ret == 0) panic("EOF found before being able to read name", RQ_ERROR_FILE_READING);
        
        int len_of_phone_number;
        ret = fread(&len_of_phone_number, sizeof(int), 1, f);
        if (ret == 0) panic("EOF found before being able to read length of phone number", RQ_ERROR_FILE_READING);

        // we allocate +1, that way there is an extra zero at the end, 
        // meaning this will be used as a regular c string
        char *contact_info = (char *) calloc(len_of_phone_number + 1, sizeof(char));
        if (contact_info == NULL) {
            panic("Failed to allocate enough memory", RQ_ERROR_NOT_ENOUGH_MEMORY);
        }
        ret = fread(contact_info, sizeof(char), len_of_phone_number, f);
        if (ret == 0) panic("EOF found before being able to read name", RQ_ERROR_FILE_READING);
        pb_add_contact(pb, id, name, contact_info);
    }
}

void pb_init(PhoneBook *pb, FILE *f) {
    pb->contacts = (Contact *) malloc(INITIAL_LEN_OF_CONTACTS * sizeof(Contact));
    if (pb->contacts == NULL) {
        panic("Failed to allocate enough memory", RQ_ERROR_NOT_ENOUGH_MEMORY);
    }
    pb->length = 0;
    pb->allocated = INITIAL_LEN_OF_CONTACTS;
    fseek(f, 0, SEEK_SET);
    pb_read_contacts(pb, f);
}

void pb_print(PhoneBook *pb) {
    clear_screen();
    printf("Id\tName\tContact info:\n");
    for (uint i = 0; i < pb->length; i++) {
        printf("%d\t%s\t%s\n", pb->contacts[i].id, pb->contacts[i].name, pb->contacts[i].contact_info);
    }
    get_key_code(); // we wait until input
}

/* Combines name and contact info into a char * that has to be freed after use */
char *pb_name_contact_info(char *name, char *contact_info) {
    uint name_actual_len = strlen(name)+1;
    uint contact_info_actual_len = strlen(contact_info)+1;
    char *combination = malloc(sizeof(char) * (name_actual_len) + sizeof(char) * (contact_info_actual_len));
    if (combination == NULL) {
        panic("Failed to allocate enough memory", RQ_ERROR_NOT_ENOUGH_MEMORY);
    }
    for (uint i = 0; i < (name_actual_len + contact_info_actual_len); i++) {
        if (i < name_actual_len-1) {
            combination[i] = name[i]; 
        } else if (i == name_actual_len-1) {
            combination[i] = '\t';
        } else {
            combination[i] = contact_info[i - name_actual_len];
        }
    }
    return combination;
}

typedef struct {
    uint distance;
    uint pos;
} Distance;

int cmp_func(Distance *d1, Distance *d2) {
    return d1->distance > d2->distance;
}

void pb_find(PhoneBook *pb) {
    clear_screen();
    printf("Search: ");
    char *input = ask_for_string();

    Distance *distances = malloc(sizeof(Distance)*pb->length);
    if (distances == NULL) panic("Couldn't allocate enough memory", RQ_ERROR_NOT_ENOUGH_MEMORY);

    // TODO: Instead of levenshtein we could use something else cuz it seems my stolen code isn't very good
    // Probably could check if string contains this as substring
    uint len = 0;   
    for (uint i = 0; i < pb->length; i++) {
        if (strlen(pb->contacts[i].name)+strlen(pb->contacts[i].contact_info) >= strlen(input)) {
            char *nc = pb_name_contact_info(pb->contacts[i].name, pb->contacts[i].contact_info);
            Distance d = { .distance = distance(input, nc), .pos = i };
            distances[len] = d;
            free(nc);
            len++;
        }
    }

    qsort(distances, len, sizeof(Distance), (int (*)(const void *, const void *)) cmp_func);

    clear_screen();
    printf("Id\tName\tContact info:\n");
    for (uint i = 0; i < len; i++) {
        printf("%i\t%s\t%s\n", pb->contacts[distances[i].pos].id, pb->contacts[distances[i].pos].name, pb->contacts[distances[i].pos].contact_info);
    }
    

    get_key_code();
    free(distances);
    free(input);
}

void pb_create(PhoneBook *pb) {
    clear_screen();
    printf("Please input a name:\n");
    char *name = ask_for_string();
    printf("Please input the contact information:\n");
    char *contact_info = ask_for_string();
    pb_add_contact(pb, pb->length, name, contact_info);
}

void pb_remove(PhoneBook *pb) {
    char **names = malloc(sizeof(char *) * pb->length);
    if (names == NULL) {
        panic("Failed to allocate enough memory", RQ_ERROR_NOT_ENOUGH_MEMORY);
    }
    for (uint i = 0; i < pb->length; i++) {
        names[i] = pb_name_contact_info(pb->contacts[i].name, pb->contacts[i].contact_info);
    }
    uint selected = select_from_menu("Please select a contact to remove:\r\n\tName:\tContact Info:\r\n", names, pb->length);
    for (uint i = 0; i < pb->length; i++) {
        free(names[i]);
    }
    free(names);
    if (selected == pb->length) {
        return;
    }
    Contact *new_contacts = malloc(sizeof(Contact) * (pb->allocated));
    if (selected == 0) {
        for (uint i = 1; i < pb->length; i++) {
            new_contacts[i-1] = pb->contacts[i];
        }
    } else {
        for (uint i = 0; i < selected; i++) {
            new_contacts[i] = pb->contacts[i];
        }
        for (uint i = selected+1; i < pb->length; i++) {
            new_contacts[i-1] = pb->contacts[i];
        }
    }
    pb->length -= 1;
    free(pb->contacts);
    pb->contacts = new_contacts;
}

void pb_save(PhoneBook *pb, FILE *f) {
    f = freopen("nums", "w+b", f);
    if (f == NULL) {
        panic("Couldn't reopen file", RQ_ERROR_FILE_READING)
    }
    uint name_len;
    uint contact_info_len;
    for (uint i = 0; i < pb->length; i++) {
        fwrite(&pb->contacts[i].id, sizeof(uint), 1, f);

        name_len = strlen(pb->contacts[i].name);
        fwrite(&name_len, sizeof(uint), 1, f); // print name 
        fprintf(f, "%s", pb->contacts[i].name);
        
        contact_info_len = strlen(pb->contacts[i].contact_info);
        fwrite(&contact_info_len, sizeof(uint), 1, f); // print contact_info
        fprintf(f, "%s", pb->contacts[i].contact_info);
    }
}

// TODO: introduce a menu for this
void pb_save_as(PhoneBook *pb, char *file_path) {
    FILE *f = fopen(file_path, "w+b");
    if (f == NULL) {
        printf("Couldn't open file.");
        get_key_code();
    }

    uint name_len;      // TODO: get rid of code duplication
    uint contact_info_len;
    for (uint i = 0; i < pb->length; i++) {
        fwrite(&pb->contacts[i].id, sizeof(uint), 1, f);

        name_len = strlen(pb->contacts[i].name);
        fwrite(&name_len, sizeof(uint), 1, f); // print name 
        fprintf(f, "%s", pb->contacts[i].name);
        
        contact_info_len = strlen(pb->contacts[i].contact_info);
        fwrite(&contact_info_len, sizeof(uint), 1, f); // print contact_info
        fprintf(f, "%s", pb->contacts[i].contact_info);
    }
    
    fclose(f);
}

void pb_destroy(PhoneBook *pb, FILE *f) {
    pb_save(pb, f);
    for (uint i = 0; i < pb->length; i++) {
        free(pb->contacts[i].name);
        free(pb->contacts[i].contact_info);
    }
}
